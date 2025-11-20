#include "stdafx.h"
#include "Setting.h"
#include "cJSON.h"
#include "ComTool.h"

CSetting::CSetting()
	:m_strErrMsg("")
{
}


CSetting::~CSetting()
{
}

BOOL CSetting::SetJsonPath(CString strJsonPath)
{
	m_strSettingJsonPath = strJsonPath;
	return TRUE;
}

BOOL CSetting::Save()
{
	BOOL Ret = TRUE;
	char *strTmp = NULL;
	m_strErrMsg = "";

	cJSON * Root = cJSON_CreateObject();
	cJSON_AddStringToObject(Root, "ProgFileFolder", (LPSTR)(LPCSTR)strProgFileFolder);
	cJSON_AddStringToObject(Root, "ProjTemplateFolder", (LPSTR)(LPCSTR)strProjTemplateFolder);
	cJSON_AddStringToObject(Root, "ProjSaveFolder", (LPSTR)(LPCSTR)strProjSaveFolder);
	cJSON_AddStringToObject(Root, "CurExec", (LPSTR)(LPCSTR)strCurExec);
	cJSON_AddStringToObject(Root, "AutoTaskFolder", (LPSTR)(LPCSTR)strAutoTaskFolder);
	cJSON_AddStringToObject(Root, "ACServerFolder", (LPSTR)(LPCSTR)strACServerFolder);
	cJSON_AddStringToObject(Root, "ReportFolder", (LPSTR)(LPCSTR)strReportFolder);
	cJSON_AddNumberToObject(Root, "ElectricInsertionCheck", nElectricInsertCheck);
	cJSON_AddStringToObject(Root, "strWorkOrder", (LPSTR)(LPCSTR)strWorkOrder);
	cJSON_AddStringToObject(Root, "strOperator", (LPSTR)(LPCSTR)strOperator);
	cJSON_AddNumberToObject(Root, "MaxProduceValue", MaxProduceValue);
	cJSON_AddStringToObject(Root, "AutoTaskFileExt", (LPSTR)(LPCSTR)strAutoTaskFileExt);
	cJSON_AddNumberToObject(Root, "AutoTaskLoadCmd", nAutoTaskLoadCmd);

	cJSON_AddStringToObject(Root, "WebServiceInterface", (LPSTR)(LPCSTR)strWebServiceInterface);
	cJSON_AddStringToObject(Root, "MESMode", (LPSTR)(LPCSTR)strMesWordMode);
	cJSON_AddStringToObject(Root, "ProjectMode", (LPSTR)(LPCSTR)strProjectMode);
	cJSON_AddStringToObject(Root, "AutoMode", (LPSTR)(LPCSTR)strAutoMode);
	cJSON_AddStringToObject(Root, "AutomaticType", (LPSTR)(LPCSTR)strAutomaticType);
	cJSON_AddStringToObject(Root, "ReportURL", (LPSTR)(LPCSTR)strReportURL);

	cJSON_AddNumberToObject(Root, "EnBarPrint", nEnBarPrint);
	cJSON_AddNumberToObject(Root, "ScanComPort", nScanComPort);


	cJSON_AddNumberToObject(Root, "ReInitIndex", nReInitIndex);
	cJSON_AddNumberToObject(Root, "StartIndex", nStartIndex);
	cJSON_AddNumberToObject(Root, "SyslogEnable", nSyslogEnable);
	cJSON_AddNumberToObject(Root, "TotalNum", nTotalNum);
	cJSON_AddNumberToObject(Root, "ProgramIndex", nProgramIndex);

	cJSON_AddStringToObject(Root, "ItemNum", (LPSTR)(LPCSTR)strItemNum);
	cJSON_AddStringToObject(Root, "SoftVer", (LPSTR)(LPCSTR)strSoftVer);
	cJSON_AddStringToObject(Root, "Manufactor", (LPSTR)(LPCSTR)strManufactor);
	cJSON_AddStringToObject(Root, "IcNum", (LPSTR)(LPCSTR)strIcNum);
	cJSON_AddStringToObject(Root, "DeviceName", (LPSTR)(LPCSTR)strDeviceName);
	cJSON_AddStringToObject(Root, "SoftMainVer", (LPSTR)(LPCSTR)strSoftMainVer);

	cJSON_AddStringToObject(Root, "Account", (LPSTR)(LPCSTR)strAccount);
	cJSON_AddStringToObject(Root, "Pwd", (LPSTR)(LPCSTR)strPwd);
	cJSON_AddStringToObject(Root, "UseCode", (LPSTR)(LPCSTR)strUseCode);
	cJSON_AddStringToObject(Root, "Serviceproviderid", (LPSTR)(LPCSTR)strServiceproviderid);

	cJSON_AddNumberToObject(Root, "CheckDay", nCheckDay);
	cJSON_AddNumberToObject(Root, "LocalServer", nLocalServer);
	cJSON_AddNumberToObject(Root, "ServerTest", nServerTest);
	cJSON_AddNumberToObject(Root, "DoHttpTest", nDoHttpTest);
	cJSON_AddStringToObject(Root, "WorkStationID", (LPSTR)(LPCSTR)strWorkStationID);

	cJSON_AddNumberToObject(Root, "EnPrint", nEnPrint);
	cJSON_AddNumberToObject(Root, "PrintPort", nPrintPort);
	cJSON_AddStringToObject(Root, "PrintIP", (LPSTR)(LPCSTR)strPrintIP);
	cJSON_AddStringToObject(Root, "ProgramMode", (LPSTR)(LPCSTR)strProgramMode);
	cJSON_AddStringToObject(Root, "UfsWriterFolder", (LPSTR)(LPCSTR)strUfsWriterFolder);

	cJSON_AddStringToObject(Root, "ModuleName", (LPSTR)(LPCSTR)strModuleName);

	strTmp = cJSON_Print(Root);
	cJSON_Delete(Root);

	CStdioFile StdFile;
	if (StdFile.Open(m_strSettingJsonPath, CFile::modeWrite | CFile::modeCreate, NULL) == FALSE) {
		m_strErrMsg.Format("保存设置错误: 打开Json配置文件出错，路径:%s", m_strSettingJsonPath);
		Ret = FALSE; goto __end;
	}

	StdFile.WriteString(strTmp);

	StdFile.Flush();
	StdFile.Close();
__end:
	if (strTmp) {
		cJSON_free(strTmp);
	}
	return Ret;
}

BOOL CSetting::Load()
{
	BOOL Ret = TRUE;
	CFile File;
	INT FileLen;
	cJSON*Root = NULL;
	BYTE *pTmpData = NULL;
	m_strErrMsg = "";
	if (File.Open(m_strSettingJsonPath, CFile::modeRead | CFile::shareDenyNone, NULL) == FALSE) {
		m_strErrMsg.Format("加载设置错误: 打开Json配置文件出错，路径:%s", m_strSettingJsonPath);
		return FALSE;
	}

	FileLen = (INT)File.GetLength();
	pTmpData = new BYTE[FileLen];
	if (!pTmpData) {
		m_strErrMsg.Format("加载设置错误: 分配内存错误，长度:%d Bytes", FileLen);
		Ret = FALSE; goto __end;
	}

	File.Read(pTmpData, FileLen);

	Root = cJSON_Parse((char*)pTmpData);
	if (Root == NULL) {
		m_strErrMsg.Format("加载设置错误: 解析Json字符串错误");
		Ret = FALSE; goto __end;
	}

	strProgFileFolder.Format("%s", cJSON_GetObjectItem(Root, "ProgFileFolder")->valuestring);
	strProjTemplateFolder.Format("%s", cJSON_GetObjectItem(Root, "ProjTemplateFolder")->valuestring);
	strProjSaveFolder.Format("%s", cJSON_GetObjectItem(Root, "ProjSaveFolder")->valuestring);
	strCurExec.Format("%s", cJSON_GetObjectItem(Root, "CurExec")->valuestring);
	strAutoTaskFolder.Format("%s", cJSON_GetObjectItem(Root, "AutoTaskFolder")->valuestring);
	strACServerFolder.Format("%s", cJSON_GetObjectItem(Root, "ACServerFolder")->valuestring);
	strReportFolder.Format("%s", cJSON_GetObjectItem(Root, "ReportFolder")->valuestring);
	nElectricInsertCheck = cJSON_GetObjectItem(Root, "ElectricInsertionCheck")->valueint;
	strWorkOrder.Format("%s", cJSON_GetObjectItem(Root, "strWorkOrder")->valuestring);
	strOperator.Format("%s", cJSON_GetObjectItem(Root, "strOperator")->valuestring);
	MaxProduceValue = cJSON_GetObjectItem(Root, "MaxProduceValue")->valuedouble;
	strAutoTaskFileExt.Format("%s", cJSON_GetObjectItem(Root, "AutoTaskFileExt")->valuestring);
	nAutoTaskLoadCmd = cJSON_GetObjectItem(Root, "AutoTaskLoadCmd")->valueint;

	strWebServiceInterface.Format("%s", cJSON_GetObjectItem(Root, "WebServiceInterface")->valuestring);
	strMesWordMode.Format("%s", cJSON_GetObjectItem(Root, "MESMode")->valuestring);
	strProjectMode.Format("%s", cJSON_GetObjectItem(Root, "ProjectMode")->valuestring);

	if (cJSON_GetObjectItem(Root, "ProgramMode") != NULL) {
		strProgramMode.Format("%s", cJSON_GetObjectItem(Root, "ProgramMode")->valuestring);
	}
	else {
		strProgramMode.Format("%s", "Program");
	}
	if (cJSON_GetObjectItem(Root, "AutoMode") != NULL) {
		strAutoMode.Format("%s", cJSON_GetObjectItem(Root, "AutoMode")->valuestring);
	}
	else {
		strAutoMode.Format("%s", "Auto");//default value is Auto
	}
	//自动机类型
	if (cJSON_GetObjectItem(Root, "AutomaticType") != NULL) {
		strAutomaticType.Format("%s", cJSON_GetObjectItem(Root, "AutomaticType")->valuestring);
	}
	else {
		strAutomaticType.Format("%s", "IPS5000");//default value IPS5000
	}

	if (cJSON_GetObjectItem(Root, "CheckDay") != NULL) {
		nCheckDay = cJSON_GetObjectItem(Root, "CheckDay")->valueint;
	}
	else {
		nCheckDay = 7;//default value is 7
	}
	if (cJSON_GetObjectItem(Root, "LocalServer") != NULL) {
		nLocalServer = cJSON_GetObjectItem(Root, "LocalServer")->valueint;
	}
	else {
		nLocalServer = 0;
	}
	if (cJSON_GetObjectItem(Root, "ServerTest") != NULL) {
		nServerTest = cJSON_GetObjectItem(Root, "ServerTest")->valueint;
	}
	else {
		nServerTest = 0;
	}
	if (cJSON_GetObjectItem(Root, "DoHttpTest") != NULL) {
		nDoHttpTest = cJSON_GetObjectItem(Root, "DoHttpTest")->valueint;
	}
	else {
		nDoHttpTest = 0;
	}

	if (cJSON_GetObjectItem(Root, "WorkStationID") != NULL) {
		strWorkStationID.Format("%s", cJSON_GetObjectItem(Root, "WorkStationID")->valuestring);
	}
	else {
		strWorkStationID.Format("%s", "1001");//default value is 1001
	}

	strReportURL.Format("%s", cJSON_GetObjectItem(Root, "ReportURL")->valuestring);

	nEnPrint = cJSON_GetObjectItem(Root, "EnPrint")->valueint;
	strPrintIP.Format("%s", cJSON_GetObjectItem(Root, "PrintIP")->valuestring);
	nPrintPort = cJSON_GetObjectItem(Root, "PrintPort")->valueint;

	nEnBarPrint = cJSON_GetObjectItem(Root, "EnBarPrint")->valueint;

	nScanComPort = cJSON_GetObjectItem(Root, "ScanComPort")->valueint;

	nReInitIndex = cJSON_GetObjectItem(Root, "ReInitIndex")->valueint;
	nStartIndex = cJSON_GetObjectItem(Root, "StartIndex")->valueint;
	if (cJSON_GetObjectItem(Root, "SyslogEnable") != NULL) {
		nSyslogEnable = cJSON_GetObjectItem(Root, "SyslogEnable")->valueint;
	}
	else {
		nSyslogEnable = 0;//default is disable.
	}
	nTotalNum = cJSON_GetObjectItem(Root, "TotalNum")->valueint;
	nProgramIndex = cJSON_GetObjectItem(Root, "ProgramIndex")->valueint;

	strItemNum.Format("%s", cJSON_GetObjectItem(Root, "ItemNum")->valuestring);
	strSoftVer.Format("%s", cJSON_GetObjectItem(Root, "SoftVer")->valuestring);
	if (cJSON_GetObjectItem(Root, "Manufactor") != NULL) {
		strManufactor.Format("%s", cJSON_GetObjectItem(Root, "Manufactor")->valuestring);
	}
	else {
		strManufactor = "Com";
	}
	strIcNum.Format("%s", cJSON_GetObjectItem(Root, "IcNum")->valuestring);
	strDeviceName.Format("%s", cJSON_GetObjectItem(Root, "DeviceName")->valuestring);
	strSoftMainVer.Format("%s", cJSON_GetObjectItem(Root, "SoftMainVer")->valuestring);

	strAccount.Format("%s", cJSON_GetObjectItem(Root, "Account")->valuestring);
	strPwd.Format("%s", cJSON_GetObjectItem(Root, "Pwd")->valuestring);
	strUseCode.Format("%s", cJSON_GetObjectItem(Root, "UseCode")->valuestring);
	strServiceproviderid.Format("%s", cJSON_GetObjectItem(Root, "Serviceproviderid")->valuestring);

	////////////////////////////////
	if (strProjSaveFolder == "") {
		strProjSaveFolder.Format("%s\\ProjSave", ::GetCurrentPath());
	}

	if (strReportFolder == "") {
		strReportFolder.Format("%s\\Report", ::GetCurrentPath());
	}

	if (strAutoTaskFolder == "") {
		strAutoTaskFolder.Format("%s\\AutoTask", ::GetCurrentPath());
	}

	if (strACServerFolder == "") {
		strACServerFolder = "C:\\ACROVIEW\\MultiAprog";
	}

	if (cJSON_GetObjectItem(Root, "UfsWriterFolder") != NULL) {
		strUfsWriterFolder.Format("%s", cJSON_GetObjectItem(Root, "UfsWriterFolder")->valuestring);
	}
	else {
		strUfsWriterFolder.Format("%s", "D:\\UFSWriterSoftware\\UFSWriter-V1220");//default value is 1001
	}

	if (cJSON_GetObjectItem(Root, "ModuleName") != NULL) {
		strModuleName.Format("%s", cJSON_GetObjectItem(Root, "ModuleName")->valuestring);
	}
	else {
		strModuleName.Format("%s", "LC610N");
	}

__end:
	if (Root) {
		cJSON_Delete(Root);
	}
	if (pTmpData) {
		delete[] pTmpData;
	}
	File.Close();
	return Ret;
}
