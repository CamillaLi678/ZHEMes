#pragma once

#include "Setting.h"
#include "afxwin.h"
#include "IStatus.h"
#include "ComTool.h"

#include <map>
#include <vector>
using namespace std;

// CDlgSetting 对话框

class CDlgSetting : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSetting)

public:
	CDlgSetting(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgSetting();

	void AttachData(CSetting*pSetting);
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIGSETTING };
#endif
	BOOL SaveSettings();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CSetting *m_pSetting;
	afx_msg void OnBnClickedOk();
	void AttachIStatus(IStatus *pIStatus) { m_pIStatus = pIStatus; }

private:
	CString m_strProjSaveFolder;
	CString m_strACServerFolder;
	CString m_strAutoTaskFolder;
	CString m_strCurExec;  

	CString m_strWordMode;
	CString m_strProjectMode;
	CString m_strAutoMode;
	CString m_strAutomaticType;
	
	IStatus *m_pIStatus;

	std::map<CString, int>m_ExecCmdMap;
	std::map<CString, int>m_WorkModeMap;
	std::map<CString, int>m_ProjectModeMap;
	std::map<CString, int>m_AutoModeMap;
	std::map<CString, int>m_AutomaticTypeMap;

	std::vector<CString> m_vMesWorkMode;
	std::vector<CString> m_vProjectMode;
	std::vector<CString> m_vExecCmd;
	std::vector<CString> m_vMesAutoMode;

	//CString m_strProgramFilePath;
	//CString m_strTemplatePath;
	//CString m_strProjectPath;
	//CString m_strSNCFilePath;
	//CString m_strAutoFilePath;
	

public:
	CComboBox m_cmbExeCmd;
	CComboBox m_cmbWorkMode;
	CComboBox m_cmbProjectMode;
	
	INT GetDirSelect(CString Title, CString&strDirPath);
	void GetFileOpenPath(CString strExts, CString&strFileOpen);

	afx_msg void OnBnClickedBtnselseverpath();
	afx_msg void OnBnClickedBtnselprojdavefolder();
	afx_msg void OnBnClickedBtnselreportfolder();
	afx_msg void OnBnClickedBtnselTskPath();

	CString m_strReportFolder;	
	int m_nElectricInsertionCheck;
	CString m_strWebService;
	CString m_strAutoTaskFileExt;
	CComboBox m_cmbAutoModeComBox;
	CString m_strWorkStationID;
	CComboBox m_comboAutomaticType;

	CString strLang;
	CString m_strUfsWriterExePath;
	afx_msg void OnBnClickedBtnselseverpathUfsSelExePath();
};
