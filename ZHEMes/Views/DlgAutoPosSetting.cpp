// DlgAutoPosSetting.cpp : 实现文件
//

#include "stdafx.h"
#include "Mes.h"
#include "DlgAutoPosSetting.h"
#include "afxdialogex.h"


// CDlgAutoPosSetting 对话框

IMPLEMENT_DYNAMIC(CDlgAutoPosSetting, CDialogEx)

CDlgAutoPosSetting::CDlgAutoPosSetting(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGAUTOPOSSETTING, pParent)
	, m_bProvideTrayPosEn(FALSE)
	, m_bOKTrayPosEn(FALSE)
	, m_bNGTrayPosEn(FALSE)
	, m_ProvideTrayXPos(0)
	, m_ProvideTrayYPos(0)
	, m_OKTrayXPos(0)
	, m_OKTrayYPos(0)
	, m_NGTrayXPos(0)
	, m_NGTrayYPos(0)
	, m_ReelPos(0)
{

}

CDlgAutoPosSetting::~CDlgAutoPosSetting()
{
}

void CDlgAutoPosSetting::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHKPROVIDETRAYPOS, m_bProvideTrayPosEn);
	DDX_Check(pDX, IDC_CHKNGTRAYPOS, m_bNGTrayPosEn);
	DDX_Check(pDX, IDC_CHKOKTRAYPOS, m_bOKTrayPosEn);
	DDX_Text(pDX, IDC_EDITPROVIDETRAYX, m_ProvideTrayXPos);
	DDX_Text(pDX, IDC_EDITPROVIDETRAYY, m_ProvideTrayYPos);
	DDX_Text(pDX, IDC_EDITOKTRAYX, m_OKTrayXPos);
	DDX_Text(pDX, IDC_EDITOKTRAYY, m_OKTrayYPos);
	DDX_Text(pDX, IDC_EDITNGTRAYX, m_NGTrayXPos);
	DDX_Text(pDX, IDC_EDITNGTRAYY, m_NGTrayYPos);
	DDX_Text(pDX, IDC_EDITREELPOS, m_ReelPos);
}


BEGIN_MESSAGE_MAP(CDlgAutoPosSetting, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgAutoPosSetting::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgAutoPosSetting 消息处理程序

#define DLG_BACKCOLOR (RGB(230,230,250))
BOOL CDlgAutoPosSetting::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetBackgroundColor(DLG_BACKCOLOR);
	// TODO:  在此添加额外的初始化
	m_bProvideTrayPosEn=m_pAutoPosSetting->bProvideTrayPosEn;
	m_bOKTrayPosEn= m_pAutoPosSetting->bOKTrayPosEn;
	m_bNGTrayPosEn = m_pAutoPosSetting->bNGTrayPosEn;
	m_ProvideTrayXPos= m_pAutoPosSetting->ProvideTrayXPos;
	m_ProvideTrayYPos = m_pAutoPosSetting->ProvideTrayYPos;
	m_OKTrayXPos = m_pAutoPosSetting->OKTrayXPos;
	m_OKTrayYPos = m_pAutoPosSetting->OKTrayYPos;;
	m_NGTrayXPos = m_pAutoPosSetting->NGTrayXPos;
	m_NGTrayYPos = m_pAutoPosSetting->OKTrayYPos;
	m_ReelPos = m_pAutoPosSetting->ReelPos;

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CDlgAutoPosSetting::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	m_pAutoPosSetting->bProvideTrayPosEn= m_bProvideTrayPosEn;
	m_pAutoPosSetting->bOKTrayPosEn= m_bOKTrayPosEn;
	m_pAutoPosSetting->bNGTrayPosEn= m_bNGTrayPosEn;
	m_pAutoPosSetting->ProvideTrayXPos= m_ProvideTrayXPos;
	m_pAutoPosSetting->ProvideTrayYPos= m_ProvideTrayYPos;
	m_pAutoPosSetting->OKTrayXPos= m_OKTrayXPos;
	m_pAutoPosSetting->OKTrayYPos= m_OKTrayYPos;
	m_pAutoPosSetting->NGTrayXPos = m_NGTrayXPos;
	m_pAutoPosSetting->NGTrayYPos = m_NGTrayYPos;
	m_pAutoPosSetting->ReelPos = m_ReelPos;
	CDialogEx::OnOK();
}
