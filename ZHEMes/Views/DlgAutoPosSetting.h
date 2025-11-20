#pragma once

class CAutoPosSetting {
public:
	CAutoPosSetting() { Init(); };
	BOOL bProvideTrayPosEn;
	BOOL bOKTrayPosEn;
	BOOL bNGTrayPosEn;
	UINT ProvideTrayXPos;
	UINT ProvideTrayYPos;
	UINT OKTrayXPos;
	UINT OKTrayYPos;
	UINT NGTrayXPos;
	UINT NGTrayYPos;
	UINT ReelPos;

	void Init() {
		bProvideTrayPosEn = FALSE;
		bOKTrayPosEn = FALSE;
		bNGTrayPosEn = FALSE;
		ProvideTrayXPos = 0;
		ProvideTrayYPos = 0;
		OKTrayXPos = 0;
		OKTrayYPos = 0;
		NGTrayXPos = 0;
		NGTrayYPos = 0;
		ReelPos = 0;
	}

};

// CDlgAutoPosSetting 对话框

class CDlgAutoPosSetting : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgAutoPosSetting)

public:
	CDlgAutoPosSetting(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgAutoPosSetting();

	void AttachPosSetting(CAutoPosSetting*pAutoPosSetting) { m_pAutoPosSetting = pAutoPosSetting; };

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGAUTOPOSSETTING };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

	CAutoPosSetting* m_pAutoPosSetting;
public:
	virtual BOOL OnInitDialog();
	BOOL m_bProvideTrayPosEn;
	BOOL m_bOKTrayPosEn;
	BOOL m_bNGTrayPosEn;
	DWORD m_ProvideTrayXPos;
	DWORD m_ProvideTrayYPos;
	DWORD m_OKTrayXPos;
	DWORD m_OKTrayYPos;
	DWORD m_NGTrayXPos;
	DWORD m_NGTrayYPos;
	DWORD m_ReelPos;
	afx_msg void OnBnClickedOk();
};
