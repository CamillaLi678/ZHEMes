#pragma once
#include "BaseModuleProgram.h"
#include "WorkThread.h"
#include "ILog.h"
#include "DlgAutoPosSetting.h"
#include "MesInterface.h"
#include <list>
#include <functional>
class CSetting;

typedef void(*FunClearUIToastDlg)(CString strTitleInfo);

typedef struct ModuleInfo
{
	CString strBaudrate;//波特率
	CString strProgramPID_VID;//
	CString strVerifyPID_VID;
	CString strMode;//烧录模式
	CString strCmdLine;//命令行
	CString strExePath;
	CString strLogPath;
	CString strComMap;
	long nTimeOut;//烧录时间 /s
	CString strOKFlag;//烧录成功标志
}tModuleInfo;

class ModuleProgram : public BaseModuleProgram {
public:
	ModuleProgram(CString ModuleName,CSetting *pSetting,ILog *pILog);
	int Init();
	virtual ~ModuleProgram();
	virtual INT StartQueryCom(HWND MainHwd, CString strProjPath);
	INT HandleMsg(char*Msg, char*MsgData);
	INT StartProgram(HWND MainHwd, CProgRecord progRecord, INT nSktEnArray[], INT SktMax, std::map<CString, tBurnStatus> SiteSnMap);
	void SetCallback(std::function<void(CString)> callback) { m_ClearUICallback = callback; }
protected:
	virtual INT SetMsgHandler();
protected:
	std::function<void(CString)> m_ClearUICallback;
private:
	void MoveLogFile(CString OldLogPath,BOOL bResult);
	INT GetModuleJson();
	INT StartupQueryCom();
	BOOL GetAllSitesInitResult();
	BOOL GetStrDevFromAttachIdx(int nConnectIdx, CString& strDev);
	BOOL GetAttachIdxFromStrDev(CString strDev, int& nConnectIdx);
	int SetSiteEnForInfoMap(INT nSktEnArray[], INT SktMax);
	//Task控制流程线程
	void StartDoProgramTaskWork();
	INT DoProgramTaskWorkProc(MSG msg, void *Para);
	int DoTask();
	//取外部烧录程序的结果线程
	INT DoGetExternProgramResult(int nIdx, int skt);
	INT RunExcuCmdLine(int nIdx, int skt);
	INT GetExternProgramResultThreadProc(MSG msg, void *Para);
	INT GetProgramResultFromLog(CString SiteSn,CString Flag,CString strLogPath);//成功返回1，失败0
private:
	FunClearUIToastDlg *m_pFunClearUIToastDlg;
	INT QueryComThreadProc(MSG msg, void *Para);
	CWorkThread m_QueryComThread;
	CWorkThread m_wTaskWorkThread;
	INT m_nQueryComFlag;
	volatile int m_nPowerOffForQueryCom;
	std::map<CString, tBurnStatus> m_BurnStatusMap;
private:
	tModuleInfo m_ModuleInfo;//模组信息
	volatile bool m_bExitApp;
	volatile BOOL m_bQuit;
	volatile UINT  m_nIsProdcingCnt; //正在生产中的数量，包括正在烧录中的。
	volatile tYieldSites m_YieldSites;
	// 界面上的工单号
	CString m_strWorkOrder;
	CString m_strComponentName;
	// 实际生产期望数
	LONG m_ExpectICNum;
	CString m_strAllSiteInitResult;
	INT m_nConnectSiteCnt;
	UINT m_uSlotNum;
	void ReleaseMem();
	CMutex m_YieldSitesMutex;
	CMutex m_SiteInitRetMutex;
	BOOL m_bGetCmd4Success;
	BOOL m_bGetMissionResult;
	//是否收到自动化LotEnd消息，判断可以退出
	BOOL m_bRecvAutomaticLotEnd;
	BOOL m_bConfigCom;
	CString m_strModuleName;
	CString m_strCmdLine;
	CString m_ExeLogPath;
	BOOL m_IsProgramEnd;//烧录结束

	std::map<CString, tBurnStatus> m_SiteSnMapFromUI;
private:
	//消息回调处理
	std::list<tStdMesMessage> m_RecvProgramMsgList;
	CWorkThread m_MsgListHandleThread;
	CMutex m_ProgramMsgListMutex;
	INT ThreadMsgHandler(MSG msg, void *Para);
	INT DoHandleMsgList();
};