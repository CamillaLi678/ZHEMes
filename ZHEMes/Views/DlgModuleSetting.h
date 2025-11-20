#pragma once

#include "IMesAccess.h"
#include <map>
#include "afxwin.h"
#include "Setting.h"
// CDlgModuleSetting 对话框


typedef struct ModuleInfos
{
	CString strBaudrate;//波特率
	CString strProgramPID_VID;//
	CString strVerifyPID_VID;
	CString strMode;//烧录模式
	CString strCmdLine;//命令行
	CString strExePath;
	CString strLogPath;
	long nTimeOut;//烧录时间 /s
}tModuleInfos;

class CDlgModuleSetting : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgModuleSetting)

public:
	CDlgModuleSetting(ILog *pILog,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgModuleSetting();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PROGRAM_MOULE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();

private:
	CSetting m_Setting;
	ILog *m_pILog;
	std::map<INT, CString> m_ModuleNameMap;
	std::map<CString, tModuleInfos> m_InfoMap;
private:
	INT GetModuleJson();
public:
	CString m_strModuleName;
	CString m_strBaudrate;
	CString m_strPID_VID;
	CString m_strCmdLine;
	CString m_strExePath;
	CString m_strLogPath;
	CComboBox m_CmbModuleName;
	afx_msg void OnCbnSelchangeCmbModulename();
	CComboBox m_CmbMOde;
	CString m_strVerifyPID;
	long m_nTimeOut;
};
