#pragma once
#include "BaseModuleProgram.h"
#include "WorkThread.h"
#include "ILog.h"
#include "DlgAutoPosSetting.h"
#include "MesInterface.h"
#include "StdMesApi.h"
#include <list>
class CSetting;

class NormalProgram{
public:
	NormalProgram(CSetting *pSetting, ILog *pILog);
	virtual ~NormalProgram();
	INT SetServerPath();
	void SetQCCheckResult(BOOL bSuccess);
	INT HandleMsg(char*Msg, char*MsgData);
	INT GetProgramResult(CProgramResultData &ProgramResultData);
	INT StartProgram(HWND MainHwd, CProgRecord ProgRecord);
	CString GetFinalChecksumExt();
	BOOL IsTaskDoing();
	CString GetChecksumExt();
	void UpdatePosSetting(CAutoPosSetting &AutoPosSetting) { m_AutoPosSetting = AutoPosSetting; }
protected:
	HWND m_MainHwd;
	CSetting *m_pSetting;
	CWorkThread m_wTaskWorkThread;
	//要烧录的内容
	CProgRecord m_ProgRecord;
	//烧录结果
	CProgramResultData m_ProgramResult;
private:
	INT StartService();
	INT LoadProject();
	bool IsMesMode();
	bool IsAutoCreateProject();
	INT CreateProjectByTemplate();
	void StartDoProgramTaskWork();
	INT WaitJobDone(CStdMesApi* pStdMesApi);
	CString GetAbsReportSavePath();
	INT GetProjectInfo();
	BOOL GetStatusJsonResult(INT &Total, INT &Pass, INT& Fail);
	BOOL m_bFinishStartupACServer;
	INT DoProgramTaskWorkProc(MSG msg, void *Para);
	INT ConstructAutoTaskDataWithCmd3(CString&strTaskData);
	INT CommitAlarmInfo(CString alarmCode, CString alarmMsg, CString alarmcStartTime, CString alarmKillTime, int alarmFlag);
	INT CommitTaskInfo(CString timeEnd, CString timeRun, CString timeStop);
	INT SendReport2Mes();
	INT UploadProgramerInfo2Mes();
	//扩展checksum
	CString GetChecksumExtJson();
	BOOL IsChipNameMatched(CString SoftChipName, CString RealChipName);
	CString GetChecksumExtForChip(CString ChipName, CString ChecksumExt, std::map<CString, CString> mapChipList);

	void GetCmd4RetInfo();

	INT CompareChecksumFile();
	INT CheckServerLic();
	INT LoadSNC();
	int DoTask();
	INT CompareChecksumProj();
	INT ConfigAutoTaskData();
	bool IsAutoMode();
	INT SaveReprot2Local(const char*strJson, INT Size);

private:
	ILog *m_pILog;
	CStdMesApi m_StdMes;  ///昂科MES对接接口
	volatile bool m_bExitApp;
	volatile BOOL m_bQuit;
	volatile bool m_bCloseDll;
	CString m_strSNCPath;
	BOOL m_bDoCheckQC;
	bool m_bFirstRunFlag;
	CTime m_DevBegainTime;
	CTime m_DevOverTime;
	volatile BOOL m_bTaskDoing;
	volatile UINT  m_nIsProdcingCnt; //正在生产中的数量，包括正在烧录中的。
	volatile tYieldSites m_YieldSites;
	CAutoPosSetting m_AutoPosSetting;
	// 界面上的工单号
	CString m_strChipName;
	CString m_strWorkOrder;
	CString m_strComponentName;
	CString m_strRealChecksum; //工程文件实际校验值
	CString m_strRealChecksumExt; //工程文件实际扩展校验值
	std::map<CString, CString> m_MapChecksumExtList;
	// 实际生产期望数
	LONG m_ExpectICNum;
	CMutex m_YieldSitesMutex;
	CMutex m_SiteInitRetMutex;
	BOOL m_bGetCmd4Success;
	BOOL m_bGetMissionResult;

	//是否收到自动化LotEnd消息，判断可以退出
	BOOL m_bRecvAutomaticLotEnd;
	CString m_lastYieldChangeJson;
private:
	//消息回调处理
	std::list<tStdMesMessage> mRecvProgramMsgList;
	CWorkThread m_MsgListHandleThread;
	CMutex m_ProgramMsgListMutex;
	INT ThreadMsgHandler(MSG msg, void *Para);
	INT DoHandleMsgList();

};