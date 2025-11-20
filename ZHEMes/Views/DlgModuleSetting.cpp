// DlgModuleSetting.cpp : 实现文件
//

#include "stdafx.h"
#include "DlgModuleSetting.h"
#include "afxdialogex.h"
#include "resource.h"
#include "ComTool.h"
#include "cJSON.h"

// CDlgModuleSetting 对话框

IMPLEMENT_DYNAMIC(CDlgModuleSetting, CDialogEx)

CDlgModuleSetting::CDlgModuleSetting(ILog *pILog,CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_PROGRAM_MOULE, pParent)
	, m_strModuleName(_T(""))
	, m_strBaudrate(_T(""))
	, m_strPID_VID(_T(""))
	, m_strCmdLine(_T(""))
	, m_strExePath(_T(""))
	, m_strLogPath(_T(""))
	, m_strVerifyPID(_T(""))
	, m_nTimeOut(0)
{
	m_pILog = pILog;
	m_ModuleNameMap.clear();
	m_InfoMap.clear();
}

CDlgModuleSetting::~CDlgModuleSetting()
{
}

void CDlgModuleSetting::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_BAUDRATE, m_strBaudrate);
	DDX_Text(pDX, IDC_EDIT_PID, m_strPID_VID);
	DDX_Text(pDX, IDC_EDIT_CMDLINE, m_strCmdLine);
	DDX_Text(pDX, IDC_EDIT_EXEPATH, m_strExePath);
	DDX_Text(pDX, IDC_EDIT_MODULE_LOGPATH, m_strLogPath);
	DDX_Control(pDX, IDC_CMB_MODULENAME, m_CmbModuleName);
	DDX_Control(pDX, IDC_CMB_MODE, m_CmbMOde);
	DDX_Text(pDX, IDC_EDIT_VERIFY_PID, m_strVerifyPID);
	DDX_Text(pDX, IDC_EDIT_TIMEOUT, m_nTimeOut);
}


BEGIN_MESSAGE_MAP(CDlgModuleSetting, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgModuleSetting::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_CMB_MODULENAME, &CDlgModuleSetting::OnCbnSelchangeCmbModulename)
END_MESSAGE_MAP()


// CDlgModuleSetting 消息处理程序


void CDlgModuleSetting::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if (m_strPID_VID.IsEmpty() || m_strCmdLine.IsEmpty() || m_strExePath.IsEmpty() || m_strLogPath.IsEmpty()){
		AfxMessageBox("信息不完全，请确认");
		return;
	}
	GetDlgItem(IDC_CMB_MODULENAME)->GetWindowTextA(m_strModuleName);
	m_Setting.strModuleName = m_strModuleName;
	m_Setting.Save();

	CDialogEx::OnOK();
}

INT CDlgModuleSetting::GetModuleJson()
{
	INT Ret = -1;
	CString strJsonPath;
	BYTE *pData = NULL;
	CFile File;
	CString strJson;
	CString strModuleName;

	INT FileSize = 0, ArraySize = 0;

	cJSON* pRootParser = NULL;
	cJSON* pArrayItem = NULL;
	cJSON* pItemObj = NULL;

	tModuleInfos Info;

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
	if (ArraySize<= 0){
		m_pILog->PrintLog(LOGLEVEL_ERR, "内容不存在 ");
		goto __end;
	}
	for (int i = 0; i < ArraySize; ++i){
		pArrayItem = cJSON_GetArrayItem(pRootParser, i);

		pItemObj = cJSON_GetObjectItem(pArrayItem, "ModuleName");
		if (pItemObj == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析ModuleName字段错误，请确认json文件中维护的字段信息 ");
			goto __end;
		}
		m_ModuleNameMap[i] = pItemObj->valuestring;
		strModuleName = pItemObj->valuestring;

		pItemObj = cJSON_GetObjectItem(pArrayItem, "Baudrate");
		if (pItemObj == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析Baudrate字段错误，请确认json文件中维护的字段信息 ");
			goto __end;
		}
		Info.strBaudrate = pItemObj->valuestring;

		pItemObj = cJSON_GetObjectItem(pArrayItem, "Program_PIDVID");
		if (pItemObj == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析Program_PIDVID字段错误，请确认json文件中维护的字段信息 ");
			goto __end;
		}
		Info.strProgramPID_VID = pItemObj->valuestring;


		pItemObj = cJSON_GetObjectItem(pArrayItem, "Verify_PIDVID");
		if (pItemObj == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析Verify_PIDVID字段错误，请确认json文件中维护的字段信息 ");
			goto __end;
		}
		Info.strVerifyPID_VID = pItemObj->valuestring;

		pItemObj = cJSON_GetObjectItem(pArrayItem, "CmdLine");
		if (pItemObj == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析CmdLine字段错误，请确认json文件中维护的字段信息 ");
			goto __end;
		}
		Info.strCmdLine = pItemObj->valuestring;

		pItemObj = cJSON_GetObjectItem(pArrayItem, "ExePath");
		if (pItemObj == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析ExePath字段错误，请确认json文件中维护的字段信息 ");
			goto __end;
		}
		Info.strExePath = pItemObj->valuestring;

		pItemObj = cJSON_GetObjectItem(pArrayItem, "LogPath");
		if (pItemObj == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析LogPath字段错误，请确认json文件中维护的字段信息 ");
			goto __end;
		}
		Info.strLogPath = pItemObj->valuestring;

		pItemObj = cJSON_GetObjectItem(pArrayItem, "ProgramMode");
		if (pItemObj == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析ProgramMode字段错误，请确认json文件中维护的字段信息 ");
			goto __end;
		}
		Info.strMode = pItemObj->valuestring;

		pItemObj = cJSON_GetObjectItem(pArrayItem, "TimeOut");
		if (pItemObj == NULL) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "解析TimeOut字段错误，请确认json文件中维护的字段信息 ");
			goto __end;
		}
		Info.nTimeOut = pItemObj->valueint;
		m_InfoMap[strModuleName] = Info;
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


BOOL CDlgModuleSetting::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化


	CString strSettingJsonPath;
	strSettingJsonPath.Format("%s\\Setting.json", GetCurrentPath());
	m_Setting.SetJsonPath(strSettingJsonPath);
	if (m_Setting.Load()) {
		m_strModuleName = m_Setting.strModuleName;
	}


	if (m_pILog == NULL){
		AfxMessageBox("初始化失败");
	}
	m_CmbMOde.AddString("多进程烧录");
	m_CmbMOde.AddString("单进程烧录");

	GetModuleJson();

	if (m_ModuleNameMap.size() != 0 ){
		for (auto it : m_ModuleNameMap){
			m_CmbModuleName.AddString(it.second);
		}
	}
	GetDlgItem(IDC_CMB_MODULENAME)->SetWindowTextA(m_strModuleName);

	if (m_InfoMap.size() != 0) {
		for (auto it : m_InfoMap) {
			if (m_strModuleName.CompareNoCase(it.first) == 0){
				m_strBaudrate = it.second.strBaudrate;
				m_strPID_VID = it.second.strProgramPID_VID;
				m_strVerifyPID =it.second.strVerifyPID_VID;
				m_strCmdLine = it.second.strCmdLine;
				m_strExePath = it.second.strExePath;
				m_strLogPath = it.second.strLogPath;
				m_nTimeOut = it.second.nTimeOut;
				m_CmbMOde.SetWindowTextA(it.second.strMode);
			}
		}
	}

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}




void CDlgModuleSetting::OnCbnSelchangeCmbModulename()
{
	// TODO: 在此添加控件通知处理程序代码
	INT Index = m_CmbModuleName.GetCurSel();	CString strModuleName;	if (Index >= 0) {		m_CmbModuleName.GetLBText(Index, strModuleName);	}	for (auto it : m_InfoMap) {		if (it.first.CompareNoCase(strModuleName) == 0) {			m_strBaudrate = it.second.strBaudrate;			m_strPID_VID = it.second.strProgramPID_VID;			m_strVerifyPID = it.second.strVerifyPID_VID;			m_strCmdLine = it.second.strCmdLine;			m_strExePath = it.second.strExePath;			m_strLogPath = it.second.strLogPath;			m_nTimeOut = it.second.nTimeOut;			m_CmbMOde.SetWindowTextA(it.second.strMode);		}	}	UpdateData(FALSE);
}
