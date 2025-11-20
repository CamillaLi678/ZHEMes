//_commandEntries

// MesDlg.cpp : 实现文件
//
#include "stdafx.h"
#include "Mes.h"
#include "MesDlg.h"
#include "afxdialogex.h"
#include "MainData.h"
#include "DlgSetting.h"
#include "ComTool.h"
#include "ComFunc.h"
#include "Version.h"
#include "DlgLogin.h"
#include "ThirdPartyMesAccess.h"
#include "cJSON.h"
#include "DlgAutoDataRecord.h"
#include "WorkThread.h"
#include "json/json.h"
#include "AutoTaskData.h"
#include "IProgFileCheck.h"
#include "DlgCodeScan.h"
#include "DlgAdminMode.h"
#include "DlgMESTest.h"
#include "Serial.h"
#include "ComFunc.h"
#include "UserManager.h"
#include "DlgUserManager.h"
#include <direct.h>
#include<iostream>

#include <time.h>
#include <stdio.h>

#include "soapStub.h"
#include "soapH.h"
#include "MesInterface.h"
#include "SmtFtpServiceSoapBinding.nsmap"
#include "FileCacheManager.h"
#include <windows.h>
#include <wchar.h>
#include "DlgAuthority.h"
#include <tlhelp32.h>

#include "DlgModuleSetting.h"
#include "VersionSvn.h"


extern SOAP_NMAC struct Namespace namespaces[];

using namespace std;

#define  MSG_LOGINSHOW (WM_USER+0x001)
#define  MSG_START_WORK  (WM_USER+0x002)
#define  MSG_UPDATECHECKSUM (WM_USER+0x003)
#define  MSG_PARSE_START_WORK  (WM_USER+0x002)

#define  MSG_SEND_STOP_WORK   (WM_USER+0x004)
#define  UPDATE_TIMER_ID (1)

#define TMSG_RESTART  (WM_USER+0x006)

#define  MSG_CREATE_SEND_TCP_CLI (WM_USER+0x007)
#define  MSG_CREATE_RECV_TCP_CLI (WM_USER+0x008)

#define  UFX_LINE_LENGTH  10

#define  UFS_JSON_RPC_LOADTASK  6
#define  UFS_JSON_RPC_GETCOUNT  5
#define  UFS_JSON_RPC_SELECTFUNCTION  7
#define  UFS_JSON_RPC_GETSITESKTMAP   2
#define  UFS_JSON_RPC_STARTSITE       8

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define DLG_BKCOLOR (RGB(192,255,255))

#define MAX_PARAM_LEN 128
int _stdcall MsgHandle(void*Para, char*Msg, char*MsgData);

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()



// CMesDlg 对话框
CMesDlg::CMesDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MES_DIALOG, pParent)
	, m_pILog(NULL)
	, m_strWorkOrder(_T(""))
	, m_WorkOrderICNum(0)
	, m_ExpectICNum(0)
	, m_strAutoTaskFilePath(_T(""))
	, m_strSwap(_T("0"))
	, m_strProjPath(_T(""))
	, m_strProjChecksum(_T(""))
	, m_bProgRecordReady(FALSE)
	, m_bQuit(FALSE)
	, m_strOpMode(_T(""))
	, m_bNeedCheckQuit(FALSE)
	, m_endThread(false)
	, m_strChipName(_T(""))
	, m_bFirstRunFlag(FALSE)
	, m_bNeedLogin(TRUE)
	, m_bUseThirdPartyMesDll(FALSE)
	, m_bProgGetBurnQtyReady(FALSE)
	, m_ModelName(_T(""))
	, m_strProjVersion(_T(""))
	, m_strMaterialID(_T(""))
	, m_strBoxSN(_T(""))       // 箱单条码
	, m_strBatNo(_T(""))       // 批号
	, m_strStationId(_T(""))   // 机台编号
	, m_bRestartSoft(FALSE)
	, m_bMesTcpSyslog(FALSE)
	, m_UfsSelFunctionBn(FALSE)
	, m_UfsLoadTaskBn(FALSE)
	, m_UfsProgramBn(FALSE)
	, m_strFWPath(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_LOGO_ICON);

	m_strAutomaticUPH = _T("");
	m_strAutomaticTotalCnt = 0;
	m_strAutomaticPassCnt = 0;
	m_strAutomaticFailCnt = 0;
	m_strAutomaticRemoveCnt = 0;
	m_lastYieldChangeJson = _T("");
	m_strDevTimeRun = _T("000000");
	m_strDevTimeStop = _T("000000");
	pModuleProgram = NULL;

	m_strModuleName = _T("");
	m_strrModuleBaudrate = _T("");
	m_strProgramPID = _T("");
	m_strVerifyPID = _T("");
	m_strCmdLine = _T("");
	m_strModuleLogPath = _T("");
	m_strModuleExePath = _T("");
}

void CMesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDITWORKORDER, m_strWorkOrder);
	DDX_Text(pDX, IDC_EDITWORKOERDERICNUM, m_strStationId);  // 机台编号（从原来的工单IC数改为机台编号）
	DDX_Text(pDX, IDC_EDITEXPECTICNUM, m_ExpectICNum);
	DDX_Text(pDX, IDC_EDITAUTOTASKDATA, m_strAutoTaskFilePath);
	DDX_Text(pDX, IDC_EDITPROJPATH, m_strProjPath);
	DDX_Text(pDX, IDC_EDITPROJCHECKSUM_EXCEPT, m_strProjChecksum);
	DDX_Text(pDX, IDC_EDIT_REALCHECKSUM, m_strRealChecksum);
	DDX_Text(pDX, IDC_EDIT_METERIAL, m_strMaterialID);
	DDX_Control(pDX, IDC_RICHEDITLOG, m_LogRichEdit);
	DDX_Text(pDX, IDC_EDIT_FW_PROJPATH, m_strFWPath);
	DDX_Text(pDX, IDC_EDIT_BOX_SN, m_strBoxSN);  // 箱单条码 - USB扫码枪输入

	//DDX_Control(pDX, IDC_RICHEDIT22, m_ctrlMyRichEdit);
}

BEGIN_MESSAGE_MAP(CMesDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_SETTING, &CMesDlg::OnSetting)
	ON_MESSAGE(MSG_UPDATA_SETTING_CONFIG, &CMesDlg::OnUpdataSettingConfig)
	ON_MESSAGE(MSG_LOGINSHOW, CMesDlg::ShowLoginWin)
	ON_MESSAGE(MSG_UPDATECHECKSUM, CMesDlg::UpdateProjChecksum)
	ON_MESSAGE(MSG_UPDATA_SN, &CMesDlg::OnUpdataSN)
	ON_BN_CLICKED(IDC_BTNGetMesRecord, &CMesDlg::OnBnClickedBtnGetmesrecord)


	ON_MESSAGE(MSG_QUERY_COM_SITE_FINISHED, &CMesDlg::OnQueryComSiteFinished)
	ON_MESSAGE(MSG_QUERY_COM_ALL_FINISHED, &CMesDlg::OnQueryComAllFinished)
	ON_MESSAGE(MSG_UPDATE_SITE_NAME, &CMesDlg::OnUpdateSiteName)
	ON_MESSAGE(MSG_UPDATE_PROGRAM_RESULT, &CMesDlg::OnUpdateProgramResult)
	ON_MESSAGE(MSG_DO_PROGRAM_FINISH, &CMesDlg::OnProgramFinish)
	ON_MESSAGE(MSG_PROGRAM_END, &CMesDlg::OnUploadProgramRet2Mes)
	ON_MESSAGE(MSG_UPDATE_AUTO_COUNT, &CMesDlg::OnUpdateAutoProgramCount)
	ON_MESSAGE(MSG_UPDATE_PROGRAM_COUNT, &CMesDlg::OnUpdateProgramCount)

	ON_WM_MOVE()

	ON_BN_CLICKED(IDC_BTNSEL_PROGRAMFILEPATH, &CMesDlg::OnBnClickedBtnSelprogrampath)
	ON_BN_CLICKED(IDC_BTNSELAUTOTASKDATA, &CMesDlg::OnBnClickedBtnSelautotaskfilepath)
	ON_BN_CLICKED(IDC_BTNSEL_TEMPLATEFILE, &CMesDlg::OnBnClickedBtnSeltemplatepath)
	ON_BN_CLICKED(IDC_BTNSEL_SNCFILE, &CMesDlg::OnBnClickedBtnSelsncpath)
	ON_BN_CLICKED(IDC_BTNSELPROJ, &CMesDlg::OnBnClickedBtnSelprojpath)

	ON_COMMAND(ID_AUTOTASKDATA, &CMesDlg::OnAutotaskdata)
	ON_BN_CLICKED(IDC_BTNSTARTPRODUCE, &CMesDlg::OnBnClickedBtnStartproduce)
	ON_BN_CLICKED(IDC_BTNCANCELPRODUCE, &CMesDlg::OnBnClickedBtnCancelproduce)
	ON_COMMAND(ID_ADMINMODE, &CMesDlg::OnAdminmode)
	ON_COMMAND(ID_MESACCESSTEST, &CMesDlg::OnMesaccesstest)
	ON_COMMAND(ID_OPENDOWNTOOL, &CMesDlg::OnOpendownloaderdlg)
	ON_COMMAND(ID_CONFIGDOWN, &CMesDlg::OnConfigdown)
	ON_WM_CTLCOLOR()
	ON_COMMAND(ID_AUTOPOSSETTING, &CMesDlg::OnAutopossetting)
	ON_COMMAND(ID_USER_SETTING, &CMesDlg::OnUserSetting)
	ON_COMMAND(ID_ABOUT, &CMesDlg::OnAbout)
	ON_COMMAND(ID_AUTHORITY_SETTING, &CMesDlg::OnAuthoritySetting)
	ON_COMMAND(ID_ENGLISH_LANG, &CMesDlg::OnEnglishLang)
	ON_COMMAND(ID_CHINESE_ONE, &CMesDlg::OnChineseOne)
	ON_COMMAND(ID_CHINESE_LANG_TWO, &CMesDlg::OnChineseLangTwo)
	ON_COMMAND(ID_specification, &CMesDlg::Onspecification)
	ON_BN_CLICKED(IDC_BTNSEL_FW_PROJ, &CMesDlg::OnBnClickedBtnselFwProj)
	ON_BN_CLICKED(IDC_BTN_MODULESETTING, &CMesDlg::OnBnClickedBtnModelsetting)
	ON_BN_CLICKED(IDC_BTN_GETCOM, &CMesDlg::OnBnClickedBtnGetcom)
END_MESSAGE_MAP()


// CMesDlg 消息处理程序

LRESULT CMesDlg::OnQueryComAllFinished(WPARAM w, LPARAM l)
{
	GetDlgItem(IDC_BTNSTARTPRODUCE)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_GETCOM)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTNCANCELPRODUCE)->EnableWindow(FALSE);

	SetModuleCompoentEn(TRUE);
	return TRUE;
}

LRESULT CMesDlg::OnProgramFinish(WPARAM w, LPARAM l)
{
	GetDlgItem(IDC_BTNSTARTPRODUCE)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTNCANCELPRODUCE)->EnableWindow(FALSE);
	if (IsMesMode()) {
		GetDlgItem(IDC_BTNGetMesRecord)->EnableWindow(TRUE);
	}
	else {
		GetDlgItem(IDC_BTNGetMesRecord)->EnableWindow(FALSE);
	}
	

	return TRUE;
}

LRESULT CMesDlg::OnUploadProgramRet2Mes(WPARAM w, LPARAM l) 
{
	if (UploadProgramRet2Mes(m_ProgramResult.LastYieldChangeJson, m_YieldSites.TotalCnt, m_YieldSites.PassCnt, m_YieldSites.FailCnt) != 0) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "上传烧录结果失败");
	}
	else {
		m_pILog->PrintLog(LOGLEVEL_LOG, "上传烧录结果成功");
	}
	return TRUE;
}

LRESULT CMesDlg::OnUpdateAutoProgramCount(WPARAM w, LPARAM l)
{
	CProgramResultData *pData = (CProgramResultData *)w;
	if (pData != NULL) {
		m_ProgramResult = *pData;
		UpdateProgramResult2UI(pData->AutoPassCnt, pData->AutoFailCnt, pData->AutoTotalCnt);
	}

	return TRUE;
}

LRESULT CMesDlg::OnUpdateProgramCount(WPARAM w, LPARAM l)
{
	tYieldSites *pData = (tYieldSites *)w;
	UpdateProgramResult2UI(pData->PassCnt, pData->FailCnt, pData->TotalCnt);
	if (!pData->lastYieldJson.IsEmpty()) {

		cJSON* pRecvRet;
		pRecvRet = cJSON_Parse(pData->lastYieldJson.GetBuffer());
		if (pRecvRet != NULL) {
			cJSON_DeleteItemFromObject(pRecvRet, "Fail");
			cJSON_AddNumberToObject(pRecvRet, "Fail", pData->FailCnt);
			cJSON_DeleteItemFromObject(pRecvRet, "Pass");
			cJSON_AddNumberToObject(pRecvRet, "Pass", pData->PassCnt);
			cJSON_DeleteItemFromObject(pRecvRet, "Total");
			cJSON_AddNumberToObject(pRecvRet, "Total", pData->TotalCnt);
		}

		if (pRecvRet != NULL) {
			cJSON_Delete(pRecvRet);
			pRecvRet = NULL;
		}
	}
	if (pData != NULL) {
		delete pData;
		pData = NULL;
	}
	return TRUE;
}


LRESULT CMesDlg::OnUpdateProgramResult(WPARAM w, LPARAM l)
{
	INT idx = (INT)w;
	INT result = (INT)l;

	CRichEditCtrl* pRichEditCtrl = (CRichEditCtrl*)GetDlgItem(IDC_RICHEDIT22 + idx);

	if (result == PROGRAM_RESULT_DOING) {
		pRichEditCtrl->SetWindowText("DOING...");
		pRichEditCtrl->SetBackgroundColor(FALSE, RGB(255, 255, 0));
	}
	else if (result == PROGRAM_RESULT_OK) {
		pRichEditCtrl->SetWindowText("OK");
		pRichEditCtrl->SetBackgroundColor(FALSE, RGB(0, 255, 0));
	}
	else if (result == PROGRAM_RESULT_NG) {
		pRichEditCtrl->SetWindowText("NG");
		pRichEditCtrl->SetBackgroundColor(FALSE, RGB(255, 0, 0));
	}
	return TRUE;
}


LRESULT CMesDlg::OnUpdateSiteName(WPARAM w, LPARAM l)
{
	INT idx = (INT)w;
	CString *pStr = (CString *)l;
	((CEdit*)GetDlgItem(IDC_DEV1 + idx))->SetWindowText(*pStr);
	if (pStr != NULL) {
		delete pStr;
		pStr = NULL;
	}

	return TRUE;
}

LRESULT CMesDlg::OnQueryComSiteFinished(WPARAM w, LPARAM l)
{
	INT nConnectIdx = (INT)l;
	std::map<int, int> *Commap;
	Commap = (std::map<int, int>*)w;
	for (auto it = Commap->begin(); it != Commap->end(); it++) {
		CString strValue = _T("");
		strValue.Format("%d", (INT)it->second);
		(GetDlgItem(IDC_EDIT1 + nConnectIdx *SITE_COUNT + it->first ))->SetWindowText(strValue);
		((CButton*)GetDlgItem(IDC_CHECK1 + nConnectIdx *SITE_COUNT + it->first))->SetCheck(1);
	}
	SetComMap();
	return TRUE;
}

LRESULT  CMesDlg::OnUpdataSN(WPARAM w, LPARAM l)
{
	tSN* pSN = (tSN*)w;
	if (pSN && (m_Setting.nEnBarPrint == 1)) {
		//更新条码
		CString strOpt;
		CString strValue;
		strOpt.Format("%s", "SN");
		strValue.Format("%s", pSN->strSN);

		WritePrinterJsonFile(strOpt, strValue);

		if (m_pPrintFn) {
			m_pPrintFn();//启动打印
		}

		delete pSN;
		pSN = NULL;
	}
	return TRUE;
}

LRESULT CMesDlg::UpdateProjChecksum(WPARAM w, LPARAM l)
{
	if ((INT)l){
		m_strRealChecksum.Format("0x%08X", (UINT64)w);
		UpdateData(FALSE);
	}
	else{
		GetDlgItem(IDC_EDIT_REALCHECKSUM)->SetWindowText(m_strRealChecksum);
	}


	return 0;
}
LRESULT CMesDlg::ShowLoginWin(WPARAM, LPARAM)
{
	CDlgLogin LogIn;
	INT_PTR nRet = -1;
	LogIn.AttachILog(&m_LogRichEdit);
	LogIn.AttachData(&m_OperatorData);
	nRet = LogIn.DoModal();
	m_Setting.Load();

	GetDlgItem(IDC_EDITWORKORDER)->SetFocus();
	if (m_OperatorData.m_bCheckAuthPass) {
		m_pIStatus->PrintStatus(0, "操作权限认证成功");
		m_pILog->PrintLog(LOGLEVEL_LOG, "<===欢迎操作员[ %s ]使用一键烧录系统,操作权限等级[ %d ], 操作之前请阅读相关操作手册===>",
			m_OperatorData.m_strOperator, m_OperatorData.m_Level);
		if (!(m_Setting.strOperator.CompareNoCase("Admin") == 0)) {
			m_OperatorData.m_Level = 0;
		}
		else {
			m_OperatorData.m_Level = 3;
		}
		ChangeCtrlsEnAccrodOpLevel(m_OperatorData.m_Level);
	}

	if (m_OperatorData.m_bAuthCancel) {///认证取消,直接退出
		m_bNeedCheckQuit = FALSE;
		OnCancel();
	}
	SetAccountUI();

	if (m_Setting.strProgramMode.CompareNoCase("Module") == 0){
		SetModuleCompoentEn(TRUE);
	}

	CFileCacheManager &fileCacheManager = CFileCacheManager::getInstance();
	fileCacheManager.Init(&m_LogRichEdit);
	std::map<CString, CString> strResultMap;
	fileCacheManager.GetCacheFileList("json", strResultMap);
	if (strResultMap.size() > 0) {
		if (MessageBox("发现没有上传的结果，是否上传?", NULL, MB_YESNO | MB_ICONINFORMATION) == IDYES) {
			std::map<CString, CString>::iterator it;
			for (it = strResultMap.begin(); it != strResultMap.end(); ++it) {
				CMesInterface &MesInterface = CMesInterface::getInstance();
				INT Ret = MesInterface.CommitProgramRetJson2Mes(it->second);
				if (Ret == 0) {
					fileCacheManager.RemoveResultFile(it->first);
				}
			}
		}

	}
	if (m_Setting.strProgramMode.CompareNoCase("Module") == 0) {
		ComMap2UI();
	}


	return 0;
}

void CMesDlg::SetAccountUI()
{
	return;
	m_Setting.Load();
	if (m_Setting.strOperator.CompareNoCase("Admin") == 0) {
		return;
	}

	BOOL Enable = FALSE;
	GetDlgItem(IDC_EDIT_PROGRAMFILE_PATH)->EnableWindow(Enable);
	GetDlgItem(IDC_BTNSEL_PROGRAMFILEPATH)->EnableWindow(Enable);
	GetDlgItem(IDC_EDIT_TEMPLATEFILE)->EnableWindow(Enable);
	GetDlgItem(IDC_BTNSEL_TEMPLATEFILE)->EnableWindow(Enable);
	GetDlgItem(IDC_EDIT_SNCFILE)->EnableWindow(Enable);
	GetDlgItem(IDC_BTNSEL_SNCFILE)->EnableWindow(Enable);
	GetDlgItem(IDC_EDITPROJPATH)->EnableWindow(Enable);
	GetDlgItem(IDC_BTNSELPROJ)->EnableWindow(Enable);
	GetDlgItem(IDC_EDITPROGRAMFILESUM)->EnableWindow(Enable);

}

void CMesDlg::SetCompentVisible(BOOL bMesEnable)
{
	BOOL bEn = !bMesEnable;

	GetDlgItem(IDC_EDITPROJPATH)->EnableWindow(bEn);
	GetDlgItem(IDC_EDITCHECKSUM)->EnableWindow(bEn);
	GetDlgItem(IDC_EDITPROJCHECKSUM_EXCEPT)->EnableWindow(bEn);
	GetDlgItem(IDC_EDIT_REALCHECKSUM)->EnableWindow(bEn);
	GetDlgItem(IDC_BTNSELPROJ)->EnableWindow(bEn);
	GetDlgItem(IDC_BTNGetMesRecord)->EnableWindow(bMesEnable);

	if (IsAutoCreateProject()) { //
		GetDlgItem(IDC_BTNSEL_TEMPLATEFILE)->EnableWindow(TRUE); //模板
		GetDlgItem(IDC_EDIT_TEMPLATEFILE)->EnableWindow(TRUE);

		GetDlgItem(IDC_EDITPROGRAMFILESUM)->EnableWindow(TRUE);

		GetDlgItem(IDC_BTNSEL_PROGRAMFILEPATH)->EnableWindow(TRUE); //档案
		GetDlgItem(IDC_EDIT_PROGRAMFILE_PATH)->EnableWindow(TRUE);

		GetDlgItem(IDC_BTNSELPROJ)->EnableWindow(FALSE); //工程
		GetDlgItem(IDC_EDITPROJPATH)->EnableWindow(FALSE);

	}
	else {
		//GetDlgItem(IDC_EDIT_MODEL)->EnableWindow(bEn);
		//GetDlgItem(IDC_EDIT_METERIAL_CONFIRM)->EnableWindow(bEn);

		GetDlgItem(IDC_EDITAUTOTASKDATA)->EnableWindow(bEn);
		GetDlgItem(IDC_BTNSELAUTOTASKDATA)->EnableWindow(bEn);

		GetDlgItem(IDC_BTNSELPROJ)->EnableWindow(bEn);   //工程
		GetDlgItem(IDC_EDITPROJPATH)->EnableWindow(bEn);
	}
}

void CMesDlg::WritePrinterJsonFile(CString strModOpt, CString strModValue)
{
	if (strModOpt.IsEmpty()) {
		return;
	}

	DWORD ByteUsed;

	Json::Value Root;
	Json::FastWriter JWriter;
	Json::Reader Reader;
	Json::Value Para_list;

	CString strJsonData;
	std::string strJson;

	char* pReadData = NULL;
	int nWriteSize = 1024 * 5;
	char pWriteData[1024 * 5] = { 0 };
	CString strRead;
	int nSize = 0;

	CString strJsonPath = GetCurrentPath() + "\\PrintCfg.json";
	CFile jsonFile;
	if (jsonFile.Open(strJsonPath, CFile::modeReadWrite | CFile::shareExclusive, NULL) == FALSE) {
		AfxMessageBox("打开PrintCfg.json文件失败，请确认是否存在");
		return;
	}

	jsonFile.Seek(0, CFile::end);
	int nFileLen = (int)jsonFile.GetLength();
	jsonFile.Seek(0, CFile::begin);

	char* buffer = new char[nFileLen + 1];
	if (buffer == NULL) {
		AfxMessageBox("分配失败");
		goto __end;
	}
	memset(buffer, 0, nFileLen + 1);
	jsonFile.Read(buffer, nFileLen);

	jsonFile.Close();
	///清空文件内容
	jsonFile.Open(strJsonPath, CFile::modeReadWrite | CFile::modeCreate | CFile::shareExclusive, NULL);

	strRead.Format("%s", buffer);
	nSize = strlen(buffer) + 1;
	pReadData = new char[nSize];

	if (pReadData == NULL) {
		goto __end;
	}
	memset(pReadData, 0, nSize);
	if (Utf8ToMByte((LPCTSTR)buffer, pReadData, nSize, ByteUsed) != 1) {
		AfxMessageBox("转换失败");
	}

	int len = strlen(buffer);

	if (!Reader.parse(pReadData, pReadData + ByteUsed, Root)) {
		AfxMessageBox("解析PrintCfg.json失败");
		goto __end;
	}

	Para_list = Root["Para_list"];
	if (Para_list.isArray()) {
		for (int i = 0; i < (int)Para_list.size(); i++) {
			Json::Value item;
			item = Para_list[i];

			Json::Value para_item;
			para_item = item["para_item"];

			CString Output;

			Output.Format("%s", item["Output"].asCString());

			if (Output.CompareNoCase(strModOpt) == 0) {
				//item["i_value"]=(LPCTSTR)strModValue; //modify value
				Root["Para_list"][i]["i_value"] = Json::Value(strModValue);
				break;
			}
		}
	}

	strJson = JWriter.write(Root);
	strJsonData.Format("%s", strJson.c_str());

	memset(pWriteData, 0, nWriteSize);

	if (MByteToUtf8(strJsonData, pWriteData, nWriteSize, ByteUsed) != 1) {
		AfxMessageBox("转换失败");
		goto __end;
	}

	jsonFile.SeekToBegin();
	jsonFile.Write(pWriteData, ByteUsed);
	jsonFile.Flush();

__end:

	if (buffer) {
		delete[] buffer;
		buffer = NULL;
	}

	jsonFile.Close();
}

void CMesDlg::ClosePrintDll()
{
	if (m_hPrintLib = NULL) {
		return;
	}

	FreeLibrary(m_hPrintLib);
	m_hPrintLib = NULL;
}

void CMesDlg::LoadPrintDll()
{
	m_pPrintFn = NULL;
	m_hPrintLib = NULL;
	INT Ret = 0;
	CString strDllPath;
	strDllPath.Format("%s\\BarPrintHelper.dll", GetCurrentPath());
	m_hPrintLib = LoadLibrary(strDllPath); //
	if (m_hPrintLib == NULL) {
		DWORD ErrNo = GetLastError();
		AfxMessageBox("打开BarPrintHelper.dll 失败, ErrorCode=%d", ErrNo);
		Ret = -1;
		return;
	}

	m_pPrintFn = (FnAC_PrintData)GetProcAddress(m_hPrintLib, "AC_PrintData");
	if (m_pPrintFn == NULL) {
		AfxMessageBox("从BarPrintHelper.dll中导出AC_PrintData失败");
	}
	//m_pPrintFn();
}

static inline CString GetCurDateTime()
{
	CString strTime;
	CTime CurTime;
	CurTime = CTime::GetCurrentTime();//获取当前系统时间
	strTime.Format("%04d-%02d-%02d", CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay());
	return strTime;
}

void CMesDlg::SplitComMap(CString strComMap)
{
	CHAR ch;
	tSiteInfo_Com SiteInfo;
	INT Ret = 0, i, Len=0;
	INT BracketCnt = 0;
	INT Comma = 0, Pos;
	CString strSitePosition;
	CString strSiteMap;
	m_vSiteInfo_Com.clear();
	if (strComMap != "") {
		Len = strComMap.GetLength();
		for (i = 0; i < Len; ++i) {
			ch = strComMap.GetAt(i);
			if (ch == '<') {
				strSiteMap = "";
				BracketCnt++;
				Comma = 0;
			}
			else if (ch == '>') {
				CStringArray destDataArray;
				Split(strSiteMap, destDataArray, ",");
				if (destDataArray.GetCount() != 3){
					return;
				}
				Pos = destDataArray[0].Find('-');
				if (Pos >0 ) {
					SiteInfo.SiteAlias = destDataArray[0].Mid(0, Pos);
					SiteInfo.SiteSn = destDataArray[0].Mid(Pos + 1, 100);
				}
				else{
					SiteInfo.SiteAlias = "";
					SiteInfo.SiteSn = "";
				}

				INT position = atoi(destDataArray[1]);
				if (position > 10) {
					SiteInfo.SiteIdx = position / 10 - 1;
					SiteInfo.SKTIdx = position % 10 - 1;
				}
				else {
					SiteInfo.SiteIdx = -1;
					SiteInfo.SKTIdx = -1;
				}
				SiteInfo.strCom = destDataArray[2];
				m_vSiteInfo_Com.push_back(SiteInfo);
				BracketCnt--;
			}
			else {
				strSiteMap += ch;
			}
		}
	}
	if (BracketCnt != 0) {
		Ret = -1;
	}
}

INT CMesDlg::GetComMap()
{
	CString strComMap;
	CString strErrMsg;
	CString strJsonPath;
	BYTE *pData = NULL;
	CFile File;
	CString strJson;

	INT FileSize = 0, ArraySize = 0,Ret = -1;

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
		if (m_Setting.strModuleName.CompareNoCase(pItemObj->valuestring) != 0) {
			continue;
		}

		pItemObj = cJSON_GetObjectItem(pArrayItem, "ComMap");
		if (pItemObj == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析ComMap字段错误，请确认json文件中维护的字段信息 ");
			goto __end;
		}
		strComMap = pItemObj->valuestring;
	}

	if (strComMap.IsEmpty()){
		CString strText;
		GetDlgItemText(IDC_BTN_GETCOM, strText);
		strErrMsg.Format("当前配置ComMap为空，请先点击\"%s\"按钮进行COM口获取", strText);
		m_LogRichEdit.PrintLog(LOGLEVEL_ERR, "%s", strErrMsg);
		m_pIStatus->PrintStatus(0, strErrMsg);
		MessageBox(strErrMsg, NULL, MB_OK | MB_ICONERROR);
		goto __end;
	}

	SplitComMap(strComMap);
	Ret = 1;
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

void CMesDlg::ComMap2UI() {
	GetComMap();
	CString SiteAlias_SN;

	for (int i = 0; i < SITE_COUNT; i++) { //轮询ready 状态
		CString SiteSn, SiteAlias;
		for (int j = 0; j < EACH_SITE_CHIP_NUM; j++) {
			UINT idx = i * SITE_COUNT + j;//0-16

			CString foundCom{ "-1" };
			for (int k = 0; k < (int)m_vSiteInfo_Com.size(); k++) {
				if (m_vSiteInfo_Com[k].SiteIdx == i && m_vSiteInfo_Com[k].SKTIdx == j) {
					foundCom = m_vSiteInfo_Com[k].strCom;
					SiteSn = m_vSiteInfo_Com[k].SiteSn;
					SiteAlias = m_vSiteInfo_Com[k].SiteAlias;
					break;
				}
			}

			bool flag = foundCom.Compare("-1") != 0;
			((CButton*)GetDlgItem(IDC_CHECK1 + idx))->SetCheck(flag);
			(GetDlgItem(IDC_EDIT1 + i * SITE_COUNT + j))->SetWindowText(flag ? foundCom : "");
		}
		if (!SiteAlias.IsEmpty() && !SiteSn.IsEmpty()){
			SiteAlias_SN.Format("%s-%s", SiteAlias, SiteSn);
			GetDlgItem(IDC_DEV1 + i)->SetWindowTextA(SiteAlias_SN);
		}

	}
}

BOOL CMesDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	// TODO: 在此添加额外的初始化代码
	SetBackgroundColor(DLG_BKCOLOR);

	InitializeCriticalSection(&m_csDownLoad);

	GetDlgItem(IDC_EDITWORKOERDERICNUM)->EnableWindow(FALSE);
	UpdateProgramResult2UI(0, 0, 0);

	CString strSettingJsonPath;
	m_MainData.bUnderAdminMode = FALSE;
	m_MyStatus.AttachStatusBar(GetDlgItem(IDC_SHOWSTATUS));
	m_pIStatus = &m_MyStatus;
	m_LogRichEdit.bWritFile = 1;
	/*LOGFONT lf;
	::ZeroMemory(&lf, sizeof(lf));
	lf.lfHeight = 18;
	::lstrcpy(lf.lfFaceName, _T("Consolas"));
	m_LogRichEdit.SetTextFont(lf);*/
	m_LogRichEdit.Init(&m_LogRichEdit);
	m_pILog = &m_LogRichEdit;
	strSettingJsonPath.Format("%s\\Setting.json", GetCurrentPath());
	m_Setting.SetJsonPath(strSettingJsonPath);
	if (m_Setting.Load() == FALSE) {
		m_pIStatus->PrintStatus(0, m_Setting.GetErrMsg());
	}

	CString strIniLangFile;
	strIniLangFile.Format("%s\\Language.ini", GetCurrentPath());
	CHAR TmpBuf[MAX_PATH];
	memset(TmpBuf, 0, MAX_PATH);
	GetPrivateProfileString("Language", "CurLang", "English", TmpBuf, MAX_PATH, strIniLangFile);
	m_strLang.Format("%s", TmpBuf);

	CString strTitle;
	CMenu *pMenu;
	pMenu = AfxGetMainWnd()->GetMenu();
	if (m_strLang.CompareNoCase("ChineseA") == 0) {
		strTitle.Format("批量烧录应用程序%s%s", m_Setting.strSoftMainVer, MES_VERSION);
		pMenu->EnableMenuItem(ID_CHINESE_ONE, MF_DISABLED | MF_GRAYED);
	}
	else {
		if (m_strLang.CompareNoCase("ChineseB") == 0) {
			pMenu->EnableMenuItem(ID_CHINESE_LANG_TWO, MF_DISABLED | MF_GRAYED);
		}
		else {
			pMenu->EnableMenuItem(ID_ENGLISH_LANG, MF_DISABLED | MF_GRAYED);
		}
		strTitle.Format("MesClient Program SoftWare %s%s", m_Setting.strSoftMainVer, MES_VERSION);
	}
	SetWindowText(strTitle);

	InitCtrlsValue();

	BOOL bMesEn = FALSE;
	if (IsMesMode()) {
		bMesEn = TRUE;
	}

	SetCompentVisible(bMesEn);

	CString dateFolder = _T("");
	dateFolder.Format("%s\\Log\\%s", ::GetCurrentPath(), GetCurDateTime());
	if (!CComFunc::IsDirExist(dateFolder)) {
		_mkdir(dateFolder);
	}
	CString strLogFile;
	strLogFile.Format("%s\\Log\\%s\\%s_%s.log", ::GetCurrentPath(), GetCurDateTime(), m_Setting.strManufactor, ::GetCurTime('_'));
	m_LogRichEdit.bWritFile = 1;
	m_LogRichEdit.AttachFile(strLogFile);

	HWND hWnd = GetSafeHwnd();
	m_HandleScan = new CHandleScan(hWnd);
	m_HandleScan->SetLogMsg(&m_LogRichEdit);
#ifdef USE_TCP_SYSLOG
	InitTcpSyslog();
#endif
	CMsgHandler<CMesDlg, CMesDlg> parseHandler = MakeMsgHandler(this, &CMesDlg::ThreadParserHandler);
	m_ParseWorker.SetMsgHandle(parseHandler);
	m_ParseWorker.CreateThread();
	mParseWorkerExit = FALSE;
	m_ParseWorker.PostMsg(MSG_PARSE_START_WORK, 0, 0);

	m_Setting.Load();

	//在登陆之前
	INT UserCnt = 0;
	CUserManager &UserManagerInterface = CUserManager::getInstance();
	UserManagerInterface.Init(&m_LogRichEdit,"Admin","11111111", ADMIN_USER_ROLE);
	UserCnt = UserManagerInterface.GetUserCnt();
	if (UserCnt == 0){
		UserManagerInterface.AddUser("Admin", "11111111", ADMIN_USER_ROLE);
		UserManagerInterface.AddUser("Engineer", "22222222", ENGINEER_USER_ROLE);
		UserManagerInterface.AddUser("Operator", "33333333", OPERATOR_USER_ROLE);
		UserManagerInterface.AddRole(ENGINEER_USER_ROLE, ENGINEER_PERMISSION);
		UserManagerInterface.AddRole(OPERATOR_USER_ROLE, OPERATOR_PERMISSION);
		UserManagerInterface.AddRole(ADMIN_USER_ROLE, ADMIN_PERMISSION);
	}

	m_DlgProduceInfo.Create(IDD_DLGPRODUCEINFO, NULL);

	if (m_Setting.nEnBarPrint == 1) {
		LoadPrintDll();
	}
	m_pILog->PrintLog(LOGLEVEL_LOG, "Init OK %s", MES_VERSION);
	if (m_Setting.nLocalServer) {
		AfxMessageBox("这是本地调试版本，不要对外发布");
	}
#ifdef _DEBUG
	if (!m_Setting.nLocalServer) {
		AfxMessageBox("这是Debug版本，不要对外发布");
	}
#endif
	if (IsMesMode()) {//本地模式，不连接MES
		if (m_bUseThirdPartyMesDll) {
			CThirdPartyMesAccess &MesInterface = CThirdPartyMesAccess::getInstance();
			CString strDllPath;
			strDllPath.Format("%s\\MES2Interface.dll", GetCurrentPath());
			MesInterface.Init(&m_LogRichEdit, strDllPath, m_Setting.strAutoTaskFolder, m_Setting.strProjSaveFolder,
				m_Setting.nLocalServer);
		}
		else {
			CMesInterface &MesInterface = CMesInterface::getInstance();
			MesUserInfo userInfo;
			userInfo.useCode = m_Setting.strOperator;
			userInfo.providerId = m_Setting.strServiceproviderid;
			userInfo.account = m_Setting.strAccount;
			userInfo.password = m_Setting.strPwd;

			MesInterfaceInfo interfaceInfo;
			interfaceInfo.aprFolder = m_Setting.strProjSaveFolder;
			interfaceInfo.tskFolder = m_Setting.strAutoTaskFolder;
			interfaceInfo.stationId = m_Setting.strWorkStationID;
			//interfaceInfo.deviceId = m_Setting.strDeviceID;
			//interfaceInfo.factoryId = m_Setting.strFactoryID;
			interfaceInfo.bLocal = m_Setting.nLocalServer;
			interfaceInfo.bServerTset = m_Setting.nServerTest;

			MesInterface.Init(&m_LogRichEdit, userInfo, m_Setting.strWebServiceInterface, interfaceInfo);
			/*nToken = MesInterface.GetMesLoginToServer();
			if (nToken != 0){
				m_pILog->PrintLog(LOGLEVEL_LOG, "获取token失败");

			}*/
		}

	}

	if (m_bMesTcpSyslog) {
		InitMesSyslog();
		m_MesSyslog.MBTcpServerInit();
	}

	if (m_bNeedLogin) {
		PostMessage(MSG_LOGINSHOW, 0, 0);
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}
#ifdef USE_TCP_SYSLOG
INT CMesDlg::InitTcpSyslog()
{
	INT Ret = 0;
	m_TcpSyslog.Uninit();
	m_TcpSyslog.m_EventDumpLog += MakeDelegate<CMesDlg, CMesDlg>(this, &CMesDlg::OnDumpLog);
	m_TcpSyslog.m_EventRecvMsg += MakeDelegate<CMesDlg, CMesDlg>(this, &CMesDlg::OnRecvMsg);
	if (m_TcpSyslog.Init(_T("127.0.0.1"), 61000) != 0) {
		m_TcpSyslog.Uninit();
		Ret = -1;
	}
	return Ret;
}

INT CMesDlg::UnInitTcpSyslog()
{
	m_TcpSyslog.Uninit();
	return 0;
}

#define LOGOUT(LOG_LEVEL, ...)  (m_pILog->PrintLog(LOG_LEVEL, __VA_ARGS__))

#define SYSLOG_LOG_TAG "TCPSyslog"
bool CMesDlg::OnDumpLog(void *Para)
{
	//CString *pStrLog = (CString*)Para;
	//if (GetSafeHwnd() && IsWindowVisible()) {
	//	LOGOUT(LOGLEVEL_ERR, "%s %s", SYSLOG_LOG_TAG, pStrLog->GetString());
	//}
	return TRUE;
}

bool CMesDlg::OnRecvMsg(void *Para)
{
	std::string *pStrMsg = (std::string*)Para;
	//m_pILog->PrintLog(LOGLEVEL_LOG, "TCPSyslog Recv msg: %s", pStrMsg->c_str());

	//push back msg to message list, wait ParseTask to parse.
	m_MsgListMutex.Lock();
	mRecvMsgList.push_back(*pStrMsg);
	m_MsgListMutex.Unlock();

	return TRUE;
}

#endif

INT CMesDlg::HandleSetPrintResult(const char * MsgData) {
	CString strRecvJson;
	std::string strJson(MsgData);
	Json::Value Root;
	Json::Reader JReader;
	if (JReader.parse(strJson, Root) == false) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "解析json失败");
	}
	else {
		CString StrEid = _T("");
		CString StrSN = _T("");
		StrEid.Format("%s", Root["Uid"].asCString());
		StrSN.Format("%s", Root["Text"].asCString());
		INT Result = Root["Result"].asUInt();
		m_pILog->PrintLog(LOGLEVEL_LOG, "Eid:%s SN:%s Result:%d", StrEid, StrSN, Result);
		//0：丢了，1：成功 2：镭雕失败
		CMesInterface &MesInterface = CMesInterface::getInstance();
	}
	return 0;
}

INT CMesDlg::HandleSetPrintContent(const char * MsgData) {
	CString strRecvJson;
	std::string strJson(MsgData);
	Json::Value Root;
	Json::Reader JReader;
	if (JReader.parse(strJson, Root) == false) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "解析json失败");
	}
	else {
		CString StrEid = _T("");
		CString StrSN = _T("");
		if (Root["Content"].isArray() && Root["ContentEn"].asUInt() == 1) {
			for (UINT i = 0; i < Root["Content"].size();i++) {
				Json::Value OneItem = Root["Content"][i];
				StrEid.Format("%s", OneItem["Uid"].asCString());
				StrSN.Format("%s", OneItem["Text"].asCString());
				INT Valid = OneItem["Valid"].asUInt();
				m_pILog->PrintLog(LOGLEVEL_LOG, "HandleSetPrintContent Eid:%s SN:%s Valid:%d", StrEid, StrSN, Valid);
				//失败直接提交给IPS
				if (Valid == 0) {
					CMesInterface &MesInterface = CMesInterface::getInstance();
				}
			}

		}
	}
	return 0;
}

INT CMesDlg::DoParseTask()
{
	while (!mParseWorkerExit) {//FIXME later. how to exit thread.

		std::string buffer;
		bool getMsg = false;
		m_MsgListMutex.Lock();
		if (!mRecvMsgList.empty()) {
			buffer = mRecvMsgList.front();
			mRecvMsgList.pop_front();
			getMsg = true;
		}
		m_MsgListMutex.Unlock();
		if (!getMsg) {
			Sleep(5);
		}
		else {
			cJSON* pRootParser;
			cJSON* pEName;
			CString eName;
			pRootParser = cJSON_Parse((const char *)buffer.c_str());
			if (pRootParser == NULL) {
				m_pILog->PrintLog(LOGLEVEL_ERR, "自动化返回的不符合Json数据格式 ");
				continue;
			}
			pEName = cJSON_GetObjectItem(pRootParser, "EName");
			if (pEName == NULL) {
				m_pILog->PrintLog(LOGLEVEL_ERR, "自动化返回的EName字段错误，请确认自动化维护的字段信息, ");
				if (pRootParser != NULL) {
					cJSON_Delete(pRootParser);
					pRootParser = NULL;
				}
				continue;
			}
			eName.Format("%s", pEName->valuestring);
			if (eName.CompareNoCase("Alarm") == 0) {
				parseAutoAlarmMsg(pRootParser);
			}
			else if (eName.CompareNoCase("LotStart") == 0) {
				parseLotStartMsg(pRootParser);
			}
			else if (eName.CompareNoCase("Opecall") == 0) {
				parseOpecallMsg(pRootParser);
			}
			else if (eName.CompareNoCase("LotEnd") == 0) {
				parseLotEndMsg(pRootParser);
			}
			else if (eName.CompareNoCase("ProductStsSet") == 0) {
				//parseProductStsSetMsg(pRootParser);
			}

			if (pRootParser != NULL) {
				cJSON_Delete(pRootParser);
				pRootParser = NULL;
			}
		}

		if (mParseWorkerExit) {
			break;
		}

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
		}
		else {
			const char *Msg = stdMesMsg.msg.c_str();
			const char *MsgData = stdMesMsg.json.c_str();
			if (_stricmp(Msg, "YieldChange") == 0) {
				CString strRecvJson;
				std::string strJson(MsgData);
				Json::Value Root;
				Json::Reader JReader;
				if (JReader.parse(strJson, Root) == false) {
					m_pILog->PrintLog(LOGLEVEL_ERR, "解析json失败");
				}
				else {
					m_YieldSitesMutex.Lock();
					m_YieldSites.TotalCnt += Root["CurTotal"].asInt();
					m_YieldSites.FailCnt += Root["CurFail"].asInt();
					m_YieldSites.PassCnt += Root["CurPass"].asInt();
					m_YieldSitesMutex.Unlock();

					strRecvJson.Format("%s", MsgData);
					
					cJSON* pRecvRet = cJSON_Parse(strRecvJson.GetBuffer());
					cJSON* pFailReason = NULL;
					if (pRecvRet != NULL && (pFailReason = cJSON_GetObjectItem(pRecvRet, "FailReason")) != NULL) {
						for (int i = 0; i < cJSON_GetArraySize(pFailReason); i++)
						{
							cJSON* subitem = cJSON_GetArrayItem(pFailReason, i);
							cJSON* pSktIdx = cJSON_GetObjectItem(subitem, "SktIdx");
							cJSON* pErrCode = cJSON_GetObjectItem(subitem, "ErrCode");
							cJSON* pErrMsg = cJSON_GetObjectItem(subitem, "ErrMsg");
							//FIXME, last SktIdx reason.
							//m_lastYieldChangeJson.Format("SktIdx: %d, ErrCode: %s, ErrMsg: %s", pSktIdx->valueint, pErrCode->valuestring, pErrMsg->valuestring);
						}
					}
					m_lastYieldChangeJson = strRecvJson;
					//if (UploadProgramRet2Mes(strRecvJson) != 0) {
					//	m_pILog->PrintLog(LOGLEVEL_LOG, "上传烧录结果失败");
					//}
					
				}
			}
			else if (_stricmp(Msg, "StatusChange") == 0) {

			}
			else if (_stricmp(Msg, "SetPrintContent") == 0) {
				m_pILog->PrintLog(LOGLEVEL_LOG, "SetPrintContent: %s", MsgData);
				HandleSetPrintContent(MsgData);
			}
			else if (_stricmp(Msg, "SetPrintResult") == 0) {
				m_pILog->PrintLog(LOGLEVEL_LOG, "SetPrintResult: %s", MsgData);
				HandleSetPrintResult(MsgData);
			}
			else if (_stricmp(Msg, "ShowLog") == 0) {
			}
			else if (_stricmp(Msg, "MissionResult") == 0) {
				m_pILog->PrintLog(LOGLEVEL_LOG, "收到MissionResult消息");
				m_bGetMissionResult = TRUE;
			}
			else if (_stricmp(Msg, "FileImportInfo_Json") == 0) {
				//m_pILog->PrintLog(LOGLEVEL_LOG, "FileImportInfo_Json");
				CString strRecvJson;
				std::string strJson(MsgData);
				Json::Value Root;
				Json::Reader JReader;
				if (JReader.parse(strJson, Root) == false) {
					m_pILog->PrintLog(LOGLEVEL_ERR, "解析json失败");
				}
				else {
					CString StrVersion = _T("");
					StrVersion.Format("%s", Root["Version"].asCString());
					float fVar = (float)atof(StrVersion);
					strRecvJson.Format("%s", MsgData);
					cJSON* pRecvRet = cJSON_Parse(strRecvJson.GetBuffer());
					cJSON* pOrgFiles = NULL;
					if (pRecvRet != NULL && (pOrgFiles = cJSON_GetObjectItem(pRecvRet, "OrgFiles")) != NULL) {
						for (int i = 0; i < cJSON_GetArraySize(pOrgFiles); i++)
						{
							cJSON* subitem = cJSON_GetArrayItem(pOrgFiles, i);
							cJSON* pFileName = cJSON_GetObjectItem(subitem, "FileName");
							if (fVar >= 3.0f) {
								cJSON* pChecksumType = cJSON_GetObjectItem(subitem, "ChecksumType");
								cJSON* pChecksumValue = cJSON_GetObjectItem(subitem, "ChecksumValue");
								//m_pILog->PrintLog(LOGLEVEL_LOG, "pFileName: %s ChecksumType: %s ChecksumValue: %s",
								//	pFileName->valuestring, pChecksumType->valuestring, pChecksumValue->valuestring);
							}
						}
					}
					if (pRecvRet != NULL) {
						cJSON_Delete(pRecvRet);
						pRecvRet = NULL;
					}
				}
			}
		}
	}

	return 0;
}

BOOL CMesDlg::parseLotStartMsg(void * rootParser)
{
	cJSON* pETime = NULL;
	CString eTime;
	cJSON* pRootParser = (cJSON*)rootParser;
	pETime = cJSON_GetObjectItem(pRootParser, "ETime");
	if (pETime == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "自动化返回的ETime字段错误，请确认自动化维护的字段信息, ");
		return false;
	}
	eTime.Format("%s", pETime->valuestring);
	convertTimeFormat(eTime, m_strStartTime);
	m_pILog->PrintLog(LOGLEVEL_LOG, "StartTime: %s ", m_strStartTime);
	return TRUE;
}

BOOL CMesDlg::parseOpecallMsg(void * rootParser)
{
	cJSON* pEData = NULL;
	cJSON* pCode = NULL;
	CString Code;
	cJSON* pRootParser = (cJSON*)rootParser;
	pEData = cJSON_GetObjectItem(pRootParser, "EData");
	if (pEData == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "自动化返回的ETime字段错误，请确认自动化维护的字段信息, ");
		return false;
	}
	pCode = cJSON_GetObjectItem(pEData, "Code");
	Code.Format("%s", pCode->valuestring);
	if (Code == "MSG_O09000") {
		m_bRecvAutomaticSupplyStop = TRUE;
		m_pILog->PrintLog(LOGLEVEL_LOG, "收到自动机供给停止，Code: %s ", Code);
	}
	
	return TRUE;
}

void CMesDlg::convertTimeFormat(CString oldTime, CString &newTime) {
	/* 2022-04-11-14-25-11-051 change to 2022-04-11 14:25:11 */
	if (oldTime.GetLength() != 23) {
		newTime = oldTime;
		return;
	}
	int nYear, nMonth, nDate, nHour, nMin, nSec, nMinsec;
	sscanf(oldTime, "%d-%d-%d-%d-%d-%d-%d", &nYear, &nMonth, &nDate, &nHour, &nMin, &nSec, &nMinsec);
	newTime.Format("%04d-%02d-%02d %02d:%02d:%02d", nYear, nMonth, nDate, nHour, nMin, nSec);
}

BOOL CMesDlg::parseLotEndMsg(void * rootParser)
{
	cJSON* pETime = NULL;
	cJSON* pEData = NULL;
	cJSON* pEdataItemOperatingTime = NULL;
	cJSON* pEdataItemManualStopTime = NULL;

	CString eTime;
	CString eOperatingTime;
	CString eManualStopTime;

	cJSON* pRootParser = (cJSON*)rootParser;
	pETime = cJSON_GetObjectItem(pRootParser, "ETime");
	if (pETime == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "自动化返回的ETime字段错误，请确认自动化维护的字段信息, ");
		return false;
	}
	eTime.Format("%s", pETime->valuestring);
	convertTimeFormat(eTime, m_strEndTime);

	pEData = cJSON_GetObjectItem(pRootParser, "EData");
	if (pEData == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "自动化返回的EData字段错误，请确认自动化维护的字段信息, ");
		return false;
	}
	pEdataItemOperatingTime = cJSON_GetObjectItem(pEData, "OperatingTime");
	if (pEdataItemOperatingTime == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "自动化返回的Code字段错误，请确认自动化维护的字段信息, ");
		return false;
	}
	eOperatingTime.Format("%s", pEdataItemOperatingTime->valuestring);
	m_strDevTimeRun = eOperatingTime;
	m_strDevTimeRun.Replace(":", "");

	pEdataItemManualStopTime = cJSON_GetObjectItem(pEData, "ManualStopTime");
	if (pEdataItemManualStopTime == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "自动化返回的Code字段错误，请确认自动化维护的字段信息, ");
		return false;
	}
	eManualStopTime.Format("%s", pEdataItemManualStopTime->valuestring);
	m_strDevTimeStop = eManualStopTime;
	m_strDevTimeStop.Replace(":", "");
	m_pILog->PrintLog(LOGLEVEL_LOG, "EndTime: %s OperatingTime: %s ManualStopTime: %s ", m_strEndTime, eOperatingTime, eManualStopTime);

	//m_pILog->PrintLog(LOGLEVEL_LOG, "开始提交任务数据信息给Mes...");
	m_bRecvAutomaticLotEnd = TRUE;
	if (IsMesMode()) {
		if (m_Setting.strAutoMode.CompareNoCase("Auto") == 0) {
			if (CommitTaskIngfo(m_strEndTime, m_strDevTimeRun, m_strDevTimeStop) != 0) {
				m_pILog->PrintLog(LOGLEVEL_ERR, "提交任务数据信息给Mes失败...");
			}
			else {
				m_pILog->PrintLog(LOGLEVEL_LOG, "提交任务数据信息给Mes成功...");
			}
		}
	}
	return TRUE;
}

BOOL CMesDlg::parseProductStsSetMsg(void * rootParser)
{
	cJSON* pEData = NULL;
	cJSON* pEdataProductVID = NULL;
	cJSON* pEdataStorage = NULL;
	cJSON* pEdataFinalSts = NULL;

	CString strProductVID;
	CString strFinalSts;

	cJSON* pRootParser = (cJSON*)rootParser;

	pEData = cJSON_GetObjectItem(pRootParser, "EData");
	if (pEData == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "自动化返回的EData字段错误，请确认自动化维护的字段信息, ");
		return false;
	}
	pEdataProductVID = cJSON_GetObjectItem(pEData, "ProductVID");
	if (pEdataProductVID == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "自动化返回的ProductVID字段错误，请确认自动化维护的字段信息, ");
		return false;
	}
	strProductVID.Format("%s", pEdataProductVID->valuestring);

	pEdataStorage = cJSON_GetObjectItem(pEData, "Storage");
	if (pEdataStorage == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "自动化返回的Storage字段错误，请确认自动化维护的字段信息, ");
		return false;
	}
	pEdataFinalSts = cJSON_GetObjectItem(pEdataStorage, "FinalSts");
	if (pEdataFinalSts == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "自动化返回的FinalSts字段错误，请确认自动化维护的字段信息, ");
		return false;
	}
	strFinalSts.Format("%s", pEdataFinalSts->valuestring);

	//3. 烧录 PASS, 101上表面不良(烧录前) 102上表面不良(烧录后) 103下表面不良(烧录前) 104下表面不良(烧录后)
	m_pILog->PrintLog(LOGLEVEL_LOG, "strProductVID: %s strFinalSts: %s ", strProductVID, strFinalSts);

	return TRUE;
}

BOOL CMesDlg::parseAutoAlarmMsg(void * rootParser)
{
	cJSON* pESender;
	cJSON* pETime;
	cJSON* pEData;

	CString eSender;
	CString eTime;
	cJSON* pRootParser = (cJSON*)rootParser;
	pESender = cJSON_GetObjectItem(pRootParser, "ESender");
	if (pESender == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "自动化返回的ESender字段错误，请确认自动化维护的字段信息, ");
		return false;
	}
	eSender.Format("%s", pESender->valuestring);

	pETime = cJSON_GetObjectItem(pRootParser, "ETime");
	if (pETime == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "自动化返回的ETime字段错误，请确认自动化维护的字段信息, ");
		return false;
	}
	eTime.Format("%s", pETime->valuestring);

	pEData = cJSON_GetObjectItem(pRootParser, "EData");
	if (pEData == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "自动化返回的EData字段错误，请确认自动化维护的字段信息, ");
		return false;
	}

	cJSON* pEdataItemCode, *pEdataItemInfo, *pEdataItemETimeType;
	CString eCode;
	CString eInfo;
	CString eTimeType;

	pEdataItemCode = cJSON_GetObjectItem(pEData, "Code");
	if (pEdataItemCode == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "自动化返回的Code字段错误，请确认自动化维护的字段信息, ");
		return false;
	}
	eCode.Format("%s", pEdataItemCode->valuestring);
	/* MSG_E01101 change to 0x01101 */
	eCode.Replace("MSG_E", "0x");

	pEdataItemInfo = cJSON_GetObjectItem(pEData, "Info");
	if (pEdataItemInfo == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "自动化返回的Info字段错误，请确认自动化维护的字段信息, ");
		return false;
	}
	eInfo.Format("%s", pEdataItemInfo->valuestring);

	pEdataItemETimeType = cJSON_GetObjectItem(pEData, "ETimeType");
	if (pEdataItemETimeType == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "自动化返回的ETimeType字段错误，请确认自动化维护的字段信息, ");
		return false;
	}

	/*
	alarmFlag: 为 1 表示 TimeStart 有效，为 2 表示 TimeKill有效，为 3 表示都有效
	*/

	CString eAlarmKillTime;
	CString eAlarmStartTime;
	eTimeType.Format("%s", pEdataItemETimeType->valuestring);
	int alarmFlag = 3;
	if (eTimeType.CompareNoCase("AlarmStart") == 0) {
		alarmFlag = 1;
		convertTimeFormat(eTime, eAlarmStartTime);
		eAlarmKillTime = "0";
	}
	else if (eTimeType.CompareNoCase("AlarmReset") == 0) {
		alarmFlag = 2;
		eAlarmStartTime = "0";
		convertTimeFormat(eTime, eAlarmKillTime);
	}
	else {
		alarmFlag = 3;
		convertTimeFormat(eTime, eAlarmStartTime);
		convertTimeFormat(eTime, eAlarmKillTime);
	}
	if (IsMesMode() && m_bProgRecordReady) {
		CommitAlarmInfo(eCode, eInfo, eAlarmStartTime, eAlarmKillTime, alarmFlag);
	}
	return TRUE;
}

void CMesDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialogEx::OnSysCommand(nID, lParam);
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMesDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMesDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CMesDlg::OnUpdataSettingConfig(WPARAM, LPARAM)
{
	m_Setting.Load();
	BOOL bMesEn = FALSE;
	if (IsMesMode()) {
		bMesEn = TRUE;
	}
	SetCompentVisible(bMesEn);


	Invalidate();
	return 0;
}

void CMesDlg::OpenScanBarCom()
{
	if (m_HandleScan->IsComOpen()) {
		return;
	}

	tUartPara ComPara;
	ComPara.dwUartPort = m_Setting.nScanComPort;
	ComPara.dwBaudRate = CBR_115200;
	ComPara.wDataWidth = 8;
	ComPara.wParityMode = EVENPARITY  /*ODDPARITY*/; //奇校验
	ComPara.wStopWidth = ONE5STOPBITS;;//
	ComPara.wFlowCtrl = FLOWCTRL_NONE;
	ComPara.dwTimeout = 1;
	ComPara.dwRetryCnt = 3;
	ComPara.dwRetryTimeWait = 0;

	if (m_HandleScan->OpenCom(ComPara.dwUartPort, TRUE) == COMERR_OK) {
		m_HandleScan->SettingCom(ComPara);
		//m_FileLog.PrintLog(LOGLEVEL_ERR, "打开条码扫描串口成功");
		m_LogRichEdit.PrintLog(LOGLEVEL_LOG, "打开条码扫描串口成功");
	}
	else {
		//m_FileLog.PrintLog(LOGLEVEL_ERR, "打开条码扫描串口失败"); 
		m_LogRichEdit.PrintLog(LOGLEVEL_ERR, "打开条码扫描串口失败");
	}
}

void CMesDlg::OnSetting()
{
	// TODO: 在此添加命令处理程序代码
	CUserManager &UserManagerInterface = CUserManager::getInstance();
	UINT64 Permission =  UserManagerInterface.GetPermission(UserManagerInterface.GetUserRole());
	if ((Permission & PERMISSION_SETTING) || !m_bNeedLogin){
		CDlgSetting DlgSetting;
		DlgSetting.AttachData(&m_Setting);
		DlgSetting.AttachIStatus(m_pIStatus);
		if (DlgSetting.DoModal() == IDOK) {
			OnUpdataSettingConfig(0, 0);
			SetAccountUI();
			if (m_strLang.CompareNoCase("ChineseA") == 0) {
				if (MessageBox("修改设置后需要重新启动软件，现在关闭软件?", NULL, MB_YESNO | MB_ICONINFORMATION) == IDYES) {
					m_bNeedCheckQuit = FALSE;
					OnCancel();
				}
			}
			else
			{
				if (MessageBox("You need to restart the software after modifying the settings, close the software now ?", NULL, MB_YESNO | MB_ICONINFORMATION) == IDYES) {
					m_bNeedCheckQuit = FALSE;
					OnCancel();
				}
			}

		}
	}
	else {
		AfxMessageBox("当前用户没有此权限");
	}
	

}

void MyStatus::PrintStatus(INT Level, CString strMsg)
{
	if (m_pStatusBar) {
		switch (Level) {
		case LOGLEVEL_ERR:
			break;
		case LOGLEVEL_LOG:
			break;
		}
		m_pStatusBar->SetWindowTextA(strMsg);
	}
}

void CMesDlg::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类  
	if (m_bTaskDoing == TRUE) {
		if (MessageBox("自动化烧录任务正在进行，请按如下步骤结束：\n1. 在自动化软件里“取消批量”，\n2. 再按“批量生产取消”按钮。",
			NULL, MB_OK | MB_ICONINFORMATION)) {
			return;
		}
	}

	if (m_strLang.CompareNoCase("ChineseA") == 0) {
		if (m_bNeedCheckQuit == TRUE) {
			if (MessageBox("确定退出操作软件么?", NULL, MB_YESNO | MB_ICONINFORMATION) == IDNO) {
				return;
			}
		}
	}
	else
	{
		if (m_bNeedCheckQuit == TRUE) {
			if (MessageBox("Are you sure to exit the operating software? ", NULL, MB_YESNO | MB_ICONINFORMATION) == IDNO) {
				return;
			}
		}
	}


	DeleteCriticalSection(&m_csDownLoad);

	m_bQuit = TRUE;
	mParseWorkerExit = TRUE;
	m_ParseWorker.DeleteThread();
	m_Worker.DeleteThread();
	if (m_Setting.nEnBarPrint == 1) {
		ClosePrintDll();
	}

	CThirdPartyMesAccess &MesInterface = CThirdPartyMesAccess::getInstance();
	MesInterface.DeInit();

	CUserManager &UserManagerInterface = CUserManager::getInstance();
	UserManagerInterface.Save();
	UserManagerInterface.DeInit();

	if (m_bMesTcpSyslog) {
		UnInitMesSyslog();
	}


#ifdef USE_TCP_SYSLOG
	UnInitTcpSyslog();
#endif
	m_StdMes.CloseDllCom();
	SAFEDEL(pModuleProgram);

	//==>>重启关闭软件
	if (m_bRestartSoft == TRUE) {
		TCHAR szFilePath[MAX_PATH + 1];
		TCHAR *pPos = NULL;
		CString strAprogPath;
		m_bRestartSoft = FALSE;///如果从消息框退到这个地方时，将RestartSoft标志去除。
		GetModuleFileName(NULL, szFilePath, MAX_PATH);
		strAprogPath.Format("%s", szFilePath);
		WinExec(strAprogPath, SW_SHOW);
	}
	CDialogEx::OnCancel();

}

void CMesDlg::InitCtrlsValue()
{
	if (m_Setting.nLocalServer) {
		m_strWorkOrder = "001030012634";
		m_strMaterialID = "7459S26I436H0016";
		//m_strAutoTaskFilePath = "C:\\Users\\panwen\\Desktop\\1.tsk";
		//m_strProjPath = "C:\\Users\\panwen\\Desktop\\0x003FC000.apr";
		//m_strProjChecksum = "0x003FC000";
		//m_ExpectICNum = 1;
		//m_strFWPath = "C:\\Users\\panwen\\Desktop\\EYE-SV700.001.00.P011B001.pac";
		m_OperatorData.m_strOperator = "Admin";
	}
	else {
		m_strWorkOrder = _T("");// m_Setting.strWorkOrder;
		m_OperatorData.m_bCheckAuthPass = FALSE;
		m_OperatorData.m_bAuthCancel = FALSE;
		m_OperatorData.m_strOperator = m_Setting.strOperator;
	}

	UpdateData(FALSE);
}

BOOL CMesDlg::GetCtrlsValue()
{
	UpdateData(TRUE);
	m_MainData.strWorkOrder = m_strWorkOrder;
	return TRUE;
}

CString CMesDlg::GetAbsProjSaveFolder()
{
	CString strAbsPath;
	if (m_Setting.strProjSaveFolder.GetAt(0) == '.') {///是相对路径
		CString FolderName = ::GetFolderNameFromRelative(m_Setting.strProjSaveFolder);
		strAbsPath.Format("%s\\%s", GetCurrentPath(), FolderName);
	}
	else {
		strAbsPath = m_Setting.strProjSaveFolder;
	}
	return strAbsPath;
}

void CMesDlg::GetProjectPathFromAutoCreateMode(CString& strPath)
{
	CString strProjPath;
	strProjPath.Format("%s\\_%s_%s.apr", m_Setting.strProjSaveFolder, m_strWorkOrder, ::GetCurTime('_'));
	strPath.Format("%s", strProjPath);  //工程文件路径
}

BOOL CMesDlg::UpdateCtrlsValueFromRecord()
{
	BOOL Ret = TRUE;

	if (IsAutoCreateProject()) {
		GetProjectPathFromAutoCreateMode(m_strProjPath);//Mes 自动合成模式下拼凑工程文件名，否则手动选择
	}

	//m_strProjPath.Format("%s", m_strProjPath);

	m_pILog->PrintLog(0, "MES获取到的信息:");
	m_pILog->PrintLog(0, "当前工单号:%s", m_strWorkOrder);
	m_pILog->PrintLog(0, "当前工单数量:%ld", m_WorkOrderICNum);
	m_pILog->PrintLog(0, "工程校验值:%s", m_strProjChecksum); //m_strProjChecksum
	m_pILog->PrintLog(0, "自动化数据文件的路径:%s", m_strAutoTaskFilePath);
	m_pILog->PrintLog(0, "工程文件路径:%s", m_strProjPath);//m_strProjPath


	UpdateData(FALSE);
	return Ret;
}

#define CheckJsonAttri(_JsonObj,_Name) \
do{\
	if (cJSON_GetObjectItem(_JsonObj,#_Name) == NULL) {\
		m_pILog->PrintLog(0, "JSON中找不到%s属性",#_Name);\
		Ret = FALSE; goto __end;\
	}\
}while(0)

BOOL CMesDlg::GetMesRecord()
{
	if (!IsMesMode()) {
		return TRUE;
	}
	BOOL Ret = FALSE;
	MesInfo mesResult;
	if (m_bUseThirdPartyMesDll) {
		CThirdPartyMesAccess &MesInterface = CThirdPartyMesAccess::getInstance();
		MesInfo mesResult;
		INT result = MesInterface.GetMesRecord(m_strWorkOrder, m_strMaterialID, m_Setting.strCurExec, mesResult);
		if (result == 0) {
			m_strProjPath = mesResult.projPath;
			m_strAutoTaskFilePath = mesResult.autoTaskFilePath;
			m_strProjChecksum = mesResult.projChecksum;
			m_strWorkOrder = mesResult.workOrder;
			m_WorkOrderICNum = mesResult.workOrderICNum;
			m_strChipName = mesResult.chipName;
			m_strProjVersion = mesResult.projVersion;
			Ret = TRUE;
		}
	}
	else {
		CMesInterface &MesInterface = CMesInterface::getInstance();
		if (m_Setting.nServerTest){
			mesResult = MesInterface.GetACMesRecord(m_strWorkOrder, m_strMaterialID, m_Setting.strCurExec);
		}
		else{
			mesResult = MesInterface.GetMesRecord(m_strWorkOrder, m_strMaterialID, m_Setting.strCurExec);
		}
		
		
		if (mesResult.errCode == 0) {
			m_strProjPath = mesResult.projPath;
			m_strAutoTaskFilePath = mesResult.autoTaskFilePath;


			m_strProjChecksum = mesResult.projChecksum;
			m_strWorkOrder = mesResult.workOrder;
			m_strMaterialID = mesResult.materialID;
			if (m_Setting.nServerTest) {
				if (mesResult.RemainICNum == 0){
					m_WorkOrderICNum = mesResult.workOrderICNum;
				}
				else {
					m_WorkOrderICNum = mesResult.RemainICNum;///acmes 返回工单剩余数
				}
			}
			else {
				m_WorkOrderICNum = mesResult.workOrderICNum;
			}
			m_strChipName = mesResult.chipName;
			m_strProjVersion = mesResult.projVersion;
			Ret = TRUE;
		}
	}
	
	return Ret;
}
INT CMesDlg::CommitAlarmInfo(CString alarmCode, CString alarmMsg, CString alarmcStartTime, CString alarmKillTime, int alarmFlag)
{
	if (!IsMesMode()) {
		return 0;
	}
	INT Ret = -1;
	if (m_bUseThirdPartyMesDll) {
		CThirdPartyMesAccess &MesInterface = CThirdPartyMesAccess::getInstance();
		Ret = MesInterface.CommitAlarmInfo2Mes(alarmCode, alarmMsg, alarmcStartTime, alarmKillTime, alarmFlag);
	}
	else {
		CMesInterface &MesInterface = CMesInterface::getInstance();
		Ret = MesInterface.CommitAlarmInfo2Mes(alarmCode, alarmMsg, alarmcStartTime, alarmKillTime, alarmFlag);
	}
	
	return Ret;
}

INT CMesDlg::CommitTaskIngfo(CString timeEnd, CString timeRun, CString timeStop)
{
	if (!IsMesMode()) {
		return 0;
	}
	INT Ret = -1;
	if (m_bUseThirdPartyMesDll) {
		CThirdPartyMesAccess &MesInterface = CThirdPartyMesAccess::getInstance();
		Ret = MesInterface.CommitTaskInfo2Mes(m_strStartTime, timeEnd, timeRun, timeStop);
	}
	else {
		CMesInterface &MesInterface = CMesInterface::getInstance();
		Ret = MesInterface.CommitTaskInfo2Mes(m_strStartTime, timeEnd, timeRun, timeStop);
	}
	
	return Ret;

}


INT CMesDlg::UploadProgramRet2Mes(CString strLastJson, UINT TotalCnt, UINT PassCnt, UINT FailCnt)
{
	if (!IsMesMode()) {
		return 0;
	}
	INT Ret = -1;
	if (m_bUseThirdPartyMesDll) {
		CThirdPartyMesAccess &MesInterface = CThirdPartyMesAccess::getInstance();
		Ret = MesInterface.CommitProgramRet2Mes(strLastJson, TotalCnt, PassCnt, FailCnt);
	}
	else {
		CMesInterface &MesInterface = CMesInterface::getInstance();
		if (m_bGetCmd4Success && !strLastJson.IsEmpty()) {
			cJSON* pRecvRet;
			pRecvRet = cJSON_Parse(strLastJson.GetBuffer());
			cJSON_DeleteItemFromObject(pRecvRet, "Fail");
			cJSON_AddNumberToObject(pRecvRet, "Fail", FailCnt);
			cJSON_DeleteItemFromObject(pRecvRet, "Pass");
			cJSON_AddNumberToObject(pRecvRet, "Pass", PassCnt);
			cJSON_DeleteItemFromObject(pRecvRet, "Total");
			cJSON_AddNumberToObject(pRecvRet, "Total", TotalCnt);
			CString strBuildJson = cJSON_Print(pRecvRet);
			strLastJson = strBuildJson;
		}
		if (m_Setting.nServerTest){
			Ret = MesInterface.CommitProgramRet2ACMes(strLastJson);
		}
		else{
			INT maxTryTime = 3;
			CFileCacheManager &fileCacheManager = CFileCacheManager::getInstance();
			fileCacheManager.SaveResult2File(m_strCacheSavePath, strLastJson);
			while (maxTryTime-- > 0) {
				Ret = MesInterface.CommitProgramRet2Mes(strLastJson);
				if (Ret == 0) {
					break;
				}
			}
			if (Ret == 0) {
				CFileCacheManager &fileCacheManager = CFileCacheManager::getInstance();
				fileCacheManager.RemoveResultFile(m_strCacheSavePath);
			}
		}
		
	}

	return Ret;
}

bool CMesDlg::DoCompareCheckSum()
{
	bool bRet = true;

	if (!m_strProjChecksum.IsEmpty() && !m_strRealChecksum.IsEmpty()) { //
		if (m_strProjChecksum.CompareNoCase(m_strRealChecksum) != 0) {
			bRet = false;
		}
	}

	return bRet;
}



void CMesDlg::UpdataDestDataForProduce()
{
	UpdateData(TRUE);

	//将数据赋值到目的
	m_ProgRecord.DestRecord.strWorkOrder = m_strWorkOrder;
	m_ProgRecord.DestRecord.strAutoTaskPath = m_strAutoTaskFilePath;
	m_ProgRecord.DestRecord.strProjPath = m_strProjPath;
	m_ProgRecord.DestRecord.strProgFilePath = m_strProgramPath;
}

std::wstring CMesDlg::MultiByteToWideChar(const char* pszMultiByte, UINT uCodePage /* = CP_ACP */)
{
	try
	{
		if (NULL == pszMultiByte)
		{
			throw - 1;
		}

		int iMultiBytes = ::MultiByteToWideChar(uCodePage, 0, pszMultiByte, -1, NULL, 0);

		if (iMultiBytes == 0)
		{
			throw - 1;
		}

		wchar_t* pwszMultiByte = new wchar_t[iMultiBytes + 1];

		pwszMultiByte[iMultiBytes] = 0;
		if (!pwszMultiByte)
		{
			throw - 1;
		}

		::MultiByteToWideChar(uCodePage, 0, pszMultiByte, -1, pwszMultiByte, iMultiBytes);

		std::wstring wstrWideChar = pwszMultiByte;
		delete[] pwszMultiByte;
		pwszMultiByte = NULL;

		return wstrWideChar.c_str();
	}
	catch (...)
	{
		return L"";
	}
}

std::string CMesDlg::WideCharToMultiByte(const wchar_t* pwszMultiByte, UINT uCodePage /* = CP_ACP */)
{
	try
	{
		if (NULL == pwszMultiByte)
		{
			throw - 1;
		}

		int iMultiBytes = ::WideCharToMultiByte(uCodePage, 0, pwszMultiByte, -1, NULL, 0, NULL, FALSE);

		if (iMultiBytes == 0)
		{
			throw - 1;
		}

		char* pszMultiByte = new char[iMultiBytes + 1];

		pszMultiByte[iMultiBytes] = 0;
		if (!pszMultiByte)
		{
			throw - 1;
		}

		::WideCharToMultiByte(uCodePage, 0, pwszMultiByte, -1, pszMultiByte, iMultiBytes, NULL, FALSE);

		std::string strMultiChar = pszMultiByte;
		delete[] pszMultiByte;
		pszMultiByte = NULL;

		return strMultiChar.c_str();
	}
	catch (...)
	{
		return "";
	}
}

wchar_t* CMesDlg::AnsiToUnicode(const char* szStr)
{
	int nLen = ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szStr, -1, NULL, 0);
	if (nLen == 0)
	{
		return NULL;
	}
	wchar_t* pResult = new wchar_t[nLen];
	::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szStr, -1, pResult, nLen);
	return pResult;
}
//将宽字节wchar_t*转化为单字节char*
char* CMesDlg::UnicodeToAnsi(const wchar_t* szStr)
{
	int nLen = ::WideCharToMultiByte(CP_ACP, 0, szStr, -1, NULL, 0, NULL, NULL);
	if (nLen == 0)
	{
		return NULL;
	}
	char* pResult = new char[nLen];
	::WideCharToMultiByte(CP_ACP, 0, szStr, -1, pResult, nLen, NULL, NULL);
	return pResult;
}

void CMesDlg::OnBnClickedBtnGetmesrecord()
{
	UpdateData(TRUE);
	BOOL Ret = FALSE;
	CString strLogFile;
	CUserManager &UserManagerInterface = CUserManager::getInstance();
	UINT64 Permission = UserManagerInterface.GetPermission(UserManagerInterface.GetUserRole());
	if ((Permission & PERMISSION_GETMESRECOD) || !m_bNeedLogin){
		if (IsMesMode()) {
			m_bProgRecordReady = FALSE;
		}

		m_pIStatus->PrintStatus(0, "");
		m_AutoPosSetting.Init();///重新初始化自动化设备坐标
		if (m_Setting.strProgramMode.CompareNoCase("Module") == 0) {
			if (pModuleProgram != NULL) {
				pModuleProgram->UpdatePosSetting(m_AutoPosSetting);
			}
		}
		else {
			if (pNormalProgram != NULL) {
				pNormalProgram->UpdatePosSetting(m_AutoPosSetting);
			}
		}

		// 检查必需参数
		if (m_strWorkOrder.IsEmpty()) {
			if (m_strLang.CompareNoCase("ChineseA") == 0) {
				MessageBox("工单号不能为空，请确认!", NULL, MB_OK | MB_ICONERROR);
			}
			else{
				MessageBox("The ticket number cannot be empty, please confirm !", NULL, MB_OK | MB_ICONERROR);
			}
			return;
		}

		/*if (m_strMaterialID.IsEmpty()) {
			if (m_strLang.CompareNoCase("ChineseA") == 0) {
				AfxMessageBox("料号不能为空");
			}
			else{
				AfxMessageBox("The part number cannot be empty");
			}
			return;
		}*/
		
		// 检查箱单条码
		if (m_strBoxSN.IsEmpty()) {
			if (m_strLang.CompareNoCase("ChineseA") == 0) {
				AfxMessageBox("箱单条码不能为空，请使用USB扫码枪扫描！");
			}
			else{
				AfxMessageBox("Box SN cannot be empty, please scan with USB scanner!");
			}
			return;
		}
		
		// 检查机台编号（如果为空则使用默认值）
		if (m_strStationId.IsEmpty()) {
			m_strStationId = m_Setting.strWorkStationID;  // 从配置文件获取
			if (m_strStationId.IsEmpty()) {
				m_strStationId = "STATION_001";  // 默认值
			}
			UpdateData(FALSE);  // 更新界面显示
			m_pILog->PrintLog(LOGLEVEL_LOG, "机台编号为空，已设置为: %s", m_strStationId);
		}
		
		// 批号使用工单号的值
		m_strBatNo = m_strWorkOrder;
		
		// 设置MES额外参数
		CMesInterface &MesInterface = CMesInterface::getInstance();
		MesInterface.SetExtraParams(
			m_strBoxSN,              // 箱单条码（USB扫码枪输入）
			m_strBatNo,              // 批号（使用工单号）
			m_strStationId,          // 机台编号
			m_Setting.strOperator    // 操作员账号（登录用户）
		);
		
		// 打印参数信息用于调试
		m_pILog->PrintLog(LOGLEVEL_LOG, "MES请求参数:");
		m_pILog->PrintLog(LOGLEVEL_LOG, "  工单号(mo_no): %s", m_strWorkOrder);
		m_pILog->PrintLog(LOGLEVEL_LOG, "  箱单条码(box_sn): %s", m_strBoxSN);
		m_pILog->PrintLog(LOGLEVEL_LOG, "  批号(bat_no): %s", m_strBatNo);
		m_pILog->PrintLog(LOGLEVEL_LOG, "  机台编号(rs_no): %s", m_strStationId);
		m_pILog->PrintLog(LOGLEVEL_LOG, "  操作员(wk_no): %s", m_Setting.strOperator);

		m_Setting.strWorkOrder = m_strWorkOrder;
		m_Setting.Save();

		m_ProgRecord.DestRecord.bProjSelDirect = FALSE;

		m_pILog->PrintLog(0, "正在进行访问获取Mes信息... ");
		m_pIStatus->PrintStatus(0, "正在访问获取Mes信息... ");

		Ret = GetMesRecord();
		if (Ret != TRUE) {
			goto __end;
		}

		UpdateCtrlsValueFromRecord();

		if (IsMesMode()) {
			m_bProgRecordReady = TRUE;
		}
	}
	else {
		AfxMessageBox("当前用户没有此权限");
	}
	

__end:
	if (Ret == TRUE) {
		m_pILog->PrintLog(0, "获取生产任务信息成功");
		m_pIStatus->PrintStatus(0, "获取生产任务信息成功");
	}
	else {
		m_pILog->PrintLog(LOGLEVEL_ERR, "获取生产任务信息失败");
		m_pIStatus->PrintStatus(LOGLEVEL_ERR, "获取生产任务信息失败");
	}
	return;
}

void CMesDlg::UpdateProduceWinPos()
{
	CRect ProcWin;
	CRect NewProcWin;
	if (this->GetSafeHwnd() && m_DlgProduceInfo.GetSafeHwnd()) {
		RECT RectClient;
		GetClientRect(&RectClient);
		ClientToScreen(&RectClient);
		m_DlgProduceInfo.GetWindowRect(ProcWin);
		NewProcWin.top = RectClient.top;
		NewProcWin.left = RectClient.right;
		NewProcWin.right = ProcWin.Width() + NewProcWin.left;
		//NewProcWin.bottom = ProcWin.Height() + NewProcWin.top;
		NewProcWin.bottom = NewProcWin.top + RectClient.bottom - RectClient.top;
		m_DlgProduceInfo.MoveWindow(NewProcWin);
		m_DlgProduceInfo.ShowWindow(SW_SHOW);
	}
}

void CMesDlg::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);
	if (m_DlgProduceInfo.GetSafeHwnd()) {
		if (m_DlgProduceInfo.IsWindowVisible() == TRUE) {
			UpdateProduceWinPos();
		}
	}
	// TODO: 在此处添加消息处理程序代码
}

CString CMesDlg::GetAbsAutoTaskDir()
{
	return m_strAutoTaskFilePath;
}

CString CMesDlg::GetAutoTaskFolder()
{
	CString strAbsPath;
	if (m_Setting.strAutoTaskFolder.GetAt(0) == '.') {///是相对路径
		CString FolderName = ::GetFolderNameFromRelative(m_Setting.strAutoTaskFolder);
		strAbsPath.Format("%s\\%s", GetCurrentPath(), FolderName);
	}
	else {
		strAbsPath = m_Setting.strAutoTaskFolder;
	}
	return strAbsPath;
}


CString CMesDlg::GetProjSaveAbsPath()
{
	CString strAbsPath;
	if (m_Setting.strProjSaveFolder.GetAt(0) == '.') {///是相对路径
		CString FolderName = ::GetFolderNameFromRelative(m_Setting.strProjSaveFolder);
		strAbsPath.Format("%s\\%s", GetCurrentPath(), FolderName);
	}
	else {
		strAbsPath = m_Setting.strProjSaveFolder;
	}
	return strAbsPath;
}

void CMesDlg::GetDirFullPath(CString strExts, CString&strFileOpen)
{
	CFileDialog Dlg(TRUE, NULL, NULL, OFN_PATHMUSTEXIST, strExts, this);
	if (Dlg.DoModal() == IDOK) {
		strFileOpen = Dlg.GetPathName();
	}
	else {
		strFileOpen = "";
	}
}

INT CMesDlg::GetDirSelect(CString Title, CString&strDirPath)
{
	char szPath[MAX_PATH];     //存放选择的目录路径 
	ZeroMemory(szPath, sizeof(szPath));
	BROWSEINFO bi;
	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = szPath;
	bi.lpszTitle = Title.GetBuffer();
	bi.ulFlags = 0;
	bi.lpfn = NULL;
	bi.lParam = 0;
	bi.iImage = 0;
	//弹出选择目录对话框
	LPITEMIDLIST lp = SHBrowseForFolder(&bi);
	if (lp && SHGetPathFromIDList(lp, szPath)) {
		strDirPath.Format("%s", szPath);
		return 0;
	}
	else {
		//MessageBox("无效的目录，请重新选择",NULL,MB_OK|MB_ICONERROR);
		return -1;
	}
}

void CMesDlg::OnBnClickedBtnSelprogrampath()
{
	GetDirFullPath("", m_strProgramPath);
	UpdateData(FALSE);
}

void CMesDlg::OnBnClickedBtnSeltemplatepath()
{
	GetDirFullPath("Project Template(*.aprtpl)|*.aprtpl||", m_strTemplateFilePath);
	UpdateData(FALSE);
}

void CMesDlg::OnBnClickedBtnSelsncpath()
{
	GetDirFullPath("SN Config(*.snc)|*.snc||", m_strSNCPath);
	//LoadSNC();
	UpdateData(FALSE);
}

void CMesDlg::OnBnClickedBtnSelprojpath()
{
	UpdateData(TRUE);
	GetDirFullPath("Acroview Programmer Project(*.apr)|*.apr||", m_strProjPath);
	//if (GetPrjCheckSumFromName(m_strProjPath, m_strProjChecksum) == FALSE) { //统一放到这只做一次检查
		//AfxMessageBox("获取工程检验值失败");
	//}
	UpdateData(FALSE);
}

void CMesDlg::OnBnClickedBtnSelautotaskfilepath()
{
	UpdateData(TRUE);
	CString strAutoTaskDataExt = m_Setting.strAutoTaskFileExt;
	strAutoTaskDataExt.Format("tsk");//固定为tsk
	CString strFileFilter;
	if (strAutoTaskDataExt.IsEmpty()) {
		strFileFilter.Format("%s", "");
	}
	else {
		strFileFilter.Format("AutoTaskData(*.%s)|*.%s|| ", strAutoTaskDataExt, strAutoTaskDataExt);
	}

	GetDirFullPath(strFileFilter, m_strAutoTaskFilePath);

	UpdateData(FALSE);
}

void CMesDlg::OnAutotaskdata()
{
	// TODO: 在此添加命令处理程序代码
	CDlgAutoDataRecord DlgAutoData;
	MessageBox("请使用自动化软件进行自动化数据的录入，并将结果文件保存到设置界面中设定的自动化任务数据所在文件夹位置");
	//DlgAutoData.AttachSetting(&m_Setting);
	//DlgAutoData.DoModal();
}


CString CMesDlg::GetTemplateAbsPath()
{
	CString strAbsFolder;
	CString strAbsPath;
	if (m_Setting.strProjTemplateFolder.GetAt(0) == '.') {///是相对路径
		CString FolderName = ::GetFolderNameFromRelative(m_Setting.strProjTemplateFolder);
		strAbsFolder.Format("%s\\%s", GetCurrentPath(), FolderName);
	}
	else {
		strAbsFolder = m_Setting.strProjTemplateFolder;
	}
	strAbsPath.Format("%s\\%s", strAbsFolder, m_ProgRecord.ExcelRecord.strTemplateFile);
	return strAbsPath;
}

CString CMesDlg::GetProgFileAbsSettingFolder()
{
	CString strAbsFolder;
	CString strAbsPath;
	if (m_Setting.strProgFileFolder.GetAt(0) == '.') {///是相对路径
		CString FolderName = ::GetFolderNameFromRelative(m_Setting.strProgFileFolder);
		strAbsFolder.Format("%s\\%s", GetCurrentPath(), FolderName);
	}
	else {
		strAbsFolder = m_Setting.strProgFileFolder;
	}
	strAbsPath.Format("%s\\%s", strAbsFolder, m_ProgRecord.ExcelRecord.strProjFile);
	return strAbsPath;
}

INT CMesDlg::StartService()
{
	INT Ret = 0, RtnCall;
	BOOL Rtn;
	CString strMsg;

	if (m_Setting.strProgramMode.CompareNoCase("UFSWriter") == 0) {

		CallAutoApp();//==启动 UFS Writer UFS 软件 ，使用命令行自带起来  start.exe

					  //== 这里顺便把TCP 客户端起来.。
		Ret = CreateTcpClient();
		if (Ret != 0){
			Ret = -1; goto __end;
		}

	}
	else {
		if (1) {
			m_LogRichEdit.PrintLog(LOGLEVEL_LOG, "启动服务器...");
			Rtn = m_StdMes.MesStartService();
		}
		else {
			m_LogRichEdit.PrintLog(LOGLEVEL_LOG, "启动简单服务器...");
			Rtn = m_StdMes.MesStartServiceSimple();
		}

		m_LogRichEdit.PrintLog(LOGLEVEL_LOG, "启动服务器 %s", Rtn == TRUE ? "成功" : "失败");
		if (Rtn == FALSE) {
			Ret = -1; goto __end;
		}

		m_pILog->PrintLog(LOGLEVEL_LOG, "设置编程控制系统消息回调函数...");
		RtnCall = m_StdMes.MesSetMsgHandle(MsgHandle, this);
		m_pILog->PrintLog(LOGLEVEL_LOG, "设置消息回调函数 %s", RtnCall == 1 ? "成功" : "失败");
		if (RtnCall == 0) {
			Ret = -1; goto __end;
		}
	}

	

__end:
	return Ret;
}

BOOL CMesDlg::GetPrjCheckSumFromName(CString strPath, CString& strPrjCheckSum)
{
	BOOL Ret = FALSE;
	int nIdx = 0;
	if (strPath.IsEmpty()) {
		goto __end;
	}
	nIdx = strPath.ReverseFind('-');
	if (nIdx <= 0) {
		goto __end;
	}

	strPath.Delete(0, (nIdx + 1));
	strPath.Replace(".apr", "");
	strPrjCheckSum.Format("%s", strPath);

	Ret = TRUE;

__end:

	if (Ret == FALSE) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "获取校验值失败");
	}
	return Ret;
}


int _stdcall MsgHandle(void*Para, char*Msg, char*MsgData)
{
	int Ret = 0;
	CMesDlg *pDlg = (CMesDlg*)Para;
	if (pDlg) {
		pDlg->HandleMsg(Msg, MsgData);
	}
	return Ret;
}

int CMesDlg::HandleMsg(char*Msg, char*MsgData)
{
	int Ret = 0;
	CString strMsg;

	//push back msg to message list, wait ParseTask to parse.
	if (_stricmp(Msg, "RecvSyslog")==0) {
		std::string strMsg = MsgData;
		//m_pILog->PrintLog(LOGLEVEL_LOG, "TCPSyslog Recv msg: %s", strMsg.c_str());

		//push back msg to message list, wait ParseTask to parse.
		m_MsgListMutex.Lock();
		mRecvMsgList.push_back(strMsg);
		m_MsgListMutex.Unlock();
	}
	else {
		m_ProgramMsgListMutex.Lock();
		tStdMesMessage stdMesMsg;
		stdMesMsg.msg = Msg;
		stdMesMsg.json = MsgData;
		mRecvProgramMsgList.push_back(stdMesMsg);
		m_ProgramMsgListMutex.Unlock();
	}

	return Ret;
}

INT CMesDlg::LoadSNC()
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
		m_LogRichEdit.PrintLog(LOGLEVEL_WARNING, "请注意未使用SNC配置文件 ");
		return TRUE;
	}

	if (m_Setting.nReInitIndex == 1) { //使用配置文件中的值
		m_LogRichEdit.PrintLog(LOGLEVEL_LOG, "SNC配置使用的是config中的自定义参数, nStartIndex=%d, nTotalNum=%d, nProgramIndex=%d, ",
			m_Setting.nStartIndex, m_Setting.nTotalNum, m_Setting.nProgramIndex);
		m_StdMes.MesLoadSNC((LPSTR)(LPCSTR)strFileName, 1, m_Setting.nStartIndex, m_Setting.nTotalNum, m_Setting.nProgramIndex);
		goto __end;
	}

	if (cFile.Open(strFileName, CFile::modeRead, &ex) == FALSE) {
		TCHAR szError[1024];
		ex.GetErrorMessage(szError, 1024);
		strErr.Format("打开SNC文件失败，路径=%s, 错误原因=%s", (LPSTR)(LPCSTR)strFileName, szError);
		m_LogRichEdit.PrintLog(LOGLEVEL_ERR, "%s", strErr);
		Ret = FALSE;
		goto __end;
	}

	FileSize = (DWORD)cFile.GetLength();
	pBuf = new BYTE[FileSize];
	if (pBuf == NULL) {
		m_LogRichEdit.PrintLog(LOGLEVEL_ERR, "Memory alloc failed for reading snc file");
		Ret = FALSE; goto __end;
	}

	if (cFile.Read(pBuf, FileSize) != FileSize) {
		m_LogRichEdit.PrintLog(LOGLEVEL_ERR, "Read SNC File failed");
		Ret = FALSE; goto __end;
	}
	lSerial.SerialInBuff(pBuf, FileSize);

	lSerial >> crc32;
	lSerial >> dwStartIndex >> dwTotalNum;
	lSerial >> strChipName >> strCurSNGen >> InitBufSize;

	pSNCFGInitBuf = new BYTE[InitBufSize];
	if (pSNCFGInitBuf == NULL) {
		m_LogRichEdit.PrintLog(LOGLEVEL_ERR, "Memory alloc failed for reading snc file");
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

INT CMesDlg::ThreadParserHandler(MSG msg, void *Para)
{
	INT Ret = 0;
	if (msg.message == MSG_PARSE_START_WORK) {
		Ret = DoParseTask();
	}
	return -1;///为了让线程处理完消息之后自动退出，返回-1不在进行消息的获取
}

INT CMesDlg::ThreadSendStopCmdHandler(MSG msg, void *Para)
{
	INT Ret = 0;
	if (msg.message == MSG_SEND_STOP_WORK) {
		Ret = DoSendStopCmdTask();
	}
	return -1;///为了让线程处理完消息之后自动退出，返回-1不在进行消息的获取
}

///成功返回0，失败返回-1.
///成功时参数返回实际档案最终路径
INT CMesDlg::GetProgFileAbsPath(CString&strProgFileAbsPath)
{
	INT Ret = 0;

	if (m_ProgRecord.MesRecord.strProgFile != "") {
		strProgFileAbsPath = m_ProgRecord.MesRecord.strProgFile;
	}
	else {
		if (m_ProgRecord.ExcelRecord.strProgFile != "") {
			CString FileName = m_ProgRecord.ExcelRecord.strProgFile;
			CString FileFolder = GetProgFileAbsSettingFolder();
			strProgFileAbsPath.Format("%s\\%s", FileFolder, FileName);
		}
		else {
			m_pILog->PrintLog(LOGLEVEL_ERR, "找不到烧录档案路径");
			Ret = -1; goto __end;
		}
	}

__end:

	return Ret;
}

///开始生产之前更新数据
BOOL CMesDlg::UpdateRecordForProduce()
{
	BOOL Ret = TRUE;
	UpdateData(TRUE);
	m_ProgRecord.ResetDestRecord();

	if (m_ExpectICNum == 0) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "实际期望OK数不能为0，请确认");
		Ret = FALSE; goto __end;
	}
	if (IsMesMode()) {
		if (m_ExpectICNum > m_WorkOrderICNum) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "实际加工OK数不能大于工单剩余数量");
			Ret = FALSE; goto __end;
		}
	}

	if (m_ExpectICNum >= 5) {
		if (MessageBox("批量生产前请进行首样验证，否则会造成芯片批量损坏\r\n选择“是”继续，“否”退出", NULL, MB_YESNO | MB_ICONINFORMATION) == IDNO) {
			Ret = FALSE; goto __end;
		}
		m_pILog->PrintLog(LOGLEVEL_WARNING, "客户确定已经进行首样验证。");
	}
	if (m_strAutoTaskFilePath.IsEmpty() && IsAutoMode()) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "自动化任务文件不能为空，请确认");
		Ret = FALSE; goto __end;
	}

	if (!IsMesMode() && IsAutoCreateProject()) {
		//mes未使能，自动合成
		if (m_strProgramPath.IsEmpty()) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "自动合成模式下烧录档案路径不能为空");
			Ret = FALSE; goto __end;
		}

		if (m_strTemplateFilePath.IsEmpty()) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "自动合成模式下模板路径不能为空 ");
			Ret = FALSE; goto __end;
		}

		GetProjectPathFromAutoCreateMode(m_strProjPath);
		UpdateData(FALSE);
	}

	if (!IsMesMode() && !IsAutoCreateProject()) {
		//mes未使能，非自动合成
		if (m_strProjPath.IsEmpty()) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "此模式下工程文件路径不能为空");
			Ret = FALSE; goto __end;
		}

		if (m_strProjChecksum.IsEmpty()) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "工程文件检验值不能为空");
			Ret = FALSE; goto __end;
		}
	}

	m_ProgRecord.DestRecord.strWorkOrder = m_strWorkOrder;
	m_ProgRecord.DestRecord.strOperatorID = m_OperatorData.m_strOperator;
	m_ProgRecord.DestRecord.ExpertIDNum = m_ExpectICNum;
	m_ProgRecord.DestRecord.strAutoTaskPath = GetAbsAutoTaskDir();
	m_ProgRecord.DestRecord.strProjPath = m_strProjPath;
	m_ProgRecord.DestRecord.strMaterialID = m_strMaterialID;

	//if (GetPrjCheckSumFromName(m_strProjPath, m_strProjChecksum) == FALSE){ //统一放到这只做一次检查
	//	Ret = FALSE; goto __end;
	//}

	m_ProgRecord.DestRecord.strProjChecksum = m_strProjChecksum;
	m_ProgRecord.DestRecord.strChipName = m_strChipName;
	m_ProgRecord.DestRecord.strSoftVer = m_strProjVersion;

	m_ProgRecord.DestRecord.strProgFilePath = m_strProgramPath;
	m_ProgRecord.DestRecord.strProgFileChecksum = m_strProgFileChecksum;
	m_ProgRecord.DestRecord.strTempFilePath = m_strTemplateFilePath;

	m_ProgRecord.DestRecord.m_WorkOrderICNum.Format("%ld", m_WorkOrderICNum);

	m_pILog->PrintLog(LOGLEVEL_LOG, "======批量生产任务信息======");
	m_pILog->PrintLog(LOGLEVEL_LOG, "当前工单号:%s", m_ProgRecord.DestRecord.strWorkOrder);
	m_pILog->PrintLog(LOGLEVEL_LOG, "当前工单数量:%s", m_ProgRecord.DestRecord.m_WorkOrderICNum);
	m_pILog->PrintLog(LOGLEVEL_LOG, "当前期望OK数量:%d", m_ProgRecord.DestRecord.ExpertIDNum);

	//m_pILog->PrintLog(LOGLEVEL_LOG, "当前工单号:%s", m_strWorkOrder);
	//m_pILog->PrintLog(LOGLEVEL_LOG, "芯片型号:%s", m_ProgRecord.DestRecord.strChipName);
	//m_pILog->PrintLog(LOGLEVEL_LOG, "软件料号:%s", m_ProgRecord.DestRecord.strSoftVer);

	/*m_pILog->PrintLog(LOGLEVEL_LOG, "模板路径:%s", m_ProgRecord.DestRecord.strTempFilePath);
	m_pILog->PrintLog(LOGLEVEL_LOG, "烧录档案路径:%s", m_ProgRecord.DestRecord.strProgFilePath);
	m_pILog->PrintLog(LOGLEVEL_LOG, "烧录档案校验值:%s", m_ProgRecord.DestRecord.strProgFileChecksum);*/

	m_pILog->PrintLog(LOGLEVEL_LOG, "自动化数据文件的路径:%s", m_ProgRecord.DestRecord.strAutoTaskPath);
	m_pILog->PrintLog(LOGLEVEL_LOG, "工程校验值:%s", m_ProgRecord.DestRecord.strProjChecksum);
	m_pILog->PrintLog(LOGLEVEL_LOG, "工程文件路径:%s", m_ProgRecord.DestRecord.strProjPath);
	m_pILog->PrintLog(LOGLEVEL_LOG, "========================");

__end:
	return Ret;
}


INT CMesDlg::SetElectricInsertionCheck()
{
	INT Ret = 0;
	BOOL RetWrite;
	INT bRemoveChkEnable = 0;
	INT bInsertCheckEnable = 0;
	INT bMesMode = 0;
	INT bSyslogMode = 0;
	INT bPushSyslog = 0;
	INT ServerPort = 0;
	CString strLotEndTemp;
	CString strIniFile;
	CHAR TmpBuf[MAX_PATH];
	CString strAutomaicIniPath = _T("");
	if (m_Setting.strAutomaticType.CompareNoCase("IPS5000") == 0) {
		strAutomaicIniPath = "C:\\PH-5000\\Config\\System.ini";
	}else if (m_Setting.strAutomaticType.CompareNoCase("IPS3000") == 0 || m_Setting.strAutomaticType.CompareNoCase("IPS5200") == 0) {
		strAutomaicIniPath = "C:\\IPS\\Config\\System.ini";
	}else if (m_Setting.strAutomaticType.CompareNoCase("IPS7000") == 0 || m_Setting.strAutomaticType.CompareNoCase("PHA2000") == 0) {
		m_LogRichEdit.PrintLog(LOGLEVEL_LOG, "自动化软件版本 必须为 Ver.1.0.11.15(L3) 版本");
		strAutomaicIniPath = "C:\\PH-A2000\\SysData\\SysData.ini";
	}else {
		m_LogRichEdit.PrintLog(LOGLEVEL_ERR, "不支持的自动化设备类型%s", m_Setting.strAutomaticType);
	}
	if (strAutomaicIniPath.IsEmpty() || (PathFileExists(strAutomaicIniPath) == FALSE)) {
		m_LogRichEdit.PrintLog(LOGLEVEL_ERR, "没有找到自动化配置文件%s", strAutomaicIniPath);
	}
	else {
		if (!m_Setting.nLocalServer)
		{
			//== 检查自动化软件版本
			bMesMode = GetPrivateProfileInt("SYSTEM", "MesMode", 1, strAutomaicIniPath);
			if (bMesMode != 1) {
				MessageBox("自动化配置 System.ini 文件,请配置 MesMode 字段 ", NULL, MB_OK | MB_ICONERROR);
				m_LogRichEdit.PrintLog(LOGLEVEL_ERR, "自动化配置System.ini文件, MesMode 字段，请添加字段！");
				Ret = -1; goto __end;
			}

			bSyslogMode = GetPrivateProfileInt("SysLog", "SysLogEnable", 1, strAutomaicIniPath);

			bPushSyslog = GetPrivateProfileInt("SysLog", "PushMothod", 1, strAutomaicIniPath);
			if (bPushSyslog != 1 || bSyslogMode != 1) {
				MessageBox("自动化配置 System.ini 文件,请开启Syslog推送 ", NULL, MB_OK | MB_ICONERROR);
				m_LogRichEdit.PrintLog(LOGLEVEL_ERR, "自动化配置System.ini文件, 请将 SysLogEnable字段设置为 1，PushMothod字段设置为 1 ");
				Ret = -1; goto __end;
			}

			ServerPort = GetPrivateProfileInt("MES", "SeverPort", 1, strAutomaicIniPath);
			if (ServerPort != 1000) {
				MessageBox("自动化系统环境配置,请开启1000端口 ", NULL, MB_OK | MB_ICONERROR);
				m_LogRichEdit.PrintLog(LOGLEVEL_ERR, "自动化系统环境配置,开启Mes 1000端口 ");
				Ret = -1; goto __end;
			}


			memset(TmpBuf, 0, MAX_PATH);
			GetPrivateProfileString("Other", "RemoveChkEnable", "FALSE", TmpBuf, MAX_PATH, strAutomaicIniPath);
			if (_stricmp(TmpBuf, "FALSE") == 0) {
				bRemoveChkEnable = 0;
			}
			else if (_stricmp(TmpBuf, "TRUE") == 0) {
				bRemoveChkEnable = 1;
			}

			memset(TmpBuf, 0, MAX_PATH);
			GetPrivateProfileString("CustomizationFunction", "LotEndMultiAprogReport", "FALSE", TmpBuf, MAX_PATH, strAutomaicIniPath);

			if (TmpBuf[0] != 0)
			{
				strLotEndTemp.Format("%s", TmpBuf);
				if (strLotEndTemp.CompareNoCase("FALSE") == 0) {
					MessageBox("自动化配置 System.ini 文件, LotEndMultiAprogReport字段 需设置为 TRUE ", NULL, MB_OK | MB_ICONERROR);
					m_LogRichEdit.PrintLog(LOGLEVEL_LOG, "自动化配置 System.ini 文件, LotEndMultiAprogReport字段 需设置为 TRUE ，请修改配置字段！");
					Ret = -1; goto __end;
				}
			}
			bInsertCheckEnable = GetPrivateProfileInt("Other", "UseRecontact", 1, strAutomaicIniPath);

			m_pILog->PrintLog(LOGLEVEL_LOG, "自动机残料检测功能:%s 接触检查次数:%d", bRemoveChkEnable ? "开启" : "关闭", bInsertCheckEnable);
			m_Setting.nElectricInsertCheck = bRemoveChkEnable || bInsertCheckEnable > 0;
			m_Setting.Save();
			m_pILog->PrintLog(LOGLEVEL_LOG, "Mes电子检测功能:%s", m_Setting.nElectricInsertCheck ? "开启" : "关闭");
		}
	
	}
	if (m_Setting.strACServerFolder != "") {///
		memset(TmpBuf, 0, MAX_PATH);
		sprintf(TmpBuf, "%d", m_Setting.nElectricInsertCheck);
		strIniFile.Format("%s\\mes\\StdMes.ini", m_Setting.strACServerFolder);
		RetWrite = WritePrivateProfileString("Config", "ElectricInsertionCheck", (LPCTSTR)TmpBuf, strIniFile);
		if (RetWrite == FALSE) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "残料检测功能同步失败");
			Ret = -1;
		}
		else {
			m_pILog->PrintLog(LOGLEVEL_LOG, "残料检测功能同步成功");
		}
	}
	else {
		m_pILog->PrintLog(LOGLEVEL_ERR, "请先设置服务器的路径");
		Ret = -1;
	}

__end:

	return Ret;
}

bool CMesDlg::IsMesMode()
{
	bool bRet = false;
	if (m_Setting.strMesWordMode.CompareNoCase("Enable") == 0) {
		bRet = true;
	}
	return bRet;
}

bool CMesDlg::IsAutoMode() {
	bool bRet = false;
	if (m_Setting.strAutoMode.CompareNoCase("Auto") == 0) {
		bRet = true;
	}
	return bRet;
}

bool CMesDlg::IsAutoCreateProject()
{
	return false; //固定为非自动合成

	bool bRet = false;
	if (m_Setting.strProjectMode.CompareNoCase("AutoCreate") == 0) {
		bRet = true;
	}
	return bRet;
}


bool CMesDlg::CheckSelComboCom()
{
	bool bRet = true;
	CString strValue;

	for (int i = 0; i < SITE_COUNT; i++) {
		if (((CButton*)GetDlgItem(IDC_CHECK1 + i))->GetCheck()) {
			(GetDlgItem(IDC_EDIT1 + i))->GetWindowText(strValue);

			if (strValue.IsEmpty()) {
				bRet = false;
				break;
			}
		}
	}

	return bRet;
}

INT CMesDlg::SetComMapToJson(CString strComMap)
{
	INT Ret = 0;
	CString strjsonPath;
	CFile jsonFile;
	CString strMsg, strjson, strBuildJson, dateFolder;
	BYTE *pData = NULL;
	cJSON* RootBuild = NULL;
	cJSON* JsonData = NULL;
	cJSON* pRootParser = NULL;
	cJSON* pArrayItem = NULL;
	cJSON* pItemObj = NULL;
	INT Size = 0, Total = 0;

	strjsonPath.Format("%s\\Module.json", ::GetCurrentPath());

	//打开文件
	if (jsonFile.Open(strjsonPath, CFile::modeNoTruncate | CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive, NULL) == FALSE) {
		strMsg.Format("打开%s文件失败\r\n", strjsonPath);
		m_pILog->PrintLog(LOGLEVEL_ERR, strMsg.GetBuffer());
		goto __end;
	}

	Size = (INT)jsonFile.GetLength();
	pData = new BYTE[Size + 1];
	if (!pData) {
		goto __end;
	}
	memset(pData, 0, Size + 1);
	jsonFile.Read(pData, Size);
	strjson.Format("%s", (pData));

	jsonFile.Close();
	if (jsonFile.Open(strjsonPath, CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive, NULL) == FALSE) {//清空文件内容
		strMsg.Format("打开%s文件失败\r\n", strjsonPath);
		m_pILog->PrintLog(LOGLEVEL_ERR, strMsg.GetBuffer());
		goto __end;
	}
	pRootParser = cJSON_Parse(strjson);
	if (pRootParser == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "%s文件不符合Json数据格式\r\n ", strjsonPath);
		goto __end;
	}

	Size = cJSON_GetArraySize(pRootParser);
	for (INT i = 0; i < Size; ++i) {
		pArrayItem = cJSON_GetArrayItem(pRootParser, i);
		pItemObj = cJSON_GetObjectItem(pArrayItem, "ModuleName");
		if (pItemObj == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析返回的ModuleName字段错误，请确认文件中维护的字段信息\r\n ");
			goto __end;
		}
		if (m_Setting.strModuleName.CompareNoCase(pItemObj->valuestring) == 0){
		
			cJSON_ReplaceItemInObject(pArrayItem, "ComMap", cJSON_CreateString(strComMap));
			break;
		}
	}

	strBuildJson = cJSON_Print(pRootParser);
	jsonFile.Write(strBuildJson, strBuildJson.GetLength());

	Ret = 1;
__end:
	if (JsonData != NULL) {
		cJSON_Delete(JsonData);
		JsonData = NULL;
	}
	if (pRootParser != NULL) {
		cJSON_Delete(pRootParser);
		pRootParser = NULL;
	}
	if (pData) {
		delete[] pData;
	}
	if (jsonFile.m_hFile != CFile::hFileNull) {
		jsonFile.Close();
	}
	return Ret;
}

INT CMesDlg::SetComMap()
{
	CString strComMap = _T("");
	for (int i = 0; i < SITE_COUNT; i++) { //轮询ready 状态

		for (int j = 0; j < EACH_SITE_CHIP_NUM; j++) {
			INT idx = i*EACH_SITE_CHIP_NUM + j;//0-16
			INT mapIdx = (i + 1) * 10 + (j + 1);//

			CString com_num{ "-1" };
			CString SiteAlias_SN;
			if (((CButton*)GetDlgItem(IDC_CHECK1 + idx))->GetCheck()) {
				GetDlgItem(IDC_EDIT1 + idx)->GetWindowText(com_num);
			}

			GetDlgItem(IDC_DEV1 + i)->GetWindowText(SiteAlias_SN);
			CString content = _T("");
			content.Format("<%s,%d,%s>", SiteAlias_SN, mapIdx, com_num);
			strComMap += content;
		}
	}
	m_pILog->PrintLog(LOGLEVEL_LOG, "Generate Com Map: %s", strComMap);
	SetComMapToJson(strComMap);
	return 0;
}


void CMesDlg::OnBnClickedBtnStartproduce()
{
	INT Ret = 0;
	BOOL RtnCall;
	m_bQuit = FALSE;
	m_bRecvAutomaticLotEnd = FALSE;
	m_pIStatus->PrintStatus(0, "");

	CUserManager &UserManagerInterface = CUserManager::getInstance();
	UINT64 Permission = UserManagerInterface.GetPermission(UserManagerInterface.GetUserRole());
	if ((Permission & PERMISSION_STARTPRODUCE) || !m_bNeedLogin) {
		if (IsMesMode()) {
			if (m_bProgRecordReady == FALSE) {
				CString strErrMsg;
				CString strText;
				GetDlgItemText(IDC_BTNGetMesRecord, strText);
				strErrMsg.Format("请先点击\"%s\"按钮进行信息获取", strText);
				m_LogRichEdit.PrintLog(LOGLEVEL_ERR, "%s", strErrMsg);
				m_pIStatus->PrintStatus(0, strErrMsg);
				MessageBox(strErrMsg, NULL, MB_OK | MB_ICONERROR);
				return;
			}
		}

		UpdateData(TRUE);

		m_MainData.streMMCMID = "";

		RtnCall = UpdateRecordForProduce();
		if (RtnCall == FALSE) {
			m_pIStatus->PrintStatus(0, "确认生产信息失败");
			return;
		}
		if (IsAutoMode()) {
			Ret = SetElectricInsertionCheck();
			if (Ret != 0) {
				return;
			}
		}

		if (m_Setting.strProgramMode.CompareNoCase("Module") == 0) {
			int Cnt = 0;
			int tmpValue = 0;

			if (pModuleProgram == NULL ) {//如果是直接烧录则先初始化
				if (!InitModule(TRUE)) {
					return;
				}
			}

			for (int i = 0; i < SITE_COUNT; i++) {
				if (((CButton*)GetDlgItem(IDC_CHECK1 + i))->GetCheck()) {
					tmpValue += (1 << i);
					Cnt++;
				}
			}

			if (Cnt == 0 || (m_ExpectICNum <= 0)) {
				CString strTips;
				strTips.Format("%s", "未使能任何站点，请至少勾选一个");
				if (m_ExpectICNum <= 0) {
					strTips.Format("%s", "设定的生产任务数量不能小于0");
				}
				AfxMessageBox(strTips);
				return;
			}

			if (!CheckSelComboCom()) {
				AfxMessageBox("请检查已使能的站点，是否已正确开启Com端口");
				return;
			}

			if (IsAutoMode()){
				if (m_strAutoTaskFilePath.IsEmpty() || (PathFileExists(m_strAutoTaskFilePath) == FALSE)) {
					AfxMessageBox("请正确选择自动化任务文件");
					return;
				}
			}
		

			if (m_strFWPath.IsEmpty() || (PathFileExists(m_strFWPath) == FALSE)) {
				AfxMessageBox("请正确选择Programmer Path文件");
				return;
			}

			INT nSktEnArray[MAX_SKTCNT] = { 0 };
			for (INT i = 0; i < MAX_SKTCNT; i++) {
				if (((CButton*)GetDlgItem(IDC_CHECK1 + i))->GetCheck()) {
					nSktEnArray[i] = 1;
				}
				else {
					nSktEnArray[i] = 0;
				}
			}
			m_ProgRecord.DestRecord.strFWPath = m_strFWPath;
			m_strCacheSavePath.Format("ProgramRet_%s_%s_%s.json", m_strWorkOrder, m_strMaterialID, ::GetCurTime('_'));

			INT Ret = pModuleProgram->StartProgram(GetSafeHwnd(),m_ProgRecord, nSktEnArray, SITE_COUNT, GetSiteSn());
			//GetDlgItem(IDC_BTN_GETCOM)->EnableWindow(Ret);
			if (Ret  ==  0) {
				m_pILog->PrintLog(LOGLEVEL_ERR, "StartProgram失败");
				return;
			}

		}
		else {
			if (pNormalProgram == NULL) {
				pNormalProgram = new NormalProgram(&m_Setting, m_pILog);
			}
			if (pNormalProgram == NULL) {
				m_pILog->PrintLog(LOGLEVEL_ERR, "NormalProgram初始化失败");
				return;
			}
			INT Ret = pNormalProgram->SetServerPath();
			if (Ret < 0) {
				m_LogRichEdit.PrintLog(LOGLEVEL_ERR, "SetServerPath失败");
				return;
			}
			m_strDevTimeRun.Empty();
			m_strDevTimeStop.Empty();
			m_strCacheSavePath.Format("ProgramRet_%s_%s_%s.json", m_strWorkOrder, m_strMaterialID, ::GetCurTime('_'));

			if (pNormalProgram->StartProgram(GetSafeHwnd(), m_ProgRecord) == 0) {
				SetCompentVisible(FALSE);
			}
			else {
				m_LogRichEdit.PrintLog(LOGLEVEL_ERR, "StartProgram失败");
				return;
			}
		}

		GetDlgItem(IDC_BTNSTARTPRODUCE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTNCANCELPRODUCE)->EnableWindow(TRUE);
	}
	else {
		AfxMessageBox("当前用户没有此权限");
	}

	return;
}

std::map<CString, tBurnStatus> CMesDlg::GetSiteSn()
{
	std::map<CString, tBurnStatus> BurnStatusMap;
	
	for (int i = 0; i < SITE_COUNT; i++) { 
		int Pos = -1;
		CString SiteAlias_SN;
		CString SiteSn;
		GetDlgItem(IDC_DEV1 + i)->GetWindowText(SiteAlias_SN);
		Pos = SiteAlias_SN.Find("-");
		if (Pos >= 0){
			SiteSn = SiteAlias_SN.Mid(Pos+1,100);
		}
		for (int j = 0; j < EACH_SITE_CHIP_NUM; j++) {
			CString ComPort;
			if (((CButton*)GetDlgItem(IDC_CHECK1 + i * SITE_COUNT + j))->GetCheck()) {
				(GetDlgItem(IDC_EDIT1 + i * SITE_COUNT + j))->GetWindowText(ComPort);
				BurnStatusMap[SiteSn].nComPort[0] = atoi(ComPort);
				BurnStatusMap[SiteSn].m_SktComMap[j] = atoi(ComPort);
			}
		}
	}
	return BurnStatusMap;
}

void CMesDlg::SendLotEnd2Auto() {
	m_pILog->PrintLog(LOGLEVEL_WARNING, "执行CMD4 SupplyStopSWReq, 停止批量...");
	char res[4096] = { 0 };
	m_bRecvAutomaticSupplyStop = FALSE;
	if (!m_StdMes.MesSendCmdToAutomatic(4, "4,{\"Method\":\"SupplyStopSWReq\"}", res, 4096)) {
		m_pILog->PrintLog(0, "执行CMD4 SupplyStopSWReq失败！");
		return;
	}
	m_pILog->PrintLog(0, "供给停止返回值 = %s ", res);

	CMsgHandler<CMesDlg, CMesDlg> StopMsgHandler = MakeMsgHandler(this, &CMesDlg::ThreadSendStopCmdHandler);
	m_StopWork.SetMsgHandle(StopMsgHandler);
	m_StopWork.CreateThread();
	m_StopWork.PostMsg(MSG_SEND_STOP_WORK, 0, 0);

	return;

	//while (!m_bRecvAutomaticSupplyStop) {
	//	Sleep(1000);
	//}

	//m_pILog->PrintLog(0, "执行CMD4 LotEndReq, 批次请求结束...");
	//memset(res, 0, 4096);
	//if (!m_StdMes.MesSendCmdToAutomatic(4, "4,{\"Method\":\"LotEndReq\"}", res, 4096)) {
	//	m_pILog->PrintLog(0, "执行CMD4 LotEndReq失败！");
	//	return;
	//}
}

void CMesDlg::OnBnClickedBtnCancelproduce()
{
	// TODO: 在此添加控件通知处理程序代码
	m_LogRichEdit.PrintLog(LOGLEVEL_WARNING, "用户准备手动取消批量");
	if (IsAutoMode()) {
		if (m_bTaskDoing) {
			if (MessageBox("烧录任务正在进行，是否“执行”结束批量 ?", NULL, MB_YESNO|MB_ICONINFORMATION) == IDYES) {
				m_LogRichEdit.PrintLog(LOGLEVEL_WARNING, "用户确定选择手动结束批量");
				SendLotEnd2Auto();
				return;
			}
			return;
		}
		
	}
}

INT CMesDlg::DoSendStopCmdTask()
{
	INT Ret = -1;
	BOOL bFuncRet = FALSE;
	char respBuffer[4096];

	while (1)
	{
		if (m_bRecvAutomaticSupplyStop) {
			m_pILog->PrintLog(0, "执行CMD4 StopSWReq,暂停设备......");
			memset(respBuffer, 0, 4096);
			bFuncRet = m_StdMes.MesSendCmdToAutomatic(4, "4,{\"Method\":\"StopSWReq\"}", respBuffer, 4096);
			m_pILog->PrintLog(0, "暂停设备返回值 = %s ", respBuffer);
			if (bFuncRet != TRUE) {
				m_pILog->PrintLog(LOGLEVEL_ERR, "执行CMD4,StopSWReq失败！");
			}

			m_pILog->PrintLog(0, "执行CMD4 LotEndReq,批次请求结束......");
			memset(respBuffer, 0, 4096);
			bFuncRet = m_StdMes.MesSendCmdToAutomatic(4, "4,{\"Method\":\"LotEndReq\"}", respBuffer, 4096);
			m_pILog->PrintLog(0, "批量 LotEnd 返回值 = %s ", respBuffer);
			if (bFuncRet != TRUE) {
				m_pILog->PrintLog(LOGLEVEL_ERR, "执行CMD4 , LotEndReq失败！");
			}
			else
			{
				m_bRecvAutomaticSupplyStop = FALSE;
				break;
			}
		}
	}

	return Ret;
}


/************************
等待命令执行完成，对GetResult的封装
返回值
-1表示获取结果错误
0表示获取结果成功，
1表示获取结果成功
************************/
INT CMesDlg::WaitJobDone(CStdMesApi* pStdMesApi)
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
			m_LogRichEdit.PrintLog(LOGLEVEL_ERR, "命令执行失败:%s", TmpBuf);
			break;
		}
	}

	return Ret;
}


void CMesDlg::OnBnClickedBtnSelproj()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_bProgRecordReady == FALSE) {
		m_pIStatus->PrintStatus(0, "请先获取生产任务信息");
		return;
	}
	//m_ProgRecord.DestRecord.bProjSelDirect = FALSE;
	//if (MessageBox("直接选择工程后，也要进行校验值的比对，EXCEL中是否已经填入对应的校验值信息?", NULL, MB_YESNO|MB_ICONINFORMATION) == IDNO) {
	//	return;
	//}

	if (IsAutoCreateProject()) {
		return;
	}

	CString strFilter;
	strFilter.Format("%s", "Acroview Programmer Project(*.apr)|*.apr||");
	GetDirFullPath(strFilter, m_strProjPath);

	/*if (IsAutoCreateProject() == false) {
		CString strProjSavePath;
		CFileDialog Dlg(TRUE, NULL, NULL, OFN_PATHMUSTEXIST, "Acroview Programmer Project(*.apr)|*.apr||", this);
		strProjSavePath = GetProjSaveAbsPath();
		Dlg.m_ofn.lpstrInitialDir = strProjSavePath.GetBuffer();
		if (Dlg.DoModal() == IDOK) {
			m_strProjPath = Dlg.GetPathName();
		}
		else {
			m_strProjPath = "";
		}
		UpdateData(FALSE);
		m_ProgRecord.DestRecord.bProjSelDirect = TRUE;
		m_pILog->PrintLog(0, "工程文件路径重新被选过,直接指定为:%s", m_strProjPath);
	}
	else {
		CString strMsg;
		strMsg.Format("%s", "指定工程模式为自动合成方式，不能选择工程!");
		m_pIStatus->PrintStatus(0, strMsg);
		MessageBox(strMsg, NULL, MB_OK | MB_ICONERROR);
	}*/

}

///根据权限切换使能
BOOL CMesDlg::ChangeCtrlsEnAccrodOpLevel(INT OpLevel)
{
	BOOL En = FALSE;
	CMenu *pMenu;
	
	if (OpLevel > 1) {
		En = TRUE;
		m_strOpMode.Format("当前使用者:管理员-%s", m_OperatorData.m_strOperator);
	}
	else {
		En = FALSE;
		m_strOpMode.Format("当前使用者:操作员-%s", m_OperatorData.m_strOperator);
		pMenu = AfxGetMainWnd()->GetMenu();
		pMenu->EnableMenuItem(ID_AUTHORITY_SETTING, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}
	//GetDlgItem(IDC_BTNSELAUTOTASKDATA)->EnableWindow(En);
	//GetDlgItem(IDC_BTNSELPROJ)->EnableWindow(En);
	UpdateData(FALSE);
	return TRUE;
}

void CMesDlg::OnAdminmode()
{
	// TODO: 在此添加命令处理程序代码
	/*if (m_OperatorData.m_Level> 1) {///
		m_pILog->PrintLog(0, "当前操作员已经是管理员，无需切换");
		return;
	}*/
	m_Setting.Load();
	CDlgAdminMode DlgAdmin;
	DlgAdmin.m_strWebService = m_Setting.strWebServiceInterface;
	DlgAdmin.AttachILog(&m_LogRichEdit);
	DlgAdmin.AttachData(&m_OperatorData);
	DlgAdmin.DoModal();
	ChangeCtrlsEnAccrodOpLevel(m_OperatorData.m_Level);

	m_Setting.strOperator = m_OperatorData.m_strOperator;
	m_Setting.Save();

	OnUpdataSettingConfig(0, 0);
	SetAccountUI();
}


void CMesDlg::OnMesaccesstest()
{
	// TODO: 在此添加命令处理程序代码
	CDlgMESTest DlgMesTest;
	DlgMesTest.AttachILog(m_pILog);
	DlgMesTest.DoModal();
}


HBRUSH CMesDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性

	// TODO:  如果默认的不是所需画笔，则返回另一个画
	if (pWnd->GetDlgCtrlID() == IDC_SHOWSTATUS) {
		pDC->SetTextColor(m_StatusColor);
	}
	return hbr;
}


void CMesDlg::OnOpendownloaderdlg()
{
	if (this->m_bStartDown)
	{
		AfxMessageBox(_T("批量生产中正在下载，无法打开！"));
		return;
	}
	// TODO: 在此添加命令处理程序代码
	//CDlgDownLoader DlgDownLoader;
	//DlgDownLoader.DoModal();
}

void CMesDlg::OnConfigdown()
{
	// TODO: 在此添加命令处理程序代码
	//CDlgConfigDownLoad dlg;
	//dlg.AttachData((&m_Setting));
	//dlg.DoModal();
}

void CMesDlg::OnAutopossetting()
{
	// TODO: 在此添加命令处理程序代码
	CUserManager &UserManagerInterface = CUserManager::getInstance();
	UINT64 Permission = UserManagerInterface.GetPermission(UserManagerInterface.GetUserRole());

	if ((Permission & PERMISSION_AUTOPOSSETTING) || !m_bNeedLogin) {
		CDlgAutoPosSetting DlgAutoPosSetting;
		DlgAutoPosSetting.AttachPosSetting(&m_AutoPosSetting);
		DlgAutoPosSetting.DoModal();

		if (m_Setting.strProgramMode.CompareNoCase("Module") == 0) {
			if (pModuleProgram != NULL) {
				pModuleProgram->UpdatePosSetting(m_AutoPosSetting);
			}
		}
		else {
			if (pNormalProgram != NULL) {
				pNormalProgram->UpdatePosSetting(m_AutoPosSetting);
			}
		}
	}
	else {
		AfxMessageBox("当前用户没有此权限");
	}

	
	return;
}

void CMesDlg::OnUserSetting()
{
	// TODO: 在此添加命令处理程序代码
	CDlgUserManager DlgAutoPosSetting;
	DlgAutoPosSetting.DoModal();
	return;
}

void CMesDlg::OnAbout()
{
	// TODO: 在此添加命令处理程序代码
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}

void CMesDlg::PrintZPL(CString strZPL) {
	BOOL Ret = FALSE;
	CString strIP;
	CString strSendData;
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		return;
	}

	//打印ZPL内容
	strSendData = strZPL;

	strIP.Format("%s", m_Setting.strPrintIP);
	UINT nPort = m_Setting.nPrintPort;
	int TryCnt = 3;
	struct sockaddr_in serv_addr;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(strIP);
	serv_addr.sin_port = htons(nPort);

	//InetPton(AF_INET, CString(strIP.GetBuffer())), &serv_addr.sin_addr);

	unsigned long ul = 1;
	while (TryCnt > 0) {
		if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != -1) {
			Ret = TRUE;
			break;
		}

		Sleep(200);
		TryCnt--;
	}
	m_pILog->PrintLog(LOGLEVEL_LOG, "发送条码内容给打印服务器");

	if (Ret == TRUE) {
		if (send(sock,/*(LPCTSTR)*/ (LPSTR)(LPCSTR)strSendData, strSendData.GetLength(), 0) == -1) {
			
			m_pILog->PrintLog(LOGLEVEL_ERR, "发送给打印服务器失败，请检查下网络环境！");
			Ret = FALSE;
		}
	}
	else {
		Ret = FALSE;
		m_pILog->PrintLog(LOGLEVEL_ERR, "连接打印服务器失败，请检查网络环境！");

	}

	Sleep(200); //用网路助手作为server不用休眠能秒接，但是打印服务器必须要休眠下才能收到，否则不能正常接受
	m_pILog->PrintLog(LOGLEVEL_LOG, "打印完成");

	closesocket(sock);
}


void CMesDlg::OnAuthoritySetting()
{
	// TODO: 在此添加命令处理程序代码
	CDlgAuthority DlgAuthority;
	DlgAuthority.DoModal();
}


//==>>修改语言
void CMesDlg::OnEnglishLang()
{
	int iRet = MessageBox(_T("确定切换到英文版本? 请重新启动软件！"), _T("提示"), MB_YESNO| MB_ICONQUESTION);
	if (iRet == 6) {
		BOOL RetWrite;
		CString ExePath = CComFunc::GetCurrentPath();
		CString strIniFile = ExePath + "/Language.ini";
		RetWrite = WritePrivateProfileString("Language", "CurLang", "English", strIniFile);
		if (RetWrite == FALSE) {
			return;
		}

		m_bRestartSoft = TRUE;
		PostMessage(WM_CLOSE);
	}
	else {
		return;
	}
}


void CMesDlg::OnChineseOne()
{
	int iRet = MessageBox(_T("Are you sure to switch to Chinese? Please restart the software！"), _T("prompt"), MB_YESNO | MB_ICONQUESTION);
	if (iRet == 6) {
		BOOL RetWrite;
		CString ExePath = CComFunc::GetCurrentPath();
		CString strIniFile = ExePath + "/Language.ini";
		RetWrite = WritePrivateProfileString("Language", "CurLang", "ChineseA", strIniFile);
		if (RetWrite == FALSE) {
			return;
		}

		m_bRestartSoft = TRUE;
		PostMessage(WM_CLOSE);
	}
	else {
		return;
	}
}


void CMesDlg::OnChineseLangTwo()
{

	int iRet = MessageBox(_T("Are you sure to switch to Chinese? Please restart the software！"), _T("prompt"), MB_YESNO);
	if (iRet == 6) {
		BOOL RetWrite;
		CString ExePath = CComFunc::GetCurrentPath();
		CString strIniFile = ExePath + "/Language.ini";
		RetWrite = WritePrivateProfileString("Language", "CurLang", "ChineseB", strIniFile);
		if (RetWrite == FALSE) {
			return;
		}

		m_bRestartSoft = TRUE;
		PostMessage(WM_CLOSE);
	}
	else {
		return;
	}
}

INT CMesDlg::InitMesSyslog()
{
	INT Ret = -1;

	Ret = m_MesSyslog.Init(&m_LogRichEdit, m_Setting.strPrintIP, m_Setting.nPrintPort);
	if (Ret != 0) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "MesSyslog 初始化失败");
	}
	return Ret;
}


INT CMesDlg::UnInitMesSyslog()
{
	INT Ret = -1;
	Ret = m_MesSyslog.Uninit();

	return Ret;
}

void CMesDlg::Onspecification()
{
	// TODO: 在此添加命令处理程序代码
	CString FilePath;
	CFileFind finder;
	BOOL IsFind = finder.FindFile(GetCurrentPath() + "\\Docs\\*.pdf");
	if (!IsFind) {
		m_pILog->PrintLog(LOGLEVEL_WARNING, "当前目录下不包含pdf文件 %s\\Docs", GetCurrentPath());
		return;
	}
	while (IsFind) {
		IsFind = finder.FindNextFile();
		FilePath.Format("%s\\%s", GetCurrentPath(), finder.GetFileName());
	}

	ShellExecute(NULL, "open", "iexplore.exe", FilePath, NULL, SW_MAXIMIZE);
}


INT CMesDlg::SelftestFunc()
{
	INT Ret = -1;

	//== 1.syslog

	//== 先判断什么设备机型
	//== 选择对应路径的syslog,如果不是深圳出去的机型，而是台湾出去的设备，那么路径肯定不是自带的，而且system.ini名字也不对
	//== (在不增加配置文件的情况下，怎么实现，)
	//== 进去判断字段对应值

	//== 

	return Ret;

}

void CMesDlg::OnBnClickedBtnselFwProj()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	CString strFileFilter;
	strFileFilter.Format("");

	GetDirFullPath(strFileFilter, m_strFWPath);

	UpdateData(FALSE);
}


BOOL CMesDlg::IsProccessRunning(CString strProccess)
{
	BOOL BRunning = FALSE;
	PROCESSENTRY32 processEntry32;
	HANDLE toolHelp32Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (((int)toolHelp32Snapshot) != -1)
	{
		processEntry32.dwSize = sizeof(processEntry32);
		if (Process32First(toolHelp32Snapshot, &processEntry32))
		{
			do {
				CString str;
				str.Format("%s", processEntry32.szExeFile);
				if (str.CompareNoCase(strProccess) == 0) {
					BRunning = TRUE;
					//AfxMessageBox(L"程序正在运行");
					break;
				}
			} while (Process32Next(toolHelp32Snapshot, &processEntry32));
		}
		CloseHandle(toolHelp32Snapshot);
	}
	return BRunning;
}

BOOL CMesDlg::CallAutoApp()
{
	BOOL Rtn = FALSE;
	CString strMsg;
	CString strUfsWriterExe;
	if (m_Setting.strProgramMode.CompareNoCase("UfsWriter") == 0) {
		if (PathFileExists(m_Setting.strUfsWriterFolder) == FALSE) {
			strMsg.Format("The path of AutoApp is not available,please check, path: %s", m_Setting.strUfsWriterFolder);
			if (MessageBoxEx(GetSafeHwnd(), TEXT(strMsg), TEXT("Warning"), MB_ICONWARNING | MB_OK,
				MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)) == IDOK) {
				//return;
			}
			goto __end;
		}

		if (IsProccessRunning("UFSWriter.exe") == TRUE) {
			Rtn = TRUE;
			goto __end;
		}

		strUfsWriterExe.Format("%s\\%s", m_Setting.strUfsWriterFolder, "UFSWriter.exe");
		WinExec(strUfsWriterExe, SW_SHOW);
	}

__end:
	return Rtn;
}


INT CMesDlg::GetUfxFileChecksum(CString strUfxFile)
{
	INT Ret = -1;
	UINT64 Checksum = 0;
	UINT64 Checksumdesired = 0;
	CFile File;
	INT FileLen;
	CString strExpectChecksum;
	CString strUfxChecksum;
	INT Line = 0;

	strExpectChecksum = m_strProgFileChecksum;
	strExpectChecksum.Replace("0x", "");
	sscanf(strExpectChecksum.GetBuffer(), "%I64X", &Checksumdesired);

	if (File.Open(strUfxFile, CFile::modeRead | CFile::shareDenyNone, NULL) == FALSE) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "Open File Failed: %s", strUfxFile);
		Ret = -1; goto __end;
	}
	else {

		ULONGLONG fileSize = File.GetLength();
		const DWORD bufferSize = static_cast<DWORD>(min(fileSize, 1024 * 1024));
		BYTE* buffer = new BYTE[bufferSize];

		DWORD bytesRead = 0;
		File.Seek(12, SEEK_SET);
		if (File.Read(buffer, bufferSize) > 0) {

			const BYTE* newlinePos = static_cast<const BYTE*>(memchr(buffer, '\n', bytesRead));
			if (!newlinePos) {
				newlinePos = static_cast<const BYTE*>(memchr(buffer, '\r', bytesRead));
			}

			size_t firstLineLength = newlinePos ? (newlinePos - buffer) : bytesRead;
			CString firstLine(reinterpret_cast<const char*>(buffer), static_cast<int>(firstLineLength));

			firstLine.Format("%s", buffer);
			FileLen = firstLine.Find("CRC=");
			if (FileLen > 0) {
				strUfxChecksum = firstLine.Mid((FileLen + 5), (firstLine.Find(">") - (FileLen + 6)));

				//strUfxChecksum = "7988";
				sscanf(strUfxChecksum.GetBuffer(), "%I64X", &Checksum);

			}
		}

		delete[] buffer;
	}

	m_strRealChecksum.Format("0x%08X", Checksum);
	PostMessage(MSG_UPDATECHECKSUM, 0, 0);

	if (Checksum != Checksumdesired) {
		m_LogRichEdit.PrintLog(LOGLEVEL_ERR, "校验值比对错误: 服务器返回的校验值为0x%I64X, 从UFX文件中获取到的的校验值为0x%I64X", Checksumdesired, Checksum);
		Ret = -1; goto __end;
	}
	else {
		m_LogRichEdit.PrintLog(LOGLEVEL_LOG, "校验值比对成功");
		Ret = 0;
	}

__end:
	File.Close();
	return Ret;
}


//== 发送UFS工程至UFS软件
INT  CMesDlg::LoadUfxProject()
{
	INT Ret = -1, nRetCode = 0;
	CString strProjFile;
	CString strjson_rpc;

	strProjFile = m_strProjPath;

	cJSON* pLoadTaskRoot = NULL;
	cJSON* pProFile = NULL;

	pLoadTaskRoot = cJSON_CreateObject();
	pProFile = cJSON_CreateObject();

	cJSON_AddStringToObject(pLoadTaskRoot, "jsonrpc", "2.0");
	cJSON_AddStringToObject(pLoadTaskRoot, "method", "LoadTask");
	cJSON_AddNumberToObject(pLoadTaskRoot, "id", 6);

	cJSON_AddStringToObject(pProFile, "File", strProjFile);
	cJSON_AddItemToObject(pLoadTaskRoot, "params", pProFile);

	strjson_rpc = cJSON_Print(pLoadTaskRoot);
	//== 组合json格式
	//示例: {"jsonrpc":"2.0","method":"LoadTask","params":{"File":"C:\\Data\\UFS Test\\Task\\TOSHIBA THGAF8G8T23BAILB.UFX"},"id":6}
	m_CliSendWorker.PostMsg(MSG_CREATE_SEND_TCP_CLI, (LPARAM)(LPCTSTR)strjson_rpc, 0);

	Sleep(200);

	while (1)
	{
		if (m_UfsLoadTaskBn) {
			Ret = 0;
			break;
		}
		else {
			Sleep(500);
			++nRetCode;
		}

		if (nRetCode > 5) { break; }
	}

	if (Ret != 0) {
		m_LogRichEdit.PrintLog(LOGLEVEL_ERR, "发送UFS设置LoadTask失败...");
	}

	cJSON_Delete(pLoadTaskRoot);
	pLoadTaskRoot = NULL;


	return Ret;
}


//==  发送执行烧录命令至UFS
INT CMesDlg::DoSelectProgramFunc()
{
	INT Ret = -1, nRetCode = 0;
	CString strProjFile;
	CString strjson_rpc;

	strProjFile = m_strProjPath;

	cJSON* pLoadTaskRoot = NULL;
	cJSON* pProFile = NULL;

	pLoadTaskRoot = cJSON_CreateObject();
	pProFile = cJSON_CreateObject();

	cJSON_AddStringToObject(pLoadTaskRoot, "jsonrpc", "2.0");
	cJSON_AddStringToObject(pLoadTaskRoot, "method", "SelectFunction");
	cJSON_AddNumberToObject(pLoadTaskRoot, "id", 7);

	cJSON_AddStringToObject(pProFile, "FunctionName", "Write and Verify");
	cJSON_AddItemToObject(pLoadTaskRoot, "params", pProFile);

	strjson_rpc = cJSON_Print(pLoadTaskRoot);
	//== 组合json格式
	//示例: {"jsonrpc":"2.0","method":"LoadTask","params":{"File":"C:\\Data\\UFS Test\\Task\\TOSHIBA THGAF8G8T23BAILB.UFX"},"id":6}
	m_CliSendWorker.PostMsg(MSG_CREATE_SEND_TCP_CLI, (LPARAM)(LPCTSTR)strjson_rpc, 0);

	Sleep(200);

	while (1)
	{
		if (m_UfsSelFunctionBn) {
			Ret = 0;
			break;
		}
		else {
			Sleep(500);
			++nRetCode;
		}

		if (nRetCode > 5) { break; }
	}

	if (Ret != 0) {
		m_LogRichEdit.PrintLog(LOGLEVEL_ERR, "发送UFS执行SelectFunction失败...");
	}

	cJSON_Delete(pLoadTaskRoot);
	pLoadTaskRoot = NULL;

	return Ret;
}


//==  发送读取显示次数信息
INT CMesDlg::DoGetCounterFunc()
{
	INT Ret = 0, nRetCode = 0;
	CString strjson_rpc;

	cJSON* pLoadTaskRoot = NULL;
	cJSON* pProFile = NULL;

	pLoadTaskRoot = cJSON_CreateObject();

	cJSON_AddStringToObject(pLoadTaskRoot, "jsonrpc", "2.0");
	cJSON_AddStringToObject(pLoadTaskRoot, "method", "GetCounter");
	cJSON_AddNumberToObject(pLoadTaskRoot, "id", 5);
	cJSON_AddStringToObject(pLoadTaskRoot, "params", nullptr);

	strjson_rpc = cJSON_Print(pLoadTaskRoot);
	m_CliSendWorker.PostMsg(MSG_CREATE_SEND_TCP_CLI, (LPARAM)(LPCTSTR)strjson_rpc, 0);

	Sleep(300);

	if (Ret != 0) {
		m_LogRichEdit.PrintLog(LOGLEVEL_ERR, "发送UFS执行DoGetCounterFunc失败......");
	}

	cJSON_Delete(pLoadTaskRoot);
	pLoadTaskRoot = NULL;

	return Ret;
}



//== 获取站点座子映射表
INT CMesDlg::DoGetSiteSocketMapFunc()
{
	INT Ret = 0, nRetCode = 0;
	CString strjson_rpc;

	cJSON* pLoadTaskRoot = NULL;
	cJSON* pProFile = NULL;

	pLoadTaskRoot = cJSON_CreateObject();

	cJSON_AddStringToObject(pLoadTaskRoot, "jsonrpc", "2.0");
	cJSON_AddStringToObject(pLoadTaskRoot, "method", "GetSiteSocketMap");
	cJSON_AddNumberToObject(pLoadTaskRoot, "id", 2);
	cJSON_AddStringToObject(pLoadTaskRoot, "params", nullptr);

	strjson_rpc = cJSON_Print(pLoadTaskRoot);
	m_CliSendWorker.PostMsg(MSG_CREATE_SEND_TCP_CLI, (LPARAM)(LPCTSTR)strjson_rpc, 0);

	Sleep(300);

	if (Ret != 0) {
		m_LogRichEdit.PrintLog(LOGLEVEL_ERR, "发送UFS执行DoGetSiteSocketMapFunc失败......");
	}
	else {
		m_LogRichEdit.PrintLog(LOGLEVEL_LOG, "正在获取UFS可使用站点Map......");
	}

	cJSON_Delete(pLoadTaskRoot);
	pLoadTaskRoot = NULL;

	return Ret;
}


//== 指定对应站点 开始工作
INT CMesDlg::DoStartSiteFunc()
{
	INT Ret = 0, nRetCode = 0;
	CString strjson_rpc;

	cJSON* pLoadTaskRoot = NULL;
	cJSON* pRootParser = NULL;

	if (VecGetSktMap.empty()) {
		m_LogRichEdit.PrintLog(LOGLEVEL_ERR, "获取站点座子映射表失败，请重新获取.....");
		Ret = -1; goto __end;
	}

	for (auto aloop = 0; aloop < VecGetSktMap.size(); aloop++)
	{
		pLoadTaskRoot = cJSON_CreateObject();
		cJSON_AddStringToObject(pLoadTaskRoot, "jsonrpc", "2.0");
		cJSON_AddStringToObject(pLoadTaskRoot, "method", "StartSite");
		cJSON_AddNumberToObject(pLoadTaskRoot, "id", 8);

		pRootParser = cJSON_CreateObject();
		cJSON_AddNumberToObject(pRootParser, "Site", VecGetSktMap[aloop].SiteGroup);
		cJSON_AddNumberToObject(pRootParser, "SocketMap", VecGetSktMap[aloop].SocketMap);
		cJSON_AddItemToObject(pLoadTaskRoot, "params", pRootParser);

		strjson_rpc = cJSON_Print(pLoadTaskRoot);
		m_CliSendWorker.PostMsg(MSG_CREATE_SEND_TCP_CLI, (LPARAM)(LPCTSTR)strjson_rpc, 0);

		Sleep(300);
	}

	m_LogRichEdit.PrintLog(LOGLEVEL_LOG, "指定站点烧录......");

__end:
	cJSON_Delete(pLoadTaskRoot);
	pLoadTaskRoot = NULL;
	return Ret;
}


//== 创建TCP 客户端 实现长连接
INT CMesDlg::CreateTcpClient()
{
	INT Rtn = -1;
	BOOL Ret = FALSE;
	CString strSendData;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		m_LogRichEdit.PrintLog(LOGLEVEL_LOG, "创建Socket套接字失败");
		return -1;
	}

	CString strIP = "127.0.0.1";
	UINT nPort = 5050;
	int TryCnt = 3;
	struct sockaddr_in serv_addr;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(strIP);
	serv_addr.sin_port = htons(nPort);

	while (TryCnt > 0) {
		if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != -1) {
			Ret = TRUE;
			break;
		}

		Sleep(200);
		TryCnt--;
	}

	if (Ret == TRUE) {

		CMsgHandler<CMesDlg, CMesDlg> TcpCliSendHandler = MakeMsgHandler(this, &CMesDlg::ThdTcpCliSendHandler);
		m_CliSendWorker.SetMsgHandle(TcpCliSendHandler);
		m_CliSendWorker.CreateThread();

		CMsgHandler<CMesDlg, CMesDlg> TcpCliRecvHandler = MakeMsgHandler(this, &CMesDlg::ThdTcpCliRecvHandler);
		m_CliRecvWorker.SetMsgHandle(TcpCliRecvHandler);
		m_CliRecvWorker.CreateThread();
		m_CliRecvWorker.PostMsg(MSG_CREATE_RECV_TCP_CLI, 0, 0);
		Rtn = 0;
		m_pILog->PrintLog(LOGLEVEL_LOG, "连接UFS服务器成功，请继续！.....");
	}
	else {
		Ret = FALSE;
		closesocket(sock);
		m_pILog->PrintLog(LOGLEVEL_ERR, "连接UFS服务器失败，请检查网络环境！");
	}

	return Rtn;
}



INT CMesDlg::ThdTcpCliSendHandler(MSG msg, void *Para)
{
	INT Ret = 0;
	if (msg.message == MSG_CREATE_SEND_TCP_CLI) {

		char * SendData = (char*)msg.wParam;

		CString strSendData;
		strSendData.Format("%s", SendData);
		if (send(sock, (LPSTR)(LPCSTR)strSendData, strSendData.GetLength(), 0) == -1) {
			m_pILog->PrintLog(LOGLEVEL_LOG, "发送给 UFS 服务器失败，请检查下网络环境！");
			Ret = -1;
		}
	}
	return Ret;
}

INT CMesDlg::ThdTcpCliRecvHandler(MSG msg, void *Para)
{
	INT Ret = 0;
	if (msg.message == MSG_CREATE_RECV_TCP_CLI) {

		char buffer[2048];
		while (!mParseWorkerExit) {
			memset(buffer, 0, 2048);
			if (recv(sock, buffer, 2048, 0) <= 0) {
				if (sock != INVALID_SOCKET)
				{
					closesocket(sock);
					sock = INVALID_SOCKET;
				}
				m_UfsProgramBn = TRUE;
				break;
			}
			else {

				cJSON* pRootParser;
				cJSON* pEName;
				INT JsonId = -1;
				pRootParser = cJSON_Parse((const char *)buffer);
				if (pRootParser == NULL) {
					m_pILog->PrintLog(LOGLEVEL_ERR, "UFS服务器返回的不符合Json数据格式 ");
					continue;
				}
				pEName = cJSON_GetObjectItem(pRootParser, "id");
				if (pEName == NULL) {
					m_pILog->PrintLog(LOGLEVEL_ERR, "自动化返回的id字段错误，请确认自动化维护的字段信息, ");
					continue;
				}
				JsonId = pEName->valueint;
				if (JsonId == UFS_JSON_RPC_LOADTASK) {
					ParseRecvLoadTask(pRootParser);//== loadTask
				}
				else if (JsonId == UFS_JSON_RPC_GETCOUNT) {
					ParseRecvGetCounter(pRootParser);//== Getcount
				}
				else if (JsonId == UFS_JSON_RPC_SELECTFUNCTION) {
					ParseRecvSelectFunction(pRootParser);//== SelectFunction
				}
				else if (JsonId == UFS_JSON_RPC_GETSITESKTMAP) {
					ParseRecvGetSiteSktMapFunction(pRootParser);//== GetSiteSocketMap
				}
				else if (JsonId == UFS_JSON_RPC_STARTSITE) {
					ParseRecvStartSiteFunction(pRootParser);//== StartSite
				}
			}
		}

	}
	return -1;///为了让线程处理完消息之后自动退出，返回-1不在进行消息的获取
}



INT CMesDlg::ParseRecvGetSiteSktMapFunction(void * RecvBuffer)
{

	cJSON* pJsonRpc = NULL;
	cJSON* pResult = NULL;
	cJSON* pId = NULL;

	cJSON* pItemResult = NULL;
	cJSON* pItemArray = NULL;
	cJSON* pSite = NULL;
	cJSON* pSocketMap = NULL;

	CString strJsonRpc;
	INT Id_count = -1;
	INT ArraySize = 0;
	tGetSocketMap MapInfo;


	cJSON* pRootParser = (cJSON*)RecvBuffer;
	pJsonRpc = cJSON_GetObjectItem(pRootParser, "jsonrpc");
	if (pJsonRpc == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "UFS服务器返回的jsonrpc字段错误，请确认UFS维护的字段信息, ");
		return -1;
	}
	strJsonRpc.Format("%s", pJsonRpc->valuestring);

	pResult = cJSON_GetObjectItem(pRootParser, "result");
	if (pResult == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "UFS服务器返回的result字段错误，请确认UFS服务器的字段信息, ");
		return -1;
	}
	else {

		ArraySize = cJSON_GetArraySize(pResult);
		for (INT i = 0; i < ArraySize; i++)
		{
			pItemArray = cJSON_GetArrayItem(pResult, i);

			pSite = cJSON_GetObjectItem(pItemArray, "Site");
			if (pSite == NULL) {
				m_pILog->PrintLog(LOGLEVEL_ERR, "UFS服务器返回的pSite字段错误，请确认UFS服务器的字段信息, ");
				return -1;
			}
			MapInfo.SiteGroup = pSite->valueint;

			pSocketMap = cJSON_GetObjectItem(pItemArray, "SocketMap");
			if (pSocketMap == NULL) {
				m_pILog->PrintLog(LOGLEVEL_ERR, "UFS服务器返回的pSocketMap字段错误，请确认UFS服务器的字段信息, ");
				return -1;
			}
			MapInfo.SocketMap = pSocketMap->valueint;
			VecGetSktMap.push_back(MapInfo);
		}
	}

	return 0;

}


INT CMesDlg::ParseRecvStartSiteFunction(void * RecvBuffer)
{
	cJSON* pJsonRpc = NULL;
	cJSON* pResult = NULL;
	cJSON* pId = NULL;

	cJSON* pItemSite = NULL;
	cJSON* pItemMap = NULL;
	cJSON* passMap = NULL;

	CString strJsonRpc;
	INT Id_count = -1;
	INT ArraySize = 0;

	cJSON* pRootParser = (cJSON*)RecvBuffer;
	pJsonRpc = cJSON_GetObjectItem(pRootParser, "jsonrpc");
	if (pJsonRpc == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "UFS服务器返回的jsonrpc字段错误，请确认UFS维护的字段信息, ");
		return -1;
	}
	strJsonRpc.Format("%s", pJsonRpc->valuestring);

	pResult = cJSON_GetObjectItem(pRootParser, "result");
	if (pResult == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "UFS服务器返回的result字段错误，请确认UFS服务器的字段信息, ");
		return -1;
	}
	else {
		pItemSite = cJSON_GetObjectItem(pResult, "Site");
		if (pItemSite == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "UFS服务器返回的Site字段错误，请确认UFS服务器的字段信息, ");
			return -1;
		}

		pItemMap = cJSON_GetObjectItem(pResult, "SktMap");
		if (pItemMap == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "UFS服务器返回的SktMap字段错误，请确认UFS服务器的字段信息, ");
			return -1;
		}

		passMap = cJSON_GetObjectItem(pResult, "PassMap");
		if (passMap == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "UFS服务器返回的PassMap字段错误，请确认UFS服务器的字段信息, ");
			return -1;
		}
	}

	return 0;
}


INT CMesDlg::ParseRecvLoadTask(void * RecvBuffer)
{
	cJSON* pJsonRpc = NULL;
	cJSON* pResult = NULL;
	cJSON* pId = NULL;
	cJSON* pErrorde = NULL;

	cJSON* pMessage = NULL;
	cJSON* pCode = NULL;

	CString strJsonRpc;
	INT nResult = -1;
	INT Id_count = -1;
	INT nCode = -1;

	cJSON* pRootParser = (cJSON*)RecvBuffer;
	pJsonRpc = cJSON_GetObjectItem(pRootParser, "jsonrpc");
	if (pJsonRpc == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "UFS服务器返回的jsonrpc字段错误，请确认UFS维护的字段信息, ");
		return -1;
	}
	strJsonRpc.Format("%s", pJsonRpc->valuestring);

	pResult = cJSON_GetObjectItem(pRootParser, "result");
	if (pResult == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "UFS服务器返回的result字段错误，请确认UFS服务器的字段信息, ");

		pErrorde = cJSON_GetObjectItem(pRootParser, "error");
		if (pErrorde == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "UFS服务器返回的error字段错误，请确认UFS服务器的字段信息, ");
			return -1;
		}
		else {
			pCode = cJSON_GetObjectItem(pErrorde, "code");
			if (pCode == NULL) {
				m_pILog->PrintLog(LOGLEVEL_ERR, "UFS服务器返回的code字段错误，请确认UFS服务器的字段信息, ");
			}

			pMessage = cJSON_GetObjectItem(pErrorde, "message");
			if (pMessage == NULL) {
				m_pILog->PrintLog(LOGLEVEL_ERR, "UFS服务器返回的message字段错误，请确认UFS服务器的字段信息, ");
			}
			else {
				m_pILog->PrintLog(LOGLEVEL_LOG, "MES 执行UFS发送 LoadTask 失败，code = %d, Message=%s", pCode->valueint, pMessage->valuestring);
			}
		}

		return -1;
	}
	else {
		nResult = pResult->valueint;
		m_pILog->PrintLog(LOGLEVEL_LOG, "MES发送LoadTask， UFS执行成功");
		m_UfsLoadTaskBn = TRUE;
	}

	return 0;
}

INT CMesDlg::ParseRecvGetCounter(void * RecvBuffer)
{
	cJSON* pJsonRpc = NULL;
	cJSON* pResult = NULL;
	cJSON* pId = NULL;
	cJSON* pErrorde = NULL;

	cJSON* pCodeFail = NULL;
	cJSON* pCodePass = NULL;

	CString strJsonRpc;
	INT nResult = -1;
	INT Id_count = -1;
	INT nCode = -1;

	INT nPass = 0;
	INT nFail = 0;
	INT nTotal = 0;

	cJSON* pRootParser = (cJSON*)RecvBuffer;
	pJsonRpc = cJSON_GetObjectItem(pRootParser, "jsonrpc");
	if (pJsonRpc == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "UFS服务器返回的jsonrpc字段错误，请确认UFS维护的字段信息, ");
		return -1;

	}
	strJsonRpc.Format("%s", pJsonRpc->valuestring);

	pResult = cJSON_GetObjectItem(pRootParser, "result");
	if (pResult == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "UFS服务器返回的result字段错误，请确认UFS维护的字段信息, ");
		return -1;

	}
	else {
		pCodePass = cJSON_GetObjectItem(pResult, "Pass");
		if (pCodePass == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "UFS服务器返回的Pass字段错误，请确认UFS维护的字段信息, ");
			return -1;
		}
		nPass = pCodePass->valueint;

		pCodeFail = cJSON_GetObjectItem(pResult, "Fail");
		if (pCodeFail == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "UFS服务器返回的Fail字段错误，请确认UFS维护的字段信息, ");
			return -1;
		}
		nFail = pCodeFail->valueint;
		nTotal = nPass + nFail;

		m_YieldSitesMutex.Lock();
		m_YieldSites.TotalCnt = nTotal;
		m_YieldSites.FailCnt = nFail;
		m_YieldSites.PassCnt = nPass;
		m_YieldSitesMutex.Unlock();

	}

	return 0;
}



INT CMesDlg::ParseRecvSelectFunction(void * RecvBuffer)
{

	cJSON* pJsonRpc = NULL;
	cJSON* pResult = NULL;
	cJSON* pId = NULL;
	CString strJsonRpc;
	INT nResult = -1;
	INT Id_count = -1;

	cJSON* pRootParser = (cJSON*)RecvBuffer;
	pJsonRpc = cJSON_GetObjectItem(pRootParser, "jsonrpc");
	if (pJsonRpc == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "UFS服务器返回的jsonrpc字段错误，请确认UFS维护的字段信息, ");
		return -1;
	}
	strJsonRpc.Format("%s", pJsonRpc->valuestring);

	pResult = cJSON_GetObjectItem(pRootParser, "result");
	if (pResult == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "UFS服务器返回的result字段错误，请确认UFS服务器的字段信息, ");
		return -1;
	}
	else {
		nResult = pResult->valueint;
		if (nResult == 1) {
			m_pILog->PrintLog(LOGLEVEL_LOG, "MES发送SelectFunction， UFS执行成功");
			m_UfsSelFunctionBn = TRUE;
		}
	}

	return 0;

}




BOOL CMesDlg::SendCmd4ForAuto(CString strAutoTaskInfo)
{
	BOOL Ret = FALSE;
	BOOL Rtn = FALSE;
	BYTE RetFromAuto = 0;
	INT TryCnt = 4;
	CString strTemp;

	m_pILog->PrintLog(LOGLEVEL_LOG, "发送自动化任务数据至自动化..., %s", strAutoTaskInfo);

	SOCKET AutoSock = socket(AF_INET, SOCK_STREAM, 0);
	if (AutoSock == INVALID_SOCKET) {
		m_LogRichEdit.PrintLog(LOGLEVEL_LOG, "创建Socket套接字失败");
		return -1;
	}

	CString AutoIPAddr = "127.0.0.1";
	INT     AutoPort = 1000;
	struct sockaddr_in serv_addr;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(AutoIPAddr);
	serv_addr.sin_port = htons(AutoPort);

	while (TryCnt > 0) {
		if (connect(AutoSock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != -1) {
			Ret = TRUE;
			break;
		}

		Sleep(200);
		TryCnt--;
	}

	if (Ret == TRUE)
	{
		if (send(AutoSock, (LPSTR)(LPCSTR)strAutoTaskInfo, strAutoTaskInfo.GetLength(), 0) == -1) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "发送自动化任务数据至自动化失败...");
		}

		RawReceive(&RetFromAuto, 1, AutoSock);
		closesocket(AutoSock);

		Sleep(100);
		if (RetFromAuto != '0') {
			m_pILog->PrintLog(LOGLEVEL_ERR, "Set Task Data Failed, ErrCode From AutoApp=%c", RetFromAuto);
		}
		else {
			m_pILog->PrintLog(LOGLEVEL_LOG, "设置自动化任务数据成功");
			Rtn = TRUE;
		}
	}

	return Rtn;
}

INT CMesDlg::RawReceive(BYTE*pData, INT Size, SOCKET sock)
{
	INT Rtn = 0;
	INT  iRecvLen;
	INT uiCompleted = 0;
	INT uiPkgSize = Size;
	m_SktMutex.Lock();

	while (uiCompleted < uiPkgSize) {
		iRecvLen = recv(sock, (char*)pData + uiCompleted, uiPkgSize - uiCompleted, 0);
		if (iRecvLen == 0) {//==Socket被关闭
			m_pILog->PrintLog(LOGLEVEL_ERR, "Receive Fail, Socket Close");
			Rtn = 0; goto __end;
		}
		else if (iRecvLen<0) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "Receive Fail, ErrCode=%d", WSAGetLastError());
			Rtn = -1; goto __end;
		}
		uiCompleted += iRecvLen;
	}
	Rtn = uiCompleted;

__end:
	m_SktMutex.Unlock();
	return Rtn;
}


//== 关闭客户端的套接字，子线程
INT CMesDlg::CloseSockThd()
{
	if (sock != INVALID_SOCKET)
	{
		closesocket(sock);
		sock = INVALID_SOCKET;
	}

	Sleep(200);
	m_CliSendWorker.DeleteThread();
	m_CliRecvWorker.DeleteThread();

	//== 关闭UFS软件
	TerminateProcessByName("UFSWriter.exe");
	return 0;
}

// 关闭指定进程名字的函数------------UFSWriter.exe
BOOL CMesDlg::TerminateProcessByName(const char* szProcessName)
{
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe;

	pe.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hSnapShot, &pe)) {
		CloseHandle(hSnapShot);
		return FALSE;
	}

	//  L"UFSWriter.exe"
	do {
		if (_stricmp(pe.szExeFile, "UFSWriter.exe") == 0) {
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
			if (hProcess != NULL) {
				TerminateProcess(hProcess, 0);
				CloseHandle(hProcess);
				CloseHandle(hSnapShot);
				return TRUE;
			}
		}
	} while (Process32Next(hSnapShot, &pe));

	CloseHandle(hSnapShot);
	return FALSE;
}


void CMesDlg::OnBnClickedBtnModelsetting()
{
	// TODO: 在此添加控件通知处理程序代码
	CDlgModuleSetting DlgModuleSetting(&m_LogRichEdit);
	if (DlgModuleSetting.DoModal() == IDOK){
		m_strModuleName = DlgModuleSetting.m_strModuleName;
		m_strrModuleBaudrate = DlgModuleSetting.m_strBaudrate;
		m_strProgramPID = DlgModuleSetting.m_strPID_VID;
		m_strVerifyPID = DlgModuleSetting.m_strVerifyPID;
		m_strCmdLine = DlgModuleSetting.m_strCmdLine;
		m_strModuleExePath = DlgModuleSetting.m_strExePath;
		m_strModuleLogPath = DlgModuleSetting.m_strLogPath;
		m_nTimeOut = DlgModuleSetting.m_nTimeOut;
	}
}


void CMesDlg::SetModuleCompoentEn(BOOL bEn)
{

	for (int i = 0; i < MAX_SKTCNT; i++) {
		GetDlgItem(IDC_CHECK1 + i)->EnableWindow(bEn);
	}

	//GetDlgItem(IDC_BTN_MODULESETTING)->EnableWindow(bEn);
	GetDlgItem(IDC_BTN_GETCOM)->EnableWindow(bEn);
	GetDlgItem(IDC_EDIT_FW_PROJPATH)->EnableWindow(bEn);
	GetDlgItem(IDC_BTNSEL_FW_PROJ)->EnableWindow(bEn);

}


INT CMesDlg::DoProductionCheck() {
	m_bQuit = FALSE;
	m_bRecvAutomaticLotEnd = FALSE;
	m_pIStatus->PrintStatus(0, "");
	m_ProgramResult.DevTimeRun = _T("000000");
	m_ProgramResult.DevTimeStop = _T("000000");

	UpdateProgramResult2UI(0, 0, 0);

	if (IsMesMode()) {
		if (m_bProgRecordReady == FALSE) {
			CString strErrMsg;
			CString strText;
			GetDlgItemText(IDC_BTNGetMesRecord, strText);
			strErrMsg.Format("请先点击\"%s\"按钮进行信息获取", strText);
			m_LogRichEdit.PrintLog(LOGLEVEL_ERR, "%s", strErrMsg);
			m_pIStatus->PrintStatus(0, strErrMsg);
			MessageBox(strErrMsg, NULL, MB_OK | MB_ICONERROR);
			return -1;
		}
	}

	UpdateData(TRUE);

	if (m_strProjChecksum.IsEmpty()) {
		AfxMessageBox("期望检验值不能为空");
		return -1;
	}
	m_MainData.streMMCMID = "";

	if (UpdateRecordForProduce() == FALSE) {
		m_pIStatus->PrintStatus(0, "确认生产信息失败");
		return -1;
	}
	return 0;
}

BOOL CMesDlg::InitModule(BOOL IsProgram)
{
	BOOL Ret = FALSE;


	if (pModuleProgram == NULL) {
		m_strModuleName = m_Setting.strModuleName;
		pModuleProgram = new ModuleProgram(m_strModuleName, &m_Setting, m_pILog);
	}
	if (pModuleProgram == NULL || pModuleProgram->Init() != 0) {
		m_LogRichEdit.PrintLog(LOGLEVEL_ERR, "模组烧录初始化失败");
		return Ret;
	}
	for (int i = 0; i < MAX_SKTCNT; i++) {
		CRichEditCtrl* pRichEditCtrl = (CRichEditCtrl*)GetDlgItem(IDC_RICHEDIT22 + i);
		pRichEditCtrl->SetWindowText("N/A");
		pRichEditCtrl->SetBackgroundColor(FALSE, RGB(255,255,255));

		if (!IsProgram){
			((CButton*)GetDlgItem(IDC_CHECK1 + i))->SetCheck(FALSE);
			(GetDlgItem(IDC_EDIT1 + i))->SetWindowText("");
			if (i<SITE_COUNT) {
				CString SiteAlias;
				SiteAlias.Format("站点0%d", 1 + i);
				((CEdit*)GetDlgItem(IDC_DEV1 + i))->SetWindowText(SiteAlias);
			}
		}
		
	}
	{
		//设置回调
		using std::placeholders::_1;
		std::function<void(CString)> f = std::bind(&CMesDlg::ClearUIToastDlg, this, _1);
		((ModuleProgram *)pModuleProgram)->SetCallback(f);
	}

	Ret = TRUE;
	return Ret;
}

void CMesDlg::OnBnClickedBtnGetcom()
{
	// TODO: 在此添加控件通知处理程序代码
	if (DoProductionCheck() < 0) {
		return;
	}
	GetDlgItem(IDC_BTNSTARTPRODUCE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_GETCOM)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTNCANCELPRODUCE)->EnableWindow(FALSE);

	m_LogRichEdit.PrintLog(LOGLEVEL_LOG, "===>正在启动中...");
	
	if (!InitModule(FALSE)){
		return;
	}

	pModuleProgram->StartQueryCom(GetSafeHwnd(), m_ProgRecord.DestRecord.strProjPath);
}


void CMesDlg::ClearUIToastDlg(CString strTitleInfo)
{
	int nRetry = 0;
	CString strMsg;
	m_LogRichEdit.PrintLog(LOGLEVEL_LOG, "ClearUIToastDlg");
	while (true) {

		CWnd* pWnd = FindWindow(NULL, strTitleInfo);//
		if (pWnd == NULL) {
			goto __end;
		}

		::SendMessage(pWnd->GetSafeHwnd(), SW_SHOWMINNOACTIVE, 0, 0);
		::SendMessage(pWnd->GetSafeHwnd(), WM_CLOSE, 0, 0);

		strMsg.Format("====>关闭原厂弹窗，TitleInfo:%s", strTitleInfo);
		m_LogRichEdit.PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);

		Sleep(80);
		nRetry++;
		if (FindWindow(NULL, strTitleInfo) != NULL) {
			if (nRetry >= 10) {
				break;
			}
			strMsg.Format("正在尝试Retry关闭原厂弹窗");
			m_LogRichEdit.PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);
			continue;
		}
		break;
	}

__end:
	return;
}

INT CMesDlg::UpdateProgramResult2UI(UINT PassCnt, UINT FailCnt, UINT TotalCnt) {
	CString strSum;
	m_YieldSites.TotalCnt = TotalCnt;
	m_YieldSites.FailCnt = FailCnt;
	m_YieldSites.PassCnt = PassCnt;
	strSum.Format("%u", PassCnt);
	GetDlgItem(IDC_EDIT_OK_SUM)->SetWindowText(strSum);

	strSum.Format("%u", FailCnt);
	GetDlgItem(IDC_EDIT_NG_SUM)->SetWindowText(strSum);

	strSum.Format("%u", TotalCnt);
	GetDlgItem(IDC_EDIT_FINISHCNT)->SetWindowText(strSum);
	return 0;
}


BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	CString strVersion;
	CString strSVN;
	strSVN.Format("%d", VER_REVISIONSVN);
	strVersion.Format("svn:%s", strSVN);
	((CStatic*)GetDlgItem(IDC_STATIC_SVNVERSION))->SetWindowText(strVersion);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
