// DlgeMMCMIDScan.cpp : 实现文件
//

#include "stdafx.h"
#include "Mes.h"
#include "DlgCodeScan.h"
#include "afxdialogex.h"


// CDlgCodeScan 对话框

IMPLEMENT_DYNAMIC(CDlgCodeScan, CDialogEx)

CDlgCodeScan::CDlgCodeScan(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGMIDSCAN, pParent)
	, m_strBurnBeforeCode(_T(""))
	, m_strMuPianInfo1(_T(""))
	, m_strMuPianInfo2(_T(""))
	, m_strBurnAfterCode(_T(""))
{

}

CDlgCodeScan::~CDlgCodeScan()
{
}

void CDlgCodeScan::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDITEMMCMID, m_strBurnBeforeCode);
	DDX_Text(pDX, IDC_EDITEMMCMID2, m_strMuPianInfo1);
	DDX_Text(pDX, IDC_EDITEMMCMID3, m_strMuPianInfo2);
	DDX_Text(pDX, IDC_EDITEMMCMID4, m_strBurnAfterCode);
}

BOOL CDlgCodeScan::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	if (!m_strTitle.IsEmpty()) {
		SetWindowText(m_strTitle);
	}
	if (!m_strInputPrompt.IsEmpty()) {
		CStatic* pCStaticInput = (CStatic *)GetDlgItem(IDC_STATIC);
		pCStaticInput->SetWindowTextA(m_strInputPrompt);
	}

	((CEdit *)GetDlgItem(IDC_EDITEMMCMID))->SetLimitText(28);
	((CEdit *)GetDlgItem(IDC_EDITEMMCMID4))->SetLimitText(28);

	GetDlgItem(IDC_EDITEMMCMID)->SetFocus();

	return FALSE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


BEGIN_MESSAGE_MAP(CDlgCodeScan, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgCodeScan::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgCodeScan 消息处理程序


void CDlgCodeScan::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	CDialogEx::OnOK();
}