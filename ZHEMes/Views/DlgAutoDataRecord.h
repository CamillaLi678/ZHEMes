#pragma once
#include "afxwin.h"
#include "Setting.h"

// CDlgAutoDataRecord 对话框

class CDlgAutoDataRecord : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgAutoDataRecord)

public:
	CDlgAutoDataRecord(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgAutoDataRecord();

	void AttachSetting(CSetting *pSetting) {
		m_pSetting = pSetting;
	};

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum {
		IDD = IDD_DLGAUTODATA
	};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

	void GetFileSavePath(CString strExts, CString&strFileSave,CString strExt);
	void GetFileOpenPath(CString strExts, CString&strFileOpen);
	CString GetAutoTaskFolder();

public:
	CSetting*m_pSetting;
	virtual BOOL OnInitDialog();
	CComboBox m_cmbMachineID;
	CString m_strChipName;
	CString m_strAdapterName;
	// 卷带是否使能
	BOOL m_bTapeEn;
	DWORD m_PosX;
	DWORD m_PosY;
	CString m_strAdapterData;
	CString m_strTrayData;
	CString m_strTapeData;
	afx_msg void OnBnClickedBtnsaveautodata();
	afx_msg void OnBnClickedBtnloadautodata();
	afx_msg void OnBnClickedBtnseladapterdata();
	afx_msg void OnBnClickedBtnseltraydata();
	afx_msg void OnBnClickedBtnseltapedata();
};
