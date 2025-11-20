#include "stdafx.h"
#include "ModuleProgram.h"
#include "WorkThread.h"
#include "json/json.h"
#include "Setting.h"
#include "ComTool.h"
#include "ProgRecord.h"
#include "cJSON.h"
#include "ComFunc.h"

INT m_gSktEnArray[MAX_SKTCNT];
ModuleProgram::ModuleProgram(CString ModuleName, CSetting *pSetting, ILog *pILog):
	BaseModuleProgram(ModuleName, pSetting, pILog)
{
	m_nQueryComFlag = 0;
	m_strModuleName = ModuleName;
	m_pFunClearUIToastDlg = NULL;
	m_bExitApp = FALSE;
}

int ModuleProgram::Init() 
{
	int Ret = 0;
	Ret = GetModuleJson();
	return Ret;
}


ModuleProgram::~ModuleProgram()
{
	m_bExitApp = TRUE;
	ReleaseMem();
	m_BurnStatusMap.clear();
}

static int _stdcall MsgHandle(void*Para, char*Msg, char*MsgData)
{
	int Ret = 0;
	ModuleProgram *pProgram = (ModuleProgram*)Para;
	if (pProgram) {
		pProgram->HandleMsg(Msg, MsgData);
	}
	return Ret;
}

int ModuleProgram::HandleMsg(char*Msg, char*MsgData)
{
	int Ret = 0;
	CString strMsg;

	//push back msg to message list, wait ParseTask to parse.
	m_ProgramMsgListMutex.Lock();
	tStdMesMessage stdMesMsg;
	stdMesMsg.msg = Msg;
	stdMesMsg.json = MsgData;
	m_RecvProgramMsgList.push_back(stdMesMsg);
	m_ProgramMsgListMutex.Unlock();

	return Ret;
}



INT ModuleProgram::ThreadMsgHandler(MSG msg, void *Para)
{
	INT Ret = 0;
	if (msg.message == MSG_LIST_HANDLE) {
		Ret = DoHandleMsgList();
	}
	return -1;///为了让线程处理完消息之后自动退出，返回-1不在进行消息的获取
}

INT ModuleProgram::DoHandleMsgList()
{
	while (!m_bExitApp) {
		tStdMesMessage stdMesMsg;
		bool getProgramMsg = false;
		m_ProgramMsgListMutex.Lock();
		if (!m_RecvProgramMsgList.empty()) {
			stdMesMsg = m_RecvProgramMsgList.front();
			m_RecvProgramMsgList.pop_front();
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
		else if (_stricmp(Msg, "SitesEnvInitRet") == 0) {
			m_pILog->PrintLog(LOGLEVEL_LOG, "收到消息回调SitesEnvInitRet");
			CString strRecvJson;
			std::string strJson(MsgData);
			Json::Value Root;
			Json::Reader JReader;
			if (JReader.parse(strJson, Root) == false) {
				m_pILog->PrintLog(LOGLEVEL_LOG, "解析json失败");
			}
			else {
				m_SiteInitRetMutex.Lock();
				int nCnt = Root["SiteList"].size();
				for (int i = 0; i < nCnt; i++) {
					Json::Value OneSite;
					CString strDevSN;

					OneSite = Root["SiteList"][i];
					strDevSN.Format("%s", OneSite["DevSN"].asCString());

					if (strDevSN.IsEmpty()) {
						continue;
					}

					if (m_BurnStatusMap.find(strDevSN) != m_BurnStatusMap.end()) {
						m_BurnStatusMap[strDevSN].SiteEnvInit = OneSite["EnvInitRet"].asInt();
					}
				}
				m_SiteInitRetMutex.Unlock();

			}
		}
		/*else if (_stricmp(Msg, "NotifyProgram") == 0) {
			CString strTemp;
			strTemp.Format("%s", MsgData);

			m_pILog->PrintLog(LOGLEVEL_LOG, "收到消息回调NotifyProgram,%s", strTemp);
			CString strRecvJson;
			std::string strJson(MsgData);
			Json::Value Root;
			Json::Reader JReader;
			if (JReader.parse(strJson, Root) == false) {
				m_pILog->PrintLog(LOGLEVEL_LOG, "解析json失败");
			}
			else {

				CString strDevSN;
				strDevSN.Format("%s", Root["DevSN"].asCString());
				UINT ChipEnAuto = Root["ChipEnAuto"].asUInt();

				if (m_BurnStatusMap.find(strDevSN) != m_BurnStatusMap.end()) {
					m_BurnStatusMap[strDevSN].pMutexForChipReady->Lock();
					m_BurnStatusMap[strDevSN].nPhase = ePhaseReady;
					m_BurnStatusMap[strDevSN].pMutexForChipReady->Unlock();
				}
			}
		}*/
		else if (_stricmp(Msg, "PoweronOK") == 0) {
			CString strTemp;
			strTemp.Format("%s", MsgData);

			m_pILog->PrintLog(LOGLEVEL_LOG, "收到消息回调PoweronOK,%s", strTemp);

			CString strRecvJson;
			std::string strJson(MsgData);
			Json::Value Root;
			Json::Reader JReader;
			if (JReader.parse(strJson, Root) == false) {
				m_pILog->PrintLog(LOGLEVEL_LOG, "解析json失败");
			}
			else {

				CString strDevSN;
				strDevSN.Format("%s", Root["SiteSN"].asCString());

				if (m_BurnStatusMap.find(strDevSN) != m_BurnStatusMap.end()) {
					m_BurnStatusMap[strDevSN].pMutexForChipReady->Lock();
					m_BurnStatusMap[strDevSN].nPhase = ePhaseReady;
					m_BurnStatusMap[strDevSN].pMutexForChipReady->Unlock();
				}
			}
		}
	}

	return 0;
}

//启动服务查询com口
INT ModuleProgram::StartQueryCom(HWND MainHwd, CString strProjPath) {

	INT Ret = SetServerPath();
	if (Ret < 0) {
		return 0;
	}
	m_ProgRecord.DestRecord.strProjPath = strProjPath;
	m_MainHwd = MainHwd;

	CMsgHandler<ModuleProgram, ModuleProgram> MsgHandler = MakeMsgHandler(this, &ModuleProgram::QueryComThreadProc);
	m_QueryComThread.SetMsgHandle(MsgHandler);
	m_QueryComThread.CreateThread();
	m_QueryComThread.PostMsg(MSG_START_QUERYCOM, 0, 0);
	return 0;
}

INT ModuleProgram::QueryComThreadProc(MSG msg, void *Para) {
	INT Ret = 0;
	ModuleProgram *pModuleProgram = (ModuleProgram*)Para;

	if (msg.message == MSG_START_QUERYCOM) {
		Ret = DoStartupACServer();
		Ret = StartupQueryCom();
		if (m_bFinishStartupACServer) {
			m_ExternBurnApi.MesStopService();
			m_bConfigCom = TRUE;
		}
		PostMessage(m_MainHwd, MSG_QUERY_COM_ALL_FINISHED, (WPARAM)(0), (LPARAM)0);

	}

	return -1;
}


INT ModuleProgram::StartupQueryCom()
{
	int nRtn = -1;

	Json::Value Root;
	Json::Reader Reader;
	std::string strJson;
	CString strRaw;
	int nSiteList = 0;

	int nWaitTime = 8;//m_pSetting->nAutoSearChChannelTime;

	int nSize = 1024 * 500;
	char* pOutJson = new char[nSize];

	//m_nQueryComFlag = 1;
	m_nPowerOffForQueryCom = 0;

	GetAllSitesInitResult();

	INT Ret = LoadProject();
	if (Ret != 0) {
		return Ret;
		//goto __end;
	}


	if (pOutJson == NULL) {
		goto __end;
	}
	memset(pOutJson, 0, nSize);

	m_ExternBurnApi.MesQueryCom(pOutJson, nSize, m_ModuleInfo.strProgramPID_VID.GetBuffer(), nWaitTime);
	strRaw.Format("%s", pOutJson);

	strJson.assign((LPSTR)(LPCTSTR)strRaw, strRaw.GetLength());
	if (Reader.parse(strJson, Root) == false) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "Parse json failed,原始json=%s", strRaw);
		goto __end;
	}

	nSiteList = Root["SiteListInfo"].size();
	if (nSiteList <= 0) {
		goto __end;
	}

	for (int i = 0; i < nSiteList; i++) {
		CString strValue;
		std::map<INT, INT>* SktComMap;
		Json::Value OneSite;
		OneSite = Root["SiteListInfo"][i];

		CString strDevSN;
		strDevSN.Format("%s", OneSite["SiteSN"].asCString());

		if (strDevSN.IsEmpty()) {
			continue;
		}

		for (UINT j = 0; j < OneSite["SktInfo"].size(); j++) {
			if (m_BurnStatusMap.find(strDevSN) != m_BurnStatusMap.end()) {
				m_BurnStatusMap[strDevSN].nComPort[j] = OneSite["SktInfo"][j]["ComPort"].asInt();
				m_BurnStatusMap[strDevSN].m_SktComMap[OneSite["SktInfo"][j]["Index"].asInt() - 1] = OneSite["SktInfo"][j]["ComPort"].asInt();
			}
			
		}

		strValue.Format("%d", m_BurnStatusMap[strDevSN].nComPort[0]);
		int nConnectIdx = -1;
		if (GetAttachIdxFromStrDev(strDevSN, nConnectIdx) == TRUE && nConnectIdx < SITE_COUNT) {
			SktComMap = new std::map<INT, INT>;
			*SktComMap = m_BurnStatusMap[strDevSN].m_SktComMap;
			//PostMessage(m_MainHwd, MSG_QUERY_COM_SITE_FINISHED, (WPARAM)(nConnectIdx), (LPARAM)m_BurnStatusMap[strDevSN].nComPort[0]);
			PostMessage(m_MainHwd, MSG_QUERY_COM_SITE_FINISHED, (WPARAM)SktComMap, (LPARAM)(nConnectIdx));
		}

	}

	m_pILog->PrintLog(LOGLEVEL_LOG, "接口返回的原始信息:%s", strRaw);
	m_nQueryComFlag = 1;


__end:
	m_ExternBurnApi.MesStopProject();
	if (pOutJson != NULL) {
		delete[] pOutJson;
		pOutJson = NULL;
	}

	return nRtn;
}

BOOL ModuleProgram::GetAttachIdxFromStrDev(CString strDev, int& nConnectIdx)
{
	BOOL Rtn = FALSE;
	std::map<CString, tBurnStatus>::iterator it;
	if (strDev.IsEmpty()) {
		goto __end;
	}

	for (it = m_BurnStatusMap.begin(); it != m_BurnStatusMap.end(); it++) {
		if (strDev.CompareNoCase(it->first) == 0) {
			nConnectIdx = m_BurnStatusMap[strDev].nAttachIdx;
			Rtn = TRUE;
			break;
		}
	}
__end:
	return Rtn;
}

//初始化结果，一台烧录器起4个线程
BOOL ModuleProgram::GetAllSitesInitResult()
{
	BOOL Rtn = FALSE;
	char* pData = NULL;
	int nSize = 1024 * 100;
	Json::Value Root;
	Json::Reader Reader;
	std::string strJson;

	m_pILog->PrintLog(LOGLEVEL_LOG, "==>GetAllSitesInitResult start");

	pData = new char[nSize];
	if (pData == NULL) {
		goto __end;
	}
	memset(pData, 0, nSize);
	if (m_ExternBurnApi.MesGetAllSitesInitResult(pData, nSize) != TRUE) {
		goto __end;
	}
	m_strAllSiteInitResult.Format("%s", pData);

	m_pILog->PrintLog(LOGLEVEL_LOG, "==>Parse json start");
	//将json转为结构体便于提取
	strJson.assign((LPSTR)(LPCTSTR)m_strAllSiteInitResult, m_strAllSiteInitResult.GetLength());
	if (Reader.parse(strJson, Root) == false) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "Parse json failed");
		goto __end;
	}
	m_uSlotNum = Root["SlotNum"].asUInt();
	m_nConnectSiteCnt = Root["SiteInfo"].size();
	if (m_nConnectSiteCnt <= 0) {
		goto __end;
	}

	////////////////////////
	m_pILog->PrintLog(LOGLEVEL_LOG, "==>初始化信息");
	ReleaseMem();
	m_BurnStatusMap.clear();
	///////////////////////
	for (int i = 0; i < m_nConnectSiteCnt; i++) {
		CString strDevSN;
		CString strDevAlias;
		CString strShow;
		Json::Value ItemSite;

		ItemSite = Root["SiteInfo"][i];
		strDevSN.Format("%s", ItemSite["SiteSN"].asCString());
		if (strDevSN.IsEmpty()) {
			continue;
		}
		tBurnStatus ItemStatus;
		std::pair<CString, tBurnStatus> kvPair;

		ItemStatus.SiteEnvInit = ItemSite["EnvInit"].asInt();
		ItemStatus.SiteIdxAuto = ItemSite["IdxAuto"].asInt();
		ItemStatus.AdapterEn = ItemSite["AdapterEn"].asUInt();
		ItemStatus.AdapterEnPut = ItemSite["AdapterEnPut"].asUInt();
		ItemStatus.nAttachIdx = ItemSite["AttachConnectIdx"].asInt();
		//////////////////////////////
		ItemStatus.nPhase = ePhaseInit;
		////////////////////
		CMutex* pMutex = new CMutex;
		ItemStatus.pMutexForChipReady = pMutex;

		for (int j = 0; j < EACH_SITE_CHIP_NUM; j++){//一个烧录器先起4个线程
			CWorkThread* ItemWokThread = new CWorkThread;
			CMsgHandler<ModuleProgram, ModuleProgram> MsgHandler = MakeMsgHandler(this, &ModuleProgram::GetExternProgramResultThreadProc);
			ItemWokThread->SetMsgHandle(MsgHandler);
			ItemWokThread->CreateThread();

			ItemStatus.vBurnWokThread.push_back(ItemWokThread);
		}


		kvPair.first = strDevSN;
		kvPair.second = ItemStatus;

		m_BurnStatusMap.insert(kvPair);

		strDevAlias.Format("%s-%s", ItemSite["SiteAlias"].asCString(), ItemSite["SiteSN"].asCString());
		strShow.Format("%s", strDevAlias);
		if (ItemStatus.nAttachIdx >= 0 && ItemStatus.nAttachIdx < SITE_COUNT) {
			CString *pValue = new CString(strDevAlias);
			PostMessage(m_MainHwd, MSG_UPDATE_SITE_NAME, (WPARAM)(ItemStatus.nAttachIdx), (LPARAM)pValue);
		}
		else {
			m_pILog->PrintLog(LOGLEVEL_ERR, "当前[%s]AttachIdx=%d有误,超过了当前连接的数量", strDevSN, ItemStatus.nAttachIdx);
		}

	}

	Rtn = TRUE;
__end:

	m_pILog->PrintLog(LOGLEVEL_LOG, "==>GetAllSitesInitResult end");

	if (pData != NULL) {
		delete[] pData;
		pData = NULL;
	}
	if (Rtn == TRUE) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "获取站点初始化信息成功，数据:%s", m_strAllSiteInitResult);
	}
	else {
		m_pILog->PrintLog(LOGLEVEL_LOG, "获取站点初始化信息失败");
	}
	return Rtn;
}

void ModuleProgram::ReleaseMem()
{
	std::map<CString, tBurnStatus>::iterator it;
	for (it = m_BurnStatusMap.begin(); it != m_BurnStatusMap.end(); it++) {
		if (it->second.pMutexForChipReady != NULL) {
			delete it->second.pMutexForChipReady;
			it->second.pMutexForChipReady = NULL;
		}

		for (UINT i = 0; i < it->second.vBurnWokThread.size(); i++) {
			if (it->second.vBurnWokThread[i] != NULL) {
				delete it->second.vBurnWokThread[i];
				it->second.vBurnWokThread[i] = NULL;
			}
		}
	}
}

INT ModuleProgram::GetExternProgramResultThreadProc(MSG msg, void *Para)
{
	INT Ret = 0;
	m_pILog->PrintLog(LOGLEVEL_LOG, "GetExternProgramResultThreadProc");
	ModuleProgram *pDlg = (ModuleProgram*)Para;
	if (msg.message == MSG_STARTPGETRET) {
		int nIdx = (int)msg.wParam;
		int skt = (int)msg.lParam;
		DoGetExternProgramResult(nIdx, skt);
	}

	//return -1;
	return 0;//返回0让此函数可以重复进入
}

INT ModuleProgram::DoGetExternProgramResult(int nIdx, int skt)
{
	INT nRtn = -1;
	if (nIdx < 0) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "=====>idx=%d错误，退出", nIdx);
		goto __end;
	}

	m_pILog->PrintLog(LOGLEVEL_LOG, "=====>Thread[%d] startup", nIdx);
	int nAttachIdx = -1;

	while (true) {
		if (nAttachIdx == -1) {
			nAttachIdx = nIdx;
		}

		Sleep(300);
		if (m_bQuit) {
			break;
		}

		UINT nTerminalCnt = (UINT)m_YieldSites.PassCnt;

		if (nTerminalCnt >= (UINT)m_ExpectICNum) {
			//完成当前的任务了
			CString strMsg;
			strMsg.Format("Attach Index=%d, 生产数量达成，结束烧录线程", nAttachIdx);
			m_pILog->PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);
			break;
		}

		CString strMsg;
		CString strDevSN;
		if (GetStrDevFromAttachIdx(nAttachIdx, strDevSN) != TRUE) {
			continue;
		}

		if (m_BurnStatusMap[strDevSN].nPhase != ePhaseReady) {
			continue;
		}

		/////////////////////////////////////////////////////////////

		if (m_nIsProdcingCnt > 0 && (UINT)m_ExpectICNum <= nTerminalCnt) { //完成的数量已经大于
			m_pILog->PrintLog(LOGLEVEL_LOG, "当前生产的数量=%d，完成的数量=%d, 需要烧录的数量=%d", m_nIsProdcingCnt, nTerminalCnt, m_ExpectICNum);
			continue;
		}
		///////////////////////////////////////////////////////////////////////

		m_nIsProdcingCnt++;
		
		PostMessage(m_MainHwd, MSG_UPDATE_PROGRAM_RESULT, (WPARAM)(skt), (LPARAM)PROGRAM_RESULT_DOING);

		strMsg.Format("Site:[%s]已经Ready,当前已经烧录完的数量=%d,生产任务数量=%d, 开始进入烧录...", strDevSN, m_YieldSites.TotalCnt, m_ExpectICNum);
		m_pILog->PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);

		m_BurnStatusMap[strDevSN].pMutexForChipReady->Lock();
		m_BurnStatusMap[strDevSN].nPhase = ePhaseLaunchBurn;
		m_BurnStatusMap[strDevSN].pMutexForChipReady->Unlock();

		int nCmdExcuRet = RunExcuCmdLine(nAttachIdx,skt);
		if (nCmdExcuRet == -1) {
			Sleep(100);
		}
		CString strShowRet;
		strShowRet.Format("NG");
		int nBurnRet = 2;
		if (nCmdExcuRet == 1) {
			strShowRet.Format("OK");
			nBurnRet = 1;
			PostMessage(m_MainHwd, MSG_UPDATE_PROGRAM_RESULT, (WPARAM)(skt), (LPARAM)PROGRAM_RESULT_OK);
		}
		else {
			PostMessage(m_MainHwd, MSG_UPDATE_PROGRAM_RESULT, (WPARAM)(skt), (LPARAM)PROGRAM_RESULT_NG);
		}

		m_BurnStatusMap[strDevSN].nSktBurnResult[skt] = nCmdExcuRet; //烧录结果
		m_BurnStatusMap[strDevSN].nPhase = ePhaseBurnOver;

		////////////////////////
		std::string strJson;
		Json::Value Root;
		Json::StyledWriter JWriter;

		Root["Result"] = Json::Value(nBurnRet);

		strJson = JWriter.write(Root);

		m_ExternBurnApi.MesSetProgramResult((LPSTR)(LPCTSTR)strDevSN, (LPSTR)strJson.c_str());
		strMsg.Format("Site:[%s]完成烧录进入告知烧录结果阶段,%s", strDevSN, strShowRet);
		m_pILog->PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);
		Sleep(1000);
	}

__end:
	m_pILog->PrintLog(LOGLEVEL_LOG, "=====>Work thread exit");

	return nRtn;
}

BOOL ModuleProgram::GetStrDevFromAttachIdx(int nConnectIdx, CString& strDev)
{
	BOOL Rtn = FALSE;
	std::map<CString, tBurnStatus>::iterator it;
	for (it = m_BurnStatusMap.begin(); it != m_BurnStatusMap.end(); it++) {
		if (nConnectIdx == it->second.nAttachIdx) {
			strDev.Format("%s", it->first);
			Rtn = TRUE;
			break;
		}
	}
	return Rtn;
}


INT ModuleProgram::GetProgramResultFromLog(CString SiteSn, CString Flag, CString strLogPath)
{
	INT nRtn = 0;
	CStdioFile LogFile;
	CString strMsg;
	CString strRow;
	CString strTempData;

	char pTempBuff[100] = { 0 };
	int nRetryCnt = 3;
	
	OutputDebugString("========Auto=====>打开日志文件");

	while (nRetryCnt > 0){
		nRetryCnt--;
		if (LogFile.Open(strLogPath, CFile::modeRead | CFile::shareDenyNone, NULL) == FALSE) {
			strMsg.Format("Site:[%s],打开升级的日志文件失败,路径=%s, ErrCode=%d", SiteSn, strLogPath, GetLastError());
			m_pILog->PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);
			continue;
		}
		strMsg.Format("Site:[%s]正在查找中...", SiteSn);
		m_pILog->PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);

		if (LogFile.GetLength() >= 100) {
			LogFile.Seek(-100, CFile::end);
			LogFile.Read(pTempBuff, 100);
			strTempData.Format("%s", pTempBuff);
		}

		if (LogFile.m_hFile != CFile::hFileNull) {
			LogFile.Close();
		}

		if (strTempData.Find(Flag) >= 0) {
			strMsg.Format("WorkOrder:%s Site:[%s]搜寻到成功", m_ProgRecord.DestRecord.strWorkOrder, SiteSn);
			m_pILog->PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);
			nRtn = 1; //成功的是1
			return nRtn;
		}
		else {
			strMsg.Format("WorkOrder:%s Site:[%s]搜寻到失败", m_ProgRecord.DestRecord.strWorkOrder, SiteSn);
			m_pILog->PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);
			nRtn = 0; //失败的是0
			return nRtn;
		}
	}

}

int ModuleProgram::RunExcuCmdLine(int nIdx, int skt)
{
	int nRetryOpenProcCnt = 0;

	int nRtn = -1;
	CString strCurCmd;
	int nFileLen;
	char pFileData[4096] = { 0 };
	CString strClsName;
	CString strTitleInfo;

	CFile BatFile;
	CString strBatPath;

	CString strMsg;
	CString strTempDir;
	CString strCom;
	CString strEachCmdLine;

	CString strDay;
	CString strCurTime;
	CString strLogDataPath;

	CString strFWPath = m_ProgRecord.DestRecord.strFWPath;

	CString strDevSN;

	clock_t StartTime, EndTime;
	UINT nTimeOut = 0;

	m_IsProgramEnd = FALSE;
	char* pOutData = NULL;
	if (GetStrDevFromAttachIdx(nIdx, strDevSN) != TRUE) {
		goto __end;
	}
	if (strDevSN.IsEmpty()) {
		goto __end;
	}


	int nOutSize = 1024 * 1024 * 10;
	pOutData = new char[nOutSize];
	if (pOutData == NULL) {
		strMsg.Format("分配缓存内存失败");
		m_pILog->PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);
		goto __end;
	}
	memset(pOutData, 0, nOutSize);

	int nComPort = m_BurnStatusMap[strDevSN].m_SktComMap[skt];
	////////////////////////////////////创建bat///////////////////////////
	strBatPath.Format("%s\\ac_tmp%d.bat", GetCurrentPath(), nIdx);
	if ((PathFileExists(strBatPath) == TRUE)) {
		DeleteFile(strBatPath);
	}

	if (BatFile.Open(strBatPath, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite | CFile::shareDenyNone, NULL) == FALSE) {
		strMsg.Format("创建bat文件失败，请确认路径是否正确，路径=%s, ErrCode=0x%x", strBatPath, GetLastError());
		m_pILog->PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);
		DeleteFile(strBatPath);
		goto __end;
	}
	///////////////////////////////////////
	strLogDataPath.Format("%s", m_ModuleInfo.strLogPath);

	if (!PathIsDirectory(strLogDataPath)) {
		CreateDirectory(strLogDataPath, 0);//不存在文件夹则创建
	}

	strCurTime.Format("%s", GetCurTime('_'));
	strDay.Format("\\%s", strCurTime.Left(8));
	strLogDataPath += strDay;

	if (!PathIsDirectory(strLogDataPath)) {
		CreateDirectory(strLogDataPath, 0);//不存在文件夹则创建
	}
	
	strDay.Format("\\%s_Log_%s_%s.data", strCurTime, strDevSN, m_ProgRecord.DestRecord.strWorkOrder);
	strLogDataPath += strDay;
	if ((PathFileExists(strLogDataPath) == TRUE)) {
		DeleteFile(strLogDataPath);
	}
	////////////////////////////////////////////////
	strEachCmdLine.Format("%s", m_ModuleInfo.strCmdLine);
	strTempDir.Format("bin%d", nIdx);
	strEachCmdLine.Replace("ExePath", m_ModuleInfo.strExePath);
	strEachCmdLine.Replace("FWPath", strFWPath);
	strCom.Format("%d",nComPort);
	strEachCmdLine.Replace("ComPort", strCom);
	strEachCmdLine.Replace("LogPath", strLogDataPath);
	strEachCmdLine.Replace("bin0", strTempDir);
	strEachCmdLine.Replace("Baudrate", m_ModuleInfo.strBaudrate);
	strCurCmd += strEachCmdLine;

	strMsg.Format("Site:[%s]cmd指令内容%s", strDevSN, strCurCmd);
	m_pILog->PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);

	//////////////////////内容写入bat////////////
	nFileLen = strCurCmd.GetLength();
	memcpy(pFileData, (LPSTR)(LPCSTR)strCurCmd, nFileLen);
	BatFile.Write(pFileData, nFileLen);
	BatFile.Close();
	///////////////////////////////////////////////////////////////////////
	SECURITY_ATTRIBUTES sa; // 此结构体包含一个对象的安全描述符，并指定检索到制定这个结构的句柄是否是可继承的
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL; // 安全描述符
	sa.bInheritHandle = TRUE; // 安全描述的对象能否被创建的进程继承，TRUE表示能被继承

	HANDLE h_read, h_write;
	if (!CreatePipe(&h_read, &h_write, &sa, 0)) {
		strMsg.Format("Site:[%s]创建管道失败", strDevSN);
		m_pILog->PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);
		goto __end;
	}

__RetryOpenProc:

	STARTUPINFO si/* = { sizeof(STARTUPINFO) }*/; // 此结构体用于指定新进程的主窗口特性 //si.cb = sizeof(STARTUPINFO);
												  /*GetStartupInfo(&si);*/
	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.wShowWindow = SW_HIDE; // 窗口设为隐藏
	si.hStdError = NULL;     //h_write; // 标识控制台窗口的缓存，可指定管道
	si.hStdOutput = h_write; // 同上
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;// 使用wShowWindow成员，使用hStdInput、hStdOutput、hStdError成员

	StartTime = clock();

	PROCESS_INFORMATION pi; // 此结构返回有关新进程及其主线程的信息
	if (!CreateProcess((LPSTR)(LPCSTR)strBatPath,
		(LPSTR)(LPCSTR)/*strCurCmd*/NULL,
		NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi)) {
		strMsg.Format("Site:[%s]执行指令失败，指令内容%s, nIdx:%d, LastError:0x%X", strDevSN, strCurCmd, nIdx, GetLastError());
		m_pILog->PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);

		if (nRetryOpenProcCnt > 10){
			nRtn = -1;
			goto __end;
		}
		else {
			nRetryOpenProcCnt++;
			Sleep(2000);
			goto __RetryOpenProc;
		}
	}
	CloseHandle(h_write);

	OutputDebugString("========Auto=====>执行完成");
	strMsg.Format("Site:[%s]命令执行中...", strDevSN);
	m_pILog->PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);

	char buffer[1024] = { 0 };
	DWORD i = 0;

	strMsg.Format("Site:[%s]等待进程结束...", strDevSN);
	m_pILog->PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);


	int nWait = WaitForSingleObject(pi.hProcess,100*1000);///等待进程结束

	if (WAIT_TIMEOUT == nWait) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "=====>[%s]烧录超时", strDevSN);

	}
	strClsName.Format("#32770 (对话框)");
	strTitleInfo.Format("Download");
	m_ClearUICallback(strTitleInfo);

	strClsName.Format("#32770 (对话框)");
	strTitleInfo.Format("Microsoft Visual C++ Runtime Library");
	m_ClearUICallback(strTitleInfo);

	EndTime = clock();
	nTimeOut = (EndTime - StartTime)/ CLOCKS_PER_SEC ;
	strMsg.Format("Site:[%s]当前的烧录时间为:%d 秒", strDevSN, nTimeOut);

	if (nTimeOut < m_ModuleInfo.nTimeOut / 2) {
		m_pILog->PrintLog(LOGLEVEL_ERR, (LPSTR)(LPCTSTR)strMsg);
	}
	else {
		m_pILog->PrintLog(LOGLEVEL_WARNING, (LPSTR)(LPCTSTR)strMsg);
	}


	strMsg.Format("Site:[%s]进程结束", strDevSN);
	m_pILog->PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);
	m_IsProgramEnd = TRUE;

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	CloseHandle(h_read);

	strMsg.Format("Site:[%s]执行cmd完成", strDevSN);
	m_pILog->PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);

	if ((PathFileExists(strLogDataPath) == FALSE)) { 
		strMsg.Format("Site:[%s]日志生成失败", strDevSN);
		m_pILog->PrintLog(LOGLEVEL_ERR, (LPSTR)(LPCTSTR)strMsg);
		goto __end;
	}

	if (nTimeOut < m_ModuleInfo.nTimeOut / 2) {//如果烧录时长比设置的一半时间还短则直接报fail
		nRtn = 0;
	}
	else{
		nRtn = GetProgramResultFromLog(strDevSN, m_ModuleInfo.strOKFlag, strLogDataPath);
	}


	MoveLogFile(strLogDataPath, nRtn);//成功移至pass文件夹，失败移至fail
__end:

	if (pOutData) {
		delete[] pOutData;
		pOutData = NULL;
	}

	if (BatFile.m_hFile != CFile::hFileNull) {
		BatFile.Close();
	}

	if ((PathFileExists(strLogDataPath) == TRUE)) {
		DeleteFile(strLogDataPath);
	}

	if ((PathFileExists(strBatPath) == TRUE)) {
		DeleteFile(strBatPath);
	}

	Sleep(2000);//原厂说增加几秒延迟下电

	return nRtn;
}


void ModuleProgram::MoveLogFile(CString OldLogPath,BOOL bResult)
{
	Sleep(1000);
	CString newPath = _T("");
	CString fileName = GetFileNameWithSuffix(OldLogPath);
	newPath = CComFunc::GetFilePath(OldLogPath);
	if (bResult) {
		newPath.Format("%s\\Pass", newPath);
		if (!PathIsDirectory(newPath)) {
			CreateDirectory(newPath, 0);
		}
		newPath.Format("%s\\%s", newPath, fileName);
	}
	else {
		newPath.Format("%s\\Fail", newPath);
		if (!PathIsDirectory(newPath)) {
			CreateDirectory(newPath, 0);
		}
		newPath.Format("%s\\%s", newPath, fileName);
	}
	m_pILog->PrintLog(LOGLEVEL_LOG, "===>OLD:%s, MoveLogFile to %s", OldLogPath, newPath);
	if (!MoveFile(OldLogPath, newPath)){
		m_pILog->PrintLog(LOGLEVEL_ERR, "MoveLogFile Fail %d", GetLastError());
	}
	
}

void ModuleProgram::StartDoProgramTaskWork()
{
	CMsgHandler<ModuleProgram, ModuleProgram> MsgHandler = MakeMsgHandler(this, &ModuleProgram::DoProgramTaskWorkProc);
	m_wTaskWorkThread.SetMsgHandle(MsgHandler);
	m_wTaskWorkThread.CreateThread();
	m_wTaskWorkThread.PostMsg(MSG_STARTWORK, 0, 0);
}

INT ModuleProgram::DoProgramTaskWorkProc(MSG msg, void *Para)
{
	INT Ret = 0;

	if (msg.message == MSG_STARTWORK) {
		while (1) {
			Sleep(100);
			Ret = DoTask();
			if (Ret != 0) {
				break;
			}
		}
	}

	return Ret;
}

INT ModuleProgram::SetMsgHandler() {
	INT Ret = 0, RtnCall;
	m_pILog->PrintLog(LOGLEVEL_LOG, "设置编程控制系统消息回调函数...");
	CMsgHandler<ModuleProgram, ModuleProgram> MsgListHandler = MakeMsgHandler(this, &ModuleProgram::ThreadMsgHandler);
	m_MsgListHandleThread.SetMsgHandle(MsgListHandler);
	m_MsgListHandleThread.CreateThread();
	m_MsgListHandleThread.PostMsg(MSG_LIST_HANDLE, 0, 0);

	RtnCall = m_ExternBurnApi.MesSetMsgHandle(MsgHandle, this);
	m_pILog->PrintLog(LOGLEVEL_LOG, "设置消息回调函数 %s", RtnCall == 1 ? "成功" : "失败");
	if (RtnCall == 0) {
		Ret = -1; goto __end;
	}
__end:
	return Ret;
}

int ModuleProgram::DoTask()
{
	INT Ret = 0;
	INT PreTotal = -1;
	BOOL bRtn = FALSE;
	m_bGetCmd4Success = FALSE;
	m_bGetMissionResult = FALSE;
	m_bRecvAutomaticLotEnd = FALSE;
	m_ProgramResult.Reset();
	CString strMsg;
	INT Total = 0, Pass = 0, Fail = 0;

	Ret = DoStartupACServer();
	if (Ret != 0) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "加载MultiAprog失败");
		Ret = -1; goto __end;
	}

	if (m_BurnStatusMap.size() == 0 ){
		GetAllSitesInitResult();//获取使能情况
	}

	SetSiteEnForInfoMap(m_gSktEnArray, SITE_COUNT);//根据界面勾选情况使能座子

	for (auto it = m_BurnStatusMap.begin(); it != m_BurnStatusMap.end(); it++) {
		for (int SktIdx = 0; SktIdx < EACH_SITE_CHIP_NUM; SktIdx++){
			if ((it->second.nCurrEnSkt >> SktIdx & 0x01) == 0x01) {//判断座子使能情况
				Sleep(200);
				int nAttachIdx = it->second.nAttachIdx;
				if (!m_nQueryComFlag) {//如果没有查询com口，通过界面上显示的com口赋值
					for (auto itSiteSn : m_SiteSnMapFromUI) {
						if (itSiteSn.first == it->first) {
							m_BurnStatusMap[it->first].nComPort[0] = itSiteSn.second.nComPort[0];
							m_BurnStatusMap[it->first].m_SktComMap[SktIdx] = itSiteSn.second.m_SktComMap[SktIdx];
						}
					}
				}
				BOOL bResult = (it->second.vBurnWokThread[SktIdx])->PostMsg(MSG_STARTPGETRET, (WPARAM)it->second.nAttachIdx, (LPARAM)SktIdx);
				CString strMsg;
				strMsg.Format("[%s]启动烧录线程[%d]%s", it->first, it->second.nAttachIdx * SITE_COUNT + SktIdx, bResult ? "成功" : "失败");
				m_pILog->PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);
			}
		}
	}

	Ret = SetMsgHandler();
	if (Ret != 0) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "SetMsgHandler失败");
		Ret = -1; goto __end;
	}

	Ret = LoadProject();
	if (Ret != 0) {
		Ret = -1; goto __end;
	}

	//UploadProgramerInfo2Mes();
	Ret = CompareChecksumProj();
	if (Ret != 0) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "批量生产终止");
		Ret = -1; goto __end;
	}
	else {
		char *pRealChksum = new char[8];
		memcpy(pRealChksum, m_Checksum, 8);
		PostMessage(m_MainHwd, MSG_UPDATECHECKSUM, (WPARAM)pRealChksum, 0);
	}

	m_pILog->PrintLog(LOGLEVEL_LOG, "启动工程任务...");
	bRtn = m_ExternBurnApi.MesStartProject();
	if (bRtn != TRUE) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "启动工程任务失败");
		Ret = -1; goto __end;
	}

	Ret = ConfigAutoTaskData();
	if (Ret != 0) {
		Ret = -1; goto __end;
	}

	m_bTaskDoing = TRUE;

	while (true) {
		m_YieldSitesMutex.Lock();
		Total = m_YieldSites.TotalCnt;
		Pass = m_YieldSites.PassCnt;
		Fail = m_YieldSites.FailCnt;
		m_YieldSitesMutex.Unlock();

		Sleep(150); ///两次取状态的间隔为200ms
		if (m_bQuit) {///客户需要退出
			m_pILog->PrintLog(LOGLEVEL_LOG, "操作员主动取消批量操作");
			break;
		}
		if (m_bGetMissionResult) {
			m_pILog->PrintLog(LOGLEVEL_LOG, "收到自动机通知MissionResult:%d LotEnd:%d", m_bGetMissionResult, m_bRecvAutomaticLotEnd);
			break;
		}
		if (PreTotal != Total) {///有可能两次取得的结果是一样的，不需要重复打印
			m_pILog->PrintLog(LOGLEVEL_LOG, "当前生产总个数:%-5d, 成功个数:%-5d, 失败个数:%-5d", Total, Pass, Fail);
			PreTotal = Total;
		}
		if (Pass >= m_ExpectICNum) {///只循环到Pass为StopQuantity截止。///打印最后一次
			m_pILog->PrintLog(LOGLEVEL_LOG, "当前生产总个数:%-5d, 成功个数:%-5d, 失败个数:%-5d", Total, Pass, Fail);
			break;
		}
	}

	UINT tryTimes = 1;
	if (IsAutoMode()){
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
	}
	else {
		Sleep(3000);
	}

	m_pILog->PrintLog(LOGLEVEL_LOG, "结束工程任务...");
	m_ExternBurnApi.MesStopProject();
	m_pILog->PrintLog(LOGLEVEL_LOG, "工程任务结束");
	tYieldSites *result = new tYieldSites();
	m_YieldSitesMutex.Lock();
	result->PassCnt = m_YieldSites.PassCnt;
	result->FailCnt = m_YieldSites.FailCnt;
	result->TotalCnt = m_YieldSites.TotalCnt;
	m_YieldSitesMutex.Unlock();
	PostMessage(m_MainHwd, MSG_UPDATE_PROGRAM_COUNT, (WPARAM)(result), (LPARAM)0);

	//从自动化CMD4取任务数据
	if (IsAutoMode()) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "开始发送CMD4从自动机取任务数据");
		m_bGetCmd4Success = GetCmd4RetInfo();
		if (m_bGetCmd4Success) {
			PostMessage(m_MainHwd, MSG_UPDATE_AUTO_COUNT, (WPARAM)(&m_ProgramResult), (LPARAM)0);
		}
		m_pILog->PrintLog(LOGLEVEL_LOG, "结束发送CMD4从自动机取任务数据");
	}

	PostMessage(m_MainHwd, MSG_PROGRAM_END, 0, 0);
	Ret = SendReport2Mes(m_ProgRecord.DestRecord.strWorkOrder, m_strComponentName);
__end:
	m_bQuit = TRUE;
	if (m_ExternBurnApi.MesCheckService() == TRUE) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "正在关闭服务器中...");
		m_ExternBurnApi.MesStopService();
		m_pILog->PrintLog(LOGLEVEL_LOG, "服务器已结束");
	}
	m_bTaskDoing = FALSE;


	PostMessage(m_MainHwd, MSG_DO_PROGRAM_FINISH, 0, 1000);

	return -1;
}


int ModuleProgram::SetSiteEnForInfoMap(INT nSktEnArray[], INT SktMax)
{
	
	int nRtn = TRUE;

	CString strBuildJson;
	Json::Value Root;
	Json::StyledWriter JWriter;
	CString strMsg;

	int nSktMaxCnt = EACH_SITE_CHIP_NUM;
	int nConnectIdx = -1;
	for ( auto it = m_BurnStatusMap.begin(); it != m_BurnStatusMap.end(); it++) {
		UINT nSktEn = 0;
		CString strDevSN;
		//tSiteEnInfo item;
		Json::Value ItemSite;
		Json::Value ItemSktEn;

		strDevSN.Format("%s", it->first);
		if (strDevSN.IsEmpty()) {
			continue;
		}

		ItemSite["SiteSN"] = Json::Value(strDevSN);
		if (GetAttachIdxFromStrDev(strDevSN, nConnectIdx) == TRUE) {//获取连接的第几台
			if (nConnectIdx >= 0 && nConnectIdx < SITE_COUNT) {
				for (int SktIdx = 0; SktIdx < EACH_SITE_CHIP_NUM; SktIdx++){
					if (nSktEnArray[nConnectIdx*SITE_COUNT + SktIdx] == 1) {
						nSktEn |= (0x01)<< SktIdx;
					}
				}
			}
			else {
				strMsg.Format("当前[%s]AttachIdx=%d有误", strDevSN, nConnectIdx);
				m_pILog->PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);
			}

		}

		ItemSite["SktEn"] = Json::Value(nSktEn);
		Root["SiteLists"].append(ItemSite);

		////
		m_BurnStatusMap[strDevSN].nCurrEnSkt = nSktEn;

		m_ExternBurnApi.MesSetAdapterEn((LPSTR)(LPCTSTR)strDevSN, nSktEn);
		Sleep(800);

		strMsg.Format("当前[%s]的座子使能Skt=%d", strDevSN, nSktEn);
		m_pILog->PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);
	}

	std::string strJson;
	strJson = JWriter.write(Root);

	m_ExternBurnApi.MesSetSktEnForInitAuto((LPSTR)strJson.c_str());

	strMsg.Format("初始化的使能信息：%s", strJson.c_str());
	m_pILog->PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);

	return nRtn;
}

INT ModuleProgram::StartProgram(HWND MainHwd, CProgRecord progRecord, INT nSktEnArray[], INT SktMax, std::map<CString, tBurnStatus> SiteSnMap)
{
	if (!m_bConfigCom) {
		m_pILog->PrintLog(LOGLEVEL_WARNING, "当前COM口为上一次烧录保存的配置，如设备连接或者COM口发生改变，请先点击获取Com口按钮更新Com口配置。");
		if (MessageBox(NULL, "当前COM口为上一次烧录保存的配置，如设备连接或者COM口发生改变，请先点击获取Com口按钮更新Com口配置。点击“是”则开始烧录", NULL, MB_YESNO | MB_ICONINFORMATION) == IDNO) {
			return 0;
		}
	}
	if (!m_ProgRecord.DestRecord.strProjPath.IsEmpty()){
		if (m_ProgRecord.DestRecord.strProjPath != progRecord.DestRecord.strProjPath) {
			m_pILog->PrintLog(LOGLEVEL_LOG, "工程文件地址有变化，请重新点击获取Com口按钮更新Com口配置。");
			AfxMessageBox("工程文件地址有变化，请重新点击获取Com口按钮更新Com口配置。");
			return 0;
		}
	}

	if (!m_bServerIsRun) {
		INT Ret = SetServerPath();
		if (Ret < 0) {
		return 0;
		}
	}

	if (m_MainHwd == NULL){
		m_MainHwd = MainHwd;
	}
	m_SiteSnMapFromUI.clear();
	m_SiteSnMapFromUI = SiteSnMap;
	memcpy(m_gSktEnArray, nSktEnArray, MAX_SKTCNT*4);
	m_ProgRecord = progRecord;
	m_ExpectICNum = m_ProgRecord.DestRecord.ExpertIDNum;

	m_bTaskDoing = TRUE;

	//ClearODMComponent();
	m_nIsProdcingCnt = 0;
	m_nQueryComFlag = 0;
	m_bQuit = FALSE;

	//开启后所有显示计数清零
	//memset((char*)&m_YieldSites, 0, sizeof(tYieldSites));
	m_YieldSites.TotalCnt = 0;
	m_YieldSites.PassCnt = 0;
	m_YieldSites.FailCnt = 0;

	StartDoProgramTaskWork();
	return 1;
}

INT ModuleProgram::GetModuleJson()
{
	INT Ret = -1;
	CString strErrMsg;
	CString strJsonPath;
	BYTE *pData = NULL;
	CFile File;
	CString strJson;
	CString strModuleName;

	INT FileSize = 0, ArraySize = 0;

	cJSON* pRootParser = NULL;
	cJSON* pArrayItem = NULL;
	cJSON* pItemObj = NULL;

	strJsonPath.Format("%s\\Module.json", GetCurrentPath());

	if (File.Open(strJsonPath, CFile::modeRead | CFile::shareDenyNone) == FALSE) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "打开报告文件失败,请确认文件路径!");
		goto __end;
	}
	FileSize = File.GetLength();
	pData = new BYTE[FileSize + 1];
	if (!pData) {
		goto __end;
	}
	memset(pData, 0, FileSize + 1);
	File.Read(pData, FileSize);
	strJson.Format("%s", pData);

	pRootParser = cJSON_Parse(strJson.GetBuffer());
	if (pRootParser == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "不符合Json数据格式 ");
		goto __end;
	}

	ArraySize = cJSON_GetArraySize(pRootParser);
	if (ArraySize <= 0) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "内容不存在 ");
		goto __end;
	}
	for (int i = 0; i < ArraySize; ++i) {
		pArrayItem = cJSON_GetArrayItem(pRootParser, i);

		pItemObj = cJSON_GetObjectItem(pArrayItem, "ModuleName");
		if (pItemObj == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析ModuleName字段错误，请确认json文件中维护的字段信息 ");
			goto __end;
		}
		strModuleName = pItemObj->valuestring;
		if (strModuleName.CompareNoCase(m_strModuleName) != 0){
			continue;
		}

		pItemObj = cJSON_GetObjectItem(pArrayItem, "Baudrate");
		if (pItemObj == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析Baudrate字段错误，请确认json文件中维护的字段信息 ");
			goto __end;
		}
		m_ModuleInfo.strBaudrate = pItemObj->valuestring;

		pItemObj = cJSON_GetObjectItem(pArrayItem, "Program_PIDVID");
		if (pItemObj == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析Program_PIDVID字段错误，请确认json文件中维护的字段信息 ");
			goto __end;
		}
		m_ModuleInfo.strProgramPID_VID = pItemObj->valuestring;


		pItemObj = cJSON_GetObjectItem(pArrayItem, "Verify_PIDVID");
		if (pItemObj == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析Verify_PIDVID字段错误，请确认json文件中维护的字段信息 ");
			goto __end;
		}
		m_ModuleInfo.strVerifyPID_VID = pItemObj->valuestring;

		pItemObj = cJSON_GetObjectItem(pArrayItem, "CmdLine");
		if (pItemObj == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析CmdLine字段错误，请确认json文件中维护的字段信息 ");
			goto __end;
		}
		m_ModuleInfo.strCmdLine = pItemObj->valuestring;

		pItemObj = cJSON_GetObjectItem(pArrayItem, "ExePath");
		if (pItemObj == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析ExePath字段错误，请确认json文件中维护的字段信息 ");
			goto __end;
		}
		m_ModuleInfo.strExePath = pItemObj->valuestring;

		pItemObj = cJSON_GetObjectItem(pArrayItem, "LogPath");
		if (pItemObj == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析LogPath字段错误，请确认json文件中维护的字段信息 ");
			goto __end;
		}
		m_ModuleInfo.strLogPath = pItemObj->valuestring;

		pItemObj = cJSON_GetObjectItem(pArrayItem, "ProgramMode");
		if (pItemObj == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析ProgramMode字段错误，请确认json文件中维护的字段信息 ");
			goto __end;
		}
		m_ModuleInfo.strMode = pItemObj->valuestring;

		pItemObj = cJSON_GetObjectItem(pArrayItem, "ComMap");
		if (pItemObj == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析ComMap字段错误，请确认json文件中维护的字段信息 ");
			goto __end;
		}
		m_ModuleInfo.strComMap = pItemObj->valuestring;
		if (m_ModuleInfo.strComMap.IsEmpty()){
			strErrMsg.Format("请先点击\"获取COM口\"按钮进行COM口获取");
			AfxMessageBox(strErrMsg);
			m_pILog->PrintLog(LOGLEVEL_ERR, "%s", strErrMsg);
			goto __end;
		}

		pItemObj = cJSON_GetObjectItem(pArrayItem, "TimeOut");
		if (pItemObj == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析TimeOut字段错误，请确认json文件中维护的字段信息 ");
			goto __end;
		}
		m_ModuleInfo.nTimeOut = pItemObj->valueint;

		pItemObj = cJSON_GetObjectItem(pArrayItem, "SuccessFlag");
		if (pItemObj == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析SuccessFlag字段错误，请确认json文件中维护的字段信息 ");
			goto __end;
		}
		m_ModuleInfo.strOKFlag = pItemObj->valuestring;
	}

	Ret = 0;

__end:

	if (pRootParser != NULL) {
		cJSON_Delete(pRootParser);
		pRootParser = NULL;
	}
	if (File.m_hFile != CFile::hFileNull) {
		File.Close();
	}
	if (pData) {
		delete[] pData;
	}
	return Ret;
}