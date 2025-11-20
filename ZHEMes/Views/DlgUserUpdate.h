#pragma once
#include "afxwin.h"


// CDlgUserUpdate 对话框

class CDlgUserUpdate : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgUserUpdate)

public:
	CDlgUserUpdate(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgUserUpdate();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_USER_UPDATE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_NewPassword;
	CString m_NewRole;
	CComboBox m_Box_Role;
	virtual BOOL OnInitDialog();
	CString m_UserName;
	BOOL m_bIsAdd;
	BOOL m_ShowPassword;
	afx_msg void OnBnClickedChkshowpasswd();
	CString m_strConfirm_Pas;
};
