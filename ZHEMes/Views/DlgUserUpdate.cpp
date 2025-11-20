// DlgUserUpdate.cpp : 实现文件
//

#include "stdafx.h"
#include "DlgUserUpdate.h"
#include "afxdialogex.h"
#include "resource.h"
#include "UserManager.h"
#include "DlgUserManager.h"

// CDlgUserUpdate 对话框

IMPLEMENT_DYNAMIC(CDlgUserUpdate, CDialogEx)

CDlgUserUpdate::CDlgUserUpdate(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_USER_UPDATE, pParent)
	, m_NewPassword(_T(""))
	, m_NewRole(_T(""))
	, m_UserName(_T(""))
	, m_ShowPassword(FALSE)
	, m_strConfirm_Pas(_T(""))
{
	m_bIsAdd = FALSE;
}

CDlgUserUpdate::~CDlgUserUpdate()
{
}

void CDlgUserUpdate::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_NEWPASSWORD, m_NewPassword);
	DDX_CBString(pDX, IDC_COMBO_NEWROLE, m_NewRole);
	DDX_Control(pDX, IDC_COMBO_NEWROLE, m_Box_Role);
	DDX_Text(pDX, IDC_EDIT_USER_NAME, m_UserName);
	DDX_Check(pDX, IDC_CHKSHOWPASSWD, m_ShowPassword);
	DDX_Text(pDX, IDC_EDIT_NEWPASSWORD2, m_strConfirm_Pas);
}


BEGIN_MESSAGE_MAP(CDlgUserUpdate, CDialogEx)
	ON_BN_CLICKED(IDC_CHKSHOWPASSWD, &CDlgUserUpdate::OnBnClickedChkshowpasswd)
END_MESSAGE_MAP()


// CDlgUserUpdate 消息处理程序


BOOL CDlgUserUpdate::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	m_Box_Role.AddString("管理员");
	m_Box_Role.AddString("操作员");
	m_Box_Role.AddString("工程师");
	//m_Box_Role.AddString("设置用户");

	CDlgUserManager* pDlg = (CDlgUserManager*)GetParent();

	for (int iItem = 0; iItem < pDlg->m_listUser.GetItemCount(); iItem++)
	{
		if (pDlg->m_listUser.GetItemState(iItem, LVIS_SELECTED) == LVIS_SELECTED /*|| m_listUser.GetCheck(iItem)*/) {
			CString UserRole;
			UserRole = pDlg->m_listUser.GetItemText(iItem, 2);
			if (UserRole == "管理员") {
				m_Box_Role.SetCurSel(0);
			}
			else if (UserRole == "操作员") {
				m_Box_Role.SetCurSel(1);
			}
			else if (UserRole == "工程师") {
				m_Box_Role.SetCurSel(2);
			}
			/*else if (UserRole == "设置用户") {
				m_Box_Role.SetCurSel(3);
			}*/
			else {
				m_Box_Role.SetCurSel(2);
			}
		}
	}	
	CUserManager &UserManagerInterface = CUserManager::getInstance();
	if (m_UserName.Compare("Admin") == 0 || UserManagerInterface.GetUserRole().Compare("Admin") != 0) {
		m_Box_Role.EnableWindow(FALSE);
	}
	if (m_bIsAdd) {
		SetWindowText("添加新用户"); 
	}
	else {
		GetDlgItem(IDC_EDIT_USER_NAME)->EnableWindow(FALSE);
	}
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CDlgUserUpdate::OnBnClickedChkshowpasswd()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	CEdit *pEdit1;
	pEdit1 = (CEdit*)(GetDlgItem(IDC_EDIT_NEWPASSWORD));
	if (m_ShowPassword == TRUE) {
		pEdit1->SetPasswordChar(NULL);
	}
	else {
		pEdit1->SetPasswordChar('*');
	}
	pEdit1->SetWindowText(m_NewPassword);
}
