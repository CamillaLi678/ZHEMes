#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "UserManager.h"

//每一位表示一个权限，角色由多个权限位组成。

#define PERMISSION_TOTAL (5)


#define PERMISSION_SETTING						(1)//设置权限	
#define PERMISSION_AUTOPOSSETTING				(1<<1)//自动化坐标权限	
#define PERMISSION_PRINTLABEL					(1<<2)//打印权限	
#define PERMISSION_GETMESRECOD					(1<<3)//获取信息
#define PERMISSION_STARTPRODUCE					(1<<4)//批量烧录

//管理员拥有所有权限。
#define ADMIN_PERMISSION				0xFFFFFFFF
#define ENGINEER_PERMISSION		(PERMISSION_SETTING | PERMISSION_AUTOPOSSETTING | PERMISSION_PRINTLABEL |PERMISSION_GETMESRECOD |PERMISSION_STARTPRODUCE)
#define OPERATOR_PERMISSION	    (PERMISSION_STARTPRODUCE | PERMISSION_GETMESRECOD|PERMISSION_AUTOPOSSETTING |PERMISSION_PRINTLABEL)

// CDlgAuthority 对话框

class CDlgAuthority : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgAuthority)

public:
	CDlgAuthority(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgAuthority();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_AUTHORITY };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_Authority_List;
	virtual BOOL OnInitDialog();
	afx_msg void OnNMDblclkListAuthority(NMHDR *pNMHDR, LRESULT *pResult);
	CComboBox m_cmbRoles;
	afx_msg void OnCbnSelchangeComboRoles();
	afx_msg void OnBnClickedOk();
	std::vector<RoleInfo> m_roleList;
};
