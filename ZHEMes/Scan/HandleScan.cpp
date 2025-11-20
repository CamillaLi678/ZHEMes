#include "stdafx.h"
#include "HandleScan.h"

#define PrintLog(_Level,fmt,...) \
	if(m_pLogMsg){\
	m_pLogMsg->PrintLog(_Level,fmt,__VA_ARGS__);\
	}


CHandleScan::CHandleScan(HWND hWnd)
{
	m_EventRecvPack += MakeDelegate(this, &CHandleScan::OnComRecvPack);
	m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	InitializeCriticalSection(&m_csModBus);
	m_hParentWnd = hWnd;
}


CHandleScan::~CHandleScan()
{
	if (m_hEvent != NULL) {
		CloseHandle(m_hEvent);
	}
	DeleteCriticalSection(&m_csModBus);
}

INT newDataToStr(BYTE*pData, INT Size, CString&strData)
{
	INT i, cnt = 0;
	CString Tmp;
	BYTE cData;
	strData = "";
	for (i = 0; i < Size; ++i) {
		cData = pData[i];
		if ((cData >> 4) >= 0x0A) {
			strData.Insert(cnt++, (cData >> 4) - 0x0A + 'A');
		}
		else {
			strData.Insert(cnt++, (cData >> 4) + '0');
		}

		if ((cData & 0x0F) >= 0x0A) {
			strData.Insert(cnt++, (cData & 0x0F) - 0x0A + 'A');
		}
		else {
			strData.Insert(cnt++, (cData & 0x0F) + '0');
		}
	}
	strData.Insert(strData.GetLength(), 0x0D);////回车换行结束
	return 0;
}

int newStrToData(CByteArray& ByteArrayIn, CByteArray& ByteArrayOut)
{
	INT i, Ret = -1;
	char cData;
	BYTE tmpData;
	int Size = (INT)ByteArrayIn.GetSize();
	for (i = 0; i < Size; ++i) {
		if (ByteArrayIn[i] == 0x0D) {
			Ret = 0;
			break;
		}
		cData = ByteArrayIn[i];
		if (isdigit(cData)) {
			cData = cData - '0';
		}
		else if (cData >= 'A'&&cData <= 'F') {
			cData = cData - 'A' + 10;
		}
		else if (cData >= 'a'&&cData <= 'f') {
			cData = cData - 'a' + 10;
		}
		else {
			goto __end;
		}
		if (i % 2 == 0) {
			tmpData = 0;
			tmpData |= cData << 4;
		}
		else {
			tmpData |= cData;
			ByteArrayOut.Add(tmpData);
		}
	}
__end:
	return Ret;
}

int newStrToData(BYTE*pData, INT Size, CByteArray& ByteArray)
{
	INT i, Ret = -1;
	char cData;
	for (i = 0; i < Size; ++i) {
		if (pData[i] == 0x0D) {
			Ret = 0;
			break;
		}
		cData = pData[i];
		if (isdigit(cData)) {
			cData = cData - '0';
		}
		else if (cData >= 'A'&&cData <= 'F') {
			cData = cData - 'A' + 10;
		}
		else if (cData >= 'a'&&cData <= 'f') {
			cData = cData - 'a' + 10;
		}
		else {
			goto __end;
		}
		ByteArray.Add(cData);
	}
__end:
	return Ret;
}



void newBytArrayToString(CByteArray&ByteArray, CString&strOut)
{
	int i;
	for (i = 0; i < ByteArray.GetSize(); ++i) {
		strOut.Insert(i, ByteArray[i]);
	}
}

bool CHandleScan::OnComRecvPack(void* Para)
{
	tUartEvent *pUartEvent = (tUartEvent*)Para;
	CByteArray ByteArrayOut;
	m_UartEvent = *pUartEvent;

	if (m_UartEvent.ErrCode == COMERR_OK) {
		CString strOut;
		newBytArrayToString(m_UartEvent.Data, strOut);
		bool bRet = true;
		PrintLog(LOGLEVEL_LOG, "CHandleScan::OnComRecvPack %s", strOut);

		tSN* pSN = new tSN;
		if (pSN){
			int i = 0;
			CString strTemp;
			strTemp.Format("%s", strOut);

			while (strTemp.GetLength() > 0){
				int nIndex = strTemp.Find(',');
				if (nIndex >=0 ){
					if (i== 0){
						strTemp.Delete(0, nIndex + 1);
					}
					
					++i;
				}else {
					break;
				}

				if (i == 2){
					CString strOld;
					strOld.Format("%s", strTemp.Left(nIndex));
					CString strNew;
					strNew.Format("%s-SW", strOld);

					strOut.Replace(strOld, strNew); //修改内容
					pSN->strSN.Format("%s", strOut); 
					PostMessage/*SendMessage*/(m_hParentWnd, MSG_UPDATA_SN, (WPARAM)pSN, 0);
					/*delete pSN;
					pSN = NULL;*/
					break;
				}
			}
			
		}
		//tSN* pSN = new tSN;
		//if (pSN){
		//	std::vector<CString> vStrAllSn;
		//	vStrAllSn.clear();
		//	vStrAllSn.push_back(strOut);
		//	pSN->strSN = vStrAllSn;
		//	/*PostMessage*/SendMessage(m_hParentWnd, MSG_UPDATA_SN, (WPARAM)pSN, 0);
		//	delete pSN;
		//	pSN = NULL;
		//}	
	}
	SetEvent(m_hEvent);
	return true;
}

void CHandleScan::SendACK2Src(bool bRet) {
	INT Ret = COMERR_OK, Rtn;

	INT PckSize = 4, TryCnt = 0;
	BYTE DataSend[4] = { 0x01,0x67 };
	CString strSend;
	strSend = "ACK|0 \r\n";
	if (bRet){
		strSend = "ACK|1 \r\n";
	}

__TryAgain:
	EnterCriticalSection(&m_csModBus);
	//newDataToStr(DataSend, PckSize, strSend);
	if (m_bDumpPackEn) {
		PrintLog(LOGLEVEL_WARNING, "PackStringtoBeSend:%s", strSend);
	}
	Rtn = WriteCom((BYTE*)(LPCSTR)strSend, strSend.GetLength());
	if (Rtn != strSend.GetLength()) {
		Ret = COMERR_WRITECOM;
		goto __end;
	}
	if (WaitEvent(m_UartPara.dwTimeout) == WAIT_TIMEOUT) {///等待响应超时，不是想要的设备
		PrintLog(LOGLEVEL_ERR, "Wait Uart Response TimeOut");
		Ret = COMERR_TIMEOUT; goto __end;
	}
	else {
		Ret = m_UartEvent.ErrCode;
		if (Ret == COMERR_OK) {
			/*switch (m_UartEvent.Data[1]) {
			case FUNCCODE_SETINITRDY:
				break;
			case FUNCCODE_SETINITRDY | FUNCCODE_EXCP:
				PrintLog(LOGLEVEL_ERR, "SetModuleInitReady : Sever do command failed, Errcode=0x%02X", m_UartEvent.Data[2]);
				Ret = COMERR_EXEFAIL;
				break;
			default:
				PrintLog(LOGLEVEL_ERR, "SetModuleInitReady : Sever function code error, FuncCode=0x%02X", m_UartEvent.Data[1]);
				Ret = COMERR_ERRPCK;
				break;
			}*/
		}
		else {
			PrintLog(LOGLEVEL_ERR, "SetModuleInitReady :  Uart Package Error, Errcode=%d", Ret);
		}
	}

__end:
	LeaveCriticalSection(&m_csModBus);
	if (Ret != COMERR_OK) {
		if ((INT)m_UartPara.dwRetryCnt > TryCnt) {
			TryCnt++;
			PrintLog(LOGLEVEL_WARNING, "CHandleScan::SendACK2Src=SetModuleInitReady : Try again, Cnt=%d", TryCnt);
			Sleep(m_UartPara.dwRetryTimeWait);
			goto __TryAgain;
		}
	}
	
}

void CHandleScan::SendRet2Uart(UINT nAdpStatus) {
	INT Ret = COMERR_OK, Rtn;
	INT TryCnt = 0;

	CString strSend;
	strSend += "R|";
	for (int i = 0; i < 8; i++){
		UINT adpRet = (nAdpStatus>>(2*i))&0x03;
		CString value;
		int n = 0;
		if (adpRet == 2){
			n = 1;
		}else if (adpRet == 0){
			n = 2;
		}
		value.Format("%d|", n);
		strSend += value;
	}
	strSend += "\r\n";

__TryAgain:
	EnterCriticalSection(&m_csModBus);

	Rtn = WriteCom((BYTE*)(LPCSTR)strSend, strSend.GetLength());
	if (Rtn != strSend.GetLength()) {
		Ret = COMERR_WRITECOM;
		goto __end;
	}
	if (WaitEvent(m_UartPara.dwTimeout) == WAIT_TIMEOUT) {///等待响应超时，不是想要的设备
		PrintLog(LOGLEVEL_ERR, "Wait Uart Response TimeOut");
		Ret = COMERR_TIMEOUT; goto __end;
	}
	else {
		Ret = m_UartEvent.ErrCode;
		if (Ret == COMERR_OK) {
		}
		else {
			PrintLog(LOGLEVEL_ERR, "SetModuleInitReady :  Uart Package Error, Errcode=%d", Ret);
		}
	}

__end:
	LeaveCriticalSection(&m_csModBus);
	if (Ret != COMERR_OK) {
		if ((INT)m_UartPara.dwRetryCnt > TryCnt) {
			TryCnt++;
			PrintLog(LOGLEVEL_WARNING, "CHandleScan::SendRet2Uart=SetModuleInitReady : Try again, Cnt=%d", TryCnt);
			Sleep(m_UartPara.dwRetryTimeWait);
			goto __TryAgain;
		}
	}

}

BOOL CHandleScan::IsEventReady()
{
	return m_hEvent == NULL ? FALSE : TRUE;
}

BOOL CHandleScan::ClearEvent()
{
	//return ResetEvent(m_hEvent);
	return TRUE;
}

INT CHandleScan::WaitEvent(UINT mTimeOut)
{
	if (mTimeOut == 0) {
		WaitForSingleObject(m_hEvent, INFINITE);
		return 0;
	}
	else {
		return WaitForSingleObject(m_hEvent, mTimeOut);
	}
}

void CHandleScan::test() {
	PrintLog(LOGLEVEL_WARNING, "is");
	//std::vector<CString> vStrAllSn;
	//vStrAllSn.clear();
	//tSN* tvSN = new tSN;
	//vStrAllSn.push_back("123456SN");
	//tvSN->strSN = vStrAllSn;
	//PostMessage(m_hParentWnd, MSG_UPDATA_SN, (WPARAM)tvSN, 0);
	//PostMessage(m_hParentWnd, MSG_UPDATA_SN, 0, 0);
}