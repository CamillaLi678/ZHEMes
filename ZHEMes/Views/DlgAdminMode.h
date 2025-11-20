#pragma once

#include "IMesAccess.h"
#include "OperatorData.h"
#include "UserManager.h"
// CDlgAdminMode 对话框

class CDlgAdminMode : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgAdminMode)

public:
	CDlgAdminMode(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgAdminMode();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGADMINMODE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();

	void AttachILog(ILog *pILog) { m_pILog = pILog; }
	void AttachData(COperatorData *pOperatorData) {
		m_pOperatorData = pOperatorData;
	};
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedChkshowpasswd();
	BOOL m_bShowPasswd;
	std::string  WideCharToMultiByte(const wchar_t* pwszMultiByte, UINT uCodePage = CP_ACP);
	CString m_strWebService;
private:
	ILog *m_pILog;
	COperatorData *m_pOperatorData;
	CString m_strOperatorID;
	CString m_strAdminPasswd;
};
