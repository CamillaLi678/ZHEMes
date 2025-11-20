// DlgProjectSel.cpp : 实现文件
//

#include "stdafx.h"
#include "DlgProjectSel.h"
#include "afxdialogex.h"
#include "Resource.h"
#include <vector>

// CDlgProjectSel 对话框

IMPLEMENT_DYNAMIC(CDlgProjectSel, CDialogEx)

CDlgProjectSel::CDlgProjectSel(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_SEL_PROJECT, pParent)
{
	m_nSelectedItem = -1;
}

CDlgProjectSel::~CDlgProjectSel()
{
}

void CDlgProjectSel::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SEL_PROJ, m_listProj);
}


BEGIN_MESSAGE_MAP(CDlgProjectSel, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgProjectSel::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgProjectSel 消息处理程序

#define DLG_BACKCOLOR (RGB(230,230,250))
BOOL CDlgProjectSel::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetBackgroundColor(DLG_BACKCOLOR);

	// Ask Mfc to create/insert a column
	m_listProj.InsertColumn(0,"订单名", LVCFMT_CENTER, 100);
	m_listProj.InsertColumn(1, "组件名", LVCFMT_CENTER, 150);
	m_listProj.InsertColumn(2, "订单数量", LVCFMT_CENTER, 100);
	m_listProj.InsertColumn(3, "工程文件名", LVCFMT_CENTER, 200);
	m_listProj.InsertColumn(4, "自动化文件名", LVCFMT_CENTER, 200);
	m_listProj.InsertColumn(5, "版本", LVCFMT_CENTER, 100);

	LONG lStyle;
	lStyle = GetWindowLong(m_listProj.m_hWnd, GWL_STYLE);
	lStyle &= ~LVS_TYPEMASK;
	lStyle |= LVS_REPORT;
	SetWindowLong(m_listProj.m_hWnd, GWL_STYLE, lStyle);
	DWORD dwStyle = m_listProj.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT; 
	dwStyle |= LVS_EX_GRIDLINES;
	m_listProj.SetExtendedStyle(dwStyle);

	for (size_t i = 0; i < m_listInfo.size(); ++i){
		MesInfo oneMes = m_listInfo[i];
		int nItem = m_listProj.GetItemCount();	//获得列表行数
		m_listProj.InsertItem(nItem, oneMes.workOrder);
		m_listProj.SetItemText(nItem, 1, oneMes.materialID);
		CString ICNum;
		ICNum.Format("%d", oneMes.workOrderICNum);
		m_listProj.SetItemText(nItem, 2, ICNum);
		m_listProj.SetItemText(nItem, 3, oneMes.projPath);
		m_listProj.SetItemText(nItem, 4, oneMes.autoTaskFilePath);
		m_listProj.SetItemText(nItem, 5, oneMes.projVersion);
	}


	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CDlgProjectSel::addItems(std::vector<MesInfo> &items) {
	m_listInfo = items;
}

MesInfo *CDlgProjectSel::getSelectedItem() {
	if (m_nSelectedItem != -1) {
		return &m_listInfo[m_nSelectedItem];
	}
	else {
		return NULL;
	}
}

void CDlgProjectSel::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	POSITION pos = m_listProj.GetFirstSelectedItemPosition();
	int selected = -1;
	if (pos != NULL) {
		int nItem = m_listProj.GetNextSelectedItem(pos);
		m_nSelectedItem = nItem;
	}
	else {
		m_nSelectedItem = -1;
	}

	CDialogEx::OnOK();
}
