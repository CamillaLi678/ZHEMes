#pragma once


// CDlgCodeScan 对话框

class CDlgCodeScan : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgCodeScan)

public:
	CDlgCodeScan(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgCodeScan();
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGMIDSCAN };
#endif
	virtual BOOL OnInitDialog();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();

	CString m_strTitle;
	CString m_strInputPrompt;

	CString m_strBurnBeforeCode;
	CString m_strMuPianInfo1;
	CString m_strMuPianInfo2;
	CString m_strBurnAfterCode;
};
