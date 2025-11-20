// DlgProduceInfo.cpp : 实现文件
//

#include "stdafx.h"
#include "Mes.h"
#include "DlgProduceInfo.h"
#include "afxdialogex.h"


// CDlgProduceInfo 对话框

IMPLEMENT_DYNAMIC(CDlgProduceInfo, CDialogEx)

CDlgProduceInfo::CDlgProduceInfo(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGPRODUCEINFO, pParent)
{

}

CDlgProduceInfo::~CDlgProduceInfo()
{
}

void CDlgProduceInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDITPRODUCEINFO, m_strInfo);
}


BEGIN_MESSAGE_MAP(CDlgProduceInfo, CDialogEx)
END_MESSAGE_MAP()


// CDlgProduceInfo 消息处理程序


BOOL CDlgProduceInfo::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
#define DLG_BKCOLOR (RGB(192,255,255))
	SetBackgroundColor(DLG_BKCOLOR);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CDlgProduceInfo::SetProduceInfo(CString strProduceInfo)
{
	m_strInfo = strProduceInfo;
	UpdateData(FALSE);
}
