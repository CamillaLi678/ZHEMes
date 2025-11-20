// DlgMESTest.cpp : 实现文件
//

#include "stdafx.h"
#include "Mes.h"
#include "DlgMESTest.h"
#include "afxdialogex.h"
#include "MesInterface.h"
#include "ComTool.h"
#include "ComFunc.h"

// CDlgMESTest 对话框

IMPLEMENT_DYNAMIC(CDlgMESTest, CDialogEx)

CDlgMESTest::CDlgMESTest(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGMESTEST, pParent)
	, m_pILog(NULL)
	, m_strResultPath(_T(""))
	, m_strStatusFile(_T(""))
	, m_strProgInfoFile(_T(""))
	, m_strWorkOrder(_T(""))
	, m_strMaterialID(_T(""))
	, m_strResult(_T(""))
{

}

CDlgMESTest::~CDlgMESTest()
{
}

void CDlgMESTest::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//DDX_Text(pDX, IDC_EDIT2, m_strStatusFile);
	DDX_Text(pDX, IDC_EDIT3, m_strProgInfoFile);
	DDX_Text(pDX, IDC_EDIT_TEST_WORKORDER, m_strWorkOrder);
	DDX_Text(pDX, IDC_EDIT_TEST_MATERIAL, m_strMaterialID);
	DDX_Text(pDX, IDC_EDIT_TEST_RESULT, m_strResultPath);
}


BEGIN_MESSAGE_MAP(CDlgMESTest, CDialogEx)
	ON_BN_CLICKED(IDC_BTNSELREPORT, &CDlgMESTest::OnBnClickedBtnselResult)
	ON_BN_CLICKED(IDC_BTNSENDREPORTTOMES, &CDlgMESTest::OnBnClickedBtnsendResultToMes)
	ON_BN_CLICKED(IDC_BTNSENDSTATUS2MES, &CDlgMESTest::OnBnClickedBtnGetMesRecord)
	ON_BN_CLICKED(IDC_BTNSENDPROGINFO2MES, &CDlgMESTest::OnBnClickedBtnsendproginfo2mes)
	ON_BN_CLICKED(IDC_BTNSELSTATUS, &CDlgMESTest::OnBnClickedBtnselstatus)
	ON_BN_CLICKED(IDC_BTNSEPROGINFO, &CDlgMESTest::OnBnClickedBtnseproginfo)
END_MESSAGE_MAP()


// CDlgMESTest 消息处理程序


void CDlgMESTest::OnBnClickedBtnselResult()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog Dlg(TRUE, NULL, NULL, OFN_PATHMUSTEXIST,"Result File(*.json) | *.json|Result File(*.txt) | *.txt||" , this);
	if (Dlg.DoModal() == IDOK) {
		m_strResultPath = Dlg.GetPathName();
	}
	else {
		m_strResultPath = "";
	}
	UpdateData(FALSE);
}


void CDlgMESTest::OnBnClickedBtnsendResultToMes()
{
	// TODO: 在此添加控件通知处理程序代码
	INT Ret = 0;
	BOOL RtnCall = FALSE;
	UpdateData(TRUE);
	CFile File;
	BYTE *pData = NULL;
	INT Size =0;
	CMesInterface &MesInterface = CMesInterface::getInstance();
	CString json;
	if (File.Open(m_strResultPath, CFile::modeRead | CFile::shareDenyNone) == FALSE) {
		MessageBox("打开报告文件失败,请确认文件路径!");
		return;
	}
	Size = (INT)File.GetLength();
	pData = new BYTE[Size+1];
	if (!pData) {
		goto __end;
	}
	memset(pData, 0, Size + 1);
	File.Read(pData, Size);

	json.Format("%s", (pData));
	if (m_Setting.nServerTest) {
		Ret = MesInterface.CommitProgramRet2ACMes(json);
	}
	else {
		Ret = MesInterface.CommitProgramRet2Mes(json);
	}

__end:
	if (File.m_hFile != CFile::hFileNull) {
		File.Close();
	}
	if (pData) {
		delete[] pData;
	}
	return;
}




BOOL CDlgMESTest::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetBackgroundColor((RGB(147, 220, 255)));

	m_strProgInfoFile.Format("%s\\MESTest\\programmer_info.json", GetCurrentPath());
	m_strResultPath.Format("%s\\MESTest\\result.json", GetCurrentPath());

	CString strSettingJsonPath;
	strSettingJsonPath.Format("%s\\Setting.json", GetCurrentPath());
	m_Setting.SetJsonPath(strSettingJsonPath);
	m_Setting.Load();

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CDlgMESTest::OnBnClickedBtnGetMesRecord()
{
	// TODO: 在此添加控件通知处理程序代码
	INT Ret = 0;
	BOOL RtnCall = FALSE;
	UpdateData(TRUE);
	MesInfo mesResult;
	CMesInterface &MesInterface = CMesInterface::getInstance();
	if (m_Setting.nServerTest){
		mesResult = MesInterface.GetACMesRecord(m_strWorkOrder, m_strMaterialID, "Program");
	}
	else {
		mesResult = MesInterface.GetMesRecord(m_strWorkOrder, m_strMaterialID, "Program");
	}
	
	
	if (mesResult.errCode == 0) {
		Ret = TRUE;
	}

	return;
}


void CDlgMESTest::OnBnClickedBtnsendproginfo2mes()
{
	// TODO: 在此添加控件通知处理程序代码
	INT Ret = 0;
	BOOL RtnCall = FALSE;
	UpdateData(TRUE);
	CFile File;
	BYTE *pData = NULL;
	INT Size = 0;
	CMesInterface &MesInterface = CMesInterface::getInstance();
	CString json;
	if (File.Open(m_strProgInfoFile, CFile::modeRead | CFile::shareDenyNone) == FALSE) {
		MessageBox("打开编程器信息文件失败,请确认文件路径!");
		return;
	}
	Size = (INT)File.GetLength();
	pData = new BYTE[Size + 1];
	if (!pData) {
		goto __end;
	}
	memset(pData, 0, Size + 1);
	File.Read(pData, Size);


	json.Format("%s", (pData));
	Ret = MesInterface.CommitProgramerInfo2Mes(json);


__end:
	if (File.m_hFile != CFile::hFileNull) {
		File.Close();
	}
	if (pData) {
		delete[] pData;
	}
	return;
}


void CDlgMESTest::OnBnClickedBtnselstatus()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog Dlg(TRUE, NULL, NULL, OFN_PATHMUSTEXIST, "Status File(*.json) | *.json||", this);
	if (Dlg.DoModal() == IDOK) {
		m_strStatusFile = Dlg.GetPathName();
	}
	else {
		m_strStatusFile = "";
	}
	UpdateData(FALSE);
}


void CDlgMESTest::OnBnClickedBtnseproginfo()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog Dlg(TRUE, NULL, NULL, OFN_PATHMUSTEXIST, "Programmers Info File(*.json) | *.json||", this);
	if (Dlg.DoModal() == IDOK) {
		m_strProgInfoFile = Dlg.GetPathName();
	}
	else {
		m_strProgInfoFile = "";
	}
	UpdateData(FALSE);
}

