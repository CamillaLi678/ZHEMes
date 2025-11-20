#include "stdafx.h"
#include "ThirdPartyMesAccess.h"
#include "ComTool.h"
#include "cJSON.h"
#include "MesCommon.h"

#define MAX_PARAM_LEN 2048

#define CHECK_INTERFACE(_FName) \
do{if (m_ThirdPartyMesDll.pfn##_FName == NULL) {m_pILog->PrintLog(LOGLEVEL_ERR, "未实现接口: %s\r\n", ""#_FName); }\
} while (0);

CThirdPartyMesAccess::CThirdPartyMesAccess()
{
}


CThirdPartyMesAccess::~CThirdPartyMesAccess()
{
}

void CThirdPartyMesAccess::Init(ILog *log, CString strDllPath, CString tskFolder, CString aprFolder, BOOL isLocal) {

	DeInit();

	if (!m_bInited) {
		m_pILog = log;
		m_strAutoTaskFolder = tskFolder;
		m_strProgFileFolder = aprFolder;
		AttachDll(strDllPath, TRUE);
		m_bInited = TRUE;
		isLocalServer = isLocal;
	}
}

void CThirdPartyMesAccess::DeInit() {
	if (m_bInited) {
		m_bInited = FALSE;
		DetachDll();
		m_strAutoTaskFolder = _T("");
		m_strProgFileFolder = _T("");
	}
}

INT CThirdPartyMesAccess::GetTokenFromServer(){
	return 0;
}

INT CThirdPartyMesAccess::GetMesRecord(CString workOrder, CString materialID, CString strCurExec, MesInfo &mesResult) {
	if (m_ThirdPartyMesDll.pfnGetSoftVision == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "pfnGetSoftVision函数没找到");
		return -1;
	}

	/************************************************************************************
	函数：        GetSoftVision
	功能：        根据DID获取软件版本等信息
	参数：
	parDID[]             DID
	*retSoftVisionInfo   返回软件版本等信息,格式:qty:数量|shoporder:工单|datecode:DATE_CODE|lotcode:LOT_CODE|ProductId:料号|softvision:软件版本|Model:Model|manufacturer:AC|LibraryName:LibraryName
	*retMessage          返回的错误消息
	返回：        如果成功则返回1，否则返回0
	作者：        BYD 电子事业群第九事业部 ZQ 2022年10月28日
	**********************************************************************
	Data_API int GetSoftVision(char parDID[], char shoporder[], char *retSoftVisionInfo, char *retMessage);*/
	memset(parDID, 0, sizeof(parDID));
	strncpy(parDID, materialID.GetBuffer(), materialID.GetLength());
	materialID.ReleaseBuffer();

	memset(m_strShoporder, 0, sizeof(m_strShoporder));
	strncpy(m_strShoporder, workOrder.GetBuffer(), workOrder.GetLength());
	workOrder.ReleaseBuffer();

	char retSoftVisionInfo[MAX_PARAM_LEN] = { 0 };
	char retMessage[MAX_PARAM_LEN] = { 0 };
	CStringArray strInfoArray;
	CString strRetSoftVisionInfo;
	mesResult.errCode = 0;
	mesResult.materialID = materialID;
	mesResult.curExec = strCurExec;
	CString logMsg = _T("");
	if (!isLocalServer) {
		logMsg.Format("GetSoftVision函数指针 %p parDID:%s Shoporder: %s", m_ThirdPartyMesDll.pfnGetSoftVision, parDID, m_strShoporder);
		m_pILog->PrintLog(LOGLEVEL_ERR, "%s", logMsg);
		//AfxMessageBox(logMsg);

		INT ret = m_ThirdPartyMesDll.pfnGetSoftVision(parDID, m_strShoporder, retSoftVisionInfo, retMessage);
		if (ret == 0) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "pfnGetSoftVision函数返回错误 retMessage: %s", retMessage);
			return -1;
		}

		strRetSoftVisionInfo.Format("%s", retSoftVisionInfo);
	}
	else {
		strRetSoftVisionInfo.Format("%s", "qty:800|shoporder:5030017587|datecode:20221019|lotcode:|ProductId:1221|LibraryName:ADAS|softvision:1.0.1|Model:ADASSN");
	}
	
	logMsg.Format("GetSoftVision函数 call OK retSoftVisionInfo %s", strRetSoftVisionInfo);
	m_pILog->PrintLog(LOGLEVEL_ERR, "%s", logMsg);
	//AfxMessageBox(logMsg);

	Split(strRetSoftVisionInfo, strInfoArray, "|");
	for (INT i = 0; i < strInfoArray.GetCount(); i++) {
		//去掉开始的空格
		strInfoArray[i] = strInfoArray[i].TrimLeft();
		if (strInfoArray[i].Find("Model:") == 0) {
			mesResult.Model = strInfoArray[i].Mid(strlen("Model:"));
		}
		else if (strInfoArray[i].Find("ProductId:") == 0) {
			mesResult.ProductId = strInfoArray[i].Mid(strlen("ProductId:"));
		}
		else if (strInfoArray[i].Find("shoporder:") == 0) {
			mesResult.workOrder = strInfoArray[i].Mid(strlen("shoporder:"));
		}
		else if (strInfoArray[i].Find("LibraryName:") == 0) {
			mesResult.LibraryName = strInfoArray[i].Mid(strlen("LibraryName:"));
		}
		else if (strInfoArray[i].Find("lotcode:") == 0) {
			mesResult.Lotcode = strInfoArray[i].Mid(strlen("lotcode:"));
		}
		else if (strInfoArray[i].Find("datecode:") == 0) {
			mesResult.Datecode = strInfoArray[i].Mid(strlen("datecode:"));
		}
		else if (strInfoArray[i].Find("manufacturer:") == 0) {
			mesResult.Manufacturer = strInfoArray[i].Mid(strlen("manufacturer:"));
		}
		else if (strInfoArray[i].Find("softvision:") == 0) {
			mesResult.projVersion = strInfoArray[i].Mid(strlen("softvision:"));
			mesResult.projPath = mesResult.projVersion;
			mesResult.autoTaskFilePath = mesResult.projVersion;
		}
		else if (strInfoArray[i].Find("qty:") == 0) {
			mesResult.workOrderICNum = atoi(strInfoArray[i].Mid(strlen("qty:")));
		}
	}

	m_MesResult.workOrderICNum = mesResult.workOrderICNum;
	m_MesResult.materialID = mesResult.materialID;
	m_MesResult.curExec = mesResult.curExec;

	m_MesResult.workOrder = mesResult.workOrder;
	m_MesResult.projVersion = mesResult.projVersion;
	m_MesResult.projPath = mesResult.projPath;

	m_MesResult.autoTaskFilePath = mesResult.autoTaskFilePath;
	m_MesResult.Datecode = mesResult.Datecode;
	m_MesResult.Lotcode = mesResult.Lotcode;
	m_MesResult.Manufacturer = mesResult.Manufacturer;
	m_MesResult.LibraryName = mesResult.LibraryName;

	m_MesResult.Model = mesResult.Model;
	m_MesResult.ProductId = mesResult.ProductId;

	logMsg.Format("GetSoftVision函数解析 shoporder %s qty %ld", m_MesResult.workOrder, mesResult.workOrderICNum);
	m_pILog->PrintLog(LOGLEVEL_ERR, "%s", logMsg);
	//AfxMessageBox(logMsg);

	return 0;
}

INT CThirdPartyMesAccess::CommitProgramerInfo2Mes(CString strJson){
	return 0;
}

INT CThirdPartyMesAccess::CommitProgramRet2Mes(CString strLastJson, UINT TotalCnt, UINT PassCnt, UINT FailCnt){
	m_pILog->PrintLog(LOGLEVEL_ERR, "CommitProgramRet2Mes PassCnt:%d", PassCnt);
	if (m_ThirdPartyMesDll.pfnStartAndPrintlabel == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "pfnStartAndPrintlabel函数没找到");
		return -1;
	}

	cJSON* pRootParser;
	pRootParser = cJSON_Parse(strLastJson.GetBuffer());

	INT Ret = -1;
	INT CurFail = 0;
	INT CurPass = 0;
	INT Reprint = 0;
	if (pRootParser == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "解析烧录结果的信息不符合json格式 ");
		return Ret;
	}

	if (cJSON_GetObjectItem(pRootParser, "CurFail") != NULL) {
		CurFail = cJSON_GetObjectItem(pRootParser, "CurFail")->valueint;
	}
	else {
		m_pILog->PrintLog(LOGLEVEL_ERR, "解析烧录结果的信息CurFail为空");
		return Ret;
	}
	if (cJSON_GetObjectItem(pRootParser, "CurPass") != NULL) {
		CurPass = cJSON_GetObjectItem(pRootParser, "CurPass")->valueint;
	}
	else {
		m_pILog->PrintLog(LOGLEVEL_ERR, "解析烧录结果的信息CurPass为空");
		return Ret;
	}

	/************************************************************************************
	函数：        StartAndPrintlabel
	功能：        根据DID生成SFC并打印相关信息
	参数：
	parDID[]             DID
	parQty[]			   数量
	parShoporder[]	   工单
	parProductID[]	   产品料号
	parSoftVision[]	   软件版本
	parDataCode[]		   DATE_CODE
	parLotCode[]		   LOT_CODE
	parModel[]		   号码库Model
	parLibrary[]		   号码库名称
	*retMessage          返回的错误消息
	返回：        如果成功则返回1，否则返回0
	作者：        BYD 电子事业群第九事业部 ZQ 2022年10月28日
	**********************************************************************
	Data_API int StartAndPrintlabel(char parDID[], char parQty[], char parShoporder[], 
		char parProductID[], char parSoftVision[], char parDataCode[], char parLotCode[], char parModel[], char parLibrary[], char manufacturer[], char *retMessage);
	*/
	CString strParQty = _T("");
	strParQty.Format("%d", PassCnt);//使用最后Pass数
	char * parQty = strParQty.GetBuffer();
	strParQty.ReleaseBuffer();

	char * parDID = m_MesResult.materialID.GetBuffer();
	m_MesResult.materialID.ReleaseBuffer();

	char * parShoporder = m_MesResult.workOrder.GetBuffer();
	m_MesResult.workOrder.ReleaseBuffer();

	char * parProductID = m_MesResult.ProductId.GetBuffer();
	m_MesResult.ProductId.ReleaseBuffer();
	char * parSoftVision = m_MesResult.projVersion.GetBuffer();
	m_MesResult.projVersion.ReleaseBuffer();
	char * parDataCode = m_MesResult.Datecode.GetBuffer();
	m_MesResult.Datecode.ReleaseBuffer();
	char * parLotCode = m_MesResult.Lotcode.GetBuffer();
	m_MesResult.Lotcode.ReleaseBuffer();
	char * parManufacturer = m_MesResult.Manufacturer.GetBuffer();
	m_MesResult.Manufacturer.ReleaseBuffer();
	char * parMode = m_MesResult.Model.GetBuffer();
	m_MesResult.Model.ReleaseBuffer();
	char * parLibrary = m_MesResult.LibraryName.GetBuffer();
	m_MesResult.LibraryName.ReleaseBuffer();

	char retSoftVisionInfo[MAX_PARAM_LEN] = { 0 };
	char retMessage[MAX_PARAM_LEN] = { 0 };
	CString logMsg = _T("");
	if (!isLocalServer) {
		logMsg.Format("StartAndPrintlabel函数指针 %p parDID:%s", m_ThirdPartyMesDll.pfnStartAndPrintlabel, parDID);
		m_pILog->PrintLog(LOGLEVEL_ERR, "%s", logMsg);

		INT ret = m_ThirdPartyMesDll.pfnStartAndPrintlabel(parDID, parQty, parShoporder, parProductID,
			parSoftVision, parDataCode, parLotCode, parMode, parLibrary, parManufacturer, retMessage);
		if (ret == 0) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "pfnStartAndPrintlabel函数返回错误 retMessage: %s", retMessage);
			//return -1;
		}
		m_pILog->PrintLog(LOGLEVEL_ERR, "pfnStartAndPrintlabel函数返回 retMessage: %s", retMessage);
	}

	m_pILog->PrintLog(LOGLEVEL_ERR, "pfnStartAndPrintlabel parDID: %s parQty: %s parShoporder: %s", parDID, parQty, parShoporder);
	m_pILog->PrintLog(LOGLEVEL_ERR, "parProductID: %s parSoftVision: %s parDataCode: %s ", parProductID, parSoftVision, parDataCode);
	m_pILog->PrintLog(LOGLEVEL_ERR, "parLotCode: %s parManufacturer:%s parMode: %s parLibrary: %s ", parLotCode, parManufacturer, parMode, parLibrary);
	return 0;
}

INT CThirdPartyMesAccess::CommitTaskInfo2Mes(CString timeStart, CString timeEnd, CString timeRun, CString timeStop) { 
	return 0; 
}

INT CThirdPartyMesAccess::CommitAlarmInfo2Mes(CString alarmCode, CString alarmMsg,
	CString alarmcStartTime, CString alarmKillTime, int alarmFlag) {
	return 0;
}

BOOL CThirdPartyMesAccess::AttachDll(CString strDllPath,BOOL bSet)
{
	BOOL Ret = TRUE;
	if (bSet == FALSE) {
		m_strDllPath.Format("%s\\%s", ::GetModulePath(NULL), strDllPath);
	}
	else {
		m_strDllPath = strDllPath;
	}
	hLib = LoadLibrary(m_strDllPath); //
	if (hLib == NULL) {
		DWORD ErrNo = GetLastError();
		m_pILog->PrintLog(LOGLEVEL_ERR, "打开%s出错，错误码:%d", strDllPath, ErrNo);
		Ret =FALSE; goto __end;
	}

	m_ThirdPartyMesDll.pfnGetSoftVision = (FnGetSoftVision)GetProcAddress(hLib, _T("GetSoftVision"));
	m_ThirdPartyMesDll.pfnStartAndPrintlabel = (FnStartAndPrintlabel)GetProcAddress(hLib, _T("StartAndPrintlabel"));

	if (m_ThirdPartyMesDll.pfnGetSoftVision == NULL
		|| m_ThirdPartyMesDll.pfnStartAndPrintlabel == NULL
		) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "动态库有部分函数未实现，请确认", strDllPath);
		CHECK_INTERFACE(GetSoftVision);
		CHECK_INTERFACE(StartAndPrintlabel);
		Ret = -1; goto __end;
	}

__end:
	return Ret;
}

BOOL CThirdPartyMesAccess::DetachDll()
{
	if (hLib != NULL) {
		FreeLibrary(hLib);
		hLib = NULL;
	}
	return TRUE;
}

