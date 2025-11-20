// DlgUserManager.cpp : 实现文件
//

#include "stdafx.h"
#include "DlgUserManager.h"
#include "afxdialogex.h"
#include "resource.h"
#include "DlgUserUpdate.h"

#define MIN_PASSWORD_LENGTH		(6)

// CDlgUserManager 对话框

IMPLEMENT_DYNAMIC(CDlgUserManager, CDialogEx)

CDlgUserManager::CDlgUserManager(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_USER_MANAGER, pParent)
{

}

CDlgUserManager::~CDlgUserManager()
{
}

void CDlgUserManager::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_USER, m_listUser);
}


BEGIN_MESSAGE_MAP(CDlgUserManager, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_USER_DEL, &CDlgUserManager::OnBnClickedButtonUserDel)
	ON_BN_CLICKED(IDC_BUTTON_USER_ADD, &CDlgUserManager::OnBnClickedButtonUserAdd)
	ON_BN_CLICKED(IDC_BUTTON_USER_UPDATE, &CDlgUserManager::OnBnClickedButtonUserUpdate)
END_MESSAGE_MAP()


// CDlgUserManager 消息处理程序

#define DLG_BACKCOLOR (RGB(230,230,250))
BOOL CDlgUserManager::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetBackgroundColor(DLG_BACKCOLOR);

	// Ask Mfc to create/insert a column
	m_listUser.InsertColumn(0, "序号", LVCFMT_CENTER, 100);
	m_listUser.InsertColumn(1, "用户名", LVCFMT_CENTER, 300);
	m_listUser.InsertColumn(2, "角色", LVCFMT_CENTER, 300);

	LONG lStyle;
	lStyle = GetWindowLong(m_listUser.m_hWnd, GWL_STYLE);
	lStyle &= ~LVS_TYPEMASK;
	lStyle |= LVS_REPORT;
	SetWindowLong(m_listUser.m_hWnd, GWL_STYLE, lStyle);
	DWORD dwStyle = m_listUser.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;
	dwStyle |= LVS_EX_GRIDLINES;
	//dwStyle |= LVS_EX_CHECKBOXES;
	m_listUser.SetExtendedStyle(dwStyle);

	CUserManager &UserManager = CUserManager::getInstance();
	UserManager.GetUserList(m_listInfo);//admin显示全部，其他的显示自己
	if (UserManager.GetUserRole().CompareNoCase("Admin") != 0){//角色不是admin，则不能新增和删除
		GetDlgItem(IDC_BUTTON_USER_ADD)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_USER_DEL)->EnableWindow(FALSE);
	}

	for (size_t i = 0; i < m_listInfo.size(); ++i) {
		UserInfo oneItem = m_listInfo[i];
		int nItem = m_listUser.GetItemCount();	//获得列表行数
		CString value = _T("");
		value.Format("%d", i + 1);
		m_listUser.InsertItem(nItem, value);
		value.Format("%s", oneItem.m_UserName);
		m_listUser.SetItemText(nItem, 1, value);
		value.Format("%s", "********");
		//value.Format("%s", oneItem.m_UserPassword);
		//m_listUser.SetItemText(nItem, 2, value);
		//value.Format("0x%X", oneItem.m_Role);
		if (strcmp((const char *)oneItem.m_Role,ADMIN_USER_ROLE)==0)
		{
			value.Format("%s", "管理员");
			m_listUser.SetItemText(nItem, 2, value);
		}
		else if (strcmp((const char *)oneItem.m_Role, ENGINEER_USER_ROLE) == 0)
		{
			value.Format("%s", "工程师");
			m_listUser.SetItemText(nItem, 2, value);
		}
		else if (strcmp((const char *)oneItem.m_Role, OPERATOR_USER_ROLE) == 0)
		{
			value.Format("%s", "操作员");
			m_listUser.SetItemText(nItem, 2, value);
		}
		/*else if (strcmp((const char *)oneItem.m_Role, LOGIN_USER_ROLE) == 0)
		{
			value.Format("%s", "登陆用户");
			m_listUser.SetItemText(nItem, 2, value);
		}*/

	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CDlgUserManager::OnBnClickedButtonUserDel()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strTemp;
	CUserManager &UserManager = CUserManager::getInstance();
	UserManager.GetUserList(m_listInfo);

	for (int iItem = m_listUser.GetItemCount() - 1; iItem >= 0; iItem--) {
		if (m_listUser.GetItemState(iItem, LVIS_SELECTED) == LVIS_SELECTED /*|| m_listUser.GetCheck(iItem)*/) {
			if (iItem == 0) {
				MessageBox("Admin不可删除");
			}
			else {
				strTemp.Format("此操作将永久删除%s用户,确认是否删除", m_listInfo[iItem].m_UserName);
				int ok = AfxMessageBox(strTemp, MB_YESNO);
				if (ok != IDYES) {
					return;
				}
				CString Name;
				Name.Format("%s", m_listInfo[iItem].m_UserName);
				UserManager.RemoveUser(Name);
				m_listUser.DeleteItem(iItem);
				CString Temp;
				for (int i = 0; i < m_listUser.GetItemCount(); i++) {
					Temp.Format(_T("%d"), i + 1);
					m_listUser.SetItemText(i, 0, Temp);
				}
				break;
			}
		}
	}

}


void CDlgUserManager::OnBnClickedButtonUserAdd()
{
	// TODO: 在此添加控件通知处理程序代码
	CUserManager &UserManager = CUserManager::getInstance();
	CDlgUserUpdate  DlgUserAddInfo;
	DlgUserAddInfo.m_bIsAdd = TRUE;

	if (IDOK == DlgUserAddInfo.DoModal()) {

		int Row = m_listUser.GetItemCount();
		if (m_listInfo.size() >= MAX_ITEM_INFO) {
			MessageBox("当前用户个数已达上限");
			return;
		}
		CString Temp = _T("");
		Temp.Format("%d", Row + 1);

		if (DlgUserAddInfo.m_UserName.IsEmpty() || DlgUserAddInfo.m_NewPassword.IsEmpty() || DlgUserAddInfo.m_NewRole.IsEmpty()|| DlgUserAddInfo.m_strConfirm_Pas.IsEmpty()) {
			MessageBox("输入信息不完整，请重新输入");
			return;
		}
		if (!DlgUserAddInfo.m_strConfirm_Pas.IsEmpty() && DlgUserAddInfo.m_strConfirm_Pas != DlgUserAddInfo.m_NewPassword) {
			CString errMsg = _T("");
			errMsg.Format("两次输入密码不一致，请重新确认");
			MessageBox(errMsg);
			return;
		}
		if (DlgUserAddInfo.m_NewPassword.GetLength() < MIN_PASSWORD_LENGTH) {
			CString errMsg = _T("");
			errMsg.Format("输入密码不能少于%d", MIN_PASSWORD_LENGTH);
			MessageBox(errMsg);
			return;
		}
		for (int i = 0; i < m_listUser.GetItemCount(); i++)
		{
			if (m_listUser.GetItemText(i, 1) == DlgUserAddInfo.m_UserName)
			{
				MessageBox("该用户已存在，请重新输入");
				return;
			}
		}
		if (DlgUserAddInfo.m_NewRole == "管理员") {
			UserManager.AddUser(DlgUserAddInfo.m_UserName, DlgUserAddInfo.m_NewPassword, ADMIN_USER_ROLE);
		}
		else if (DlgUserAddInfo.m_NewRole == "工程师") {
			UserManager.AddUser(DlgUserAddInfo.m_UserName, DlgUserAddInfo.m_NewPassword, ENGINEER_USER_ROLE);
		}
		else if (DlgUserAddInfo.m_NewRole == "操作员") {
			UserManager.AddUser(DlgUserAddInfo.m_UserName, DlgUserAddInfo.m_NewPassword, OPERATOR_USER_ROLE);
		}
		/*else if (DlgUserAddInfo.m_NewRole == "登陆用户") {
			UserManager.AddUser(DlgUserAddInfo.m_UserName, DlgUserAddInfo.m_NewPassword, LOGIN_USER_ROLE);
		}*/

		m_listUser.InsertItem(Row, Temp);
		m_listUser.SetItemText(Row, 1, DlgUserAddInfo.m_UserName);
		m_listUser.SetItemText(Row, 2, DlgUserAddInfo.m_NewRole);
	}

}


void CDlgUserManager::OnBnClickedButtonUserUpdate()
{
	// TODO: 在此添加控件通知处理程序代码

	CUserManager &UserManager = CUserManager::getInstance();
	UserManager.GetUserList(m_listInfo);
	CDlgUserUpdate  DlgUserUpdate;
	for (int iItem = m_listUser.GetItemCount() - 1; iItem >= 0; iItem--) {
		if (m_listUser.GetItemState(iItem, LVIS_SELECTED) == LVIS_SELECTED) {
			//初始化用户名编辑框内容
			CString Name;
			Name.Format("%s", m_listInfo[iItem].m_UserName);
			DlgUserUpdate.m_UserName = Name;

			if (IDOK == DlgUserUpdate.DoModal()) {
				if (!DlgUserUpdate.m_strConfirm_Pas.IsEmpty() ) {
					if (DlgUserUpdate.m_strConfirm_Pas.GetLength() < MIN_PASSWORD_LENGTH) {
						CString errMsg = _T("");
						errMsg.Format("输入密码不能少于%d", MIN_PASSWORD_LENGTH);
						MessageBox(errMsg);
						return;
					}
					if (DlgUserUpdate.m_strConfirm_Pas != DlgUserUpdate.m_NewPassword){
						CString errMsg = _T("");
						errMsg.Format("两次输入密码不一致，请重新确认");
						MessageBox(errMsg);
						return;
					}
				}

				if (!DlgUserUpdate.m_NewRole.IsEmpty()) {
					if (DlgUserUpdate.m_NewRole == "管理员") {
						UserManager.UpdateUser((const char *)m_listInfo[iItem].m_UserName, DlgUserUpdate.m_strConfirm_Pas, ADMIN_USER_ROLE);
					}
					else if (DlgUserUpdate.m_NewRole == "工程师") {
						UserManager.UpdateUser((const char *)m_listInfo[iItem].m_UserName, DlgUserUpdate.m_strConfirm_Pas, ENGINEER_USER_ROLE);
					}
					else if (DlgUserUpdate.m_NewRole == "操作员") {
						UserManager.UpdateUser((const char *)m_listInfo[iItem].m_UserName, DlgUserUpdate.m_strConfirm_Pas, OPERATOR_USER_ROLE);
					}
				}
				else {
					MessageBox("输入信息不完整，请重新输入");
					return;
				}
				m_listUser.DeleteItem(iItem);

				CString temp = _T("");
				temp.Format("%d", iItem + 1);
				CString info = _T("");

				m_listUser.InsertItem(iItem, temp);
				info.Format("%s", m_listInfo[iItem].m_UserName);
				m_listUser.SetItemText(iItem, 1, info);

				if (!DlgUserUpdate.m_NewRole.IsEmpty()) {
					m_listUser.SetItemText(iItem, 2, DlgUserUpdate.m_NewRole);
				}
				else {
					if (strcmp((const char *)m_listInfo[iItem].m_Role, ADMIN_USER_ROLE) == 0){
						info.Format("%s", "管理员");
						m_listUser.SetItemText(iItem, 2, info);
					}
					else if (strcmp((const char *)m_listInfo[iItem].m_Role, ENGINEER_USER_ROLE) == 0){
						info.Format("%s", "工程师");
						m_listUser.SetItemText(iItem, 2, info);
					}
					else if (strcmp((const char *)m_listInfo[iItem].m_Role, OPERATOR_USER_ROLE) == 0) {
						info.Format("%s", "操作员");
						m_listUser.SetItemText(iItem, 2, info);
					}
				}
			}
		}
	}

}
