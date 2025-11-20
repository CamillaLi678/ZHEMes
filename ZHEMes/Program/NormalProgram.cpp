#include "stdafx.h"
#include "NormalProgram.h"
#include "WorkThread.h"
#include "json/json.h"
#include "Setting.h"
#include "Utils/ComTool.h"
#include "ProgRecord.h"
#include "cJSON.h"
#include "IProgFileCheck.h"
#include "BlockTcpSocket.h"
#include "Serial.h"

NormalProgram::NormalProgram(CSetting *pSetting, ILog *pILog)
{
	m_pSetting = pSetting;
	m_pILog = pILog;
	m_bExitApp = FALSE;
}

NormalProgram::~NormalProgram()
{
	m_StdMes.CloseDllCom();
	m_bCloseDll = TRUE;
	m_bExitApp = TRUE;
}

static int _stdcall MsgHandle(void*Para, char*Msg, char*MsgData)
{
	int Ret = 0;
	NormalProgram *pProgram = (NormalProgram*)Para;
	if (pProgram) {
		pProgram->HandleMsg(Msg, MsgData);
	}
	return Ret;
}

int NormalProgram::HandleMsg(char*Msg, char*MsgData)
{
	int Ret = 0;
	CString strMsg;

	//push back msg to message list, wait ParseTask to parse.
	m_ProgramMsgListMutex.Lock();
	tStdMesMessage stdMesMsg;
	stdMesMsg.msg = Msg;
	stdMesMsg.json = MsgData;
	mRecvProgramMsgList.push_back(stdMesMsg);
	m_ProgramMsgListMutex.Unlock();

	return Ret;
}

INT NormalProgram::StartService()
{
	INT Ret = 0, RtnCall;
	BOOL Rtn;
	CString strMsg;
	m_bFinishStartupACServer = FALSE;
	if (1) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "启动服务器...");
		Rtn = m_StdMes.MesStartService();
	}
	else {
		m_pILog->PrintLog(LOGLEVEL_LOG, "启动简单服务器...");
		Rtn = m_StdMes.MesStartServiceSimple();
	}

	m_pILog->PrintLog(LOGLEVEL_LOG, "启动服务器 %s", Rtn == TRUE ? "成功" : "失败");
	if (Rtn == FALSE) {
		return -1;
	}

	m_pILog->PrintLog(LOGLEVEL_LOG, "设置编程控制系统消息回调函数...");
	CMsgHandler<NormalProgram, NormalProgram> MsgListHandler = MakeMsgHandler(this, &NormalProgram::ThreadMsgHandler);
	m_MsgListHandleThread.SetMsgHandle(MsgListHandler);
	m_MsgListHandleThread.CreateThread();
	m_MsgListHandleThread.PostMsg(MSG_LIST_HANDLE, 0, 0);

	RtnCall = m_StdMes.MesSetMsgHandle(MsgHandle, this);
	m_pILog->PrintLog(LOGLEVEL_LOG, "设置消息回调函数 %s", RtnCall == 1 ? "成功" : "失败");
	if (RtnCall == 0) {
		Ret = -1; goto __end;
	}

	m_bFinishStartupACServer = true;
__end:
	return Ret;
}


INT NormalProgram::CreateProjectByTemplate()
{
	INT Ret = 0, bRet, nRetCode;
	Json::Value Root;
	Json::Value JFile;
	Json::Value JProeprty;
	Json::StyledWriter JWriter;
	std::string strJson;
	CString strErrmsg;
	CString strProgFilePath, strTemplateFilePath, strProjSavePath, strSwap;

	strTemplateFilePath = m_ProgRecord.DestRecord.strTempFilePath;//GetTemplateAbsPath();
	strProgFilePath = m_ProgRecord.DestRecord.strProgFilePath;
	strProjSavePath = m_ProgRecord.DestRecord.strProjPath;
	Root["TemplatePath"] = Json::Value(strTemplateFilePath);
	JProeprty["Property"] = Json::Value("FilePath");
	JProeprty["Value"] = Json::Value(strProgFilePath);
	JFile["File"].append(JProeprty);
	///////////////////
	strSwap = "0";
	if (strTemplateFilePath.CompareNoCase("SWAP1") == 0) {
		strSwap.Format("%d", 1);
	}
	//////////////////
	/*strSwap.Format("%s", m_strSwap);
	if (strSwap.IsEmpty()) {
	strSwap.Format("%d", 0);
	}*/
	JProeprty["Property"] = Json::Value("Swap");
	JProeprty["Value"] = Json::Value(strSwap);
	JFile["File"].append(JProeprty);
	///////////////////
	Root["ImportFile"].append(JFile);
	strJson = JWriter.write(Root);

	///创建工程为异步执行，需要等待结果完成
	m_pILog->PrintLog(LOGLEVEL_LOG, "自动化合成工程文件...");
	m_pILog->PrintLog(LOGLEVEL_LOG, "烧录文件路径:%s", strProgFilePath);
	m_pILog->PrintLog(LOGLEVEL_LOG, "工程模板文件路径:%s", strTemplateFilePath);
	m_pILog->PrintLog(LOGLEVEL_LOG, "工程文件保存路径:%s", strProjSavePath);
	bRet = m_StdMes.MesCreateProjectByTemplate(strProjSavePath.GetBuffer(), (char*)strJson.c_str());
	strProjSavePath.ReleaseBuffer();
	if (bRet == FALSE) {
		strErrmsg.Format("创建工程失败");
		Ret = -1; goto __end;
	}
	nRetCode = WaitJobDone(&m_StdMes);
	if (nRetCode != 1) {
		strErrmsg.Format("创建工程失败");
		Ret = -1; goto __end;
	}
	else {
		m_pILog->PrintLog(LOGLEVEL_LOG, "创建工程成功");
	}

__end:
	if (Ret != 0) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "%s", (LPSTR)(LPCSTR)strErrmsg);
	}
	return Ret;
}

void NormalProgram::SetQCCheckResult(BOOL bSuccess) { 
	m_pILog->PrintLog(LOGLEVEL_LOG, "SetQCCheckResult bSuccess:%d", bSuccess);
	m_bDoCheckQC = bSuccess; 
}

INT NormalProgram::GetProgramResult(CProgramResultData &ProgramResultData) {
	ProgramResultData.AutoFailCnt = m_ProgramResult.AutoFailCnt;
	ProgramResultData.AutoPassCnt = m_ProgramResult.AutoPassCnt;
	ProgramResultData.AutoTotalCnt = m_ProgramResult.AutoTotalCnt;
	ProgramResultData.AutoUPH = m_ProgramResult.AutoUPH;
	ProgramResultData.DevTimeRun = m_ProgramResult.DevTimeRun;
	ProgramResultData.DevTimeStop = m_ProgramResult.DevTimeStop;
	ProgramResultData.LastYieldChangeJson = m_ProgramResult.LastYieldChangeJson;
	return 0;
}

INT NormalProgram::SetServerPath()
{
	INT Ret = 0;
	if (m_pSetting->strACServerFolder != "") {///在UI主线程中打开动态库
		CString strStdDllPath;
		m_StdMes.CloseDllCom();
		strStdDllPath.Format("%s\\StdMES.dll", m_pSetting->strACServerFolder);
		m_StdMes.SetDllPath(strStdDllPath);
		Ret = m_StdMes.OpenDllCom();
		if (Ret != 0) {
			AfxMessageBox("从StdMes.dll中导出函数接口失败，请确定服务器路径是否正确");
		}
	}
	else {
		AfxMessageBox("请先设置服务器的路径");
		Ret = -1;
	}
	return Ret;
}

INT NormalProgram::ThreadMsgHandler(MSG msg, void *Para)
{
	INT Ret = 0;
	if (msg.message == MSG_LIST_HANDLE) {
		Ret = DoHandleMsgList();
	}
	return -1;///为了让线程处理完消息之后自动退出，返回-1不在进行消息的获取
}

INT NormalProgram::DoHandleMsgList()
{
	while (!m_bExitApp) {
		tStdMesMessage stdMesMsg;
		bool getProgramMsg = false;
		m_ProgramMsgListMutex.Lock();
		if (!mRecvProgramMsgList.empty()) {
			stdMesMsg = mRecvProgramMsgList.front();
			mRecvProgramMsgList.pop_front();
			getProgramMsg = true;
		}
		m_ProgramMsgListMutex.Unlock();
		if (!getProgramMsg) {
			Sleep(5);
			continue;
		}

		const char *Msg = stdMesMsg.msg.c_str();
		const char *MsgData = stdMesMsg.json.c_str();
		if (_stricmp(Msg, "YieldChange") == 0) {
			CString strRecvJson = _T("");
			std::string strJson(MsgData);
			Json::Value Root;
			Json::Reader JReader;
			if (JReader.parse(strJson, Root) == false) {
				m_pILog->PrintLog(LOGLEVEL_LOG, "解析json失败");
			}
			else {
				tYieldSites *result = new tYieldSites();
				m_YieldSitesMutex.Lock();

				m_YieldSites.TotalCnt += Root["CurTotal"].asInt();
				m_YieldSites.FailCnt += Root["CurFail"].asInt();
				m_YieldSites.PassCnt += Root["CurPass"].asInt();
				//m_pILog->PrintLog(LOGLEVEL_LOG, "收到YieldChange PassCnt: %d", m_YieldSites.PassCnt);
				result->PassCnt = m_YieldSites.PassCnt;
				result->FailCnt = m_YieldSites.FailCnt;
				result->TotalCnt = m_YieldSites.TotalCnt;
				strRecvJson.Format("%s", MsgData);
				result->lastYieldJson = strRecvJson;
				m_YieldSitesMutex.Unlock();

				PostMessage(m_MainHwd, MSG_UPDATE_PROGRAM_COUNT, (WPARAM)(result), (LPARAM)0);
			}

			m_ProgramResult.LastYieldChangeJson = strRecvJson;
		}
		else if (_stricmp(Msg, "StatusChange") == 0) {
			/*if (m_pIMesAccess) {
			strMsg.Format("%s", MsgData);
			m_pIMesAccess->SendStatus(strMsg);
			}*/
		}
		else if (_stricmp(Msg, "ShowLog") == 0) {
		}
		else if (_stricmp(Msg, "MissionResult") == 0) {
			m_pILog->PrintLog(LOGLEVEL_LOG, "收到MissionResult消息");
			m_bGetMissionResult = TRUE;
		}
	}

	return 0;
}

INT NormalProgram::LoadProject() {
	BOOL Rtn;
	CString strErrmsg;
	INT Ret = 0, nRetCode;
	CString strProjFile;
	CString strFuncMode = m_pSetting->strCurExec;
	strProjFile = m_ProgRecord.DestRecord.strProjPath;
	m_pILog->PrintLog(LOGLEVEL_LOG, "加载工程...");
	Rtn = m_StdMes.MesLoadProjectWithLot(strProjFile.GetBuffer(), strFuncMode.GetBuffer(), 0);
	if (Rtn == FALSE) {
		strErrmsg.Format("加载工程失败");
		Ret = -1;
		goto __end;
	}

	nRetCode = WaitJobDone(&m_StdMes);
	if (nRetCode != 1) {
		strErrmsg.Format("加载工程失败");
		Ret = -1; goto __end;
	}
	else {
		m_pILog->PrintLog(LOGLEVEL_LOG, "加载工程成功");
	}
__end:
	if (Ret != 0) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "%s", strErrmsg);
	}
	return Ret;
}

INT NormalProgram::GetProjectInfo()
{
	BOOL Rtn;
	CString strErrmsg;
	INT Ret = 0;
	char buffer[4096];
	int sizeNeed = 0;

	cJSON* pRootParser;
	cJSON* pChipName;
	CString chipName = _T("");

	Rtn = m_StdMes.MesGetProjectInfo_Json(buffer, 4096, &sizeNeed);
	if (Rtn == FALSE) {
		strErrmsg.Format("查询工程文件信息失败 sizeNeed:%d", sizeNeed);
		Ret = -1;
		goto __end;
	}

	pRootParser = cJSON_Parse(buffer);
	if (pRootParser == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "Mes返回的不符合Json数据格式 ");
		Ret = -1;
		goto __end;
	}

	pChipName = cJSON_GetObjectItem(pRootParser, "ChipName");
	if (pChipName == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "解析Mes返回的pChipName字段错误，请确认Mes维护的字段信息, ");
		Ret = -1;
		goto __end;
	}

	chipName = pChipName->valuestring;
	m_strChipName = chipName;
	m_pILog->PrintLog(LOGLEVEL_LOG, "chipName: %s", chipName);
__end:
	if (Ret != 0) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "%s", strErrmsg);
	}
	return Ret;
}

BOOL NormalProgram::GetStatusJsonResult(INT &Total, INT &Pass, INT& Fail)
{
	BOOL Rtn = FALSE;
	CHAR *pStatusBuf = NULL;
	Json::Reader JReader;
	Json::Value Root;
	std::string strJson;
	INT Size = 4096;
	pStatusBuf = new CHAR[Size];///这个地方为了获得状态的成功，需要分配至少2K的Buffer才
	if (!pStatusBuf) {
		Rtn = -1; goto __end;
	}

	memset(pStatusBuf, 0, Size);
	Rtn = m_StdMes.MesGetStatus_Json(pStatusBuf, Size);
	//m_DlgLog.ShowLog(LOGLEVEL_LOG,pStatusBuf);
	if (Rtn != TRUE) {
		goto __end;
	}
	strJson.assign(pStatusBuf);
	if (JReader.parse(strJson, Root) == false) {
		Rtn = FALSE; goto __end;
	}
	Total = Root["Total"].asInt();
	Pass = Root["Pass"].asInt();
	Fail = Root["Fail"].asInt();

__end:
	if (pStatusBuf) {
		delete pStatusBuf;
	}
	return Rtn;
}


CString NormalProgram::GetChecksumExt() {
	CString Ret = _T("");
	char strChksumJson[1024];
	cJSON* pRootParser = NULL;
	cJSON* pChecksumExtend = NULL;
	cJSON* pArrayItem = NULL;
	cJSON* pChecksumName = NULL;
	cJSON* pChecksumValue = NULL;
	CString strChecksumName = _T("");
	CString strChecksumValue = _T("");

	BOOL Rtn = m_StdMes.MesGetChecksumExt_Json(strChksumJson, sizeof(strChksumJson));
	if (Rtn) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "从工程文件中获取校验值%s", strChksumJson);
		pRootParser = cJSON_Parse(strChksumJson);
		if (pRootParser == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "返回的不符合Json数据格式 ");
			goto __end;
		}

		pChecksumExtend = cJSON_GetObjectItem(pRootParser, "ChecksumExtend");
		if (pChecksumExtend == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析返回的ChecksumExtend字段错误");
			goto __end;
		}

		int size = cJSON_GetArraySize(pChecksumExtend);
		for (int i = 0; i < size; i++) {
			pArrayItem = cJSON_GetArrayItem(pChecksumExtend, i);
			pChecksumName = cJSON_GetObjectItem(pArrayItem, "Name");
			if (pChecksumName == NULL) {
				m_pILog->PrintLog(LOGLEVEL_ERR, "解析返回的Name字段错误");
				goto __end;
			}
			pChecksumValue = cJSON_GetObjectItem(pArrayItem, "Value");
			if (pChecksumValue == NULL) {
				m_pILog->PrintLog(LOGLEVEL_ERR, "解析返回的Value字段错误");
				goto __end;
			}
			CString outString = _T("");
			outString.Format(" [%s:%s]", pChecksumName->valuestring, pChecksumValue->valuestring);
			Ret += outString;
		}

	}
__end:
	if (pRootParser) {
		cJSON_Delete(pRootParser);
		pRootParser = NULL;
	}
	return Ret;
}

INT NormalProgram::WaitJobDone(CStdMesApi* pStdMesApi)
{
	INT Ret = 0;
	BOOL Rtn;
	char TmpBuf[64];
	while (1) {
		Sleep(300); ////300ms查询一次
		memset(TmpBuf, 0, 64);
		Rtn = pStdMesApi->MesGetResult(TmpBuf, 64);
		if (Rtn != TRUE) {
			Ret = -1;
			break;
		}
		if (strstr(TmpBuf, "OK") != 0) {///命令执行成功
			Ret = 1;
			break;
		}
		else if (strstr(TmpBuf, "ERROR") != 0) {///命令执行失败
			m_pILog->PrintLog(LOGLEVEL_ERR, "命令执行失败:%s", TmpBuf);
			break;
		}
	}

	return Ret;
}

void NormalProgram::StartDoProgramTaskWork()
{
	CMsgHandler<NormalProgram, NormalProgram> MsgHandler = MakeMsgHandler(this, &NormalProgram::DoProgramTaskWorkProc);
	m_wTaskWorkThread.SetMsgHandle(MsgHandler);
	m_wTaskWorkThread.CreateThread();
	m_wTaskWorkThread.PostMsg(MSG_STARTWORK, 0, 0);
}

INT NormalProgram::DoProgramTaskWorkProc(MSG msg, void *Para)
{
	INT Ret = 0;

	if (msg.message == MSG_STARTWORK) {
		while (1) {
			Sleep(100);
			Ret = DoTask();
			break;
		}
	}

	return Ret;
}

INT NormalProgram::CompareChecksumProj()
{
	INT Ret = 0;
	BOOL Rtn = FALSE;
	UINT64 Checksum = 0;
	UINT64 ChecksumExt = 0;
	UINT64 Checksumdesired = 0;
	char strChksum[8];
	char *pRealChksum = NULL;
	CString strDesired;
	memset(strChksum, 0, 8);

	//if (m_ProgRecord.DestRecord.strProjChecksum.IsEmpty() ) {
	//	m_pILog->PrintLog(LOGLEVEL_LOG, "无需比对工程的校验值");
	//	return Ret;
	//}

	strDesired = m_ProgRecord.DestRecord.strProjChecksum;
	strDesired.Replace("0x", "");
	sscanf(strDesired.GetBuffer(), "%I64X", &Checksumdesired);
	Rtn = m_StdMes.MesGetChecksum(strChksum);
	if (Rtn == FALSE) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "从工程文件中获取校验值错误");
		Ret = -1; goto __end;
	}
	else {
		m_pILog->PrintLog(LOGLEVEL_LOG, "从工程文件中获取校验值成功");
		pRealChksum = new char[8];
		memcpy(pRealChksum, strChksum, 8);
		Checksum = *(UINT64*)strChksum;
		m_strRealChecksum.Format("0x%08X", Checksum);
		CString strGetDllChecksumExt = GetChecksumExtJson();
		m_strRealChecksumExt.Format("%s%s", m_strRealChecksum, strGetDllChecksumExt);
		CString strChipChecksumExt = GetChecksumExtForChip(m_strChipName, strGetDllChecksumExt, m_MapChecksumExtList);
		if (!strChipChecksumExt.IsEmpty()) {
			strChipChecksumExt.Replace("0x", "");
			sscanf(strChipChecksumExt.GetBuffer(), "%I64X", &ChecksumExt);
			if (ChecksumExt != Checksumdesired) {
				m_pILog->PrintLog(LOGLEVEL_LOG, "扩展校验值比对错误: 工程中的校验值为0x%I64X, 从文件中获取到的的校验值为0x%I64X", ChecksumExt, Checksumdesired);
				Ret = -1;
			}
		}
		else {
			if (Checksum != Checksumdesired) {
				m_pILog->PrintLog(LOGLEVEL_LOG, "校验值比对错误: 工程中的校验值为0x%I64X, 从文件中获取到的的校验值为0x%I64X", Checksum, Checksumdesired);
				Ret = -1;
			}
		}
		{
			///准备更新给界面显示
			PostMessage(m_MainHwd, MSG_UPDATECHECKSUM, (WPARAM)pRealChksum, 0);
			m_pILog->PrintLog(LOGLEVEL_LOG, "校验值比对成功");
		}
	}

__end:
	return Ret;
}

CString NormalProgram::GetFinalChecksumExt() {
	return m_strRealChecksumExt;
}

CString NormalProgram::GetChecksumExtJson() {
	CString Ret = _T("");
	char strChksumJson[1024];
	cJSON* pRootParser = NULL;
	cJSON* pChecksumExtend = NULL;
	cJSON* pArrayItem = NULL;
	cJSON* pChecksumName = NULL;
	cJSON* pChecksumValue = NULL;
	CString strChecksumName = _T("");
	CString strChecksumValue = _T("");

	BOOL Rtn = m_StdMes.MesGetChecksumExt_Json(strChksumJson, sizeof(strChksumJson));
	if (Rtn) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "从工程文件中获取校验值%s", strChksumJson);
		pRootParser = cJSON_Parse(strChksumJson);
		if (pRootParser == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "返回的不符合Json数据格式 ");
			goto __end;
		}

		pChecksumExtend = cJSON_GetObjectItem(pRootParser, "ChecksumExtend");
		if (pChecksumExtend == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析返回的ChecksumExtend字段错误");
			goto __end;
		}

		int size = cJSON_GetArraySize(pChecksumExtend);
		for (int i = 0; i < size; i++) {
			pArrayItem = cJSON_GetArrayItem(pChecksumExtend, i);
			pChecksumName = cJSON_GetObjectItem(pArrayItem, "Name");
			if (pChecksumName == NULL) {
				m_pILog->PrintLog(LOGLEVEL_ERR, "解析返回的Name字段错误");
				goto __end;
			}
			pChecksumValue = cJSON_GetObjectItem(pArrayItem, "Value");
			if (pChecksumValue == NULL) {
				m_pILog->PrintLog(LOGLEVEL_ERR, "解析返回的Value字段错误");
				goto __end;
			}
			CString outString = _T("");
			outString.Format(" [%s:%s]", pChecksumName->valuestring, pChecksumValue->valuestring);
			Ret += outString;
		}

	}
__end:
	if (pRootParser) {
		cJSON_Delete(pRootParser);
		pRootParser = NULL;
	}
	return Ret;
}

///对比烧录档案校验值
INT NormalProgram::CompareChecksumFile()
{
	INT Ret = 0;
	IProgFileCheck *pProgFileCheck = NULL;
	CString strProgFilePath = m_ProgRecord.DestRecord.strProgFilePath;
	if (strProgFilePath.IsEmpty()) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "无需比对烧录档案校验值");
		goto __end;
	}

	if (true) {
		goto __end;
	}
	//烧录档案校验值比对目前没法进行比对。后续增加实际比对

	pProgFileCheck = GetProgFileCheckFactory(m_ProgRecord.DestRecord.FileChecksumType);
	if (!pProgFileCheck) {
		Ret = -1;
		m_pILog->PrintLog(LOGLEVEL_LOG, "创建%s校验值比较器失败", m_ProgRecord.DestRecord.FileChecksumType);
		goto __end;
	}

	pProgFileCheck->AttachILog(m_pILog);
	Ret = pProgFileCheck->CheckFile(strProgFilePath, m_ProgRecord.DestRecord.strProgFileChecksum);

__end:
	PutProgFileCheckFactory(pProgFileCheck);
	return Ret;
}


BYTE pKey[20] = { 0x1B, 0xF1, 0xE3, 0x70, 0x66, 0x80, 0x5A, 0x01, 0x9A, 0xDE, 0x1F, 0x23, 0xCD,0xA5 ,0x37 ,0x42 ,0xE4 ,0xC6, 0xCC, 0xB4 };
inline void Decrypt(BYTE* pData, BYTE* pKey, int nDataLen, int nKeyLen) {
	for (int i = 0; i < nDataLen; i++) {
		pData[i] = (byte)(((pData[i] ^ pKey[i % nKeyLen])) ^ (i & 0xFF) ^ (i % 2 == 0 ? i >> (i % 4) : i << (i % 3)));
	}
}

INT NormalProgram::CheckServerLic() {
	BlockTcpSocket Socket;
	BYTE sendData[20];
	BYTE recvData[1024];
	INT TryCnt = 5;
	INT Rtn = TRUE;
	char mByteBuffer[1024];
	DWORD outLen = 0;
	cJSON* RootBuild;
	CString strErrCode, strErrMsg;
	INT result = -1;

	while (TryCnt>0) {
		Rtn = Socket.Connect("127.0.0.1", 2020);///如果连接失败，可能服务器还没启动
		if (Rtn == TRUE) {
			break;
		}
		Sleep(1000);
		TryCnt--;
	}
	if (Rtn == FALSE) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "Connect MultiAprog Fail, ErrNo=%d", Socket.GetLastError());
		return -1;
	}

	memset((char *)sendData, 0, sizeof(sendData));
	snprintf((char *)sendData, sizeof(sendData) - 1, "GetInfo");
	Decrypt(sendData, pKey, sizeof(sendData), 20);
	int Ret = Socket.Send(sendData, sizeof(sendData));
	if (Ret != sizeof(sendData)) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "Send Failed, ErrCode = %d", Socket.GetLastError());
		Socket.Close();
		return -2;
	}
	//m_pILog->PrintLog(LOGLEVEL_LOG, "Receive Send Ret:%d", Ret);

	Ret = Socket.Receive(recvData, sizeof(recvData));
	if (Ret <= 0) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "Receive Failed, ErrCode = %d", Socket.GetLastError());
		Socket.Close();
		return -3;
	}
	Decrypt(recvData, pKey, Ret, 20);
	Socket.Close();

	memset(mByteBuffer, 0, sizeof(mByteBuffer));
	if (Utf8ToMByte((LPCTSTR)recvData + 4, mByteBuffer, Ret - 4, outLen) != 1) {
		AfxMessageBox("转换失败");
		return -4;
	}
	//m_pILog->PrintLog(LOGLEVEL_LOG, "Receive recvData:%s len:%d", mByteBuffer, Ret);
	RootBuild = cJSON_Parse((char *)(mByteBuffer));
	if (RootBuild == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "解析的信息不符合json格式");
		return -5;
	}
	result = -6;
	int funcRet = atoi(cJSON_GetObjectItem(RootBuild, "FuncRet")->valuestring);
	if (funcRet != 1) {
		strErrCode = cJSON_GetObjectItem(RootBuild, "ErrCode")->valuestring;
		strErrMsg = cJSON_GetObjectItem(RootBuild, "ErrMsg")->valuestring;
		m_pILog->PrintLog(LOGLEVEL_ERR, "查询失败 ErrCode:%s ErrMsg: %s ", strErrCode, strErrMsg);
		AfxMessageBox(strErrMsg);
		result = -7;
	}
	else {
		int TimeRemain = atoi(cJSON_GetObjectItem(RootBuild, "TimeRemain")->valuestring);
		CString AuthMode = cJSON_GetObjectItem(RootBuild, "AuthMode")->valuestring;
		if (!AuthMode.CompareNoCase("ALLOW")) {
			m_pILog->PrintLog(LOGLEVEL_LOG, "AuthMode：%s", AuthMode);
			result = 0;
		}
		else if (!AuthMode.CompareNoCase("TIME")) {
			m_pILog->PrintLog(LOGLEVEL_LOG, "查询结果：D%d-D%d", TimeRemain, m_pSetting->nCheckDay);
			if (TimeRemain >= m_pSetting->nCheckDay) {
				result = 0;
			}
			else if (TimeRemain > 0 && TimeRemain < m_pSetting->nCheckDay) {
				CString outMsg;
				outMsg.Format("证书到期不足%d天，请及时续期", m_pSetting->nCheckDay);
				AfxMessageBox(outMsg);
				result = 0;
			}
			else if (TimeRemain <= 0) {
				m_pILog->PrintLog(LOGLEVEL_ERR, "AuthMode：%s", AuthMode);
				AfxMessageBox("证书已到期，请及时续期");
				result = -8;
			}
		}
		else {
			CString outMsg;
			outMsg.Format("不支持此AuthMode：%s", AuthMode);
			AfxMessageBox(outMsg);
			result = -9;
		}
	}
	return result;
}


INT NormalProgram::LoadSNC()
{
	INT Ret = 0;
	CFile cFile;
	DWORD crc32, FileSize, InitBufSize = 0, ReuseCnt = 0;
	DWORD dwStartIndex = 0;
	DWORD dwTotalNum = 0;
	DWORD dwProgramIdx = 0;
	BYTE *pBuf = NULL;
	CSerial lSerial, lSerialSNModeInfo;
	BYTE  *pSNCFGInitBuf = NULL;

	CFileException ex;
	CString strFileName;
	CString strErr;
	CString strChipName, strCurSNGen;

	strFileName.Format("%s", m_strSNCPath);

	if (strFileName.IsEmpty() || (PathFileExists(strFileName) == FALSE)) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "请注意未使用SNC配置文件 ");
		return TRUE;
	}

	if (m_pSetting->nReInitIndex == 1) { //使用配置文件中的值
		m_pILog->PrintLog(LOGLEVEL_LOG, "SNC配置使用的是config中的自定义参数, nStartIndex=%d, nTotalNum=%d, nProgramIndex=%d, ",
			m_pSetting->nStartIndex, m_pSetting->nTotalNum, m_pSetting->nProgramIndex);
		m_StdMes.MesLoadSNC((LPSTR)(LPCSTR)strFileName, 1, m_pSetting->nStartIndex, m_pSetting->nTotalNum, m_pSetting->nProgramIndex);
		goto __end;
	}

	if (cFile.Open(strFileName, CFile::modeRead, &ex) == FALSE) {
		TCHAR szError[1024];
		ex.GetErrorMessage(szError, 1024);
		strErr.Format("打开SNC文件失败，路径=%s, 错误原因=%s", (LPSTR)(LPCSTR)strFileName, szError);
		m_pILog->PrintLog(LOGLEVEL_LOG, "%s", strErr);
		Ret = FALSE;
		goto __end;
	}

	FileSize = (DWORD)cFile.GetLength();
	pBuf = new BYTE[FileSize];
	if (pBuf == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "Memory alloc failed for reading snc file");
		Ret = FALSE; goto __end;
	}

	if (cFile.Read(pBuf, FileSize) != FileSize) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "Read SNC File failed");
		Ret = FALSE; goto __end;
	}
	lSerial.SerialInBuff(pBuf, FileSize);

	lSerial >> crc32;
	lSerial >> dwStartIndex >> dwTotalNum;
	lSerial >> strChipName >> strCurSNGen >> InitBufSize;

	pSNCFGInitBuf = new BYTE[InitBufSize];
	if (pSNCFGInitBuf == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "Memory alloc failed for reading snc file");
		Ret = FALSE; goto __end;
	}
	lSerial.SerialOutBuff(pSNCFGInitBuf, InitBufSize);
	lSerial >> crc32;
	lSerial >> dwProgramIdx >> ReuseCnt;
	m_StdMes.MesLoadSNC((LPSTR)(LPCSTR)strFileName, 0, dwStartIndex, dwTotalNum, dwProgramIdx);

__end:

	if (pBuf) {
		delete[] pBuf;
		pBuf = NULL;
	}
	return Ret;
}


bool NormalProgram::IsAutoMode() {
	bool bRet = false;
	if (m_pSetting->strAutoMode.CompareNoCase("Auto") == 0) {
		bRet = true;
	}
	return bRet;
}

INT NormalProgram::ConstructAutoTaskDataWithCmd3(CString&strTaskData)
{
	CString strItems[7];
	CString strFilePath;
	strFilePath.Format("%s", m_ProgRecord.DestRecord.strAutoTaskPath);
	strTaskData.Format("3,%d,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s,%s,%s,%s,%s,%s,%s",
		m_ProgRecord.DestRecord.ExpertIDNum,
		strFilePath,
		m_AutoPosSetting.bProvideTrayPosEn,
		m_AutoPosSetting.ProvideTrayXPos,
		m_AutoPosSetting.ProvideTrayYPos,
		m_AutoPosSetting.bOKTrayPosEn,
		m_AutoPosSetting.OKTrayXPos,
		m_AutoPosSetting.OKTrayYPos,
		m_AutoPosSetting.bNGTrayPosEn,
		m_AutoPosSetting.NGTrayXPos,
		m_AutoPosSetting.NGTrayYPos,
		m_AutoPosSetting.ReelPos,
		strItems[0], strItems[1], strItems[2], strItems[3], strItems[4], strItems[5], strItems[6]);
	return 0;
}

INT NormalProgram::ConfigAutoTaskData()
{
	INT Ret = 0;
	BOOL  RtnCall = FALSE;
	if (!IsAutoMode()) {
		goto __end;
	}
	if (m_ProgRecord.DestRecord.strAutoTaskPath != "") {
		CString strTaskData;
		m_pILog->PrintLog(LOGLEVEL_LOG, "使用CMD%d设置自动化设备任务数据...", m_pSetting->nAutoTaskLoadCmd);

		Ret = ConstructAutoTaskDataWithCmd3(strTaskData);
		RtnCall = m_StdMes.MesLoadTaskDataToAutomatic(1, strTaskData.GetBuffer());

		if (RtnCall == TRUE) {
			m_pILog->PrintLog(LOGLEVEL_LOG, "自动化设备任务数据设置成功");
		}
		else {
			m_pILog->PrintLog(LOGLEVEL_LOG, "自动化任务:%s", strTaskData);
			m_pILog->PrintLog(LOGLEVEL_LOG, "自动化设备任务数据设置失败,Ret=%d，请确认自动化软件是否已经启动", RtnCall);
			Ret = -1; goto __end;
		}
	}
	else {
		m_pILog->PrintLog(0, "无需配置自动化设备任务数据");
	}
__end:
	return Ret;
}

void NormalProgram::GetCmd4RetInfo()
{
	BOOL bFuncRet = FALSE;
	CString strResponse;
	cJSON* pRootParser;
	cJSON* pItemObj;
	cJSON* pNode;

	CString strSendCmdData;
	strSendCmdData.Format("4,%s", "{\"Method\":\"GetLotData\"}");

	char respBuffer[4096];
	memset(respBuffer, 0, sizeof(respBuffer));
	bFuncRet = m_StdMes.MesSendCmdToAutomatic(4, strSendCmdData.GetBuffer(), respBuffer, sizeof(respBuffer));
	strSendCmdData.ReleaseBuffer();
	if (bFuncRet != TRUE) {
		m_pILog->PrintLog(0, "执行CMD4失败！");
		goto __end;
	}
	strResponse.Format("%s", respBuffer);
	pRootParser = cJSON_Parse(strResponse.GetBuffer());
	strResponse.ReleaseBuffer();
	if (pRootParser == NULL) {
		m_pILog->PrintLog(0, "CMD4返回的数据格式不符合json要求！%s", strResponse);
		goto __end;
	}

	m_pILog->PrintLog(0, "执行CMD4返回: %s", strResponse);
	pItemObj = cJSON_GetObjectItem(pRootParser, "ErrCode");
	if (pItemObj == NULL) {
		m_pILog->PrintLog(0, "返回的json数据格式不符合要求ErrCode！");
		goto __end;
	}

	if (pItemObj->valueint == 0) {
		pNode = cJSON_GetObjectItem(pRootParser, "LotData");
		pItemObj = cJSON_GetObjectItem(pNode, "TimeRun");
		m_ProgramResult.DevTimeRun.Format("%s", pItemObj->valuestring);
		pItemObj = cJSON_GetObjectItem(pNode, "TimeSuspend");
		m_ProgramResult.DevTimeStop.Format("%s", pItemObj->valuestring);
		pItemObj = cJSON_GetObjectItem(pNode, "UPH");
		m_ProgramResult.AutoUPH.Format("%s", pItemObj->valuestring);
		pItemObj = cJSON_GetObjectItem(pNode, "TotalCnt");
		m_ProgramResult.AutoTotalCnt = pItemObj->valueint;
		pItemObj = cJSON_GetObjectItem(pNode, "PassCnt");
		m_ProgramResult.AutoPassCnt = pItemObj->valueint;
		pItemObj = cJSON_GetObjectItem(pNode, "FailCnt");
		m_ProgramResult.AutoFailCnt = pItemObj->valueint;
		pItemObj = cJSON_GetObjectItem(pNode, "RemoveCnt");
		m_ProgramResult.AutoRemoveCnt = pItemObj->valueint;
		m_bGetCmd4Success = TRUE;

		PostMessage(m_MainHwd, MSG_UPDATE_AUTO_COUNT, (WPARAM)(&m_ProgramResult), (LPARAM)0);
	}

__end:

	return;
}

bool NormalProgram::IsMesMode()
{
	bool bRet = false;
	if (m_pSetting->strMesWordMode.CompareNoCase("Enable") == 0) {
		bRet = true;
	}
	return bRet;
}

CString NormalProgram::GetAbsReportSavePath()
{
	CString strAbsPath;
	if (m_pSetting->strReportFolder.GetAt(0) == '.') {///是相对路径
		CString FolderName = ::GetFolderNameFromRelative(m_pSetting->strReportFolder);
		strAbsPath.Format("%s\\%s", GetCurrentPath(), FolderName);
	}
	else {
		strAbsPath = m_pSetting->strReportFolder;
	}
	return strAbsPath;
}

INT NormalProgram::SaveReprot2Local(const char*strJson, INT Size)
{
	INT Ret = 0;
	CString ReportPath;
	ReportPath.Format("%s\\Report_%s_%s.txt", GetAbsReportSavePath(), m_ProgRecord.DestRecord.strWorkOrder, GetCurTime('_'));/// m_ProgramInfo.strProjReportPath;
	CFile File;

	if (File.Open(ReportPath, CFile::modeCreate | CFile::modeWrite, NULL) == FALSE) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "保存本地报告文件失败:创建文件失败 %s", ReportPath);
		Ret = -1; goto __end;
	}
	else {
		File.Write(strJson, Size);
		File.Close();
		m_pILog->PrintLog(LOGLEVEL_LOG, "保存报告成功,报告路径:%s", ReportPath);
	}

__end:
	return Ret;
}

INT NormalProgram::SendReport2Mes()
{
	BOOL RtnCall = TRUE;
	INT Ret = 0;
	CHAR *pTmpBuf = NULL;
	INT Size = 1024 * 1024 * 1; ///
	INT SizeNeed = 0;
	CString strBase;
	CString strDetail;

	CString strPa1;
	CString strPa2;

	Json::Value Root, OverallStatistics, SiteStatistics;
	Json::Value BaseInfo;
	Json::Reader JReader;
	Json::StyledWriter JWriter;
	std::string strJsonReport;

	pTmpBuf = new CHAR[Size];///
	if (!pTmpBuf) {
		Ret = -1; goto __end;
	}
	m_pILog->PrintLog(LOGLEVEL_LOG, "获取生产报告...");
	memset(pTmpBuf, 0, Size);

	RtnCall = m_StdMes.MesGetReport_Json(pTmpBuf, Size, &SizeNeed);
	if (RtnCall == FALSE) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "获取报告失败");
		Ret = -1; goto __end;
	}

	RtnCall = JReader.parse(pTmpBuf, pTmpBuf + SizeNeed, Root);
	if (RtnCall == FALSE) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "解析生产报告发生错误!");
		Ret = -1; goto __end;
	}

	BaseInfo = Root["BasicInformation"];
	//增加对象
	BaseInfo["WorkOrder"] = Json::Value(m_strWorkOrder);
	BaseInfo["MaterialNo"] = Json::Value(m_strComponentName);
	//BaseInfo["ChipNo"] = Json::Value(m_ProgRecord.DestRecord.strChipName);
	//BaseInfo["SoftVersionNo"] = Json::Value(m_ProgRecord.DestRecord.strSoftVer);
	strJsonReport = JWriter.write(BaseInfo);
	strBase.Format("%s", strJsonReport.c_str());

	Root["WorkOrder"] = Json::Value(m_strWorkOrder);
	Root["MaterialNo"] = Json::Value(m_strComponentName);
	//Root["ChipNo"] = Json::Value(m_ProgRecord.DestRecord.strChipName);
	//Root["SoftVersionNo"] = Json::Value(m_ProgRecord.DestRecord.strSoftVer);

	OverallStatistics = Root["OverallStatistics"];
	strJsonReport = JWriter.write(OverallStatistics);
	strDetail.Format("%s", strJsonReport.c_str());

	strJsonReport = JWriter.write(Root);

	SaveReprot2Local(strJsonReport.c_str(), strJsonReport.length());

	if (IsMesMode()) {
		//m_pILog->PrintLog(0, "正在推送报告给Mes中...");
		//m_pILog->PrintLog(LOGLEVEL_LOG, "正在推送报告给Mes中...");
		if (IsAutoMode()) {
			SYSTEMTIME st;
			CString strYear;
			CString strDetail;
			CString strLotEndTime;
			GetLocalTime(&st);
			strYear.Format("%4d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
			strDetail.Format("%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
			strLotEndTime.Format("%s %s", strYear, strDetail);
			if (CommitTaskInfo(strLotEndTime, m_ProgramResult.DevTimeRun, m_ProgramResult.DevTimeStop) != 0) {
				m_pILog->PrintLog(0, "提交任务数据信息给Mes失败...");
			}
			else {
				m_pILog->PrintLog(0, "提交任务数据信息给Mes成功...");
			}
		}
	}

__end:
	if (pTmpBuf) {
		delete[] pTmpBuf;
	}
	return Ret;
}


BOOL NormalProgram::IsChipNameMatched(CString SoftChipName, CString RealChipName) {
	if (RealChipName.GetLength() != RealChipName.GetLength()) {
		return FALSE;
	}
	for (int i = 0; i < RealChipName.GetLength(); i++) {
		if (SoftChipName.GetAt(i) == '*' || RealChipName.GetAt(i) == SoftChipName.GetAt(i)) {
			continue;
		}
		else {
			return FALSE;
		}
	}

	return TRUE;
}

CString NormalProgram::GetChecksumExtForChip(CString ChipName, CString ChecksumExt, std::map<CString, CString> mapChipList) {
	CString Ret = _T("");
	std::map<CString, CString>::iterator it;

	m_pILog->PrintLog(LOGLEVEL_LOG, "GetChecksumExt ChipName: %s ChecksumExt: %s", ChipName, ChecksumExt);
	for (it = mapChipList.begin(); it != mapChipList.end(); ++it) {
		CString strFirst = it->first;
		CString extname = it->second;
		if (IsChipNameMatched(ChipName, strFirst)) {
			INT indexMaxN = ChecksumExt.GetLength();
			INT indexExtN = ChecksumExt.Find(extname);
			m_pILog->PrintLog(LOGLEVEL_LOG, "GetChecksumExt indexMaxN: %d indexExtN: %d", indexMaxN, indexExtN);
			if (indexExtN > 0) {
				CString newStr = ChecksumExt.Mid(indexExtN, indexMaxN - indexExtN);
				INT indexMN = newStr.Find(":") + 1;
				INT indexRN = newStr.Find("]");
				if (indexRN > indexMN) {
					Ret = newStr.Mid(indexMN, indexRN - indexMN);
					Ret = Ret.Trim();
					m_pILog->PrintLog(LOGLEVEL_LOG, "GetChecksumExt [%s]", Ret);
				}
			}
		}
	}
	return Ret;
}

INT NormalProgram::CommitTaskInfo(CString timeEnd, CString timeRun, CString timeStop)
{
	if (!IsMesMode()) {
		return 0;
	}
	INT Ret = -1;
	CMesInterface &MesInterface = CMesInterface::getInstance();
	Ret = MesInterface.CommitTaskInfo2Mes(m_ProgramResult.LotStartTime, timeEnd, timeRun, timeStop);
	
	return Ret;

}
bool NormalProgram::IsAutoCreateProject()
{
	return false; //固定为非自动合成

	bool bRet = false;
	if (m_pSetting->strProjectMode.CompareNoCase("AutoCreate") == 0) {
		bRet = true;
	}
	return bRet;
}


BOOL NormalProgram::IsTaskDoing() {
	return m_bTaskDoing;
}

int NormalProgram::DoTask()
{
	INT Ret = 0;
	BOOL bRtn;
	INT PreTotal = -1;
	INT StopQuantity = 0;
	CString strMsg;
	INT Total = 0, Pass = 0, Fail = 0;
	m_bGetCmd4Success = FALSE;
	m_bGetMissionResult = FALSE;
	m_bRecvAutomaticLotEnd = FALSE;
	m_ProgramResult.Reset();

	SYSTEMTIME st;
	CString strYear;
	CString strDetail;
	CString strTime;

	///启动服务器
	INT startRet = StartService();
	if (startRet != 0) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "===>StartService Fail");
		goto __end;
	}
	//memset((char*)&m_YieldSites, 0, sizeof(tYieldSites));
	m_YieldSites.TotalCnt = 0;
	m_YieldSites.PassCnt = 0;
	m_YieldSites.FailCnt = 0;

	StopQuantity = m_ProgRecord.DestRecord.ExpertIDNum;

	if (IsAutoCreateProject()) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "准备进行自动合成工程文件...");
		Ret = CompareChecksumFile();
		if (Ret != 0) {
			m_pILog->PrintLog(LOGLEVEL_LOG, "烧录档案校验值比对错误批量生产终止");
			goto __end;
		}

		///创建工程文件
		Ret = CreateProjectByTemplate();

		if (Ret != 0) {
			goto __end;
		}

		Ret = CompareChecksumProj();
		if (Ret != 0) {
			m_pILog->PrintLog(LOGLEVEL_LOG, "批量生产终止");
			goto __end;
		}
	}
	else {
		m_pILog->PrintLog(LOGLEVEL_LOG, "===>直接使用外部工程文件进行批量生产");
	}
	///加载工程
	Ret = LoadProject();
	if (Ret != 0) {
		goto __end;
	}

	//查询apr信息
	Ret = GetProjectInfo();
	if (Ret != 0) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "查询apr信息终止");
		goto __end;
	}

	if (!IsAutoCreateProject()) {///直接使用工程
								 ///不比对校验值
		Ret = CompareChecksumProj();
		if (Ret != 0) {
			m_pILog->PrintLog(LOGLEVEL_LOG, "批量生产终止");
			goto __end;
		}
	}

	//加载SNC配置文件
	//LoadSNC();
	if (UploadProgramerInfo2Mes() != 0) {
		m_pILog->PrintLog(0, "上传座子信息给Mes系统失败");
	}
	else {
		m_pILog->PrintLog(0, "上传座子信息给Mes系统成功");
	}

	m_pILog->PrintLog(LOGLEVEL_LOG, "启动工程任务...");
	bRtn = m_StdMes.MesStartProject();
	if (bRtn != TRUE) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "启动工程任务失败");
		Ret = -1; goto __end;
	}

	//查询lic
	if (!m_pSetting->nLocalServer){
		Ret = CheckServerLic();
		m_pILog->PrintLog(LOGLEVEL_LOG, "查询Ret: %d", Ret);
		if (Ret != 0) {
			Ret = -1; goto __end;
		}
	}


	Ret = ConfigAutoTaskData();
	if (Ret != 0) {
		Ret = -1; goto __end;
	}

	if (!m_bFirstRunFlag) {
		m_DevBegainTime = CTime::GetCurrentTime();
		m_bFirstRunFlag = true;
	}

	GetLocalTime(&st);
	strYear.Format("%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
	strDetail.Format("%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
	strTime.Format("%s %s", strYear, strDetail);
	m_ProgramResult.LotStartTime.Format("%s", strTime);

	while (1) {
		Sleep(200); ///两次取状态的间隔为200ms
		if (m_bQuit) {///客户需要退出
			m_pILog->PrintLog(LOGLEVEL_LOG, "操作员主动取消批量操作");
			break;
		}
		if (m_bGetMissionResult) {
			m_pILog->PrintLog(LOGLEVEL_LOG, "收到自动机通知MissionResult:%d LotEnd:%d", m_bGetMissionResult, m_bRecvAutomaticLotEnd);
			break;
		}
		m_YieldSitesMutex.Lock();
		Total = m_YieldSites.TotalCnt;
		Pass = m_YieldSites.PassCnt;
		Fail = m_YieldSites.FailCnt;
		m_YieldSitesMutex.Unlock();
		///退出循环的条件应该自己判断，一般情况下是Total已经达到需要的总数
		if (PreTotal != Total) {///有可能两次取得的结果是一样的，不需要重复打印
			strMsg.Format("当前生产总个数:%-5d, 成功个数:%-5d, 失败个数:%-5d", Total, Pass, Fail);
			m_pILog->PrintLog(LOGLEVEL_LOG, strMsg.GetBuffer());
			strMsg.ReleaseBuffer();
			PreTotal = Total;
		}

		if (Pass >= StopQuantity) {///只循环到Pass为StopQuantity截止。///打印最后一次
			strMsg.Format("批量任务达成，生产总个数:%-5d, 成功个数:%-5d, 失败个数:%-5d", Total, Pass, Fail);
			m_pILog->PrintLog(LOGLEVEL_LOG, strMsg.GetBuffer());
			strMsg.ReleaseBuffer();
			break;
		}
	}

	UINT tryTimes = 1;
	while (TRUE) {
		if (m_bGetMissionResult || m_bRecvAutomaticLotEnd) {
			m_pILog->PrintLog(LOGLEVEL_LOG, "收到自动机通知MissionResult:%d LotEnd:%d", m_bGetMissionResult, m_bRecvAutomaticLotEnd);
			break;
		}
		if (tryTimes++ % 30 == 0) {
			m_pILog->PrintLog(LOGLEVEL_LOG, "等待自动机结束批量通知，请首先在自动化软件“批量管理”里点击“执行”结束批量。");
		}
		Sleep(1000);
	}

	m_pILog->PrintLog(LOGLEVEL_LOG, "开始结束工程任务...");
	m_StdMes.MesStopProject();
	m_pILog->PrintLog(LOGLEVEL_LOG, "工程任务结束");

	//从自动化CMD4取任务数据
	m_pILog->PrintLog(LOGLEVEL_LOG, "开始发送CMD4从自动机取任务数据");
	GetCmd4RetInfo();
	m_pILog->PrintLog(LOGLEVEL_LOG, "结束发送CMD4从自动机取任务数据");

	PostMessage(m_MainHwd, MSG_PROGRAM_END, 0, 0);
	Ret = SendReport2Mes();
	 
__end:
	m_pILog->PrintLog(LOGLEVEL_LOG, "正在生成报告并关闭服务器，请稍候...");
	//Sleep(550);//延迟下关闭服务器
	//if (m_StdMes.MesCheckService() == TRUE) {
	m_StdMes.MesStopService();
	//}
	m_pILog->PrintLog(LOGLEVEL_LOG, "关闭服务器完成");
	m_bTaskDoing = FALSE;

	PostMessage(m_MainHwd, MSG_DO_PROGRAM_FINISH, 0, 1000);

	return Ret;
}

INT NormalProgram::CommitAlarmInfo(CString alarmCode, CString alarmMsg, CString alarmcStartTime, CString alarmKillTime, int alarmFlag)
{
	if (!IsMesMode()) {
		return 0;
	}
	INT Ret = -1;
	CMesInterface &MesInterface = CMesInterface::getInstance();
	Ret = MesInterface.CommitAlarmInfo2Mes(alarmCode, alarmMsg, alarmcStartTime, alarmKillTime, alarmFlag);
	return Ret;
}

INT NormalProgram::UploadProgramerInfo2Mes()
{
	INT Ret = -1;
	BOOL RtnCall = TRUE;
	CHAR *pTmpBuf = NULL;
	INT Size = 1024 * 1024; ///
	CString strProgramerInfo;
	int nRetryCnt = 8;

	pTmpBuf = new CHAR[Size];///
	if (!pTmpBuf) {
		Ret = -1; goto __end;
	}
	m_pILog->PrintLog(LOGLEVEL_LOG, "正在获取座子...");
	memset(pTmpBuf, 0, Size);

	RtnCall = m_StdMes.MesGetProgrammerInfo_Json(pTmpBuf, Size);
	if (RtnCall == FALSE) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "获取座子信息失败");
		Ret = -1; goto __end;
	}

	strProgramerInfo.Format("%s", pTmpBuf);
	if (IsMesMode()) {
		CMesInterface &MesInterface = CMesInterface::getInstance();
		Ret = MesInterface.CommitProgramerInfo2Mes(strProgramerInfo);
	}
	else {
		Ret = 0;
	}
__end:
	if (pTmpBuf != NULL) {
		delete pTmpBuf;
		pTmpBuf = NULL;
	}
	return Ret;
}

INT NormalProgram::StartProgram(HWND MainHwd, CProgRecord ProgRecord)
{
	m_bTaskDoing = TRUE;
	m_MainHwd = MainHwd;
	m_ProgRecord = ProgRecord;

	m_nIsProdcingCnt = 0;

	m_bQuit = FALSE;

	//开启后所有显示计数清零
	//memset((char*)&m_YieldSites, 0, sizeof(tYieldSites));
	m_YieldSites.TotalCnt = 0;
	m_YieldSites.PassCnt = 0;
	m_YieldSites.FailCnt = 0;

	StartDoProgramTaskWork();

	return 0;

}
