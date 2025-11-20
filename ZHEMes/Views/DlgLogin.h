#pragma once



// CDlgLogin 对话框
#include "IMesAccess.h"
#include "OperatorData.h"
#include "Setting.h"
#include "ComTool.h"

//#include "soapH.h"
//#include "soapStub.h"
////#include "OnlineWebServiceSoap.nsmap"
////#include "soapOnlineWebServiceSoapProxy.h"

#include <fstream>
#include "Shlwapi.h"
#include "afxwin.h"


class CDlgLogin : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgLogin)

public:
	CDlgLogin(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgLogin();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGLOGIN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	void AttachILog(ILog *pILog) { m_pILog = pILog; }
	void AttachData(COperatorData *pOperatorData) {
		m_pOperatorData = pOperatorData;
	};
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedChkshowpasswd();
	BOOL m_bShowPasswd;

private:
	ILog *m_pILog;
	COperatorData *m_pOperatorData;
	CSetting m_Setting;
	CString m_strOperatorID;
	CString m_strAdminPasswd;
public:
	afx_msg void OnBnClickedOk();
	virtual void OnCancel();
	std::string  WideCharToMultiByte(const wchar_t* pwszMultiByte, UINT uCodePage = CP_ACP);
	CComboBox m_CmbModel;
};
