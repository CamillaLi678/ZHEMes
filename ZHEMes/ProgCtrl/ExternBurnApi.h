#pragma once

////定义DLL接口函数指针
#define STDCALL
typedef int(_stdcall *fnMsgHandle)(void*Para, char*Msg, char*MsgData);

#ifdef STDCALL
typedef BOOL(__stdcall *FnSetServerPath)(char *Buffer);
typedef BOOL(_stdcall *FnGetReadyStatus)(int nAutoSiteIdx);
typedef BOOL(_stdcall *FnClearReadyStatus)(int nAutoSiteIdx);
typedef	BOOL(_stdcall *FnGetAllSitesInitResult)(char *Buffer, int Size);
typedef BOOL(_stdcall *FnSetSktEnForInitAuto)(char *pJson);
typedef BOOL(_stdcall *FnQueryCom)(char* pJson, int nSize, char* pPID, int nMaxWaitTime);
typedef BOOL(_stdcall *FnSetAdapterEn)(char *SiteSN, UINT AdpEn);
typedef BOOL(_stdcall *FnSetProgramResult)(char *SiteSN, char *JsonResult);
typedef BOOL(_stdcall *FnGetAdapterEn)(char *SiteSN, UINT &AdpEn);

typedef BOOL(_stdcall * FnLoadSNC)(char* strSNCPath, int ReInitIndex, int StartIndex, int TotalNum, int ProgramIndex);
typedef BOOL (_stdcall *FnStartService)();
typedef BOOL (_stdcall *FnStartServiceSimple)();
typedef BOOL (_stdcall *FnStopService)();
typedef BOOL (_stdcall *FnCheckService)();
typedef BOOL (_stdcall *FnMesCheckService)();
typedef BOOL (_stdcall *FnCreateProject)(char *ProjectFile, char* ChipInfo,char*DataFile,int Quantity, int FuncMode);
typedef BOOL (_stdcall *FnGetChecksum)(char *Buffer);
typedef BOOL (_stdcall *FnGetChecksumExt_Json)(char *Buffer, int Size);
typedef BOOL (_stdcall *FnLoadProject)(char* ProjectFile);
typedef BOOL (_stdcall *FnStartProject)();
typedef BOOL (_stdcall *FnStopProject)();
typedef BOOL (_stdcall *FnGetResult)(char *Buffer, int Size);
typedef BOOL (_stdcall *FnGetStatus)(char *Buffer, int Size);
typedef BOOL (_stdcall *FnGetStatus_Json)(char *Buffer, int Size);
typedef BOOL (_stdcall *FnGetDllVersion)(char *Buffer);
typedef BOOL (_stdcall *FnGetProgrammerInfo)(char *Buffer,int Size);
typedef BOOL (_stdcall *FnGetProgrammerInfo_Json)(char *Buffer,int Size);
typedef BOOL (_stdcall *FnSetStartMode)(int Mode);
typedef BOOL (_stdcall *FnSetAutoStartResponseTime)(int Speed);
typedef BOOL (_stdcall *FnGetAutoStartResponseTime)(int *Speed);
typedef BOOL (_stdcall *FnSetProjectParameter)(char*name,char*value);
typedef BOOL (_stdcall *FnLoadPartitionTable)(char*PartTblFile);
typedef BOOL (_stdcall *FnLoadProjectWithLot)(char* ProjectFile,char*FuncMode,int Quantity);
typedef int  (_stdcall *FnCreateProjectByTemplate)(char*ProjectFile, char*strJson);
typedef BOOL (_stdcall *FnGetReport_Json)(char *Buffer,int Size,int*pSizeNeed);
typedef BOOL (_stdcall *FnLoadTaskDataToAutomatic)(UINT32 TaskType, char* strTaskData);
typedef BOOL (_stdcall *FnSendCmdToAutomatic)(UINT32 TaskType, char* strCmd, char* strRespData, UINT32 len);
typedef int(_stdcall *FnSetMsgHandle)(fnMsgHandle MsgHandle, void*Para);

#else
typedef BOOL (*FnStartService)();
typedef BOOL (*FnStopService)();
typedef BOOL (*FnCheckService)();
typedef BOOL (*FnMesCheckService)();
typedef BOOL (*FnCreateProject)(char *ProjectFile, char* ChipInfo,char*DataFile,int Quantity, int FuncMode);
typedef BOOL(_stdcall *FnGetChecksumExt_Json)(char *Buffer, int Size);
typedef BOOL (*FnGetChecksum)(char *Buffer);
typedef BOOL (*FnLoadProject)(char* ProjectFile);
typedef BOOL (*FnStartProject)();
typedef BOOL (*FnStopProject)();
typedef BOOL (*FnGetResult)(char *Buffer, int Size);
typedef BOOL (*FnGetStatus)(char *Buffer, int Size);
typedef BOOL (*FnGetDllVersion)(char *Buffer);
typedef BOOL (*FnGetProgrammerInfo)(char *Buffer,int Size);
typedef BOOL (*FnSetStartMode)(int Mode);
typedef BOOL (*FnSetAutoStartResponseTime)(int Speed);
typedef BOOL (*FnGetAutoStartResponseTime)(int *Speed);
typedef BOOL (*FnSetProjectParameter)(char*name,char*value);
#endif 

typedef struct tagExternBurnDllApiOpts{
	FnStartService             StartService;
	FnStartServiceSimple	   StartServiceSimple;
	FnStopService              StopService;
	FnCheckService             CheckService;
	FnCreateProject            CreateProject;
	FnGetChecksum              GetChecksum;
	FnGetChecksumExt_Json	   GetChecksumExt_Json;
	FnLoadProject              LoadProject;
	FnStartProject             StartProject;
	FnStopProject              StopProject;
	FnGetResult                GetResult;
	FnGetStatus                GetStatus;
	FnGetProgrammerInfo        GetProgrammerInfo;
	FnGetStatus                GetStatus_Json;
	FnGetProgrammerInfo_Json   GetProgrammerInfo_Json;
	FnGetDllVersion            GetDllVersion;
	FnSetStartMode             SetStartMode;
	FnSetAutoStartResponseTime SetAutoStartResponseTime;
	FnGetAutoStartResponseTime GetAutoStartResponseTime;
	FnSetProjectParameter	   SetProjectParameter;	
	FnLoadPartitionTable	   LoadPartitionTable;
	FnLoadProjectWithLot	   LoadProjectWithLot;
	FnCreateProjectByTemplate  CreateProjectByTemplate;
	FnGetReport_Json GetReport_Json;
	FnSendCmdToAutomatic	   SendCmdToAutomatic;
	FnLoadTaskDataToAutomatic  LoadTaskDataToAutomatic;
	FnSetMsgHandle			   SetMsgHandle;
	FnLoadSNC LoadSNC;
	FnQueryCom QueryCom;
	FnSetSktEnForInitAuto SetSktEnForInitAuto;
	FnSetAdapterEn SetAdapterEn;
	FnSetProgramResult SetProgramResult;
	FnGetAdapterEn GetAdapterEn;
	FnGetAllSitesInitResult GetAllSitesInitResult;
	FnGetReadyStatus GetReadyStatus;
	FnClearReadyStatus ClearReadyStatus;
	FnSetServerPath SetServerPath;
}tExternBurnDllApiOpts;

class CExternBurnApi
{
public:
	CExternBurnApi();
	CExternBurnApi(CString strDllPath);
	virtual ~CExternBurnApi(void);
	void SetDllPath(CString&DllPath){m_strDllPath=DllPath;}
	INT OpenDllCom();
	INT CloseDllCom();

	BOOL MesSetServerPath(CString strServerPath);
	BOOL MesStartService();
	BOOL MesStartServiceSimple();
	BOOL MesStopService();
	BOOL MesCheckService();
	BOOL MesCreateProject(char *ProjectFile, char* ChipInfo,char*DataFile,int Quantity, int FuncMode);
	INT MesCreateProjectByTemplate(char * ProjectFile, char * strJsonPara);
	BOOL MesGetChecksum(char *Buffer);
	BOOL GetChecksumExt_Json(char*Buffer, int Size);
	BOOL MesLoadProject(char* ProjectFile);
	BOOL MesLoadProjectWithLot(char *ProjectFile, char* FuncMode, int Quantity);
	BOOL MesStartProject();
	BOOL MesStopProject();
	BOOL MesGetResult(char *Buffer, int Size);
	BOOL MesGetStatus(char *Buffer, int Size);
	BOOL MesGetProgrammerInfo(char *Buffer,int Size);
	BOOL MesGetStatus_Json(char *Buffer, int Size);
	BOOL MesGetProgrammerInfo_Json(char *Buffer,int Size);
	BOOL MesSetStartMode(int Mode);
	BOOL MesSetProjectParameter(char*name,char*value);
	BOOL MesLoadPartitionTable(char*PartTblFile);
	
	BOOL MesSendCmdToAutomatic(UINT32 TaskType, char* strCmd, char *respData, UINT32 len);
	BOOL MesLoadTaskDataToAutomatic(UINT32 TaskType, char* strTaskData);
	BOOL MesGetReport_Json( char *Buffer, int Size, int*pSizeNeed);
	int MesSetMsgHandle(fnMsgHandle MsgHandle, void*Para);

	BOOL MesLoadSNC(char* strSNCPath, int ReInitIndex, int StartIndex, int TotalNum, int ProgramIndex);
	BOOL MesQueryCom(char* pJson, int nSize, char* pPID, int nMaxWaitTime);
	BOOL MesSetSktEnForInitAuto(char *pJson);
	BOOL MesSetProgramResult(char *SiteSN, char *JsonResult);
	BOOL MesGetAdapterEn(char *SiteSN, UINT &AdpEn);
	BOOL MesSetAdapterEn(char *SiteSN, UINT AdpEn);
	BOOL MesGetAllSitesInitResult(char *Buffer, int Size);

	BOOL MesGetReadyStatus(int nAutoSiteIdx);
	BOOL MesClearReadyStatus(int nAutoSiteIdx);

private:
	CString m_strDllPath;
	HINSTANCE hLib;
	HINSTANCE save_hInstance;
	tExternBurnDllApiOpts m_DllApiOpts;
};
