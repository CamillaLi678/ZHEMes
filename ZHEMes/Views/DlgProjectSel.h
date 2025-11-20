#pragma once
#include "afxcmn.h"
#include "MesInterface.h"

// CDlgProjectSel 对话框

class CDlgProjectSel : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgProjectSel)

public:
	CDlgProjectSel(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgProjectSel();
	void addItems(std::vector<MesInfo> &items);
	MesInfo *getSelectedItem();
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SEL_PROJECT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_listProj;
	INT m_nSelectedItem;
	std::vector<MesInfo> m_listInfo;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};
