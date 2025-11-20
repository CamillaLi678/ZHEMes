#pragma once
#include "afxcmn.h"
#include "UserManager.h"


//每一位表示一个权限，角色由多个权限位组成。

#define ADMIN_USER_ROLE			"Admin"
#define ENGINEER_USER_ROLE		"Engineer"
#define OPERATOR_USER_ROLE		"Operator"
#define SETTING_USER_ROLE		"Setting"
#define PRODUCTION_USER_ROLE	"Production"
#define LOGIN_USER_ROLE			"Login"

// CDlgUserManager 对话框


class CDlgUserManager : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgUserManager)

public:
	CDlgUserManager(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgUserManager();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_USER_MANAGER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CListCtrl m_listUser;
	std::vector<UserInfo> m_listInfo;
	afx_msg void OnBnClickedButtonUserDel();
	afx_msg void OnBnClickedButtonUserAdd();
	afx_msg void OnBnClickedButtonUserUpdate();
};
