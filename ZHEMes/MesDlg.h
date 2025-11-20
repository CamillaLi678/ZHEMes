
// MesDlg.h : 头文件
//

#pragma once

#include "LogEdit.h"
#include "Setting.h"
#include "IStatus.h"
#include "IMesAccess.h"
#include "OperatorData.h"
#include "MainData.h"
#include "ExcelDB.h"
#include "ProgRecord.h"
#include "DlgProduceInfo.h"
#include "StdMesApi.h"
#include "WorkThread.h"
#include "afxlistctrl.h"
#include <list>
#include "MesSyslog.h"
#include "BaseModuleProgram.h"
#include "NormalProgram.h"
//#include "DlgConfigDownLoad.h"
//#include "CGlobalHead.h"
#include "DlgAutoPosSetting.h"
#include "ModuleProgram.h"

#define USE_TCP_SYSLOG 1
#ifdef USE_TCP_SYSLOG
#include "TcpSyslog.h"
#endif

#include "HandleScan.h"
#include "afxcmn.h"

typedef void(_stdcall *FnAC_PrintData)();

//typedef struct {
//	int TotalCnt; ///当前所有编程器生产总个数
//	int PassCnt;  ///当前所有编程器成功总个数
//	int FailCnt;  ///当前所有编程器失败总个数
//}tYieldSites;
class CMesInterface;
typedef struct tagFtpPara
{
	CString strftpHost;
	CString strftpPath;
	CString strfileName;
	CString strftpUserName;
	CString strftpPassword;
}tFtpPara;

typedef struct tagGetSktMap
{
	INT  SiteGroup;
	INT  SocketMap;
}tGetSocketMap;

//typedef struct stdMesMessage
//{
//	std::string msg;
//	std::string json;
//}tStdMesMessage;

class MyStatus :public IStatus
{
public:
	void AttachStatusBar(CWnd*pStatusBar) { m_pStatusBar = pStatusBar; }
	//void AttachMainDlg(CDl)
	void PrintStatus(INT Level, CString strMsg);
private:
	CWnd* m_pStatusBar;
};

// CMesDlg 对话框
class CMesDlg : public CDialogEx
{
	// 构造
public:
	CMesDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MES_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

public:
	int HandleMsg(char*Msg, char*MsgData);
	// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnAutotaskdata();
	DECLARE_MESSAGE_MAP()

	BOOL m_bNeedLogin;
	LRESULT ShowLoginWin(WPARAM, LPARAM);

	INT UploadProgramRet2Mes(CString strLastJson, UINT TotalCnt, UINT PassCnt, UINT FailCnt);
	INT CommitTaskIngfo(CString timeEnd, CString timeRun, CString timeStop);
	// For syslog parser
	BOOL parseAutoAlarmMsg(void * rootParser);
	BOOL parseLotStartMsg(void * rootParser);
	BOOL parseOpecallMsg(void * rootParser);
	BOOL parseLotEndMsg(void * rootParser);
	BOOL parseProductStsSetMsg(void * rootParser);
	void convertTimeFormat(CString oldTime, CString &newTime);
	INT CommitAlarmInfo(CString alarmCode, CString alarmMsg, CString alarmcStartTime, CString alarmKillTime, int alarmFlag);

	BOOL GetMesRecord();
	BOOL ScanFiles(CString& dir, CString strExtName, CString strPrefix, std::vector<CString> &v_Files);

	BOOL GetExcelRecord();
	void UpdateProduceWinPos();
	BOOL UpdateCtrlsValueFromRecord();
	CString GetAutoTaskFolder();
	INT ThreadParserHandler(MSG msg, void *Para);
	INT ThreadSendStopCmdHandler(MSG msg, void *Para);
	BOOL GetEMMCMIDRecord();

	void GetProjectPathFromAutoCreateMode(CString& strPath);

	BOOL UpdateRecordForProduce();
	CString GetAbsProjSaveFolder();
	CString GetProjSaveAbsPath();
	CString GetTemplateAbsPath();
	CString GetProgFileAbsSettingFolder();///获取烧录档案保存路径绝对值
	INT GetProgFileAbsPath(CString&strProgFileAbsPath);///根据策略获取档案路径

	void SetCompentVisible(BOOL bMesEnable);
	///ProgCtrl.dll
	INT DoParseTask();
	INT DoSendStopCmdTask();

	INT StartService();
	BOOL GetPrjCheckSumFromName(CString strPath, CString& strPrjCheckSum);
	INT LoadSNC();
	BOOL ChangeCtrlsEnAccrodOpLevel(INT OpLevel);

	void WritePrinterJsonFile(CString strModKey, CString strModValue);
	void LoadPrintDll();
	void ClosePrintDll();
	INT SetElectricInsertionCheck();

	void SetAccountUI();//
	BOOL m_bGetCmd4Success;
	BOOL m_bGetMissionResult;

	bool CheckSelComboCom();

	std::map<CString, tBurnStatus> GetSiteSn();
	void ComMap2UI();
	void SplitComMap(CString strComMap);
	INT GetComMap();
	INT SetComMap();
	INT SetComMapToJson(CString strComMap);
	INT UpdateProgramResult2UI(UINT PassCnt, UINT FailCnt, UINT TotalCnt);

private:
	//模组烧录
	void SetModuleCompoentEn(BOOL bEn);
	INT DoProductionCheck();

	BaseModuleProgram *pModuleProgram;
	NormalProgram *pNormalProgram;

private:
	CProgramResultData m_ProgramResult;
	BOOL m_bMesTcpSyslog;
	BOOL m_bNeedCheckQuit;
	CSetting m_Setting;
	CAutoPosSetting m_AutoPosSetting;
	CMainData m_MainData;
	COperatorData m_OperatorData;

	CExcelDB m_ExcelDB;
	MyStatus m_MyStatus;
	IStatus*m_pIStatus;
	CString m_strProgramFromMes;

	CProgRecord m_ProgRecord;
	CDlgProduceInfo m_DlgProduceInfo;
	CStdMesApi m_StdMes;  ///昂科MES对接接口

	CWorkThread m_Worker;
	CWorkThread m_StopWork;

	// For syslog parser
	CWorkThread m_ParseWorker;
	BOOL mParseWorkerExit;
	std::list<std::string> mRecvMsgList;
	CMutex m_MsgListMutex;
	std::list<tStdMesMessage> mRecvProgramMsgList;
	CMutex m_ProgramMsgListMutex;

	COLORREF m_StatusColor;

	bool IsMesMode();
	bool IsAutoMode();
	bool IsAutoCreateProject();

	volatile BOOL m_bProgRecordReady;///MES数据是否Ready
	BOOL m_bProgGetBurnQtyReady;
	volatile BOOL m_bQuit;
	volatile BOOL m_bTaskDoing;
	volatile tYieldSites m_YieldSites;
	CMutex m_YieldSitesMutex;
protected:
	afx_msg void OnSetting();
	CString GetExcelPath();
	virtual void OnCancel();
	afx_msg void OnBnClickedBtnGetmesrecord();
	afx_msg void OnBnClickedBtntestmes();
	INT HandleSetPrintResult(const char * MsgData);
	INT HandleSetPrintContent(const char * MsgData);
	BOOL GetCtrlsValue();
	void InitCtrlsValue();

	INT WaitJobDone(CStdMesApi* pStdMesApi);

	wchar_t* AnsiToUnicode(const char* szStr);
	char* UnicodeToAnsi(const wchar_t* szStr);

public:
	CLogEdit m_LogRichEdit;
	ILog *m_pILog;

	CHandleScan* m_HandleScan;
	CString m_strCacheSavePath;

	FnAC_PrintData m_pPrintFn;
	HINSTANCE m_hPrintLib;
	// 界面上的工单号
	CString m_strWorkOrder;
	// 芯片料号
	CString m_strMaterialID;
	
	// 芯片批号 (MES返回或用户输入)
	CString m_strBatNo;
	// 机台编号
	CString m_strStationId;

	afx_msg void OnBnClickedBtnshowrelativeinfo();
	afx_msg void OnMove(int x, int y);
	// 工单生产IC数
	LONG m_WorkOrderICNum;
	LONG m_WorkOrderCompletedICNum;
	// 实际生产期望数
	LONG m_ExpectICNum;

	CString m_strSNCPath;
	CString m_strRealChecksum; //工程文件实际校验值
	CString m_strProjPath;  //工程路径
	CString m_strProgramPath;   //烧录档案路径
	CString m_strTemplateFilePath;  //模板路径
	CString m_strProjChecksum;  //工程校验值(期望值)
	CString m_strProgFileChecksum; //烧录档案的校验值
	CString m_strAutoTaskFilePath;//自动化数据文件的路径

	CString m_strSwap;

	INT GetDirSelect(CString Title, CString&strDirPath);
	void GetDirFullPath(CString strExts, CString&strFileOpen);

	CString GetAbsAutoTaskDir();

	bool DoCompareCheckSum();
	void UpdataDestDataForProduce();

	static std::wstring MultiByteToWideChar(const char* pszMultiByte, UINT uCodePage = CP_ACP);
	static std::string  WideCharToMultiByte(const wchar_t* pwszMultiByte, UINT uCodePage = CP_ACP);

	afx_msg LRESULT  OnUpdataSettingConfig(WPARAM, LPARAM);

	//模组消息处理函数
	afx_msg LRESULT OnQueryComSiteFinished(WPARAM w, LPARAM l);
	afx_msg LRESULT OnQueryComAllFinished(WPARAM w, LPARAM l);
	afx_msg LRESULT OnUpdateSiteName(WPARAM w, LPARAM l);
	afx_msg LRESULT OnUpdateProgramResult(WPARAM w, LPARAM l);
	afx_msg LRESULT OnProgramFinish(WPARAM w, LPARAM l);
	afx_msg LRESULT OnUpdateAutoProgramCount(WPARAM w, LPARAM l);
	afx_msg LRESULT OnUpdateProgramCount(WPARAM w, LPARAM l);

	afx_msg LRESULT OnUploadProgramRet2Mes(WPARAM w, LPARAM l);

	afx_msg void OnBnClickedBtnSelprogrampath();
	afx_msg void OnBnClickedBtnSelautotaskfilepath();
	afx_msg void OnBnClickedBtnSeltemplatepath();
	afx_msg void OnBnClickedBtnSelsncpath();
	afx_msg void OnBnClickedBtnSelprojpath();
	afx_msg void OnAbout();

	afx_msg void OnBnClickedBtnStartproduce();
	afx_msg void OnBnClickedBtnCancelproduce();
	//CString m_strProjChecksumExcept;
	//CMFCListCtrl m_ExtItemListCtrl;
	afx_msg void OnBnClickedBtnSelproj();
	afx_msg LRESULT UpdateProjChecksum(WPARAM w, LPARAM l);
	afx_msg LRESULT  OnUpdataSN(WPARAM w, LPARAM l);
	afx_msg void OnAdminmode();
	CString m_strOpMode;
	afx_msg void OnMesaccesstest();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	afx_msg void OnOpendownloaderdlg();
	afx_msg void OnConfigdown();
	static DWORD WINAPI DownloadThread(LPVOID lpParam);
	volatile bool m_bStartDown;
	volatile bool m_endThread;
	volatile bool m_bFinishDownLoad;
	CRITICAL_SECTION m_csDownLoad;
	//CHelper m_helper;
	afx_msg void OnAutopossetting();
	afx_msg void OnUserSetting();
	void OpenScanBarCom();
	CString m_strChipName;
	CString m_strProjVersion;
	
	// 新增：箱单条码和批号
	CString m_strBoxSN;          // 箱单条码 (box_sn) - USB扫码枪输入
	
	void PrintZPL(CString strZPL);
	void SendLotEnd2Auto();
#ifdef USE_TCP_SYSLOG
	TcpSyslog m_TcpSyslog;
	INT InitTcpSyslog();
	INT UnInitTcpSyslog();
	bool OnDumpLog(void *Para);
	bool OnRecvMsg(void *Para);
#endif

	CString m_strToken;
	//任务开始时间
	CString m_strStartTime;
	//任务结束时间
	CString m_strEndTime;

	CString m_strDevTimeRun;
	CString m_strDevTimeStop;
	CString m_strAutomaticUPH;
	UINT m_strAutomaticTotalCnt;
	UINT m_strAutomaticPassCnt;
	UINT m_strAutomaticFailCnt;
	UINT m_strAutomaticRemoveCnt;
	//是否收到自动化LotEnd消息，判断可以退出
	BOOL m_bRecvAutomaticLotEnd;
	BOOL m_bRecvAutomaticSupplyStop;

	CTime m_DevBegainTime;
	CTime m_DevOverTime;
	bool m_bFirstRunFlag;
	CString m_strTest;

	CString m_ModelName;

	CString m_lastYieldChangeJson;

	CString m_oPSNQty;	//物料条码数量
	CString m_PSNBurnQty;	//物料条码烧录成功数量
	CString m_strMuPianInfo1;
	CString m_strMuPianInfo2;
	bool m_bUseThirdPartyMesDll;
	BOOL m_bRestartSoft;

	afx_msg void OnBnClickedBtnGetburnqty();
	afx_msg void OnAuthoritySetting();
	afx_msg void OnEnglishLang();
	afx_msg void OnChineseOne();
	afx_msg void OnChineseLangTwo();
	afx_msg void Onspecification();

	CString m_strLang;

	MesSyslog m_MesSyslog;
	INT InitMesSyslog();
	INT UnInitMesSyslog();

	INT SelftestFunc();


	afx_msg void OnBnClickedBtnselFwProj();

	BOOL IsProccessRunning(CString strProccess);
	BOOL CallAutoApp();
	INT  GetUfxFileChecksum(CString strUfxFile);
	INT  LoadUfxProject();
	INT  CreateTcpClient();
	INT  DoSelectProgramFunc();
	INT  DoGetCounterFunc();
	INT  DoGetSiteSocketMapFunc();
	INT  DoStartSiteFunc();
	INT  CloseSockThd();

	SOCKET sock;
	CWorkThread m_CliSendWorker;
	CWorkThread m_CliRecvWorker;

	INT CMesDlg::ThdTcpCliSendHandler(MSG msg, void *Para);
	INT CMesDlg::ThdTcpCliRecvHandler(MSG msg, void *Para);

	INT CMesDlg::ParseRecvLoadTask(void * RecvBuffer);
	INT CMesDlg::ParseRecvGetCounter(void * RecvBuffer);
	INT CMesDlg::ParseRecvSelectFunction(void * RecvBuffer);
	INT CMesDlg::ParseRecvGetSiteSktMapFunction(void * RecvBuffer);
	INT CMesDlg::ParseRecvStartSiteFunction(void * RecvBuffer);

	BOOL m_UfsLoadTaskBn;
	BOOL m_UfsGetCountBn;
	BOOL m_UfsSelFunctionBn;
	BOOL m_UfsProgramBn;
	CMutex m_SktMutex;
	INT CMesDlg::SendCmd4ForAuto(CString strAutoTaskFile);
	INT CMesDlg::RawReceive(BYTE*pData, INT Size, SOCKET sock);
	BOOL CMesDlg::TerminateProcessByName(const char* szProcessName);
	std::vector<tGetSocketMap> VecGetSktMap;
	afx_msg void OnBnClickedBtnModelsetting();
	afx_msg void OnBnClickedBtnGetcom();
	BOOL InitModule(BOOL IsProgram);

	//模组配置
	CString m_strModuleName;
	CString m_strrModuleBaudrate;
	CString m_strProgramPID;
	CString m_strVerifyPID;
	CString m_strCmdLine;
	CString m_strModuleLogPath;
	CString m_strModuleExePath;
	CString m_strFWPath;

	std::vector<tSiteInfo_Com> m_vSiteInfo_Com;
	long m_nTimeOut;


	void ClearUIToastDlg(CString strTitleInfo);
};
