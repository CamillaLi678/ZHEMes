// DlgAdminMode.cpp : 实现文件
//

#include "stdafx.h"
#include "Mes.h"
#include "DlgAdminMode.h"
#include "afxdialogex.h"
#include "cJSON.h"
#include "DlgUserManager.h"

//#include "soapH.h"
//#include "soapStub.h"
//#include "OnlineWebServiceSoap.nsmap"
//#include "soapOnlineWebServiceSoapProxy.h"

#include <fstream>
#include "Shlwapi.h"


// CDlgAdminMode 对话框

IMPLEMENT_DYNAMIC(CDlgAdminMode, CDialogEx)

CDlgAdminMode::CDlgAdminMode(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGADMINMODE, pParent)
{

}

CDlgAdminMode::~CDlgAdminMode()
{
}

void CDlgAdminMode::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHKSHOWPASSWD, m_bShowPasswd);
	DDX_Text(pDX, IDC_EDITOPERATOR, m_strOperatorID);
	DDX_Text(pDX, IDC_EDITPASSWD, m_strAdminPasswd);
}


BEGIN_MESSAGE_MAP(CDlgAdminMode, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgAdminMode::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CHKSHOWPASSWD, &CDlgAdminMode::OnBnClickedChkshowpasswd)
END_MESSAGE_MAP()


// CDlgAdminMode 消息处理程序


void CDlgAdminMode::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	BOOL Ret = FALSE;
	CString strErrMsg;
	if (TRUE) {
		if (m_pOperatorData->m_strOperator.CompareNoCase(m_strOperatorID) != 0) {
			CUserManager &UserManagerInterface = CUserManager::getInstance();
			UINT role = UserManagerInterface.CheckUser(m_strOperatorID, m_strAdminPasswd);
			/*if (role > 0) {
				UINT perssion = PERMISSION_SETTING;
				if ((role & perssion) > 0) {
					m_strOperatorID = _T("");
					m_strAdminPasswd = _T("");
					UpdateData(FALSE);
					Ret = TRUE;
					goto __end;
				}
				else {
					AfxMessageBox("当前账户没有此权限");
				}
			}
			else {
				AfxMessageBox("登陆失败");
			}*/
		}
		else {
			AfxMessageBox("当前账户已登陆，请输入另一个用户");
		}
	}

	if (Ret == TRUE) {
		CDialogEx::OnOK();
	}
}

std::string CDlgAdminMode::WideCharToMultiByte(const wchar_t* pwszMultiByte, UINT uCodePage /* = CP_ACP */)
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

void CDlgAdminMode::OnBnClickedChkshowpasswd()
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


BOOL CDlgAdminMode::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetBackgroundColor(RGB(147, 220, 255));
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


