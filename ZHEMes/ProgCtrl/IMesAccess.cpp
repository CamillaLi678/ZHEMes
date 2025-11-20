#include "stdafx.h"
#include "IMesAccess.h"
#include "ComTool.h"
#include "cJSON.h"
#include "json/json.h"
#define TMPSIZE_JSON (512)

IMesAccess::IMesAccess()
	:m_CharSet(CHARSET_UNICODE)
{
}


IMesAccess::~IMesAccess()
{
}


void IMesAccess::AttachLog(ILog*pILog)
{
	m_pILog = pILog;
}

BOOL IMesAccess::AttachDll(CString strDllPath, BOOL bSet)
{
	BOOL Ret = FALSE;
	return Ret;
}

BOOL IMesAccess::DetachDll()
{
	BOOL Ret = FALSE;
	return Ret;
}

INT IMesAccess::GetCharset()
{
	INT Ret = 0;
	if (m_IMesDll.pfnGetCharset) {
		Ret = m_IMesDll.pfnGetCharset();
	}
	return Ret;
}

BOOL IMesAccess::Init(CString strJsonFile)
{
	BOOL Ret = FALSE;
	if (m_IMesDll.pfnInit) {///字符串转UTF8编码
		DWORD BytesUsed;
		char strUTF8[TMPSIZE_JSON];
		memset(strUTF8, 0, TMPSIZE_JSON);
		if (CHARSET_UTF8 == m_CharSet) {
			if (MByteToUtf8(strJsonFile, strUTF8, TMPSIZE_JSON, BytesUsed) != 1) {
				Ret = FALSE;
				goto __end;
			}
		}
		else if(m_CharSet==CHARSET_UNICODE){
			if (MByteToWChar(strJsonFile,(LPWSTR)strUTF8, TMPSIZE_JSON, BytesUsed) != 1) {
				Ret = FALSE;
				goto __end;
			}
		}
		else {
			sprintf(strUTF8, "%s", strJsonFile.GetBuffer());
		}
		Ret= m_IMesDll.pfnInit(strUTF8);
	}
__end:
	return Ret;
}

BOOL IMesAccess::Deinit(void)
{
	BOOL Ret = FALSE;
	if (m_IMesDll.pfnDeinit) {
		Ret = m_IMesDll.pfnDeinit();
	}
	return Ret;
}

BOOL IMesAccess::GetErrMessage(CString&strErrMsg,CString*pErrCode)
{
	BOOL Ret = TRUE;
	cJSON* Root = NULL;
	//BYTE TmpDataTest[] = {
	//	123,0,34,0,69,0,114,0,114,0,67,0,111,0,100,0,101,0,34,0,58,0,34,0,77,0,69,0,83,0,95,0,67,0,104,0,101,0,99,0,107,0,79,0,112,0,101,0,114,0,97,0,116,0,111,0,114,0,34,0,44,0,34,0,69,0,114,0,114,0,77,0,115,0,103,0,34,0,58,0,34,0,40,117,55,98,73,0,68,0,60,0,97,0,62,0,13,78,88,91,40,87,33,0,34,0,125,0
	//};
	if (m_IMesDll.pfnGetErrMessage) {///返回的是UTF8，需要转码
		char ErrMsg[TMPSIZE_JSON];
		char strTmp[TMPSIZE_JSON];
		DWORD SizeUsed;
		memset(ErrMsg, 0, TMPSIZE_JSON);
		memset(strTmp, 0, TMPSIZE_JSON);
		Ret = m_IMesDll.pfnGetErrMessage(ErrMsg, TMPSIZE_JSON);
		//memcpy(ErrMsg, TmpDataTest, sizeof(TmpDataTest));
		if (Ret == TRUE) {
			if (m_CharSet == CHARSET_UTF8) {
				if (Utf8ToMByte((char*)ErrMsg, strTmp, TMPSIZE_JSON, SizeUsed) == 1) {
				}
				else {
					m_pILog->PrintLog(0, "MesAccess获取错误信息错误：编码转换失败");
					Ret = FALSE; goto __end;
				}
			}
			else if(m_CharSet==CHARSET_UNICODE){
				if (WCharToMByte((LPCWSTR)ErrMsg, strTmp, TMPSIZE_JSON, SizeUsed) == 1) {
				}
				else {
					m_pILog->PrintLog(0, "MesAccess获取错误信息错误：编码转换失败");
					Ret = FALSE; goto __end;
				}
			}
			else {
				memcpy(strTmp, ErrMsg, TMPSIZE_JSON);
			}
#if 0
			Json::Reader JReader;
			Json::Value Root;
			std::string strTmpJson;
			strTmpJson.assign(ErrMsg);
			if (JReader.parse(strTmpJson, Root) == true) {
				m_strErrMsg.Format("%s",Root["ErrMsg"].asCString());
				m_errCode.Format("%s", Root["ErrCode"].asCString());
			}
#endif 
			m_pILog->PrintLog(0, "MesAccess返回的信息:%s\r\n", strTmp);
			Root = cJSON_Parse(strTmp);
			cJSON *jErrCode, *jErrMsg;
			jErrCode = cJSON_GetObjectItem(Root, "ErrCode");
			if (jErrCode == NULL) {
				Ret = FALSE; goto __end;
			}
			jErrMsg = cJSON_GetObjectItem(Root, "ErrMsg");
			if (jErrMsg == NULL) {
				Ret = FALSE; goto __end;
			}
			m_strErrMsg.Format("%s", jErrMsg->valuestring);
			m_errCode.Format("%s", jErrCode->valuestring);
	
			strErrMsg = m_strErrMsg;
			if (pErrCode) {
				*pErrCode = m_errCode;
			}
		}
	}
	else {
		Ret = FALSE;
	}

__end:
	cJSON_Delete(Root);
	return Ret;
}


BOOL IMesAccess::CheckOperator(CString Operator, CString Password, CString &strJson)
{
	BOOL Ret = TRUE;
	if (m_IMesDll.pfnCheckOperator) {
		char strJsonUTF8[TMPSIZE_JSON];
		char strJsonConv[TMPSIZE_JSON];
		char strOperator[64];
		char strPasswd[64];
		DWORD SizeUsed;
		memset(strOperator, 0, 64);
		memset(strPasswd, 0, 64);
		memset(strJsonUTF8, 0, TMPSIZE_JSON);
		memset(strJsonConv, 0, TMPSIZE_JSON);
		if (m_CharSet == CHARSET_UTF8) {
			if (MByteToUtf8(Operator.GetBuffer(), strOperator, 64, SizeUsed) != 1){
				m_pILog->PrintLog(0, "MesAccess获取操作员权限错误：编码转换失败");
				Ret = FALSE; goto __end;
			}
			if (MByteToUtf8(Password.GetBuffer(), strPasswd, 64, SizeUsed) != 1) {
				m_pILog->PrintLog(0, "MesAccess获取操作员权限错误：编码转换失败");
				Ret = FALSE; goto __end;
			}
		}
		else if(m_CharSet==CHARSET_UNICODE){
			if (MByteToWChar(Operator.GetBuffer(),(LPWSTR)strOperator, 64, SizeUsed) != 1) {
				m_pILog->PrintLog(0, "MesAccess获取操作员权限错误：编码转换失败");
				Ret = FALSE; goto __end;
			}
			if (MByteToWChar(Password.GetBuffer(),(LPWSTR)strPasswd, 64, SizeUsed) != 1) {
				m_pILog->PrintLog(0, "MesAccess获取操作员权限错误：编码转换失败");
				Ret = FALSE; goto __end;
			}
		}
		else {
			memcpy(strOperator, Operator.GetBuffer(), Operator.GetLength());
			memcpy(strPasswd, Password.GetBuffer(), Password.GetLength());
		}
		try{ 
			Ret = m_IMesDll.pfnCheckOperator(strOperator, strPasswd, strJsonUTF8, TMPSIZE_JSON);
		}
		catch (...){
			m_pILog->PrintLog(0, "MesAccess获取权限有异常");
			Ret = FALSE;
		}		
		if (Ret == TRUE) {
			if (m_CharSet == CHARSET_UTF8) {
				if (Utf8ToMByte(strJsonUTF8, strJsonConv, TMPSIZE_JSON, SizeUsed) == 1) {
					strJson.Format("%s", strJsonConv);
				}
				else {
					m_pILog->PrintLog(0, "MesAccess获取操作员权限错误：编码转换失败");
					Ret = FALSE;
				}
			}
			else if(m_CharSet==CHARSET_UNICODE){
				if (WCharToMByte((LPCWSTR)strJsonUTF8, strJsonConv, TMPSIZE_JSON, SizeUsed) == 1) {
					strJson.Format("%s", strJsonConv);
				}
				else {
					m_pILog->PrintLog(0, "MesAccess获取操作员权限错误：编码转换失败");
					Ret = FALSE;
				}
			}
			else {
				strJson.Format("%s", strJsonUTF8);
			}
		}
	}
	else {
		Ret = FALSE;
	}

__end:
	return Ret;
}

BOOL IMesAccess::GetWorkOrderRecord(CString strWorkOrder, CString strMaterialNo, CString&strJsonRecode)
{
	char *pDataTmp = NULL,*pConv=NULL;
	int DataSize = 4 * 1024;
	DWORD SizeUsed;
	BOOL Ret = TRUE;
	if (m_IMesDll.pfnGetWorkOrderRecord) {
		char tmpWorkOrder[64];
		char tmpMaterialNo[64];
		pDataTmp = new char[DataSize];
		if (!pDataTmp) {
			Ret = FALSE;
			goto __end;
		}
		pConv = new char[DataSize];
		if (!pConv) {
			Ret = FALSE;
			goto __end;
		}
		memset(pDataTmp, 0, DataSize);
		memset(pConv, 0, DataSize);
		memset(tmpWorkOrder, 0, 64);
		memset(tmpMaterialNo, 0, 64);
		if (m_CharSet == CHARSET_UTF8) {
			if (MByteToUtf8(strWorkOrder.GetBuffer(), tmpWorkOrder, 64, SizeUsed) != 1) {
				m_pILog->PrintLog(0, "MesAccess获取操作员权限错误：编码转换失败");
				Ret = FALSE; goto __end;
			}
			if (MByteToUtf8(strMaterialNo.GetBuffer(), tmpMaterialNo, 64, SizeUsed) != 1) {
				m_pILog->PrintLog(0, "MesAccess获取操作员权限错误：编码转换失败");
				Ret = FALSE; goto __end;
			}
		}
		else if(m_CharSet==CHARSET_UNICODE){
			if (MByteToWChar(strWorkOrder.GetBuffer(), (LPWSTR)tmpWorkOrder, 64, SizeUsed) != 1) {
				m_pILog->PrintLog(0, "MesAccess获取操作员权限错误：编码转换失败");
				Ret = FALSE; goto __end;
			}
			if (MByteToWChar(strMaterialNo.GetBuffer(), (LPWSTR)tmpMaterialNo, 64, SizeUsed) != 1) {
				m_pILog->PrintLog(0, "MesAccess获取操作员权限错误：编码转换失败");
				Ret = FALSE; goto __end;
			}
		}
		else {
			memcpy(tmpWorkOrder, strWorkOrder.GetBuffer(), strWorkOrder.GetLength());
			memcpy(tmpMaterialNo, strMaterialNo.GetBuffer(), strMaterialNo.GetLength());
		}
		Ret=m_IMesDll.pfnGetWorkOrderRecord(tmpWorkOrder, tmpMaterialNo, pDataTmp, DataSize);
		if (Ret == TRUE) {///需要将pDataTmp从UTF8转码
			if (m_CharSet == CHARSET_UTF8) {
				if (Utf8ToMByte(pDataTmp, pConv, DataSize, SizeUsed) == 1) {
					strJsonRecode.Format("%s", pConv);
				}
				else {
					m_pILog->PrintLog(0, "MesAccess获取操作员权限错误：编码转换失败");
					Ret = FALSE;
				}
			}
			else if (m_CharSet == CHARSET_UNICODE) {
				if (WCharToMByte((LPCWSTR)pDataTmp, pConv, DataSize, SizeUsed) == 1) {
					strJsonRecode.Format("%s", pConv);
				}
				else {
					m_pILog->PrintLog(0, "MesAccess获取操作员权限错误：编码转换失败");
					Ret = FALSE;
				}
			}
			else {
				strJsonRecode.Format("%s", pDataTmp);
			}
		}
	}
	else {
		Ret = FALSE;
	}
__end:
	if (pDataTmp) {
		delete[] pDataTmp;
	}
	if (pConv) {
		delete[] pConv;
	}
	return Ret;
}

BOOL IMesAccess::GetEMMCMIDInfo(CString strWorkOrder, CString strMaterialNo, CString strEMMCMID, CString&strJsonRecode)
{
	char *pDataTmp = NULL, *pConv = NULL;
	int DataSize = 4 * 1024;
	DWORD SizeUsed;
	BOOL Ret = TRUE;
	if (m_IMesDll.pfnGetEMMCMIDInfo) {
		char tmpWorkOrder[64];
		char tmpMaterialNo[64];
		char tmpEMMCMID[64];
		pDataTmp = new char[DataSize];
		if (!pDataTmp) {
			Ret = FALSE;
			goto __end;
		}
		pConv = new char[DataSize];
		if (!pConv) {
			Ret = FALSE;
			goto __end;
		}
		memset(pDataTmp, 0, DataSize);
		memset(pConv, 0, DataSize);
		memset(tmpWorkOrder, 0, 64);
		memset(tmpMaterialNo, 0, 64);
		memset(tmpEMMCMID, 0, 64);
		if (m_CharSet == CHARSET_UTF8) {
			if (MByteToUtf8(strWorkOrder.GetBuffer(), tmpWorkOrder, 64, SizeUsed) != 1) {
				m_pILog->PrintLog(0, "MesAccess获取操作员权限错误：编码转换失败");
				Ret = FALSE; goto __end;
			}
			if (MByteToUtf8(strMaterialNo.GetBuffer(), tmpMaterialNo, 64, SizeUsed) != 1) {
				m_pILog->PrintLog(0, "MesAccess获取操作员权限错误：编码转换失败");
				Ret = FALSE; goto __end;
			}
			if (MByteToUtf8(strEMMCMID.GetBuffer(), tmpEMMCMID, 64, SizeUsed) != 1) {
				m_pILog->PrintLog(0, "MesAccess获取操作员权限错误：编码转换失败");
				Ret = FALSE; goto __end;
			}
		}
		else if (m_CharSet == CHARSET_UNICODE) {
			if (MByteToWChar(strWorkOrder.GetBuffer(), (LPWSTR)tmpWorkOrder, 64, SizeUsed) != 1) {
				m_pILog->PrintLog(0, "MesAccess获取操作员权限错误：编码转换失败");
				Ret = FALSE; goto __end;
			}
			if (MByteToWChar(strMaterialNo.GetBuffer(), (LPWSTR)tmpMaterialNo, 64, SizeUsed) != 1) {
				m_pILog->PrintLog(0, "MesAccess获取操作员权限错误：编码转换失败");
				Ret = FALSE; goto __end;
			}
			if (MByteToWChar(strEMMCMID.GetBuffer(), (LPWSTR)tmpEMMCMID, 64, SizeUsed) != 1) {
				m_pILog->PrintLog(0, "MesAccess获取操作员权限错误：编码转换失败");
				Ret = FALSE; goto __end;
			}
		}
		else {
			memcpy(tmpWorkOrder, strWorkOrder.GetBuffer(), strWorkOrder.GetLength());
			memcpy(tmpMaterialNo, strMaterialNo.GetBuffer(), strMaterialNo.GetLength());
			memcpy(tmpEMMCMID, strEMMCMID.GetBuffer(), strEMMCMID.GetLength());
		}
		Ret = m_IMesDll.pfnGetEMMCMIDInfo(tmpWorkOrder, tmpMaterialNo, tmpEMMCMID, pDataTmp, DataSize);
		if (Ret == TRUE) {///需要将pDataTmp从UTF8转码
			if (m_CharSet == CHARSET_UTF8) {
				if (Utf8ToMByte(pDataTmp, pConv, DataSize, SizeUsed) == 1) {
					strJsonRecode.Format("%s", pConv);
				}
				else {
					m_pILog->PrintLog(0, "MesAccess获取操作员权限错误：编码转换失败");
					Ret = FALSE;
				}
			}
			else if (m_CharSet == CHARSET_UNICODE) {
				if (WCharToMByte((LPCWSTR)pDataTmp, pConv, DataSize, SizeUsed) == 1) {
					strJsonRecode.Format("%s", pConv);
				}
				else {
					m_pILog->PrintLog(0, "MesAccess获取操作员权限错误：编码转换失败");
					Ret = FALSE;
				}
			}
			else {
				strJsonRecode.Format("%s", pDataTmp);
			}
		}
	}
	else {
		Ret = FALSE;
	}
__end:
	if (pDataTmp) {
		delete[] pDataTmp;
	}
	if (pConv) {
		delete[] pConv;
	}
	return Ret;
}


BOOL IMesAccess::SendProgrammerInfo(CString strProgsInfoJson)
{
	BOOL Ret = TRUE;
	DWORD BytesUsed;
	INT RetConv = 0;
	INT Size = 32 * 1024;
	char*pDestData = NULL;
	pDestData = new char[Size];
	if (!pDestData) {
		m_pILog->PrintLog(0, "发送编程器信息错误：内存分配不足\r\n");
		Ret = FALSE; goto __end;
	}
	memset(pDestData, 0, Size);
	if (CHARSET_UTF8 == m_CharSet) {
		RetConv = MByteToUtf8(strProgsInfoJson.GetBuffer(), pDestData, Size, BytesUsed);
		if (RetConv != 1) {
			m_pILog->PrintLog(0, "发送编程器信息错误：UTF8编码转换失败,ErrCode:%d",RetConv);
			Ret = FALSE; goto __end;
		}
	}
	else if (m_CharSet == CHARSET_UNICODE){
		RetConv = MByteToWChar(strProgsInfoJson.GetBuffer(), (LPWSTR)pDestData, Size, BytesUsed);
		if (RetConv != 1) {
			m_pILog->PrintLog(0, "发送编程器信息错误：UNICODE编码转换失败,ErrCode:%d",RetConv);
			Ret = FALSE; goto __end;
		}
	}
	else {
		memcpy(pDestData, strProgsInfoJson.GetBuffer(), strProgsInfoJson.GetLength());
	}
	Ret = m_IMesDll.pfnSendProgrammerInfo(pDestData);
__end:
	if (pDestData) {
		delete[] pDestData;
	}
	return Ret;
}

BOOL IMesAccess::SendStatus(CString strStatusJson)
{
	BOOL Ret = TRUE;
	DWORD BytesUsed;
	INT RetConv = 0;
	INT Size = 32 * 1024;
	char*pDestData = NULL;
	pDestData = new char[Size];
	if (!pDestData) {
		m_pILog->PrintLog(0, "发送状态错误：内存分配不足\r\n");
		Ret = FALSE; goto __end;
	}
	memset(pDestData, 0, Size);
	if (m_CharSet == CHARSET_UTF8) {
		RetConv = MByteToUtf8(strStatusJson.GetBuffer(), pDestData, Size, BytesUsed);
		if (RetConv != 1) {
			m_pILog->PrintLog(0, "发送编程器信息错误：UTF8编码转换失败,ErrCode:%d", RetConv);
			Ret = FALSE; goto __end;
		}
	}
	else if(m_CharSet==CHARSET_UNICODE){
		RetConv = MByteToWChar(strStatusJson.GetBuffer(), (LPWSTR)pDestData, Size, BytesUsed);
		if (RetConv != 1) {
			m_pILog->PrintLog(0, "发送编程器信息错误：UNICODE编码转换失败,ErrCode:%d", RetConv);
			Ret = FALSE; goto __end;
		}
	}
	else {
		memcpy(pDestData, strStatusJson.GetBuffer(), strStatusJson.GetLength());
	}
	Ret = m_IMesDll.pfnSendStatus(pDestData);
__end:
	if (pDestData) {
		delete[] pDestData;
	}
	return Ret;
}

BOOL IMesAccess::SendReport(CString strReportJson)
{
	BOOL Ret = FALSE;
	return Ret;
}

BOOL IMesAccess::SendReport(std::string strReportJson)
{
	BOOL Ret = TRUE;
	DWORD BytesUsed;
	INT RetConv = 0;
	INT Size = 512 * 1024;
	char*pDestData = NULL;
	pDestData = new char[Size];
	if (!pDestData) {
		m_pILog->PrintLog(0, "发送报告失败：内存分配不足\r\n");
		Ret = FALSE; goto __end;
	}
	memset(pDestData, 0, Size);
	if (m_CharSet == CHARSET_UTF8) {
		RetConv = MByteToUtf8(strReportJson.c_str(), pDestData, Size, BytesUsed);
		if (RetConv != 1) {
			m_pILog->PrintLog(0, "发送编程器信息错误：UTF8编码转换失败,ErrCode:%d", RetConv);
			Ret = FALSE; goto __end;
		}
	}
	else if(m_CharSet==CHARSET_UNICODE){
		RetConv = MByteToWChar(strReportJson.c_str(), (LPWSTR)pDestData, Size, BytesUsed);
		if (RetConv != 1) {
			m_pILog->PrintLog(0, "发送编程器信息错误：UNICODE编码转换失败,ErrCode:%d", RetConv);
			Ret = FALSE; goto __end;
		}
	}
	else {
		memcpy(pDestData, strReportJson.c_str(), strReportJson.length());
	}
	Ret = m_IMesDll.pfnSendReport(pDestData);
__end:
	if (pDestData) {
		delete[] pDestData;
	}
	return Ret;
}
