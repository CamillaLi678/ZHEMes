// DlgAutoDataRecord.cpp : 实现文件
//

#include "stdafx.h"
#include "Mes.h"
#include "DlgAutoDataRecord.h"
#include "afxdialogex.h"
#include "cJSON.h"
#include "ComTool.h"

#define DLG_BACKCOLOR (RGB(230,230,250))
// CDlgAutoDataRecord 对话框

IMPLEMENT_DYNAMIC(CDlgAutoDataRecord, CDialogEx)

CDlgAutoDataRecord::CDlgAutoDataRecord(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGAUTODATA, pParent)
	, m_strChipName(_T(""))
	, m_strAdapterName(_T(""))
	, m_bTapeEn(FALSE)
	, m_PosX(0)
	, m_PosY(0)
	, m_strAdapterData(_T(""))
	, m_strTrayData(_T(""))
	, m_strTapeData(_T(""))
{

}

CDlgAutoDataRecord::~CDlgAutoDataRecord()
{
}

void CDlgAutoDataRecord::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CMBMACHID, m_cmbMachineID);
	DDX_Text(pDX, IDC_EDITCHIPNAME, m_strChipName);
	DDX_Text(pDX, IDC_EDITADAPTERNAME, m_strAdapterName);
	DDX_Check(pDX, IDC_CHKTAPEEN, m_bTapeEn);
	DDX_Text(pDX, IDC_EDITXPOS, m_PosX);
	DDX_Text(pDX, IDC_EDITYPOS, m_PosY);
	DDX_Text(pDX, IDC_EDITADAPTERDATA, m_strAdapterData);
	DDX_Text(pDX, IDC_EDITTRAYDATA, m_strTrayData);
	DDX_Text(pDX, IDC_EDITTAPEDATA, m_strTapeData);
}


BEGIN_MESSAGE_MAP(CDlgAutoDataRecord, CDialogEx)
	ON_BN_CLICKED(IDC_BTNSAVEAUTODATA, &CDlgAutoDataRecord::OnBnClickedBtnsaveautodata)
	ON_BN_CLICKED(IDC_BTNLOADAUTODATA, &CDlgAutoDataRecord::OnBnClickedBtnloadautodata)
	ON_BN_CLICKED(IDC_BTNSELADAPTERDATA, &CDlgAutoDataRecord::OnBnClickedBtnseladapterdata)
	ON_BN_CLICKED(IDC_BTNSELTRAYDATA, &CDlgAutoDataRecord::OnBnClickedBtnseltraydata)
	ON_BN_CLICKED(IDC_BTNSELTAPEDATA, &CDlgAutoDataRecord::OnBnClickedBtnseltapedata)
END_MESSAGE_MAP()


// CDlgAutoDataRecord 消息处理程序


BOOL CDlgAutoDataRecord::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetBackgroundColor(DLG_BACKCOLOR);

	m_cmbMachineID.AddString("IPSXXX");
	m_cmbMachineID.AddString("PHA2000");
	m_cmbMachineID.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

CString CDlgAutoDataRecord::GetAutoTaskFolder()
{
	CString strAbsPath;
	if (m_pSetting->strAutoTaskFolder.GetAt(0) == '.') {///是相对路径
		CString FolderName = ::GetFolderNameFromRelative(m_pSetting->strAutoTaskFolder);
		strAbsPath.Format("%s\\%s", GetCurrentPath(), FolderName);
	}
	else {
		strAbsPath = m_pSetting->strAutoTaskFolder;
	}
	return strAbsPath;
}

void CDlgAutoDataRecord::GetFileSavePath(CString strExts, CString&strFileOpen,CString strExt)
{
	CFileDialog Dlg(FALSE, NULL, NULL, OFN_PATHMUSTEXIST, strExts, this);
	CString strInitPath="";
	if (m_pSetting) {
		strInitPath = GetAutoTaskFolder();
	}
	Dlg.m_ofn.lpstrInitialDir = strInitPath.GetBuffer();
	if (Dlg.DoModal() == IDOK) {
		CString FileExt;
		strFileOpen = Dlg.GetPathName();
		FileExt = Dlg.GetFileExt();
		if (FileExt == "") {
			strFileOpen += strExt;
		}
	}
	else {
		strFileOpen = "";
	}
}

void CDlgAutoDataRecord::GetFileOpenPath(CString strExts, CString&strFileOpen)
{
	CFileDialog Dlg(TRUE, NULL, NULL, OFN_PATHMUSTEXIST, strExts, this);
	CString strInitPath = "";
	if (m_pSetting) {
		strInitPath = GetAutoTaskFolder();
	}
	Dlg.m_ofn.lpstrInitialDir = strInitPath.GetBuffer();
	if (Dlg.DoModal() == IDOK) {
		strFileOpen = Dlg.GetPathName();
	}
	else {
		strFileOpen = "";
	}
}


void CDlgAutoDataRecord::OnBnClickedBtnsaveautodata()
{
	// TODO: 在此添加控件通知处理程序代码
	INT Ret = 0;
	UpdateData(TRUE);
	CString strMachineType;
	CString strSavePath;
	CString m_strErrMsg;
	char* strTmp = NULL;
	m_cmbMachineID.GetWindowText(strMachineType);

	GetFileSavePath("AutoTaskData(*.htask)|*.htask||", strSavePath,".htask");
	if (strSavePath == "") {
		return;
	}

	cJSON *Root = cJSON_CreateObject();

	cJSON_AddStringToObject(Root, "AutoMachineType", strMachineType.GetBuffer());
	cJSON_AddStringToObject(Root, "ChipName",(LPSTR)(LPCSTR)m_strChipName);
	cJSON_AddStringToObject(Root, "AdapterName", (LPSTR)(LPCSTR)m_strAdapterName);
	cJSON_AddNumberToObject(Root, "TapeIn", m_bTapeEn);
	cJSON_AddNumberToObject(Root, "XPos",m_PosX);
	cJSON_AddNumberToObject(Root, "YPos", m_PosY);
	cJSON_AddStringToObject(Root, "AdapterData", (LPSTR)(LPCSTR)m_strAdapterData);
	cJSON_AddStringToObject(Root, "TryData", (LPSTR)(LPCSTR)m_strTrayData);
	cJSON_AddStringToObject(Root, "TapeData", (LPSTR)(LPCSTR)m_strTapeData);
	cJSON_AddNumberToObject(Root, "Lot",999999999);

	strTmp = cJSON_Print(Root);


	CStdioFile StdFile;
	if (StdFile.Open(strSavePath, CFile::modeWrite | CFile::modeCreate, NULL) == FALSE) {
		m_strErrMsg.Format("保存设置错误: 打开配置文件出错，路径:%s", strSavePath);
		Ret = -1; goto __end;
	}

	StdFile.WriteString(strTmp);

	StdFile.Flush();
	StdFile.Close();

	MessageBox("保存设置成功");

__end:
	cJSON_Delete(Root);
	if (strTmp) {
		cJSON_free(strTmp);
	}
	if (Ret != 0) {
		MessageBox(m_strErrMsg,NULL,MB_OK|MB_ICONERROR);
	}
	return;
}


void CDlgAutoDataRecord::OnBnClickedBtnloadautodata()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL Ret = TRUE;
	CFile File;
	INT FileLen;
	cJSON*Root = NULL;
	BYTE *pTmpData = NULL;
	CString strLoadPath;
	CString m_strErrMsg = "";
	CString strMachineType;
	GetFileOpenPath("AutoTaskData(*.htask)|*.htask||", strLoadPath);
	if (strLoadPath == "") {
		return;
	}

	if (File.Open(strLoadPath, CFile::modeRead, NULL) == FALSE) {
		m_strErrMsg.Format("加载设置错误: 打开Json配置文件出错，路径:%s", strLoadPath);
		Ret = FALSE; goto __end;
	}

	FileLen = (INT)File.GetLength();
	pTmpData = new BYTE[FileLen+1];
	if (!pTmpData) {
		m_strErrMsg.Format("加载设置错误: 分配内存错误，长度:%d Bytes", FileLen);
		Ret = FALSE; goto __end;
	}
	memset(pTmpData, 0, FileLen + 1);
	File.Read(pTmpData, FileLen);

	Root = cJSON_Parse((char*)pTmpData);
	if (Root == NULL) {
		m_strErrMsg.Format("加载设置错误: 解析Json字符串错误");
		Ret = FALSE; goto __end;
	}

	strMachineType.Format("%s", cJSON_GetObjectItem(Root, "AutoMachineType")->valuestring);
	m_strChipName.Format("%s", cJSON_GetObjectItem(Root, "ChipName")->valuestring);
	m_strAdapterName.Format("%s", cJSON_GetObjectItem(Root, "AdapterName")->valuestring);
	m_bTapeEn = cJSON_GetObjectItem(Root, "TapeIn")->valueint;
	m_PosX = cJSON_GetObjectItem(Root, "XPos")->valueint;
	m_PosY = cJSON_GetObjectItem(Root, "YPos")->valueint;
	m_strAdapterData.Format("%s", cJSON_GetObjectItem(Root, "AdapterData")->valuestring);
	m_strTapeData.Format("%s", cJSON_GetObjectItem(Root, "TapeData")->valuestring);
	m_strTrayData.Format("%s", cJSON_GetObjectItem(Root, "TryData")->valuestring);

	m_cmbMachineID.SelectString(-1, strMachineType);

	UpdateData(FALSE);

__end:
	if (Root) {
		cJSON_Delete(Root);
	}
	if (pTmpData) {
		delete[] pTmpData;
	}
	return ;
}


void CDlgAutoDataRecord::OnBnClickedBtnseladapterdata()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	CFileDialog Dlg(TRUE, NULL, NULL, OFN_PATHMUSTEXIST, "Adapter Data(*.Adp)|*.Adp|All Files(*.*)|*.*||", this);
	if (Dlg.DoModal() == IDOK) {
		m_strAdapterData = Dlg.GetFileTitle();
	}
	else {
		m_strAdapterData = "";
	}
	UpdateData(FALSE);
}


void CDlgAutoDataRecord::OnBnClickedBtnseltraydata()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	CFileDialog Dlg(TRUE, NULL, NULL, OFN_PATHMUSTEXIST, "Tray Data(*.Try)|*.Try|All Files(*.*)|*.*||", this);
	if (Dlg.DoModal() == IDOK) {
		m_strTrayData = Dlg.GetFileTitle();
	}
	else {
		m_strTrayData = "";
	}
	UpdateData(FALSE);
}


void CDlgAutoDataRecord::OnBnClickedBtnseltapedata()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	CFileDialog Dlg(TRUE, NULL, NULL, OFN_PATHMUSTEXIST, "Tape Data(*.Rel)|*.Rel|All Files(*.*)|*.*||", this);
	if (Dlg.DoModal() == IDOK) {
		m_strTapeData = Dlg.GetFileTitle();
	}
	else {
		m_strTapeData = "";
	}
	UpdateData(FALSE);
}
