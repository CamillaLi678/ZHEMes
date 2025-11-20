#pragma once

#include "IMesAccess.h"
#include "Setting.h"
// CDlgMESTest 对话框

class CDlgMESTest : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgMESTest)

public:
	CDlgMESTest(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgMESTest();
	void AttachILog(ILog *pILog) {
		m_pILog = pILog;
	}
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGMESTEST };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnselResult();
	afx_msg void OnBnClickedBtnsendResultToMes();

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnGetMesRecord();
	afx_msg void OnBnClickedBtnsendproginfo2mes();

	afx_msg void OnBnClickedBtnselstatus();
	afx_msg void OnBnClickedBtnseproginfo();

private:
	CString m_strWorkOrder;
	CString m_strMaterialID;
	CString m_strResult;
	CString m_strResultPath;
	CString m_strStatusFile;
	CString m_strProgInfoFile;
	CSetting m_Setting;
	ILog *m_pILog;
public:

};
