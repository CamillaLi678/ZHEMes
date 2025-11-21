

#include "stdafx.h"
#include "MesInterface.h"
#include "HttpClient.h"
#include "DlgProjectSel.h"
#include "cJSON.h"
#include "ComTool.h"
#include <vector>
#include "Json/json.h"

#define MAX_BUFFER_LEN (4096)

CMesInterface::CMesInterface() {
	m_strBoxSN = "";
	m_strBatNo = "";
	m_strRsNo = "";
	m_strWkNo = "";
}

CMesInterface::~CMesInterface() {
	DeInit();
}

// 新增：设置额外参数
void CMesInterface::SetExtraParams(CString boxSN, CString batNo, CString rsNo, CString wkNo)
{
	m_strBoxSN = boxSN;
	m_strBatNo = batNo;
	m_strRsNo = rsNo;
	m_strWkNo = wkNo;
	
	m_pILog->PrintLog(LOGLEVEL_LOG, "设置MES额外参数: box_sn=%s, bat_no=%s, rs_no=%s, wk_no=%s",
		boxSN, batNo, rsNo, wkNo);
}

void CMesInterface::Init(ILog *log, MesUserInfo userInfo, CString baseUrl, MesInterfaceInfo interfaceInfo) {

	DeInit();

	if (!m_bInited) {
		m_pILog = log;
		mUserInfo = userInfo;
		m_strToken = "";
		m_strBaseUrl = baseUrl;
		m_strAutoTaskFolder = interfaceInfo.tskFolder;
		m_strProgFileFolder = interfaceInfo.aprFolder;
		m_bInited = TRUE;
		m_bLocal = interfaceInfo.bLocal;
		m_strStationId = interfaceInfo.stationId;
		m_strGetLoginUrl = baseUrl + "/mes/login";
		m_strGetTokenUrl = baseUrl + "/mes/gettoken";
		m_strGetMesRecordUrl = baseUrl + "/mes/getworkorder";
		m_strSendTaskInfoUrl = baseUrl + "/mes/sendtaskinfo";
		m_strSendAlarmInfoUrl = baseUrl + "/mes/sendalarminfo";
		m_strSendProgrammerInfoUrl = baseUrl + "/mes/sendproginfo";
		m_strSendProgResultUrl = baseUrl + "/mes/sendprogresult";
		if (interfaceInfo.bLocal) {
			m_strGetTokenUrl = baseUrl + "/mes/gettoken?type=20";
			m_strGetMesRecordUrl = baseUrl + "/mes/getworkorder?type=20";
			m_strSendProgResultUrl = baseUrl + "/mes/sendprogresult?type=20";
		}
		if (interfaceInfo.bServerTset){
			mUserInfo.account.Format("%s", "acmes");
			mUserInfo.password.Format("%s", "12345678");
			//baseUrl = "http://acserver3:9525";
			m_strGetLoginUrl = baseUrl + "/mes/api/v1/user/login";
			m_strSendProgrammerInfoUrl = baseUrl + "/mes/api/v1/socket";
			m_strGetMesRecordUrl = baseUrl + "/mes/api/v1/getworkorder";
			m_strSendProgResultUrl = baseUrl + "/mes/api/v1/sendprogresult";
		}
	}
}

void CMesInterface::DeInit() {
	if (m_bInited) {
		m_bInited = FALSE;
		m_strToken = _T("");
		m_strBaseUrl = _T("");
		m_strGetLoginUrl = _T("");
		m_strGetTokenUrl = _T("");
		m_strGetMesRecordUrl = _T("");
		m_strSendTaskInfoUrl = _T("");
		m_strSendAlarmInfoUrl = _T("");
		m_strSendProgrammerInfoUrl = _T("");
		m_strSendProgResultUrl = _T("");

		m_mesInfo.curExec = _T("");
		m_mesInfo.workOrder = _T("");
		m_mesInfo.materialID = _T("");
	}
}


/**
	MES接口: 用户登录
**/
INT CMesInterface::GetMesLoginToServer(CString User, CString PassWord)
{
	int Ret = -1;
	if (!m_bInited) {
		return Ret;
	}

	int nHttpRet = 0;
	CHttpClient Client;
	CString strURL;
	CString strResponse;
	CString strErrMsg;

	cJSON* pRootParser = NULL;
	cJSON* pResult = NULL;
	cJSON* RootBuild = NULL;
	cJSON* pResData = NULL;
	cJSON* pToken = NULL;

	CString strHeader;
	CString strBody;
	CString strBuildJson;

	strURL.Format("%s", m_strGetLoginUrl);
	strHeader.Format("Content-Type: application/json;charset=UTF-8\r\n");//

	RootBuild = cJSON_CreateObject();
	cJSON_AddStringToObject(RootBuild, "UserName", (LPSTR)(LPCSTR)User);///mUserInfo.account
	cJSON_AddStringToObject(RootBuild, "Password", (LPSTR)(LPCSTR)PassWord);///mUserInfo.password
	strBuildJson = cJSON_Print(RootBuild);
	strBody.Format("%s", strBuildJson);
	//Change to UTF8
	strBody = MByteStrToUtf8CStr(strBody);

	nHttpRet = Client.HttpPost(strURL, strHeader, strBody, strResponse);

	m_pILog->PrintLog(LOGLEVEL_LOG, "GetMesLoginToServer strURL=%s", strURL);
	m_pILog->PrintLog(LOGLEVEL_LOG, "GetMesLoginToServer HttpPost Ret=%d strResponse=%s", nHttpRet, strResponse);

	if (nHttpRet != 0) {
		if (nHttpRet == 1) {
		}
		else if (nHttpRet == 2) {
		}
		goto __end;
	}

	pRootParser = cJSON_Parse(strResponse.GetBuffer());
	if (pRootParser == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "Mes返回的不符合Json数据格式 ");
		goto __end;
	}

	pResult = cJSON_GetObjectItem(pRootParser, "Code");
	if (pResult == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "解析Mes返回的Code字段错误，请确认Mes维护的字段信息, ");
		goto __end;
	}

	if (!cJSON_IsNumber(pResult) || pResult->valueint != 0) {
		strErrMsg.Format("%s", cJSON_GetObjectItem(pRootParser, "Message")->valuestring);
		m_pILog->PrintLog(LOGLEVEL_ERR, "Mes返回登录失败，错误原因为:%s ", strErrMsg);
		goto __end;
	}

	pResData = cJSON_GetObjectItem(pRootParser, "Data");
	if (pResData == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "解析Mes返回的Data字段错误，请确认Mes维护的字段信息 ");
		goto __end;
	}

	pToken = cJSON_GetObjectItem(pResData, "Token");
	if (pToken == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "解析Mes返回的Token字段错误，请确认Mes维护的字段信息 ");
		goto __end;
	}

	m_strToken.Format("%s", pToken->valuestring);

	Ret = 0;
__end:
	if (RootBuild != NULL) {
		cJSON_Delete(RootBuild);
		RootBuild = NULL;
	}
	if (pRootParser != NULL) {
		cJSON_Delete(pRootParser);
		pRootParser = NULL;
	}
	return Ret;
}

/**
	MES接口获取 Token
**/
INT CMesInterface::GetTokenFromServer()
{
	int Ret = -1;
	if (!m_bInited) {
		return Ret;
	}

	int nHttpRet = 0;
	CHttpClient Client;
	CString strURL;
	CString strResponse;
	CString strErrMsg;

	cJSON* pRootParser = NULL;
	cJSON* pResult = NULL;
	cJSON* RootBuild = NULL;
	cJSON* pResData = NULL;

	CString strHeader;
	CString strBody;
	CString strBuildJson;

	CString strAccount;
	CString strPwd;

	try {
		strURL.Format("%s", m_strGetTokenUrl);
		strHeader.Format("Content-Type: application/json;charset=UTF-8\r\n");//

		RootBuild = cJSON_CreateObject();
		cJSON_AddStringToObject(RootBuild, "serviceproviderid", (LPSTR)(LPCSTR)mUserInfo.providerId);
		cJSON_AddStringToObject(RootBuild, "account", (LPSTR)(LPCSTR)mUserInfo.account);
		cJSON_AddStringToObject(RootBuild, "pwd", (LPSTR)(LPCSTR)mUserInfo.password);
		strBuildJson = cJSON_Print(RootBuild);
		strBody.Format("%s", strBuildJson);
		//Change to UTF8
		strBody = MByteStrToUtf8CStr(strBody);

		nHttpRet = Client.HttpPost(strURL, strHeader, strBody, strResponse);

		m_pILog->PrintLog(LOGLEVEL_LOG, "GetTokenFromServer strURL=%s", strURL);
		m_pILog->PrintLog(LOGLEVEL_LOG, "GetTokenFromServer HttpPost Ret=%d strResponse=%s", nHttpRet, strResponse);

		if (nHttpRet != 0) {
			if (nHttpRet == 1) {
			}
			else if (nHttpRet == 2) {
			}
			goto __end;
		}

		pRootParser = cJSON_Parse(strResponse.GetBuffer());
		if (pRootParser == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "Mes返回的不符合Json数据格式 ");
			goto __end;
		}

		pResult = cJSON_GetObjectItem(pRootParser, "Result");
		if (pResult == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析Mes返回的Result字段错误，请确认Mes维护的字段信息, ");
			goto __end;
		}

		if (!cJSON_IsNumber(pResult) || pResult->valueint != 1) {
			strErrMsg.Format("%s", cJSON_GetObjectItem(pRootParser, "errMsg")->valuestring);
			m_pILog->PrintLog(LOGLEVEL_ERR, "Mes返回获取Token失败，错误原因为:%s ", strErrMsg);
			goto __end;
		}

		pResData = cJSON_GetObjectItem(pRootParser, "data");
		if (pResData == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析Mes返回的data字段错误，请确认Mes维护的字段信息 ");
			goto __end;
		}

		m_strToken.Format("%s", pResData->valuestring);
		Ret = 0;
	}
	catch (...) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "GetTokenFromServer HttpPost Fail strURL: %s", strURL);
	}

__end:
	if (RootBuild != NULL) {
		cJSON_Delete(RootBuild);
		RootBuild = NULL;
	}
	if (pRootParser != NULL) {
		cJSON_Delete(pRootParser);
		pRootParser = NULL;
	}
	return Ret;
}

#define SHOW_DO_NOT_SEL_PROJECT_DLG 1
MesInfo CMesInterface::getSelectMesInfo(std::vector<MesInfo> items) {
	MesInfo mesResult;
	//Commit时会用到，保存到本地
#if SHOW_DO_NOT_SEL_PROJECT_DLG	
	mesResult = items[0];
	mesResult.errCode = 0;
	mesResult.projPath.Format("%s\\%s", m_strProgFileFolder, mesResult.projPath);
	mesResult.autoTaskFilePath.Format("%s\\%s", m_strAutoTaskFolder, mesResult.autoTaskFilePath);
	m_mesInfo = mesResult;

	return mesResult;
#else
	CDlgProjectSel Dlg;
	Dlg.addItems(items);
	if (Dlg.DoModal() == IDOK) {
		MesInfo *selectedMes = Dlg.getSelectedItem();
		if (selectedMes != NULL) {
			mesResult.errCode = 0;
			mesResult.projPath.Format("%s\\%s", m_strProgFileFolder, selectedMes->projPath);
			mesResult.autoTaskFilePath.Format("%s\\%s", m_strAutoTaskFolder, selectedMes->autoTaskFilePath);
			mesResult.projChecksum = selectedMes->projChecksum;
			mesResult.workOrderICNum = selectedMes->workOrderICNum;
			mesResult.workOrderCompletedICNum = selectedMes->workOrderCompletedICNum;
			mesResult.expectICNum = selectedMes->expectICNum;
			mesResult.projVersion = selectedMes->projVersion;
			mesResult.curExec = selectedMes->curExec;
			mesResult.workOrder = selectedMes->workOrder;
			mesResult.materialID = selectedMes->materialID;
			mesResult.chipName = selectedMes->chipName;

			//Commit时会用到，保存到本地
			m_mesInfo = mesResult;
		}
		else {
			m_pILog->PrintLog(LOGLEVEL_ERR, "没有MES任务被选择");
		}
	}
	else {
		m_pILog->PrintLog(LOGLEVEL_ERR, "取消选择MES任务");
	}
	return mesResult;
#endif
}

MesInfo CMesInterface::GetMesRecord(CString workOrder, CString materialID, CString curExec)
{
	MesInfo mesResult;
	mesResult.errCode = -1;
	
	if (!m_bInited) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "CMesInterface not Inited");
		return mesResult;
	}

	// 读取API密钥
	CString strApiKey = _T("");
	CString strApiKeyPath;
	strApiKeyPath.Format("%s\\APISLAK.txt", GetCurrentPath());
	CFile apiKeyFile;
	if (apiKeyFile.Open(strApiKeyPath, CFile::modeRead | CFile::shareDenyNone)) {
		UINT fileLen = (UINT)apiKeyFile.GetLength();
		if (fileLen > 0) {
			char* pBuffer = new char[fileLen + 1];
			memset(pBuffer, 0, fileLen + 1);
			apiKeyFile.Read(pBuffer, fileLen);
			strApiKey.Format("%s", pBuffer);
			delete[] pBuffer;
		}
		apiKeyFile.Close();
	}
	else {
		m_pILog->PrintLog(LOGLEVEL_ERR, "无法读取API密钥文件: %s", strApiKeyPath);
		return mesResult;
	}

	//reset last mesinfo
	m_mesInfo.workOrder = _T("");
	m_mesInfo.materialID = _T("");

	int nHttpRet = 0;
	CHttpClient Client;
	CString strURL;
	CString strResponse;
	CString strErrMsg;

	cJSON* pRootParser = NULL;
	cJSON* pStatus = NULL;
	cJSON* RootBuild = NULL;
	cJSON* pItemObj = NULL;

	CString strHeader;
	CString strBody;
	CString strBuildJson;
	
	try {
		// 使用新的MES接口URL
		strURL.Format("%s/zgyb/ihtml?msclass=SAPP&servclass=api.SYBACGETMO&weblis=api.Request", m_strBaseUrl);

		// 构建新的请求体
		RootBuild = cJSON_CreateObject();
		cJSON_AddStringToObject(RootBuild, "mo_no", (LPSTR)(LPCSTR)workOrder);
		cJSON_AddStringToObject(RootBuild, "box_sn", (LPSTR)(LPCSTR)m_strBoxSN);
		cJSON_AddStringToObject(RootBuild, "rs_no", (LPSTR)(LPCSTR)m_strRsNo);
		cJSON_AddStringToObject(RootBuild, "wk_no", (LPSTR)(LPCSTR)m_strWkNo);

		strBuildJson = cJSON_PrintUnformatted(RootBuild);
		strBody.Format("%s", strBuildJson);

		m_pILog->PrintLog(LOGLEVEL_LOG, "strBody = %s", strBody);
		//Change to UTF8
		strBody = MByteStrToUtf8CStr(strBody);

		strResponse.Empty();
		// 使用API密钥作为请求头
		strHeader.Format("Content-Type:application/json;charset=UTF-8\r\nAuthorization:%s\r\n", strApiKey);

		m_pILog->PrintLog(LOGLEVEL_LOG, "GetMesRecord请求参数: mo_no=%s, box_sn=%s, rs_no=%s, wk_no=%s", 
			workOrder, m_strBoxSN, m_strRsNo, m_strWkNo);

		nHttpRet = Client.HttpPost(strURL, strHeader, strBody, strResponse);

		m_pILog->PrintLog(LOGLEVEL_LOG, "GetMesRecord, strURL=%s, strResponse=%s, HttpPost Ret=%d ", strURL, strResponse, nHttpRet);

		if (nHttpRet != 0) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "HTTP请求失败, 错误码=%d", nHttpRet);
			goto __end;
		}

		pRootParser = cJSON_Parse(strResponse.GetBuffer());
		if (pRootParser == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "Mes返回的不符合Json数据格式");
			goto __end;
		}

		// 检查Status字段
		pStatus = cJSON_GetObjectItem(pRootParser, "Status");
		if (pStatus == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析Mes返回的Status字段错误");
			goto __end;
		}

		if (!cJSON_IsBool(pStatus) || !cJSON_IsTrue(pStatus)) {
			// 获取错误消息
			pItemObj = cJSON_GetObjectItem(pRootParser, "Message");
			if (pItemObj != NULL && cJSON_IsString(pItemObj)) {
				strErrMsg.Format("%s", pItemObj->valuestring);
			}
			else {
				strErrMsg = "未知错误";
			}
			m_pILog->PrintLog(LOGLEVEL_ERR, "Mes返回获取工单烧录信息失败，错误原因为:%s", strErrMsg);
			goto __end;
		}

		// 解析返回的工单号
		pItemObj = cJSON_GetObjectItem(pRootParser, "mo_no");
		if (pItemObj == NULL || !cJSON_IsString(pItemObj)) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析返回的mo_no字段错误");
			goto __end;
		}
		mesResult.workOrder = pItemObj->valuestring;

		// 解析物料号
		pItemObj = cJSON_GetObjectItem(pRootParser, "MaterialID");
		if (pItemObj == NULL || !cJSON_IsString(pItemObj)) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析返回的MaterialID字段错误");
			goto __end;
		}
		mesResult.materialID = pItemObj->valuestring;

		// 解析批号
		pItemObj = cJSON_GetObjectItem(pRootParser, "bat_no");
		if (pItemObj != NULL && cJSON_IsString(pItemObj)) {
			mesResult.chipName = pItemObj->valuestring; // 暂存到chipName字段
		}

		// 解析数量
		pItemObj = cJSON_GetObjectItem(pRootParser, "Quantity");
		if (pItemObj == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析返回的Quantity字段错误");
			goto __end;
		}
		if (cJSON_IsString(pItemObj)) {
			mesResult.workOrderICNum = atoi(pItemObj->valuestring);
		}
		else if (cJSON_IsNumber(pItemObj)) {
			mesResult.workOrderICNum = (LONG)pItemObj->valueint;
		}

		// 解析工程文件名
		pItemObj = cJSON_GetObjectItem(pRootParser, "ProjectName");
		if (pItemObj == NULL || !cJSON_IsString(pItemObj)) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析Mes返回的ProjectName字段错误");
			goto __end;
		}
		mesResult.projPath = pItemObj->valuestring;

		// 解析工程文件校验值
		pItemObj = cJSON_GetObjectItem(pRootParser, "ProjectChecksum");
		if (pItemObj == NULL || !cJSON_IsString(pItemObj)) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析Mes返回的ProjectChecksum字段错误");
			goto __end;
		}
		mesResult.projChecksum.Format("%s", pItemObj->valuestring);

		// 解析自动化任务文件名
		pItemObj = cJSON_GetObjectItem(pRootParser, "TaskName");
		if (pItemObj == NULL || !cJSON_IsString(pItemObj)) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析Mes返回的TaskName字段错误");
			goto __end;
		}
		mesResult.autoTaskFilePath.Format("%s", pItemObj->valuestring);

		// 解析软件版本号
		pItemObj = cJSON_GetObjectItem(pRootParser, "SoftVersion");
		if (pItemObj == NULL || !cJSON_IsString(pItemObj)) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析Mes返回的SoftVersion字段错误");
			goto __end;
		}
		mesResult.projVersion.Format("%s", pItemObj->valuestring);

		mesResult.curExec = curExec;
		mesResult.errCode = 0; // 成功

		m_pILog->PrintLog(LOGLEVEL_LOG, "获取MES工单信息成功:");
		m_pILog->PrintLog(LOGLEVEL_LOG, "  工单号: %s", mesResult.workOrder);
		m_pILog->PrintLog(LOGLEVEL_LOG, "  物料号: %s", mesResult.materialID);
		m_pILog->PrintLog(LOGLEVEL_LOG, "  数量: %ld", mesResult.workOrderICNum);
		m_pILog->PrintLog(LOGLEVEL_LOG, "  工程文件: %s", mesResult.projPath);
		m_pILog->PrintLog(LOGLEVEL_LOG, "  校验值: %s", mesResult.projChecksum);
		m_pILog->PrintLog(LOGLEVEL_LOG, "  任务文件: %s", mesResult.autoTaskFilePath);
		m_pILog->PrintLog(LOGLEVEL_LOG, "  软件版本: %s", mesResult.projVersion);

	}
	catch (...) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "GetMesRecord HttpPost异常, strURL: %s", strURL);
		goto __end;
	}

__end:
	if (RootBuild != NULL) {
		cJSON_Delete(RootBuild);
		RootBuild = NULL;
	}
	if (pRootParser != NULL) {
		cJSON_Delete(pRootParser);
		pRootParser = NULL;
	}
	return mesResult;
}

//////ACMES：获取工单接口
MesInfo CMesInterface::GetACMesRecord(CString workOrder, CString materialID, CString curExec)
{
	MesInfo mesResult;
	mesResult.errCode = -1;
	std::vector<MesInfo> items;
	if (!m_bInited) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "CMesInterface not Inited");
		return mesResult;
	}

	//reset last mesinfo
	m_mesInfo.workOrder = _T("");
	m_mesInfo.materialID = _T("");

	// 读取APISLAK.txt获取API密钥
	CString strApiKey = _T("");
	CString strApiKeyPath;
	
	// 获取程序运行目录
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);
	CString strExePath = szPath;
	int nPos = strExePath.ReverseFind('\\');
	if (nPos != -1) {
		strExePath = strExePath.Left(nPos);
	}
	strApiKeyPath.Format("%s\\APISLAK.txt", strExePath);
	
	CStdioFile apiFile;
	if (apiFile.Open(strApiKeyPath, CFile::modeRead | CFile::typeText)) {
		apiFile.ReadString(strApiKey);
		apiFile.Close();
		strApiKey.Trim();
		m_pILog->PrintLog(LOGLEVEL_LOG, "成功读取API密钥，路径: %s", strApiKeyPath);
	} else {
		m_pILog->PrintLog(LOGLEVEL_WARNING, "无法读取APISLAK.txt（路径: %s），将不带API密钥请求", strApiKeyPath);
	}

	int nHttpRet = 0;
	CHttpClient Client;
	CString strURL;
	CString strResponse;
	CString strErrMsg;

	cJSON* pRootParser = NULL;
	cJSON* pResult = NULL;
	cJSON* RootBuild = NULL;

	cJSON* pProjectData;
	cJSON* pItemObj;
	cJSON* pArrayItem;

	CString strHeader;
	CString strBody;
	CString strBuildJson;
	CString strErrCode;
	int nRetryCnt = 8;
	CString getWorkOrder;
	CString getMaterialID;
	try {
		// 使用图片中的URL格式
		strURL.Format("%s/zqyb/ihtml?msclass=$APP&servclass=api.SYBACGETMO&weblis=api.Request", m_strBaseUrl);

		RootBuild = cJSON_CreateObject();
		cJSON_AddStringToObject(RootBuild, "mo_no", (LPSTR)(LPCSTR)workOrder);
		
		// 添加四个必需参数
		if (!m_strBoxSN.IsEmpty()) {
			cJSON_AddStringToObject(RootBuild, "box_sn", (LPSTR)(LPCSTR)m_strBoxSN);
		}
		if (!m_strRsNo.IsEmpty()) {
			cJSON_AddStringToObject(RootBuild, "rs_no", (LPSTR)(LPCSTR)m_strRsNo);
		}
		if (!m_strWkNo.IsEmpty()) {
			cJSON_AddStringToObject(RootBuild, "wk_no", (LPSTR)(LPCSTR)m_strWkNo);
		}

		strBuildJson = cJSON_PrintUnformatted(RootBuild);
		strBody.Format("%s", strBuildJson);
		//Change to UTF8
		strBody = MByteStrToUtf8CStr(strBody);

		strResponse.Empty();
		// 添加API密钥到请求头
		if (!strApiKey.IsEmpty()) {
			strHeader.Format("Content-Type:application/json;charset:UTF-8\r\nAPISLAK: %s", strApiKey);
		} else {
			strHeader.Format("Content-Type:application/json;charset:UTF-8");
		}

		nHttpRet = Client.HttpPost(strURL, strHeader, strBody, strResponse);

		m_pILog->PrintLog(LOGLEVEL_LOG, "GetACMesRecord, strURL=%s, strBody=%s, strResponse=%s, HttpPost Ret=%d ", strURL, strBody, strResponse, nHttpRet);

		if (nHttpRet != 0) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "HTTP请求失败，错误码: %d", nHttpRet);
			goto __end;
		}

		pRootParser = cJSON_Parse(strResponse.GetBuffer());
		if (pRootParser == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "MES返回的数据不符合Json格式");
			goto __end;
		}
		
		// 解析Result字段判断请求是否成功
		pResult = cJSON_GetObjectItem(pRootParser, "Result");
		if (pResult != NULL && cJSON_IsNumber(pResult)) {
			if (pResult->valueint != 1) {
				cJSON* pErrMsg = cJSON_GetObjectItem(pRootParser, "ErrMsg");
				if (pErrMsg != NULL && cJSON_IsString(pErrMsg)) {
					strErrMsg.Format("%s", pErrMsg->valuestring);
				} else {
					strErrMsg = "未知错误";
				}
				m_pILog->PrintLog(LOGLEVEL_ERR, "MES返回错误: %s", strErrMsg);
				goto __end;
			}
		}

		// 解析工单号
		pItemObj = cJSON_GetObjectItem(pRootParser, "mo_no");
		if (pItemObj != NULL && cJSON_IsString(pItemObj)) {
			getWorkOrder = pItemObj->valuestring;
		} else {
			getWorkOrder = workOrder; // 使用传入的工单号
		}

		// 解析Material ID（可选字段）
		pItemObj = cJSON_GetObjectItem(pRootParser, "MaterialID");
		if (pItemObj != NULL && cJSON_IsString(pItemObj)) {
			getMaterialID = pItemObj->valuestring;
		} else {
			getMaterialID = materialID.IsEmpty() ? "DEFAULT_MATERIAL" : materialID;
		}

		// 解析批号（如果MES返回）
		pItemObj = cJSON_GetObjectItem(pRootParser, "bat_no");
		if (pItemObj != NULL && cJSON_IsString(pItemObj)) {
			m_strBatNo.Format("%s", pItemObj->valuestring);
			m_pILog->PrintLog(LOGLEVEL_LOG, "MES返回批号: %s", m_strBatNo);
		}

		// 创建MesInfo对象
		MesInfo info;
		info.curExec = curExec;
		info.workOrder = getWorkOrder;
		info.materialID = getMaterialID;

		// 解析ProjectChecksum（烧录校验值）- 必需字段
		pItemObj = cJSON_GetObjectItem(pRootParser, "ProjectChecksum");
		if (pItemObj == NULL || !cJSON_IsString(pItemObj)) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析MES返回的ProjectChecksum字段错误，请确认MES维护的字段信息");
			goto __end;
		}
		info.projChecksum.Format("%s", pItemObj->valuestring);
		m_pILog->PrintLog(LOGLEVEL_LOG, "烧录校验值: %s", info.projChecksum);

		// 解析ProjectName（工程文件路径）- 必需字段
		pItemObj = cJSON_GetObjectItem(pRootParser, "ProjectName");
		if (pItemObj == NULL || !cJSON_IsString(pItemObj)) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析MES返回的ProjectName字段错误，请确认MES维护的字段信息");
			goto __end;
		}
		info.projPath = pItemObj->valuestring;
		m_pILog->PrintLog(LOGLEVEL_LOG, "工程文件路径: %s", info.projPath);

		// 解析TaskName（自动化任务文件路径）- 必需字段
		pItemObj = cJSON_GetObjectItem(pRootParser, "TaskName");
		if (pItemObj == NULL || !cJSON_IsString(pItemObj)) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析MES返回的TaskName字段错误，请确认MES维护的字段信息");
			goto __end;
		}
		info.autoTaskFilePath.Format("%s", pItemObj->valuestring);
		m_pILog->PrintLog(LOGLEVEL_LOG, "自动化任务文件路径: %s", info.autoTaskFilePath);

		// 解析Quantity（工单数量）- 可选字段
		pItemObj = cJSON_GetObjectItem(pRootParser, "Quantity");
		if (pItemObj != NULL && cJSON_IsNumber(pItemObj)) {
			info.workOrderICNum = (LONG)pItemObj->valueint;
		} else {
			info.workOrderICNum = 0;
		}

		// 解析RemainQuantity（剩余数量）- 可选字段
		pItemObj = cJSON_GetObjectItem(pRootParser, "RemainQuantity");
		if (pItemObj != NULL && cJSON_IsNumber(pItemObj)) {
			info.RemainICNum = (LONG)pItemObj->valueint;
		} else {
			info.RemainICNum = 0;
		}

		// 解析SoftVersion（软件版本）- 可选字段
		pItemObj = cJSON_GetObjectItem(pRootParser, "SoftVersion");
		if (pItemObj != NULL && cJSON_IsString(pItemObj)) {
			info.projVersion.Format("%s", pItemObj->valuestring);
		}

		// 解析Id（工单ID）- 可选字段
		pItemObj = cJSON_GetObjectItem(pRootParser, "Id");
		if (pItemObj != NULL && cJSON_IsNumber(pItemObj)) {
			nOrderID = pItemObj->valueint;
		}

		items.push_back(info);
	
	}
	catch (...) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "GetMesRecord HttpPost Fail strURL: %s", strURL);
		goto __end;
	}
	if (items.size() == 0){
		m_pILog->PrintLog(LOGLEVEL_ERR, "此工单不存在");
		goto __end;
	}
	mesResult = getSelectMesInfo(items);
__end:
	if (RootBuild != NULL) {
		cJSON_Delete(RootBuild);
		RootBuild = NULL;
	}
	if (pRootParser != NULL) {
		cJSON_Delete(pRootParser);
		pRootParser = NULL;
	}
	return mesResult;
}

INT CMesInterface::CommitTaskInfo2Mes(CString timeStart, CString timeEnd, CString timeRun, CString timeStop)
{
	if (!m_bInited) {
		return -1;
	}

	if (m_mesInfo.workOrder.IsEmpty() || m_mesInfo.materialID.IsEmpty()) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "CMesInterface WorkOrder/materialID IsEmpty");
		return -1;
	}

	INT Ret = -1;
	int nHttpRet = 0;
	CHttpClient Client;
	CString strURL;
	CString strResponse;
	CString strErrMsg;

	CString strStartTime;
	CString strEndTime;
	CString strTimeRun;
	CString strTimeStop;

	cJSON* pRootParser = NULL;
	cJSON* pResult = NULL;
	cJSON* RootBuild = NULL;
	CString strHeader;
	CString strBody;
	CString strBuildJson;

	CString strYear;
	CString strDetail;
	CString strErrCode;
	int nRetryCnt = 3;

	try {

		strURL.Format("%s", m_strSendTaskInfoUrl);

		RootBuild = cJSON_CreateObject();
		cJSON_AddStringToObject(RootBuild, "Funcmode", m_mesInfo.curExec);
		cJSON_AddStringToObject(RootBuild, "OrderName", (LPSTR)(LPCSTR)/*m_ProgRecord.DestRecord.*/m_mesInfo.workOrder);
		cJSON_AddStringToObject(RootBuild, "MaterialID", (LPSTR)(LPCSTR)/*m_ProgRecord.DestRecord.*/m_mesInfo.materialID);
		cJSON_AddStringToObject(RootBuild, "pcName", (LPSTR)(LPCSTR)m_strDeviceName);
		cJSON_AddStringToObject(RootBuild, "TimeStart", (LPSTR)(LPCSTR)timeStart);
		cJSON_AddStringToObject(RootBuild, "TimeEnd", (LPSTR)(LPCSTR)timeEnd);
		cJSON_AddStringToObject(RootBuild, "TimeRun", (LPSTR)(LPCSTR)timeRun);
		cJSON_AddStringToObject(RootBuild, "TimeStop", (LPSTR)(LPCSTR)timeStop);

		strBuildJson = cJSON_PrintUnformatted(RootBuild);
		strBody.Format("%s", strBuildJson);
		//Change to UTF8
		strBody = MByteStrToUtf8CStr(strBody);
	__Again:
		strResponse.Empty();
		strHeader.Format("Content-Type:application/json;charset:UTF-8\r\n");

		nHttpRet = Client.HttpPost(strURL, strHeader, strBody, strResponse);

		m_pILog->PrintLog(LOGLEVEL_LOG, "CommitTaskIngfo strURL=%s,strHeader=%s, strBody=%s, strResponse=%s, HttpPost Ret=%d \r\n",
			strURL, strHeader, strBody, strResponse, nHttpRet);

		if (nHttpRet != 0) {
			if (nHttpRet == 1) {
			}
			else if (nHttpRet == 2) {
			}
			goto __end;
		}

		pRootParser = cJSON_Parse(strResponse.GetBuffer());
		strResponse.ReleaseBuffer();
		if (pRootParser == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "Mes返回的不符合Json数据格式 ");
			goto __end;
		}

		pResult = cJSON_GetObjectItem(pRootParser, "Result");
		if (pResult == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析Mes返回的Result字段错误，请确认Mes维护的字段信息, ");
			goto __end;
		}

		if (!cJSON_IsNumber(pResult) || pResult->valueint != 1) {
			if (cJSON_GetObjectItem(pRootParser, "errMsg") != NULL) {
				strErrMsg.Format("%s", cJSON_GetObjectItem(pRootParser, "errMsg")->valuestring);
				m_pILog->PrintLog(LOGLEVEL_ERR, "CommitTaskIngfo接口返回上传烧录结果失败，错误原因为:%s ", strErrMsg);
			}

			if (cJSON_GetObjectItem(pRootParser, "errCode") != NULL) {
				strErrCode.Format("%s", cJSON_GetObjectItem(pRootParser, "errCode")->valuestring);
			}
			if (strErrCode.CompareNoCase("1005") == 0 && nRetryCnt > 0) {
				nRetryCnt--;
				m_pILog->PrintLog(LOGLEVEL_ERR, "进行重新获取Token");
				GetTokenFromServer();
				goto __Again;
			}
			else {
				goto __end;
			}
		}
	}
	catch (...) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "CommitTaskInfo2Mes HttpPost Fail strURL: %s", strURL);
		goto __end;
	}
	Ret = 0;

__end:
	if (RootBuild != NULL) {
		cJSON_Delete(RootBuild);
		RootBuild = NULL;
	}
	if (pRootParser != NULL) {
		cJSON_Delete(pRootParser);
		pRootParser = NULL;
	}
	return Ret;

}

INT CMesInterface::CommitAlarmInfo2Mes(CString alarmCode, CString alarmMsg, CString alarmcStartTime, CString alarmKillTime, int alarmFlag)
{
	if (!m_bInited) {
		return -1;
	}

	if (m_mesInfo.workOrder.IsEmpty() || m_mesInfo.materialID.IsEmpty()) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "CMesInterface WorkOrder/materialID IsEmpty");
		return -1;
	}

	INT Ret = -1;
	int nHttpRet = 0;
	CHttpClient Client;
	CString strURL;
	CString strResponse;
	CString strErrMsg;

	CString strStartTime;
	CString strEndTime;
	CString strTimeRun;
	CString strTimeStop;

	cJSON* pRootParser = NULL;
	cJSON* pResult = NULL;
	cJSON* RootBuild = NULL;

	CString strHeader;
	CString strBody;
	CString strBuildJson;

	CString strYear;
	CString strDetail;
	CString strTime;
	CString strErrCode;
	int nRetryCnt = 3;

	try {
		/* alarmTime format "ETime": "2021-11-06-11-49-10-326"*/
		strURL.Format("%s", m_strSendAlarmInfoUrl);

		RootBuild = cJSON_CreateObject();
		cJSON_AddStringToObject(RootBuild, "OrderName", (LPSTR)(LPCSTR)/*m_ProgRecord.DestRecord.*/m_mesInfo.workOrder);
		cJSON_AddStringToObject(RootBuild, "MaterialID", (LPSTR)(LPCSTR)/*m_ProgRecord.DestRecord.*/m_mesInfo.materialID);
		cJSON_AddStringToObject(RootBuild, "pcName", (LPSTR)(LPCSTR)m_strDeviceName);

		cJSON_AddStringToObject(RootBuild, "AlarmCode", alarmCode);
		cJSON_AddStringToObject(RootBuild, "AlarmMessage", alarmMsg);
		cJSON_AddNumberToObject(RootBuild, "AlarmFlag", alarmFlag);
		cJSON_AddStringToObject(RootBuild, "timeStart", (LPSTR)(LPCSTR)alarmcStartTime);
		cJSON_AddStringToObject(RootBuild, "timeKill", (LPSTR)(LPCSTR)alarmKillTime);

		strBuildJson = cJSON_PrintUnformatted(RootBuild);
		strBody.Format("%s", strBuildJson);
		//Change to UTF8
		strBody = MByteStrToUtf8CStr(strBody);
	__Again:
		strResponse.Empty();
		strHeader.Format("Content-Type:application/json;charset:UTF-8\r\n");

		nHttpRet = Client.HttpPost(strURL, strHeader, strBody, strResponse);
		//m_pILog->PrintLog(LOGLEVEL_LOG, "CommitAlarmInfo strURL=%s,strHeader=%s, strBody=%s, strResponse=%s, HttpPost Ret=%d",
		//	strURL, strHeader, strBody, strResponse, nHttpRet);
		if (nHttpRet != 0) {
			if (nHttpRet == 1) {
			}
			else if (nHttpRet == 2) {
			}
			goto __end;
		}

		pRootParser = cJSON_Parse(strResponse.GetBuffer());
		if (pRootParser == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "Mes返回的不符合Json数据格式 ");
			goto __end;
		}

		pResult = cJSON_GetObjectItem(pRootParser, "Result");
		if (pResult == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析Mes返回的Result字段错误，请确认Mes维护的字段信息, ");
			goto __end;
		}

		if (!cJSON_IsNumber(pResult) || pResult->valueint != 1) {
			if (cJSON_GetObjectItem(pRootParser, "errMsg") != NULL) {
				strErrMsg.Format("%s", cJSON_GetObjectItem(pRootParser, "errMsg")->valuestring);
				m_pILog->PrintLog(LOGLEVEL_ERR, "CommitAlarmInfo接口返回上传报警结果失败，错误原因为:%s ", strErrMsg);
			}
			if (cJSON_GetObjectItem(pRootParser, "errCode") != NULL) {
				strErrCode.Format("%s", cJSON_GetObjectItem(pRootParser, "errCode")->valuestring);
			}
			if (strErrCode.CompareNoCase("1005") == 0 && nRetryCnt > 0) {
				nRetryCnt--;
				m_pILog->PrintLog(LOGLEVEL_ERR, "进行重新获取Token");
				GetTokenFromServer();
				goto __Again;
			}
			else {
				goto __end;
			}
		}
	}
	catch (...) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "CommitAlarmInfo2Mes HttpPost Fail strURL: %s", strURL);
		goto __end;
	}
	Ret = 0;
#ifdef _DEBUG	
	m_pILog->PrintLog(LOGLEVEL_LOG, "上传报警消息成功 code: %s alarm: %s", alarmCode, alarmMsg);
#endif
__end:
	if (RootBuild != NULL) {
		cJSON_Delete(RootBuild);
		RootBuild = NULL;
	}
	if (pRootParser != NULL) {
		cJSON_Delete(pRootParser);
		pRootParser = NULL;
	}
	return Ret;
}


INT CMesInterface::CommitProgramerInfo2Mes(CString strJson)
{
	if (!m_bInited) {
		return -1;
	}

	/*if (m_mesInfo.workOrder.IsEmpty() || m_mesInfo.materialID.IsEmpty()) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "CMesInterface WorkOrder/materialID IsEmpty");
		return -1;
	}*/

	if (m_mesInfo.workOrder.IsEmpty()) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "CMesInterface WorkOrder IsEmpty");
		return -1;
	}

	// 读取API密钥
	CString strApiKey = _T("");
	CString strApiKeyPath;
	strApiKeyPath.Format("%s\\APISLAK.txt", GetCurrentPath());
	CFile apiKeyFile;
	if (apiKeyFile.Open(strApiKeyPath, CFile::modeRead | CFile::shareDenyNone)) {
		UINT fileLen = (UINT)apiKeyFile.GetLength();
		if (fileLen > 0) {
			char* pBuffer = new char[fileLen + 1];
			memset(pBuffer, 0, fileLen + 1);
			apiKeyFile.Read(pBuffer, fileLen);
			strApiKey.Format("%s", pBuffer);
			strApiKey.Trim();
			delete[] pBuffer;
		}
		apiKeyFile.Close();
		m_pILog->PrintLog(LOGLEVEL_LOG, "成功读取API密钥文件: %s", strApiKeyPath);
	}
	else {
		m_pILog->PrintLog(LOGLEVEL_ERR, "无法读取API密钥文件: %s", strApiKeyPath);
		return -1;
	}

	BOOL RtnCall = TRUE;
	INT Ret = -1;
	INT SizeNeed = 0;

	int nHttpRet = 0;
	CHttpClient Client;
	CString strURL;
	CString strResponse;
	CString strErrMsg;

	cJSON* pRootParser = NULL;
	cJSON* pStatus = NULL;
	cJSON* RootBuild = NULL;
	cJSON* pItemObj = NULL;
	CString strHeader;
	CString strBody;
	CString strBuildJson;

	try {
		strJson.Replace("index", "Index");
		strJson.Replace("limited", "Limited");
		strJson.Replace("current", "Current");
		strJson.Replace("failcnt", "FailCnt");

		m_pILog->PrintLog(LOGLEVEL_LOG, "获取座子信息完成...");

		RootBuild = cJSON_Parse(strJson.GetBuffer());
		if (RootBuild == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析座子的信息不符合json格式，raw data=%s ", strJson);
			goto __end;
		}

		// 添加工单和物料信息
		cJSON_AddStringToObject(RootBuild, "OrderName", (LPSTR)(LPCSTR)m_mesInfo.workOrder);
		cJSON_AddStringToObject(RootBuild, "MaterialID", (LPSTR)(LPCSTR)m_mesInfo.materialID);
		
		// 添加额外参数
		if (!m_strBoxSN.IsEmpty()) {
			cJSON_AddStringToObject(RootBuild, "box_sn", (LPSTR)(LPCSTR)m_strBoxSN);
		}
		if (!m_strBatNo.IsEmpty()) {
			cJSON_AddStringToObject(RootBuild, "bat_no", (LPSTR)(LPCSTR)m_strBatNo);
		}
		if (!m_strRsNo.IsEmpty()) {
			cJSON_AddStringToObject(RootBuild, "rs_no", (LPSTR)(LPCSTR)m_strRsNo);
		}
		if (!m_strWkNo.IsEmpty()) {
			cJSON_AddStringToObject(RootBuild, "wk_no", (LPSTR)(LPCSTR)m_strWkNo);
		}

		strBuildJson = cJSON_Print(RootBuild);
		strBody.Format("%s", strBuildJson);
		//Change to UTF8
		strBody = MByteStrToUtf8CStr(strBody);

		// 使用新的MES接口URL
		strURL.Format("%s/zgyb/ihtml?msclass=SAPP&servclass=api.SYBACSLOT&weblis=api.Request", m_strBaseUrl);

		strResponse.Empty();
		// 添加API密钥到请求头
		strHeader.Format("Content-Type:application/json;charset=UTF-8\r\nAuthorization:%s\r\n", strApiKey);

		m_pILog->PrintLog(LOGLEVEL_LOG, "上传座子信息到MES，URL=%s", strURL);
		m_pILog->PrintLog(LOGLEVEL_LOG, "请求参数: %s", strBody);

		nHttpRet = Client.HttpPost(strURL, strHeader, strBody, strResponse);

		m_pILog->PrintLog(LOGLEVEL_LOG, "UploadProgramerInfo2Mes strURL=%s, strBody:%s strResponse=%s, HttpPost Ret=%d ", 
			strURL, strBody, strResponse, nHttpRet);

		if (nHttpRet != 0) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "HTTP请求失败, 错误码=%d", nHttpRet);
			Ret = -1;
			goto __end;
		}

		pRootParser = cJSON_Parse(strResponse.GetBuffer());
		if (pRootParser == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "MES返回的不符合Json数据格式");
			Ret = -1; 
			goto __end;
		}

		// 检查Status字段
		pStatus = cJSON_GetObjectItem(pRootParser, "Status");
		if (pStatus == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析MES返回的Status字段错误");
			Ret = -1; 
			goto __end;
		}

		if (!cJSON_IsBool(pStatus) || !cJSON_IsTrue(pStatus)) {
			// 获取错误消息
			pItemObj = cJSON_GetObjectItem(pRootParser, "Message");
			if (pItemObj != NULL && cJSON_IsString(pItemObj)) {
				strErrMsg.Format("%s", pItemObj->valuestring);
			}
			else {
				strErrMsg = "未知错误";
			}
			m_pILog->PrintLog(LOGLEVEL_ERR, "MES返回上传座子信息失败，错误原因为:%s", strErrMsg);
			Ret = -1;
			goto __end;
		}

		m_pILog->PrintLog(LOGLEVEL_LOG, "上传座子信息成功");
	}
	catch (...) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "CommitProgramerInfo2Mes HttpPost异常 strURL: %s", strURL);
		goto __end;
	}
	Ret = 0;
__end:
	if (RootBuild != NULL) {
		cJSON_Delete(RootBuild);
		RootBuild = NULL;
	}
	if (pRootParser != NULL) {
		cJSON_Delete(pRootParser);
		pRootParser = NULL;
	}

	return Ret;
}

INT CMesInterface::CommitProgramRetJson2Mes(CString strJson) {
	if (!m_bInited) {
		return -1;
	}
	INT Ret = -1;
	int nHttpRet = 0;
	CHttpClient Client;
	CString strURL;
	CString strResponse;
	CString strErrMsg;

	CString getResult = _T("");

	cJSON* pRootParser = NULL;
	cJSON* pResult;


	CString strHeader;
	CString strBody;
	CString strBuildJson;
	CString strErrCode;

	try {
		strURL.Format("%s", m_strSendProgResultUrl);
		strBuildJson.Format("%s", strJson);
		//change to utf8 data
		char pUTF8Data[MAX_BUFFER_LEN];
		memset(pUTF8Data, 0, MAX_BUFFER_LEN);
		DWORD ByteUsed = MAX_BUFFER_LEN;
		if (MByteToUtf8((LPCTSTR)strBuildJson.GetBuffer(), pUTF8Data, MAX_BUFFER_LEN, ByteUsed) != 1) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "Utf8ToMByte %s Failure", strBuildJson);
			return Ret;
		}
		strBody.Format("%s", pUTF8Data);

		strResponse.Empty();
		strHeader.Format("Content-Type:application/json;charset:UTF-8\r\nAuthorization:Bearer %s", m_strToken);

		nHttpRet = Client.HttpPost(strURL, strHeader, strBody, strResponse);
		m_pILog->PrintLog(LOGLEVEL_LOG, "UploadProgramRet2Mes strURL=%s,strHeader=%s, strBody=%s, strResponse=%s, HttpPost Ret=%d",
			strURL, strHeader, strBody, strResponse, nHttpRet);
		if (nHttpRet != 0) {
			if (nHttpRet == 1) {
			}
			else if (nHttpRet == 2) {
			}
			goto __end;
		}

		pRootParser = cJSON_Parse(strResponse.GetBuffer());
		if (pRootParser == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "Mes返回的不符合Json数据格式 ");
			goto __end;
		}

		pResult = cJSON_GetObjectItem(pRootParser, "Result");
		if (pResult == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析Mes返回的Result字段错误，请确认Mes维护的字段信息, ");
			goto __end;
		}

		if (!cJSON_IsNumber(pResult) || pResult->valueint != 1) {
			if (cJSON_GetObjectItem(pRootParser, "errMsg") != NULL) {
				strErrMsg.Format("%s", cJSON_GetObjectItem(pRootParser, "errMsg")->valuestring);
				m_pILog->PrintLog(LOGLEVEL_ERR, "Mes返回上传烧录结果失败，错误原因为:%s ", strErrMsg);
			}
			if (cJSON_GetObjectItem(pRootParser, "errCode") != NULL) {
				strErrCode.Format("%s", cJSON_GetObjectItem(pRootParser, "errCode")->valuestring);
			}
		}
	}
	catch (...) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "CommitProgramRet2Mes HttpPost Fail strURL: %s", strURL);
		goto __end;
	}
	Ret = 0;

__end:
	if (pRootParser != NULL) {
		cJSON_Delete(pRootParser);
		pRootParser = NULL;
	}
	return Ret;
}

INT CMesInterface::CommitProgramRet2Mes(CString strLastJson)
{
	if (!m_bInited) {
		return -1;
	}
	//if (m_mesInfo.workOrder.IsEmpty() || m_mesInfo.materialID.IsEmpty()) {
	//	m_pILog->PrintLog(LOGLEVEL_ERR, "CMesInterface WorkOrder/MaterialID IsEmpty");
	//	return -1;
	//}

	// 读取API密钥
	CString strApiKey = _T("");
	CString strApiKeyPath;
	strApiKeyPath.Format("%s\\APISLAK.txt", GetCurrentPath());
	CFile apiKeyFile;
	if (apiKeyFile.Open(strApiKeyPath, CFile::modeRead | CFile::shareDenyNone)) {
		UINT fileLen = (UINT)apiKeyFile.GetLength();
		if (fileLen > 0) {
			char* pBuffer = new char[fileLen + 1];
			memset(pBuffer, 0, fileLen + 1);
			apiKeyFile.Read(pBuffer, fileLen);
			strApiKey.Format("%s", pBuffer);
			strApiKey.Trim();
			delete[] pBuffer;
		}
		apiKeyFile.Close();
		m_pILog->PrintLog(LOGLEVEL_LOG, "成功读取API密钥文件: %s", strApiKeyPath);
	}
	else {
		m_pILog->PrintLog(LOGLEVEL_ERR, "无法读取API密钥文件: %s", strApiKeyPath);
		return -1;
	}

	INT Ret = -1;
	int nHttpRet = 0;
	CHttpClient Client;
	CString strURL;
	CString strResponse;
	CString strErrMsg;

	cJSON* pRootParser = NULL;
	cJSON* pResult = NULL;
	cJSON* pStatus = NULL;
	cJSON* RootBuild = NULL;
	cJSON* SiteQtyArray = NULL;
	cJSON* SiteObj = NULL;

	CString strHeader;
	CString strBody;
	CString strBuildJson;
	CString strErrCode;
	int nRetryCnt = 3;

	try {
		//m_pILog->PrintLog(LOGLEVEL_ERR, "YieldChange返回的数据%s ", strJson);
		RootBuild = cJSON_Parse(strLastJson.GetBuffer());

		if (RootBuild == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析烧录结果的信息不符合json格式:%s", strLastJson);
			goto __end;
		}

		// 使用新的MES接口URL（图片中的格式）
		strURL.Format("%s/zgyb/ihtml?msclass=SAPP&servclass=api.SYBACQTY&weblis=api.Request", m_strBaseUrl);

		// 创建SiteQty数组
		SiteQtyArray = cJSON_CreateArray();
		SiteObj = cJSON_CreateObject();
		
		// 从原始JSON中提取数据
		cJSON* pSiteSN = cJSON_GetObjectItem(RootBuild, "SiteSN");
		cJSON* pSiteAlias = cJSON_GetObjectItem(RootBuild, "SiteAlias");
		cJSON* pTotal = cJSON_GetObjectItem(RootBuild, "Total");
		cJSON* pPass = cJSON_GetObjectItem(RootBuild, "Pass");
		cJSON* pFail = cJSON_GetObjectItem(RootBuild, "Fail");
		cJSON* pFailReason = cJSON_GetObjectItem(RootBuild, "FailReason");
		cJSON* pFuncmode = cJSON_GetObjectItem(RootBuild, "funcmode");
		
		// 按图片顺序添加字段到SiteObj
		// 1. OrderName - 工单号
		cJSON_AddStringToObject(SiteObj, "OrderName", (LPSTR)(LPCSTR)m_mesInfo.workOrder);
		
		// 2. MaterialID - 物料号
		cJSON_AddStringToObject(SiteObj, "MaterialID", (LPSTR)(LPCSTR)m_mesInfo.materialID);
		
		// 3. Funcmode - 执行功能（从原JSON获取或使用默认值）
		if (pFuncmode != NULL && cJSON_IsString(pFuncmode)) {
			cJSON_AddStringToObject(SiteObj, "Funcmode", pFuncmode->valuestring);
		} else {
			cJSON_AddStringToObject(SiteObj, "Funcmode", m_mesInfo.curExec);
		}
		
		// 4. SiteSN - 烧录器序列号
		if (pSiteSN != NULL && cJSON_IsString(pSiteSN)) {
			cJSON_AddStringToObject(SiteObj, "SiteSN", pSiteSN->valuestring);
		} else {
			cJSON_AddStringToObject(SiteObj, "SiteSN", "");
		}
		
		// 5. SiteAlias - 烧录器名称
		if (pSiteAlias != NULL && cJSON_IsString(pSiteAlias)) {
			cJSON_AddStringToObject(SiteObj, "SiteAlias", pSiteAlias->valuestring);
		} else {
			cJSON_AddStringToObject(SiteObj, "SiteAlias", "");
		}
		
		// 6. Total - 烧录数量
		if (pTotal != NULL && cJSON_IsNumber(pTotal)) {
			cJSON_AddNumberToObject(SiteObj, "Total", pTotal->valueint);
		} else {
			cJSON_AddNumberToObject(SiteObj, "Total", 0);
		}
		
		// 7. Fail - 失败数量
		if (pFail != NULL && cJSON_IsNumber(pFail)) {
			cJSON_AddNumberToObject(SiteObj, "Fail", pFail->valueint);
		} else {
			cJSON_AddNumberToObject(SiteObj, "Fail", 0);
		}
		
		// 8. Pass - 合格数量
		if (pPass != NULL && cJSON_IsNumber(pPass)) {
			cJSON_AddNumberToObject(SiteObj, "Pass", pPass->valueint);
		} else {
			cJSON_AddNumberToObject(SiteObj, "Pass", 0);
		}
		
		// 9. StationId - 烧录器编号
		if (!m_strStationId.IsEmpty()) {
			int stationId = atoi(m_strStationId);
			if (stationId > 0) {
				cJSON_AddNumberToObject(SiteObj, "StationId", stationId);
			} else {
		cJSON_AddNumberToObject(SiteObj, "StationId", 10010);
			}
		} else {
			cJSON_AddNumberToObject(SiteObj, "StationId", 10010);
		}
		
		// 10. ProjectName - 工程文件名
		cJSON_AddStringToObject(SiteObj, "ProjectName", m_mesInfo.projPath);
		
		// 11. ProjectChecksum - 工程文件校验值
		cJSON_AddStringToObject(SiteObj, "ProjectChecksum", m_mesInfo.projChecksum);
		
		// 12. TaskName - 自动化任务文件名
		cJSON_AddStringToObject(SiteObj, "TaskName", m_mesInfo.autoTaskFilePath);
		
		// 13. SoftVersion - 版本号
		if (!m_mesInfo.projVersion.IsEmpty()) {
			cJSON_AddStringToObject(SiteObj, "SoftVersion", m_mesInfo.projVersion);
		} else {
			cJSON_AddStringToObject(SiteObj, "SoftVersion", "1.0.0");
		}
		
		// 14. 额外参数
		if (!m_strBoxSN.IsEmpty()) {
			cJSON_AddStringToObject(SiteObj, "box_sn", (LPSTR)(LPCSTR)m_strBoxSN);
		}
		if (!m_strBatNo.IsEmpty()) {
			cJSON_AddStringToObject(SiteObj, "bat_no", (LPSTR)(LPCSTR)m_strBatNo);
		}
		if (!m_strRsNo.IsEmpty()) {
			cJSON_AddStringToObject(SiteObj, "rs_no", (LPSTR)(LPCSTR)m_strRsNo);
		}
		if (!m_strWkNo.IsEmpty()) {
			cJSON_AddStringToObject(SiteObj, "wk_no", (LPSTR)(LPCSTR)m_strWkNo);
		}
		
		// 15. FailReason - 失败原因数组
		if (pFailReason != NULL && cJSON_IsArray(pFailReason)) {
			cJSON_AddItemToObject(SiteObj, "FailReason", cJSON_Duplicate(pFailReason, 1));
		} else {
			cJSON_AddArrayToObject(SiteObj, "FailReason");
		}

		// 将SiteObj添加到数组
		cJSON_AddItemToArray(SiteQtyArray, SiteObj);
		
		// 删除旧的RootBuild，创建新的
		cJSON_Delete(RootBuild);
		RootBuild = cJSON_CreateObject();
		cJSON_AddItemToObject(RootBuild, "SiteQty", SiteQtyArray);

		strBuildJson = cJSON_Print(RootBuild);
		strBody.Format("%s", strBuildJson);

		//Change to UTF8
		strBody = MByteStrToUtf8CStr(strBody);

		strResponse.Empty();
		// 添加API密钥到请求头（使用Authorization）
		strHeader.Format("Content-Type:application/json;charset=UTF-8\r\nAuthorization:%s\r\n", strApiKey);

		nHttpRet = Client.HttpPost(strURL, strHeader, strBody, strResponse);
		m_pILog->PrintLog(LOGLEVEL_LOG, "UploadProgramRet2Mes strURL=%s,strHeader=%s, strBody=%s, strResponse=%s, HttpPost Ret=%d",
			strURL, strHeader, strBody, strResponse, nHttpRet);
		if (nHttpRet != 0) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "HTTP请求失败, 错误码=%d", nHttpRet);
			goto __end;
		}

		pRootParser = cJSON_Parse(strResponse.GetBuffer());
		if (pRootParser == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "Mes返回的不符合Json数据格式 ");
			goto __end;
		}

		// 检查Status字段（根据图片中的响应格式）
		pStatus = cJSON_GetObjectItem(pRootParser, "Status");
		if (pStatus != NULL) {
			// 如果返回Status字段，使用Status判断
			if (!cJSON_IsBool(pStatus) || !cJSON_IsTrue(pStatus)) {
				cJSON* pItemObj = cJSON_GetObjectItem(pRootParser, "Message");
				if (pItemObj != NULL && cJSON_IsString(pItemObj)) {
					strErrMsg.Format("%s", pItemObj->valuestring);
				}
				else {
					strErrMsg = "未知错误";
				}
				m_pILog->PrintLog(LOGLEVEL_ERR, "Mes返回上传烧录结果失败，错误原因为:%s ", strErrMsg);
				goto __end;
			}
		}
		else {
			// 兼容原有的Result字段判断
			pResult = cJSON_GetObjectItem(pRootParser, "Result");
			if (pResult == NULL) {
				m_pILog->PrintLog(LOGLEVEL_ERR, "解析Mes返回的Result字段错误，请确认Mes维护的字段信息, ");
				goto __end;
			}

			if (!cJSON_IsNumber(pResult) || pResult->valueint != 1) {
				if (cJSON_GetObjectItem(pRootParser, "errMsg") != NULL) {
					strErrMsg.Format("%s", cJSON_GetObjectItem(pRootParser, "errMsg")->valuestring);
					m_pILog->PrintLog(LOGLEVEL_ERR, "Mes返回上传烧录结果失败，错误原因为:%s ", strErrMsg);
				}
				if (cJSON_GetObjectItem(pRootParser, "errCode") != NULL) {
					strErrCode.Format("%s", cJSON_GetObjectItem(pRootParser, "errCode")->valuestring);
				}
				goto __end;
			}
		}
		
		m_pILog->PrintLog(LOGLEVEL_LOG, "上传烧录结果成功");
	}
	catch (...) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "CommitProgramRet2Mes HttpPost Fail strURL: %s", strURL);
		goto __end;
	}
	Ret = 0;

__end:
	if (RootBuild != NULL) {
		cJSON_Delete(RootBuild);
		RootBuild = NULL;
	}
	if (pRootParser != NULL) {
		cJSON_Delete(pRootParser);
		pRootParser = NULL;
	}
	return Ret;
}


/////ACMES：上传结果接口
INT CMesInterface::CommitProgramRet2ACMes(CString strLastJson)
{
	if (!m_bInited) {
		return -1;
	}

	// 读取API密钥
	CString strApiKey = _T("");
	CString strApiKeyPath;
	strApiKeyPath.Format("%s\\APISLAK.txt", GetCurrentPath());
	CFile apiKeyFile;
	if (apiKeyFile.Open(strApiKeyPath, CFile::modeRead | CFile::shareDenyNone)) {
		UINT fileLen = (UINT)apiKeyFile.GetLength();
		if (fileLen > 0) {
			char* pBuffer = new char[fileLen + 1];
			memset(pBuffer, 0, fileLen + 1);
			apiKeyFile.Read(pBuffer, fileLen);
			strApiKey.Format("%s", pBuffer);
			strApiKey.Trim();
			delete[] pBuffer;
		}
		apiKeyFile.Close();
		m_pILog->PrintLog(LOGLEVEL_LOG, "成功读取API密钥文件: %s", strApiKeyPath);
	}
	else {
		m_pILog->PrintLog(LOGLEVEL_ERR, "无法读取API密钥文件: %s", strApiKeyPath);
		return -1;
	}

	INT Ret = -1;
	int nHttpRet = 0;
	CHttpClient Client;
	CString strURL;
	CString strResponse;
	CString strErrMsg;

	cJSON* pRootParser = NULL;
	cJSON* pResult = NULL;
	cJSON* pStatus = NULL;
	cJSON* RootBuild = NULL;

	CString strHeader;
	CString strBody;
	CString strBuildJson;
	CString strErrCode;
	int nRetryCnt = 3;

	try {
		//m_pILog->PrintLog(LOGLEVEL_ERR, "YieldChange返回的数据%s ", strJson);
		RootBuild = cJSON_Parse(strLastJson.GetBuffer());

		if (RootBuild == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析烧录结果的信息不符合json格式:%s", strLastJson);
			goto __end;
		}

		// 使用图片中显示的URL格式
		strURL.Format("%s/zqyb/ihtml?msclass=$APP&servclass=api.SYBACQTY&weblis=api.Request", m_strBaseUrl);

		if (cJSON_GetObjectItem(RootBuild, "FailReason") == NULL) {
			cJSON_AddArrayToObject(RootBuild, "FailReason");
		}

		cJSON_AddNumberToObject(RootBuild, "Id", nOrderID);
		cJSON_AddStringToObject(RootBuild, "TaskName", m_mesInfo.autoTaskFilePath);
		cJSON_AddStringToObject(RootBuild, "ProjectChecksum", m_mesInfo.projChecksum);
		//cJSON_AddStringToObject(RootBuild, "StationId", (LPSTR)(LPCSTR)m_strStationId);
		cJSON_AddNumberToObject(RootBuild, "StationId", 1001);
		cJSON_AddStringToObject(RootBuild, "Funcmode", m_mesInfo.curExec);
		cJSON_AddStringToObject(RootBuild, "ProjectName", m_mesInfo.projPath);
		cJSON_AddStringToObject(RootBuild, "OrderName", (LPSTR)(LPCSTR)/*m_ProgRecord.DestRecord.*/m_mesInfo.workOrder);
		cJSON_AddStringToObject(RootBuild, "MaterialID", (LPSTR)(LPCSTR)/*m_ProgRecord.DestRecord.*/m_mesInfo.materialID);
		cJSON_AddStringToObject(RootBuild, "SoftVersion", "V_SoftWare_1.0.1");
		
		// 新增：自动添加额外字段到烧录结果上传（ACMES）
		if (!m_strBoxSN.IsEmpty()) {
			cJSON_AddStringToObject(RootBuild, "box_sn", (LPSTR)(LPCSTR)m_strBoxSN);
		}
		if (!m_strBatNo.IsEmpty()) {
			cJSON_AddStringToObject(RootBuild, "bat_no", (LPSTR)(LPCSTR)m_strBatNo);
		}
		if (!m_strRsNo.IsEmpty()) {
			cJSON_AddStringToObject(RootBuild, "rs_no", (LPSTR)(LPCSTR)m_strRsNo);
		}
		if (!m_strWkNo.IsEmpty()) {
			cJSON_AddStringToObject(RootBuild, "wk_no", (LPSTR)(LPCSTR)m_strWkNo);
		}


		strBuildJson = cJSON_Print(RootBuild);
		strBody.Format("%s", strBuildJson);

		//Change to UTF8
		strBody = MByteStrToUtf8CStr(strBody);

		strResponse.Empty();
		// 在请求头中添加API密钥
		strHeader.Format("Content-Type:application/json;charset=UTF-8\r\nAPISLAK: %s\r\n", strApiKey);

		nHttpRet = Client.HttpPost(strURL, strHeader, strBody, strResponse);
		m_pILog->PrintLog(LOGLEVEL_LOG, "UploadProgramRet2ACMes strURL=%s,strHeader=%s, strBody=%s, strResponse=%s, HttpPost Ret=%d",
			strURL, strHeader, strBody, strResponse, nHttpRet);
		if (nHttpRet != 0) {
			if (nHttpRet == 1) {
			}
			else if (nHttpRet == 2) {
			}
			goto __end;
		}

		pRootParser = cJSON_Parse(strResponse.GetBuffer());
		if (pRootParser == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "Mes返回的不符合Json数据格式 ");
			goto __end;
		}

		// 检查Status字段（根据图片中的响应格式）
		pStatus = cJSON_GetObjectItem(pRootParser, "Status");
		if (pStatus != NULL) {
			// 如果返回Status字段，使用Status判断
			if (!cJSON_IsBool(pStatus) || !cJSON_IsTrue(pStatus)) {
				cJSON* pItemObj = cJSON_GetObjectItem(pRootParser, "Message");
				if (pItemObj != NULL && cJSON_IsString(pItemObj)) {
					strErrMsg.Format("%s", pItemObj->valuestring);
				}
				else {
					strErrMsg = "未知错误";
				}
				m_pILog->PrintLog(LOGLEVEL_ERR, "Mes返回上传烧录结果失败，错误原因为:%s ", strErrMsg);
				goto __end;
			}
		}
		else {
			// 兼容原有的Code字段判断
			pResult = cJSON_GetObjectItem(pRootParser, "Code");
			if (pResult == NULL) {
				m_pILog->PrintLog(LOGLEVEL_ERR, "解析Mes返回的Code字段错误，请确认Mes维护的字段信息, ");
				goto __end;
			}

			if (!cJSON_IsNumber(pResult) || pResult->valueint != 0) {
				if (cJSON_GetObjectItem(pRootParser, "Message") != NULL) {
					strErrMsg.Format("%s", cJSON_GetObjectItem(pRootParser, "Message")->valuestring);
					m_pILog->PrintLog(LOGLEVEL_ERR, "Mes返回上传烧录结果失败，错误原因为:%s ", strErrMsg);
				}
				goto __end;
			}
		}
	}
	catch (...) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "CommitProgramRet2ACMes HttpPost Fail strURL: %s", strURL);
		goto __end;
	}
	Ret = 0;

__end:
	if (RootBuild != NULL) {
		cJSON_Delete(RootBuild);
		RootBuild = NULL;
	}
	if (pRootParser != NULL) {
		cJSON_Delete(pRootParser);
		pRootParser = NULL;
	}
	return Ret;
}
