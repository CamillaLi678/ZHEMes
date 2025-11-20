// DlgLogin.cpp : 实现文件
//



#include "stdafx.h"
#include "Mes.h"
#include "DlgLogin.h"
#include "afxdialogex.h"
#include "cJSON.h"
#include "HttpClient.h"
#include "UserManager.h"
#include "DlgUserManager.h"
#include "MesInterface.h"
//#include "SmtFtpServiceSoapBinding.nsmap"

// CDlgLogin 对话框

IMPLEMENT_DYNAMIC(CDlgLogin, CDialogEx)

CDlgLogin::CDlgLogin(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGLOGIN, pParent)
	, m_bShowPasswd(FALSE)
{

}

CDlgLogin::~CDlgLogin()
{
}

void CDlgLogin::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHKSHOWPASSWD, m_bShowPasswd);
	DDX_Text(pDX, IDC_EDITOPERATOR, m_strOperatorID);
	DDX_Text(pDX, IDC_EDITPASSWD, m_strAdminPasswd);
	DDX_Control(pDX, IDC_COMBO_MODEL, m_CmbModel);
}


BEGIN_MESSAGE_MAP(CDlgLogin, CDialogEx)
	ON_BN_CLICKED(IDC_CHKSHOWPASSWD, &CDlgLogin::OnBnClickedChkshowpasswd)
	ON_BN_CLICKED(IDOK, &CDlgLogin::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgLogin 消息处理程序


BOOL CDlgLogin::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetBackgroundColor(RGB(147, 220, 255));
	m_pOperatorData->m_bAuthCancel = FALSE;
	m_strOperatorID = m_pOperatorData->m_strOperator;
	//m_strAdminPasswd = "QWERT";
	//m_strOperatorID="Admin";
	//m_strAdminPasswd = "123";

	CString strSettingJsonPath;
	strSettingJsonPath.Format("%s\\Setting.json", GetCurrentPath());
	m_Setting.SetJsonPath(strSettingJsonPath);
	if (m_Setting.Load()) {
		m_strOperatorID = m_Setting.strOperator;
	}

	m_CmbModel.AddString("Program");
	m_CmbModel.AddString("Module");
	m_CmbModel.AddString("UfsWriter");
	m_CmbModel.SetWindowTextA(m_Setting.strProgramMode);

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CDlgLogin::OnBnClickedChkshowpasswd()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	CEdit *pEdit1;
	pEdit1 = (CEdit*)(GetDlgItem(IDC_EDITPASSWD));
	if (m_bShowPasswd == TRUE) {
		pEdit1->SetPasswordChar(NULL);
	}
	else {
		pEdit1->SetPasswordChar('*');
	}
	pEdit1->SetWindowText(m_strAdminPasswd);
}

std::string CDlgLogin::WideCharToMultiByte(const wchar_t* pwszMultiByte, UINT uCodePage /* = CP_ACP */)
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


void CDlgLogin::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);

	INT  nToken;
	CString strHeader;
	CString strBody;
	CString strBuildJson;
	CString strToken;	
	CString strModel;

	m_Setting.strOperator  = m_strOperatorID;
	m_CmbModel.GetWindowTextA(strModel);
	m_Setting.strProgramMode = strModel;

	BOOL Ret = FALSE;
	m_pOperatorData->m_bCheckAuthPass = TRUE;
	m_pOperatorData->m_strOperator = m_strOperatorID;
	m_pOperatorData->m_Level = 3;
	
	if (m_Setting.nServerTest){
		CMesInterface &MesInterface = CMesInterface::getInstance();
		nToken = MesInterface.GetMesLoginToServer(m_strOperatorID, m_strAdminPasswd);
		if (nToken != 0) {
			m_pILog->PrintLog(LOGLEVEL_LOG, "获取token失败");
			goto __end;
		}
		Ret = TRUE;
		goto __end;
	}

	CUserManager &UserManagerInterface = CUserManager::getInstance();
	UINT role = UserManagerInterface.CheckUser(m_strOperatorID, m_strAdminPasswd);
	if (role > 0) {
		Ret = TRUE;
		goto __end;
	}
	else {
		AfxMessageBox("当前账户登录失败");
	}


__end:
	if (Ret == TRUE) {
		m_Setting.Save();
		CDialogEx::OnOK();
	}
}


void CDlgLogin::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类
	m_pOperatorData->m_bAuthCancel = TRUE;
	CDialogEx::OnCancel();
}
