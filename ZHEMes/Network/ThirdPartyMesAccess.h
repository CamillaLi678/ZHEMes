#pragma once
#include "ILog.h"
#include "MesCommon.h"

typedef int(__cdecl *FnGetSoftVision)(char parDID[], char shoporder[], char *retSoftVisionInfo, char *retMessage);
typedef int(__cdecl *FnStartAndPrintlabel)(char parDID[], char parQty[], char parShoporder[], char parProductID[], char parSoftVision[], char parDataCode[], char parLotCode[], char parModel[], char parLibrary[], char parManufacturer[], char *retMessage);

typedef struct tagThirdPartyMesDll {
	FnGetSoftVision pfnGetSoftVision;
	FnStartAndPrintlabel pfnStartAndPrintlabel;
}tThirdPartyMesDll;

class CThirdPartyMesAccess
{
public:
	~CThirdPartyMesAccess();
	static CThirdPartyMesAccess& getInstance()   // 注意调用此函数一定要使用 & 接收返回值，因为 拷贝构造函数已经被禁止
	{
		static CThirdPartyMesAccess _centralcache; // 懒汉式// 利用了 C++11 magic static 特性，确保此句并发安全
		return _centralcache;
	}
	void Init(ILog *log, CString strDllPath, CString tskFolder, CString aprFolder, BOOL isLocal);
	void DeInit();
	INT GetTokenFromServer();
	INT GetMesRecord(CString workOrder, CString materialID, CString strCurExec, MesInfo &mesResult);
	INT CommitProgramerInfo2Mes(CString strJson);
	INT CommitProgramRet2Mes(CString strLastJson, UINT TotalCnt, UINT PassCnt, UINT FailCnt);
	INT CommitTaskInfo2Mes(CString timeStart, CString timeEnd, CString timeRun, CString timeStop);
	INT CommitAlarmInfo2Mes(CString alarmCode, CString alarmMsg, CString alarmcStartTime, CString alarmKillTime, int alarmFlag);

private:
	BOOL AttachDll(CString strDllPath, BOOL bSet);
	BOOL DetachDll();
	CThirdPartyMesAccess();
	CThirdPartyMesAccess(const CThirdPartyMesAccess&) = delete;
	CThirdPartyMesAccess& operator=(const CThirdPartyMesAccess&) = delete;

	CString m_strErrMsg;
	CString m_errCode;
	tThirdPartyMesDll m_ThirdPartyMesDll;
	ILog *m_pILog;
	BOOL m_bInited;
	BOOL isLocalServer;

	CString m_strAutoTaskFolder;
	CString m_strProgFileFolder;
	CString m_strDllPath;
	HINSTANCE hLib;
	char parDID[256];
	char m_strShoporder[256];

	MesInfo m_MesResult;
};

