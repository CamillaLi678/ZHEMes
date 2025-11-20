#include "stdafx.h"
#include "BaseModuleProgram.h"
#include "ComTool.h"
#include "Setting.h"
#include "cJSON.h"
#include "json/json.h"
#include "MesInterface.h"

BaseModuleProgram::BaseModuleProgram(CString ModuleName, CSetting *pSetting, ILog *pILog)
{
	m_bServerIsRun = FALSE;
	m_strModuleName = ModuleName;
	m_pSetting = pSetting;
	m_pILog = pILog;
}


BaseModuleProgram::~BaseModuleProgram()
{
	m_ExternBurnApi.CloseDllCom();
	m_bServerIsRun = FALSE;
	m_pILog = NULL;
	m_pSetting = NULL;
}


BOOL BaseModuleProgram::IsTaskDoing() {
	return m_bTaskDoing;
}

INT BaseModuleProgram::SetServerPath()
{
	INT Ret = 0;
	CString acServerFolder = m_pSetting->strACServerFolder;
	if (acServerFolder != "") {
		CString strStdDllPath;
		m_ExternBurnApi.CloseDllCom();
		strStdDllPath.Format("%s\\ExternBurn.dll", GetCurrentPath());

		if ((PathFileExists(strStdDllPath) == FALSE)) {
			CString strTip;
			strTip.Format("当前设置的路径ExternBurn.dll是%s，路径错误。", strStdDllPath);
			::AfxMessageBox(strTip);
			return -1;
		}

		m_ExternBurnApi.SetDllPath(strStdDllPath);
		Ret = m_ExternBurnApi.OpenDllCom();
		if (Ret != 0) {
			AfxMessageBox("从ExternBurn.dll中导出函数接口失败");
		}
		m_bServerIsRun = TRUE;
		if ((PathFileExists(acServerFolder) == FALSE)) {
			CString strTip;
			strTip.Format("当前设置的MultiAprog路径是%s，路径错误。", acServerFolder);
			::AfxMessageBox(strTip);
			return -1;
		}
		m_ExternBurnApi.MesSetServerPath(acServerFolder);
	}
	else {
		AfxMessageBox("请先设置服务器的路径");
		Ret = -1;
	}

	return Ret;
}

INT BaseModuleProgram::DoStartupACServer() {
	CString strMsg;
	strMsg.Format("运行函数中的线程id:%d", GetCurrentThreadId());
	INT startRet = StartService();
	if (startRet != 0) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "===>StartService Fail");
		return -1;
	}
	if (!m_bFinishStartupACServer) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "启动服务器失败");
		m_ExternBurnApi.MesStopProject();
		m_ExternBurnApi.MesStopService();
	}
	return 0;
}


INT BaseModuleProgram::UploadProgramerInfo2Mes()
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

	RtnCall = m_ExternBurnApi.MesGetProgrammerInfo_Json(pTmpBuf, Size);
	if (RtnCall == FALSE) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "获取座子信息失败");
		Ret = -1; goto __end;
	}

	strProgramerInfo.Format("%s", pTmpBuf);
__end:
	if (pTmpBuf != NULL) {
		delete pTmpBuf;
		pTmpBuf = NULL;
	}
	return Ret;
}

INT BaseModuleProgram::StartService()
{
	BOOL Rtn;
	m_bFinishStartupACServer = FALSE;
	if (1) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "启动服务器...");
		Rtn = m_ExternBurnApi.MesStartService();
	}
	else {
		m_pILog->PrintLog(LOGLEVEL_LOG, "启动简单服务器...");
		Rtn = m_ExternBurnApi.MesStartServiceSimple();
	}

	m_pILog->PrintLog(LOGLEVEL_LOG, "启动服务器 %s", Rtn == TRUE ? "成功" : "失败");
	if (Rtn == FALSE) {
		return -1;
	}
	m_bFinishStartupACServer = true;
	return 0;
}

INT BaseModuleProgram::LoadProject() {
	BOOL Rtn;
	CString strErrmsg;
	INT Ret = 0, nRetCode;
	CString strProjFile;
	CString strFuncMode = m_pSetting->strCurExec;
	strProjFile = m_ProgRecord.DestRecord.strProjPath;
	m_pILog->PrintLog(LOGLEVEL_LOG, "加载工程...");
	Rtn = m_ExternBurnApi.MesLoadProjectWithLot(strProjFile.GetBuffer(), strFuncMode.GetBuffer(), 0);
	if (Rtn == FALSE) {
		strErrmsg.Format("加载工程失败");
		Ret = -1;
		goto __end;
	}

	nRetCode = WaitJobDone(&m_ExternBurnApi);
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

INT BaseModuleProgram::WaitJobDone(CExternBurnApi* pStdMesApi)
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

INT BaseModuleProgram::GetProgramResult(CProgramResultData &ProgramResultData) {
	ProgramResultData.AutoFailCnt = m_ProgramResult.AutoFailCnt;
	ProgramResultData.AutoPassCnt = m_ProgramResult.AutoPassCnt;
	ProgramResultData.AutoTotalCnt = m_ProgramResult.AutoTotalCnt;
	ProgramResultData.AutoUPH = m_ProgramResult.AutoUPH;
	ProgramResultData.DevTimeRun = m_ProgramResult.DevTimeRun;
	ProgramResultData.DevTimeStop = m_ProgramResult.DevTimeStop;
	ProgramResultData.LastYieldChangeJson = m_ProgramResult.LastYieldChangeJson;
	return 0;
}


BOOL BaseModuleProgram::IsAutoMode() {
	bool bRet = false;
	if (m_pSetting->strAutoMode.CompareNoCase("Auto") == 0) {
		bRet = true;
	}
	return bRet;
}

BOOL BaseModuleProgram::IsMesMode()
{
	bool bRet = false;
	if (m_pSetting->strMesWordMode.CompareNoCase("Enable") == 0) {
		bRet = true;
	}
	return bRet;
}

CString BaseModuleProgram::GetAbsReportSavePath()
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


BOOL BaseModuleProgram::GetCmd4RetInfo()
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
	bFuncRet = m_ExternBurnApi.MesSendCmdToAutomatic(4, strSendCmdData.GetBuffer(), respBuffer, sizeof(respBuffer));
	strSendCmdData.ReleaseBuffer();
	if (bFuncRet != TRUE) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "执行CMD4失败！");
		goto __end;
	}
	strResponse.Format("%s", respBuffer);
	pRootParser = cJSON_Parse(strResponse.GetBuffer());
	strResponse.ReleaseBuffer();
	if (pRootParser == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "CMD4返回的数据格式不符合json要求！%s", strResponse);
		goto __end;
	}

	m_pILog->PrintLog(0, "执行CMD4返回: %s", strResponse);
	pItemObj = cJSON_GetObjectItem(pRootParser, "ErrCode");
	if (pItemObj == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "返回的json数据格式不符合要求ErrCode！");
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
		bFuncRet = TRUE;
	}
__end:
	return bFuncRet;
}


INT BaseModuleProgram::ConstructAutoTaskDataWithCmd3(CString&strTaskData)
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

INT BaseModuleProgram::ConfigAutoTaskData()
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
		RtnCall = m_ExternBurnApi.MesLoadTaskDataToAutomatic(1, strTaskData.GetBuffer());

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

INT BaseModuleProgram::SaveReprot2Local(const char*strJson, INT Size)
{
	INT Ret = 0;
	CString ReportPath;
	ReportPath.Format("%s\\Report_%s_%s.txt", GetAbsReportSavePath(), m_ProgRecord.DestRecord.strWorkOrder, GetCurTime('_'));/// m_ProgramInfo.strProjReportPath;
	CFile File;

	if (!PathIsDirectory(GetAbsReportSavePath())) {
		CreateDirectory(GetAbsReportSavePath(), 0);//不存在文件夹则创建
	}

	if (File.Open(ReportPath, CFile::modeCreate | CFile::modeWrite, NULL) == FALSE) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "保存本地报告文件失败:创建文件失败 %s", ReportPath);
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


INT BaseModuleProgram::CompareChecksumProj()
{
	INT Ret = 0;
	BOOL Rtn = FALSE;
	UINT64 Checksum = 0;
	UINT64 Checksumdesired = 0;
	char strChksum[8];

	CString strDesired;
	memset(strChksum, 0, 8);

	strDesired = m_ProgRecord.DestRecord.strProjChecksum;
	strDesired.Replace("0x", "");
	sscanf(strDesired.GetBuffer(), "%I64X", &Checksumdesired);
	Rtn = m_ExternBurnApi.MesGetChecksum(strChksum);
	if (Rtn == FALSE) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "从工程文件中获取校验值错误");
		Ret = -1; goto __end;
	}
	else {
		m_pILog->PrintLog(LOGLEVEL_LOG, "从工程文件中获取校验值成功");
		Checksum = *(UINT64*)strChksum;
		memcpy(m_Checksum, strChksum, 8);
		if (Checksum != Checksumdesired) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "校验值比对错误: 工程中的校验值为0x%I64X, 从文件中获取到的的校验值为0x%I64X", Checksum, Checksumdesired);
			Ret = -1; goto __end;
		}
		else {
			///准备更新给界面显示
			PostMessage(m_MainHwd, MSG_UPDATECHECKSUM, (WPARAM)Checksum, 1);
			m_pILog->PrintLog(LOGLEVEL_LOG, "校验值比对成功");
		}
	}

__end:
	return Ret;
}


INT BaseModuleProgram::SendReport2Mes(CString WorkOrder, CString MaterialNo)
{
	BOOL RtnCall = TRUE;
	INT Ret = 0;
	CHAR *pTmpBuf = NULL;
	INT Size = 1024 * 1024 * 10; ///
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

	RtnCall = m_ExternBurnApi.MesGetReport_Json(pTmpBuf, Size, &SizeNeed);
	if (RtnCall == FALSE) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "获取报告失败");
		Ret = -1; goto __end;
	}

	RtnCall = JReader.parse(pTmpBuf, pTmpBuf + SizeNeed, Root);
	if (RtnCall == FALSE) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "解析生产报告发生错误!");
		Ret = -1; goto __end;
	}

	BaseInfo = Root["BasicInformation"];
	//增加对象
	BaseInfo["WorkOrder"] = Json::Value(WorkOrder);
	BaseInfo["MaterialNo"] = Json::Value(MaterialNo);
	//BaseInfo["ChipNo"] = Json::Value(m_ProgRecord.DestRecord.strChipName);
	//BaseInfo["SoftVersionNo"] = Json::Value(m_ProgRecord.DestRecord.strSoftVer);
	strJsonReport = JWriter.write(BaseInfo);
	strBase.Format("%s", strJsonReport.c_str());

	Root["WorkOrder"] = Json::Value(WorkOrder);
	Root["MaterialNo"] = Json::Value(MaterialNo);
	//Root["ChipNo"] = Json::Value(m_ProgRecord.DestRecord.strChipName);
	//Root["SoftVersionNo"] = Json::Value(m_ProgRecord.DestRecord.strSoftVer);

	OverallStatistics = Root["OverallStatistics"];
	strJsonReport = JWriter.write(OverallStatistics);
	strDetail.Format("%s", strJsonReport.c_str());

	strJsonReport = JWriter.write(Root);

	SaveReprot2Local(strJsonReport.c_str(), strJsonReport.length());

	if (IsMesMode()) {
		//m_pILog->PrintLog(0, "正在推送报告给Mes中...");
		//m_LogEdit.PrintLog(LOGLEVEL_LOG, "正在推送报告给Mes中...");
		if (IsAutoMode()) {
			SYSTEMTIME st;
			CString strYear;
			CString strDetail;
			CString strTime;
			GetLocalTime(&st);
			strYear.Format("%4d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
			strDetail.Format("%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
			strTime.Format("%s %s", strYear, strDetail);
			if (CommitTaskIngfo(strTime, m_ProgramResult.DevTimeRun, m_ProgramResult.DevTimeStop) != 0) {
				m_pILog->PrintLog(LOGLEVEL_ERR, "提交任务数据信息给Mes失败...");
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

INT BaseModuleProgram::CommitTaskIngfo(CString timeEnd, CString timeRun, CString timeStop)
{
	if (!IsMesMode()) {
		return 0;
	}
	INT Ret = -1;
	CMesInterface &MesInterface = CMesInterface::getInstance();
	Ret = MesInterface.CommitTaskInfo2Mes(m_ProgramResult.LotStartTime, timeEnd, timeRun, timeStop);
	return Ret;

}