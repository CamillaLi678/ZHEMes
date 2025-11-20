#pragma once
#include "BaseModuleProgram.h"
#include <map>
#include "MainData.h"
#include "ExternBurnApi.h"
#include "ProgRecord.h"
#include "ILog.h"
#include "DlgAutoPosSetting.h"
#include <list>
#include <functional>

class CSetting;
class BaseModuleProgram
{
public:
	BaseModuleProgram(CString ModuleName, CSetting *pSetting, ILog *pILog);
	virtual int Init() = 0 ;
	virtual INT StartQueryCom(HWND MainHwd, CString strProjPath) = 0;
	virtual INT StartProgram(HWND MainHwd, CProgRecord progRecord, INT nSktEnArray[], INT SktMax, std::map<CString, tBurnStatus> SiteSnMap) = 0;
	virtual INT GetProgramResult(CProgramResultData &ProgramResultData);
	void UpdatePosSetting(CAutoPosSetting &AutoPosSetting) { m_AutoPosSetting = AutoPosSetting; }
	virtual BOOL IsTaskDoing();
	virtual ~BaseModuleProgram();

protected:
	virtual INT SetMsgHandler() = 0;
	virtual INT SetServerPath();
protected:
	INT DoStartupACServer();
	INT UploadProgramerInfo2Mes();
	INT StartService();
	INT LoadProject();
	BOOL IsMesMode();
	BOOL IsAutoMode();
	BOOL GetCmd4RetInfo();
	CString GetAbsReportSavePath();
	INT WaitJobDone(CExternBurnApi* pStdMesApi);
	INT ConfigAutoTaskData();
	INT SendReport2Mes(CString WorkOrder, CString MaterialNo);
	INT ConstructAutoTaskDataWithCmd3(CString&strTaskData);
	INT CompareChecksumProj();
	INT CommitTaskIngfo(CString timeEnd, CString timeRun, CString timeStop);
	INT SaveReprot2Local(const char*strJson, INT Size);
protected:
	CString m_strModuleName;
	ILog *m_pILog;
	CSetting *m_pSetting;
	//工程文件实际校验值
	char m_Checksum[8]; 
	CAutoPosSetting m_AutoPosSetting;
protected:
	volatile BOOL m_bTaskDoing;
	BOOL m_bFinishStartupACServer;
	//昂科MES对接接口
	CExternBurnApi m_ExternBurnApi;
	//要烧录的内容
	CProgRecord m_ProgRecord;
	//主界面的window
	HWND m_MainHwd;
	//烧录结果
	CProgramResultData m_ProgramResult;

	BOOL m_bServerIsRun;

};

