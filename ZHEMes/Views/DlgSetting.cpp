// DlgSetting.cpp : 实现文件
//

#include "stdafx.h"
#include "Mes.h"
#include "DlgSetting.h"
#include "afxdialogex.h"

#define DLG_BACKCOLOR (RGB(230,230,250))

// CDlgSetting 对话框

IMPLEMENT_DYNAMIC(CDlgSetting, CDialogEx)

CDlgSetting::CDlgSetting(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIGSETTING, pParent)
	, m_strReportFolder(_T(""))
	, m_nElectricInsertionCheck(0)
	, m_strWebService(_T(""))
	, m_strAutoTaskFileExt(_T(""))
	, m_strWorkStationID(_T(""))
	, m_strAutoTaskFolder(_T(""))
	, m_strUfsWriterExePath(_T(""))
{

}

CDlgSetting::~CDlgSetting()
{
}

void CDlgSetting::AttachData(CSetting*pSetting)
{
	m_pSetting = pSetting;
}

void CDlgSetting::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDITPROJSAVEFOLDER, m_strProjSaveFolder);
	DDX_Text(pDX, IDC_EDITSERVERPATH, m_strACServerFolder);
	DDX_Control(pDX, IDC_CMBCMDSEL, m_cmbExeCmd);
	DDX_Text(pDX, IDC_EDITREPORTFOLDER, m_strReportFolder);
	DDX_Control(pDX, IDC_CMB_WORKMODE, m_cmbWorkMode);
	DDX_Control(pDX, IDC_CMB_PROJECTMODE, m_cmbProjectMode);
	DDX_Check(pDX, IDC_CHKELECINSCHECK, m_nElectricInsertionCheck);
	DDX_Control(pDX, IDC_CMB_AUTO_MODE, m_cmbAutoModeComBox);
	DDX_Text(pDX, IDC_EDIT_STATION_ID, m_strWorkStationID);
	DDX_Text(pDX, IDC_EDIT_WEB_ADDR, m_strWebService);
	DDX_Text(pDX, IDC_EDIT_SEL_TSK, m_strAutoTaskFolder);
	DDX_Control(pDX, IDC_COMBO_AUTO_TYPE, m_comboAutomaticType);

	DDX_Text(pDX, IDC_EDITSERVERPATH_UFS_WRITER_PATH, m_strUfsWriterExePath);
}


BEGIN_MESSAGE_MAP(CDlgSetting, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgSetting::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BTNSELPROJDAVEFOLDER, &CDlgSetting::OnBnClickedBtnselprojdavefolder)
	ON_BN_CLICKED(IDC_BTNSELSEVERPATH, &CDlgSetting::OnBnClickedBtnselseverpath)
	ON_BN_CLICKED(IDC_BTNSELREPORTFOLDER, &CDlgSetting::OnBnClickedBtnselreportfolder)
	ON_BN_CLICKED(IDC_BTN_SEL_TSK_FOLDER, &CDlgSetting::OnBnClickedBtnselTskPath)
	ON_BN_CLICKED(IDC_BTNSELSEVERPATH_UFS_SEL_EXE_PATH, &CDlgSetting::OnBnClickedBtnselseverpathUfsSelExePath)
END_MESSAGE_MAP()

// CDlgSetting 消息处理程序
INT CDlgSetting::GetDirSelect(CString Title, CString&strDirPath)
{
	char szPath[MAX_PATH];     //存放选择的目录路径 
	ZeroMemory(szPath, sizeof(szPath));
	BROWSEINFO bi;
	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = szPath;
	bi.lpszTitle = Title.GetBuffer();
	bi.ulFlags = 0;
	bi.lpfn = NULL;
	bi.lParam = 0;
	bi.iImage = 0;
	//弹出选择目录对话框
	LPITEMIDLIST lp = SHBrowseForFolder(&bi);
	if (lp && SHGetPathFromIDList(lp, szPath)) {
		strDirPath.Format("%s", szPath);
		return 0;
	}
	else {
		//MessageBox("无效的目录，请重新选择",NULL,MB_OK|MB_ICONERROR);
		return -1;
	}
}

BOOL CDlgSetting::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetBackgroundColor(DLG_BACKCOLOR);


	CString strIniLangFile;
	strIniLangFile.Format("%s\\Language.ini", GetCurrentPath());
	CHAR TmpBuf[MAX_PATH];
	memset(TmpBuf, 0, MAX_PATH);
	GetPrivateProfileString("Language", "CurLang", "English", TmpBuf, MAX_PATH, strIniLangFile);
	strLang.Format("%s", TmpBuf);
	if (strLang.CompareNoCase("ChineseA") == 0) {
		m_cmbProjectMode.AddString("自动合成");
		m_cmbProjectMode.AddString("手动合成");
		m_cmbAutoModeComBox.AddString("手动模式");
		m_cmbAutoModeComBox.AddString("自动模式");
		m_cmbWorkMode.AddString("MES对接模式");
		m_cmbWorkMode.AddString("本地模式");
	}
	else
	{
		m_cmbProjectMode.AddString("Automatic compositing");
		m_cmbProjectMode.AddString("Manual synthesis");
		m_cmbAutoModeComBox.AddString("Manual mode");
		m_cmbAutoModeComBox.AddString("Auto Model");
		m_cmbWorkMode.AddString("MES API Model");
		m_cmbWorkMode.AddString("Local Model");
	}

	m_cmbExeCmd.AddString("Program");
	m_cmbExeCmd.AddString("Verify");
	m_cmbExeCmd.AddString("Erase");

	m_comboAutomaticType.AddString("IPS5000");
	m_comboAutomaticType.AddString("IPS5200");
	m_comboAutomaticType.AddString("IPS3000");
	m_AutomaticTypeMap["IPS5000"] = 0;
	m_AutomaticTypeMap["IPS5200"] = 1;
	m_AutomaticTypeMap["IPS3000"] = 2;


	m_strProjSaveFolder=m_pSetting->strProjSaveFolder;
	m_strACServerFolder=m_pSetting->strACServerFolder;
	m_strReportFolder = m_pSetting->strReportFolder;
	m_nElectricInsertionCheck = m_pSetting->nElectricInsertCheck;

	m_vExecCmd.push_back("Program");
	m_vExecCmd.push_back("Verify");
	m_vExecCmd.push_back("Erase");

	m_vProjectMode.push_back("AutoCreate");
	m_vProjectMode.push_back("ManualCreate");

	m_vMesWorkMode.push_back("Enable");
	m_vMesWorkMode.push_back("Disable");

	m_vMesAutoMode.push_back("Manual");
	m_vMesAutoMode.push_back("Auto");

	m_ExecCmdMap["Program"] = 0;
	m_ExecCmdMap["Verify"] = 1;
	m_ExecCmdMap["Erase"] = 2;

	m_WorkModeMap["Enable"] = 0;
	m_WorkModeMap["Disable"] = 1;

	m_ProjectModeMap["AutoCreate"] = 0;
	m_ProjectModeMap["ManualCreate"] = 1;

	m_AutoModeMap["Manual"] = 0;
	m_AutoModeMap["Auto"] = 1;

	m_strWordMode = m_pSetting->strMesWordMode;
	m_strProjectMode = m_pSetting->strProjectMode;
	m_strAutoMode = m_pSetting->strAutoMode;
	m_strCurExec = m_pSetting->strCurExec;
	m_strAutomaticType = m_pSetting->strAutomaticType;

	m_cmbWorkMode.SetCurSel(m_WorkModeMap[m_strWordMode]);
	m_cmbProjectMode.SetCurSel(m_ProjectModeMap[m_strProjectMode]);
	m_cmbExeCmd.SetCurSel(m_ExecCmdMap[m_strCurExec]);

	m_cmbAutoModeComBox.SetCurSel(m_AutoModeMap[m_strAutoMode]); //default is auto
	m_comboAutomaticType.SetCurSel(m_AutomaticTypeMap[m_strAutomaticType]); //default is IPS5000


	m_strUfsWriterExePath = m_pSetting->strUfsWriterFolder;
	m_strWebService = m_pSetting->strWebServiceInterface;
	m_strAutoTaskFileExt = m_pSetting->strAutoTaskFileExt;
	m_strWorkStationID = m_pSetting->strWorkStationID;
	m_strAutoTaskFolder = m_pSetting->strAutoTaskFolder;

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


BOOL CDlgSetting::SaveSettings()
{
	int CMD[3] = { 3 };

	BOOL Ret = TRUE;
	UpdateData(TRUE);
	m_cmbExeCmd.GetWindowTextA(m_strCurExec);

	if (m_strACServerFolder == "") {
		if (strLang.CompareNoCase("ChineseA") == 0) {
			MessageBox("MultiAprog.exe所在文件夹需要指定");
		}
		else {
			MessageBox("MultiAprog.exe You need to specify the folder you are in ");
		}

		Ret = FALSE; goto __end;
	}

	if (m_strProjSaveFolder == "") {
		m_strProjSaveFolder = "./ProjSave";
	}

	m_strWordMode = m_vMesWorkMode[m_cmbWorkMode.GetCurSel()];
	m_strProjectMode = m_vProjectMode[m_cmbProjectMode.GetCurSel()];

	m_strAutoMode = m_vMesAutoMode[m_cmbAutoModeComBox.GetCurSel()];
	m_comboAutomaticType.GetWindowText(m_strAutomaticType);

	m_pSetting->strProjSaveFolder = m_strProjSaveFolder;
	m_pSetting->strACServerFolder = m_strACServerFolder;
	m_pSetting->strCurExec = m_strCurExec;
	m_pSetting->strReportFolder = m_strReportFolder;
	m_pSetting->nElectricInsertCheck = m_nElectricInsertionCheck;
	m_pSetting->nAutoTaskLoadCmd = CMD[0];
	m_pSetting->strMesWordMode = m_strWordMode;
	m_pSetting->strProjectMode =  m_strProjectMode;
	m_pSetting->strAutoMode = m_strAutoMode;
	m_pSetting->strWorkStationID = m_strWorkStationID;
	m_pSetting->strWebServiceInterface = m_strWebService;
	m_pSetting->strAutoTaskFileExt = m_strAutoTaskFileExt;
	m_pSetting->strAutoTaskFolder = m_strAutoTaskFolder;
	m_pSetting->strAutomaticType = m_strAutomaticType;
	m_pSetting->strUfsWriterFolder = m_strUfsWriterExePath;

	UpdateData(FALSE);

	Ret=m_pSetting->Save();
	if (Ret == FALSE) {
		MessageBox(m_pSetting->GetErrMsg(), NULL, MB_ICONERROR | MB_OK);
	}

__end:
	return Ret;
}


void CDlgSetting::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	if (SaveSettings() == FALSE) {
		m_pIStatus->PrintStatus(0, "设置操作失败");
		return;
	}
	else {
		m_pIStatus->PrintStatus(0, "设置操作成功");
	}

	m_pSetting->Save();
	PostMessage(MSG_UPDATA_SETTING_CONFIG, 0, 0);

	CDialogEx::OnOK();
}

//void CDlgSetting::OnBnClickedBtnselfadllfile()
//{
//	// TODO: 在此添加控件通知处理程序代码
//	GetFileOpenPath("Library(*.dll)|*.dll||", m_strFADllPath);
//	UpdateData(FALSE);
//}



void CDlgSetting::GetFileOpenPath(CString strExts,CString&strFileOpen)
{
	CFileDialog Dlg(TRUE, NULL, NULL, OFN_PATHMUSTEXIST, strExts, this);
	if (Dlg.DoModal() == IDOK) {
		strFileOpen = Dlg.GetPathName();
	}
	else {
		strFileOpen = "";
	}
}


void CDlgSetting::OnBnClickedBtnselprojdavefolder()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDirSelect("请选择工程保存文件夹", m_strProjSaveFolder);
	UpdateData(FALSE);
}


void CDlgSetting::OnBnClickedBtnselseverpath()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDirSelect("请选择MultiAprog.exe所在文件夹", m_strACServerFolder);
	UpdateData(FALSE);
}


void CDlgSetting::OnBnClickedBtnselreportfolder()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDirSelect("请选择生产报告保存文件夹", m_strReportFolder);
	UpdateData(FALSE);
}


void CDlgSetting::OnBnClickedBtnselTskPath()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDirSelect("请选择生产报告保存文件夹", m_strAutoTaskFolder);
	UpdateData(FALSE);
}


void CDlgSetting::OnBnClickedBtnselseverpathUfsSelExePath()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDirSelect("请选择UfsWriter.exe所在文件夹", m_strUfsWriterExePath);
	UpdateData(FALSE);
}
