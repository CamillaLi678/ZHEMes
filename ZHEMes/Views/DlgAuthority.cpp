// DlgAuthority.cpp : 实现文件
//
#include "stdafx.h"
#include "DlgAuthority.h"
#include "afxdialogex.h"
#include "resource.h"
#include "DlgUserManager.h"
#include <bitset>


// CDlgAuthority 对话框

IMPLEMENT_DYNAMIC(CDlgAuthority, CDialogEx)

CDlgAuthority::CDlgAuthority(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_AUTHORITY, pParent)
{

}

CDlgAuthority::~CDlgAuthority()
{
}

void CDlgAuthority::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_AUTHORITY, m_Authority_List);
	DDX_Control(pDX, IDC_COMBO_Roles, m_cmbRoles);
}


BEGIN_MESSAGE_MAP(CDlgAuthority, CDialogEx)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_AUTHORITY, &CDlgAuthority::OnNMDblclkListAuthority)
	ON_CBN_SELCHANGE(IDC_COMBO_Roles, &CDlgAuthority::OnCbnSelchangeComboRoles)
	ON_BN_CLICKED(IDOK, &CDlgAuthority::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgAuthority 消息处理程序


BOOL CDlgAuthority::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	m_cmbRoles.AddString("Admin");
	m_cmbRoles.AddString("Engineer");
	m_cmbRoles.AddString("Operator");
	m_cmbRoles.SetCurSel(0);

	CRect rect;
	m_Authority_List.GetClientRect(&rect);
	m_Authority_List.InsertColumn(0, _T("用户权限"), LVCFMT_CENTER, rect.Width(), 0);

	m_Authority_List.InsertItem(0, _T("设置权限"));
	m_Authority_List.InsertItem(1, _T("自动化坐标权限"));
	m_Authority_List.InsertItem(2, _T("打印权限"));
	m_Authority_List.InsertItem(3, _T("获取信息"));
	m_Authority_List.InsertItem(4, _T("批量烧录"));


	LONG lStyle;
	lStyle = GetWindowLong(m_Authority_List.m_hWnd, GWL_STYLE);
	lStyle &= ~LVS_TYPEMASK;
	lStyle |= LVS_REPORT;
	SetWindowLong(m_Authority_List.m_hWnd, GWL_STYLE, lStyle);
	DWORD dwStyle = m_Authority_List.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;
	dwStyle |= LVS_EX_GRIDLINES;
	dwStyle |= LVS_EX_CHECKBOXES;
	dwStyle |= LVS_SINGLESEL;
	m_Authority_List.SetExtendedStyle(dwStyle);


	INT Row = m_Authority_List.GetItemCount();
	for (INT i = 0; i < Row; i++) {////初始化显示Admin，具有全部权限
		m_Authority_List.SetCheck(i, TRUE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CDlgAuthority::OnNMDblclkListAuthority(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	BOOL bCheckStatus;
	bCheckStatus = m_Authority_List.GetCheck(pNMLV->iItem);
	m_Authority_List.SetCheck(pNMLV->iItem, !bCheckStatus);

	*pResult = 0;
}

void CDlgAuthority::OnCbnSelchangeComboRoles()
{
	// TODO: 在此添加控件通知处理程序代码
	
	
	CString strRoles;
	CUserManager &UserManagerInterface = CUserManager::getInstance();
	UserManagerInterface.GetRoleList(m_roleList);
	INT Row = m_Authority_List.GetItemCount();
	INT index = m_cmbRoles.GetCurSel();
	m_cmbRoles.GetLBText(index, strRoles);
	

	for (INT i = 0; i < m_roleList.size(); i++){
		if (strRoles == (char *)m_roleList[i].m_Role){
			std::bitset<PERMISSION_TOTAL> bit(m_roleList[i].m_Permission);
			for (INT j = 0; j < bit.size(); j++){
				m_Authority_List.SetCheck(j, FALSE);
				if (bit[j] == 1){
					m_Authority_List.SetCheck(j, TRUE);
				}
			}
		}
	}
}


void CDlgAuthority::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL Check;
	CString strRole;
	CUserManager &UserManagerInterface = CUserManager::getInstance();
	INT Row = m_Authority_List.GetItemCount();
	
	GetDlgItem(IDC_COMBO_Roles)->GetWindowText(strRole);
	UserManagerInterface.GetRoleList(m_roleList);

	for (INT i = 0; i < m_roleList.size(); i++){
		if (strRole == (char *)m_roleList[i].m_Role){
			std::bitset<PERMISSION_TOTAL> bit(m_roleList[i].m_Permission);
			for (INT j = 0; j < Row; j++) {
				Check = m_Authority_List.GetCheck(j);
				if (Check){
					bit[j] = 1;
				}
				else{
					bit[j] = 0;
				}
			}
			UserManagerInterface.UpdateRole(strRole, bit.to_ulong());
		}
	}
	

	CDialogEx::OnOK();
}
