#include "StdAfx.h"
#include "ExternBurnApi.h"

CExternBurnApi::CExternBurnApi(CString strDllPath)
:m_strDllPath(strDllPath)
{
	memset(&m_DllApiOpts,0,sizeof(tExternBurnDllApiOpts));
	save_hInstance=NULL;
	hLib=NULL;
}

CExternBurnApi::CExternBurnApi()
:m_strDllPath("")
{
	memset(&m_DllApiOpts,0,sizeof(tExternBurnDllApiOpts));
	save_hInstance=NULL;
	hLib=NULL;
}

CExternBurnApi::~CExternBurnApi(void)
{
	CloseDllCom();
}

#define ExportFunc(_FuncName)\
do{\
	m_DllApiOpts._FuncName = (Fn##_FuncName)GetProcAddress(hLib,_T("AC_"#_FuncName));\
	if(m_DllApiOpts._FuncName==NULL){\
		CString strErrMsg;\
		strErrMsg.Format("Export %s failed",#_FuncName);\
		AfxMessageBox(strErrMsg);\
		return -1;\
	}\
}while(0)
	

INT CExternBurnApi::OpenDllCom()
{
	INT Ret=0;
	hLib = LoadLibrary(m_strDllPath); //
	if (hLib==NULL){
		DWORD ErrNo=GetLastError();
		AfxMessageBox("Open ExternBurn.dll Failed, ErrorCode=%d",ErrNo);
		Ret=-1;
	}

	//m_DllApiOpts.StartService = (FnStartService)GetProcAddress(hLib,_T("AC_StartService"));
	ExportFunc(SetServerPath);
	ExportFunc(GetAllSitesInitResult);
	ExportFunc(ClearReadyStatus);
	ExportFunc(GetReadyStatus);
	ExportFunc(SetAdapterEn);
	ExportFunc(SetProgramResult);
	ExportFunc(GetAdapterEn);
	ExportFunc(SetSktEnForInitAuto);
	ExportFunc(QueryCom);
	ExportFunc(LoadSNC);
	ExportFunc(StartService);
	ExportFunc(StartServiceSimple);
	ExportFunc(StopService);
	ExportFunc(CheckService);
	ExportFunc(CreateProject);
	ExportFunc(GetChecksum);
	ExportFunc(GetChecksumExt_Json);
	ExportFunc(LoadProject);
	ExportFunc(StartProject);
	ExportFunc(StopProject);
	ExportFunc(GetResult);
	ExportFunc(GetStatus);
	ExportFunc(GetStatus_Json);
	ExportFunc(GetProgrammerInfo);
	ExportFunc(GetProgrammerInfo_Json);
	ExportFunc(GetDllVersion);
	ExportFunc(SetStartMode);
	ExportFunc(SetAutoStartResponseTime);
	ExportFunc(GetAutoStartResponseTime);
	ExportFunc(SetProjectParameter);
	ExportFunc(LoadPartitionTable);
	ExportFunc(LoadProjectWithLot);
	ExportFunc(GetReport_Json);
	ExportFunc(CreateProjectByTemplate);
	ExportFunc(LoadTaskDataToAutomatic);
	ExportFunc(SendCmdToAutomatic);
	ExportFunc(SetMsgHandle);
	return Ret;
}

INT CExternBurnApi::CloseDllCom()
{
	INT Ret=0;
	if (hLib!=NULL){
		if (MesCheckService() == TRUE) {
			Ret = MesStopService();///先关闭Service,有可能之前已经打开，但是没有正确关闭
		}
		FreeLibrary(hLib);
		hLib=NULL;
	}
	if(save_hInstance!=NULL){
		save_hInstance=NULL;
	}
	return Ret;
}

BOOL CExternBurnApi::MesSetServerPath(CString strServerPath)
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.SetServerPath)
		Ret = m_DllApiOpts.SetServerPath((LPSTR)(LPCTSTR)strServerPath);
	return Ret;
}


BOOL CExternBurnApi::MesStartService()
{
	BOOL Ret=FALSE;
	if (m_DllApiOpts.StartService) {
		Ret = m_DllApiOpts.StartService();
	}
	else {
		AfxMessageBox("请先导出函数",MB_OK|MB_ICONERROR);
	}
	return Ret;
}

BOOL CExternBurnApi::MesStartServiceSimple()
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.StartServiceSimple)
		Ret = m_DllApiOpts.StartServiceSimple();
	return Ret;
}

BOOL CExternBurnApi::MesStopService()
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.StopService)
		Ret=m_DllApiOpts.StopService();
	return Ret;
}

BOOL CExternBurnApi::MesCheckService()
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.CheckService)
		Ret = m_DllApiOpts.CheckService();
	return Ret;
}

BOOL CExternBurnApi::MesCreateProject( char *ProjectFile, char* ChipInfo,char*DataFile,int Quantity, int FuncMode )
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.CreateProject)
		Ret=m_DllApiOpts.CreateProject(ProjectFile,ChipInfo,DataFile,Quantity,FuncMode);
	return Ret;
}

INT CExternBurnApi::MesCreateProjectByTemplate(char *ProjectFile,char*strJsonPara)
{
	INT Ret = 0;
	if (m_DllApiOpts.CreateProject)
		Ret = m_DllApiOpts.CreateProjectByTemplate(ProjectFile,strJsonPara);
	return Ret;
}

BOOL CExternBurnApi::MesGetChecksum( char *Buffer )
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.GetChecksum)
		Ret=m_DllApiOpts.GetChecksum(Buffer);
	return Ret;
}

BOOL CExternBurnApi::GetChecksumExt_Json(char*Buffer, int Size)
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.GetChecksumExt_Json)
		Ret = m_DllApiOpts.GetChecksumExt_Json(Buffer, Size);
	return Ret;
}

BOOL CExternBurnApi::MesLoadProject( char* ProjectFile )
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.LoadProject)
		Ret=m_DllApiOpts.LoadProject(ProjectFile);
	return Ret;
}

BOOL CExternBurnApi::MesLoadProjectWithLot( char *ProjectFile, char* FuncMode, int Quantity )
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.LoadProjectWithLot)
		Ret=m_DllApiOpts.LoadProjectWithLot(ProjectFile,FuncMode,Quantity);
	return Ret;
}

BOOL CExternBurnApi::MesStartProject()
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.StartProject)
		Ret=m_DllApiOpts.StartProject();
	return Ret;
}

BOOL CExternBurnApi::MesStopProject()
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.StopProject)
		Ret=m_DllApiOpts.StopProject();
	return Ret;
}

BOOL CExternBurnApi::MesGetResult( char *Buffer, int Size )
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.GetResult)
		Ret=m_DllApiOpts.GetResult(Buffer,Size);
	return Ret;
}

BOOL CExternBurnApi::MesGetStatus( char *Buffer, int Size )
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.GetStatus)
		Ret=m_DllApiOpts.GetStatus(Buffer,Size);
	return Ret;
}

BOOL CExternBurnApi::MesGetProgrammerInfo( char *Buffer,int Size )
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.GetProgrammerInfo)
		Ret=m_DllApiOpts.GetProgrammerInfo(Buffer,Size);
	return Ret;
}

BOOL CExternBurnApi::MesGetStatus_Json( char *Buffer, int Size )
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.GetStatus_Json)
		Ret=m_DllApiOpts.GetStatus_Json(Buffer,Size);
	return Ret;
}

BOOL CExternBurnApi::MesGetReport_Json( char *Buffer, int Size, int*pSizeNeed)
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.GetReport_Json)
		Ret=m_DllApiOpts.GetReport_Json(Buffer,Size,pSizeNeed);
	return Ret;
}

BOOL CExternBurnApi::MesGetProgrammerInfo_Json( char *Buffer,int Size )
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.GetProgrammerInfo_Json)
		Ret=m_DllApiOpts.GetProgrammerInfo_Json(Buffer,Size);
	return Ret;
}

int CExternBurnApi::MesSetMsgHandle(fnMsgHandle MsgHandle, void*Para)
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.SetMsgHandle)
		Ret = m_DllApiOpts.SetMsgHandle(MsgHandle, Para);
	return Ret;
}


BOOL CExternBurnApi::MesSetStartMode( int Mode )
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.SetStartMode)
		Ret=m_DllApiOpts.SetStartMode(Mode);
	return Ret;
}

BOOL CExternBurnApi::MesSetProjectParameter( char*name,char*value )
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.SetProjectParameter)
		Ret=m_DllApiOpts.SetProjectParameter(name,value);
	return Ret;
}

BOOL CExternBurnApi::MesLoadPartitionTable( char*PartTblFile )
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.LoadPartitionTable)
		Ret=m_DllApiOpts.LoadPartitionTable(PartTblFile);
	return Ret;
}

BOOL CExternBurnApi::MesLoadTaskDataToAutomatic(UINT32 TaskType, char* strTaskData)
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.LoadTaskDataToAutomatic)
		Ret = m_DllApiOpts.LoadTaskDataToAutomatic(TaskType, strTaskData);
	return Ret;
}

BOOL CExternBurnApi::MesSendCmdToAutomatic(UINT32 TaskType, char* strCmd, char *respData, UINT32 len)
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.SendCmdToAutomatic)
		Ret = m_DllApiOpts.SendCmdToAutomatic(TaskType, strCmd, respData, len);
	return Ret;
}

BOOL CExternBurnApi::MesLoadSNC(char* strSNCPath, int ReInitIndex, int StartIndex, int TotalNum, int ProgramIndex)
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.LoadSNC)
		Ret = m_DllApiOpts.LoadSNC(strSNCPath, ReInitIndex, StartIndex, TotalNum, ProgramIndex);
	return Ret;

}

BOOL CExternBurnApi::MesGetAllSitesInitResult(char *Buffer, int Size)
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.GetAllSitesInitResult)
		Ret = m_DllApiOpts.GetAllSitesInitResult(Buffer, Size);
	return Ret;
}

BOOL CExternBurnApi::MesQueryCom(char* pJson, int nSize, char* pPID, int nMaxWaitTime)
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.QueryCom)
		Ret = m_DllApiOpts.QueryCom(pJson, nSize, pPID, nMaxWaitTime);
	return Ret;

}

BOOL CExternBurnApi::MesSetSktEnForInitAuto(char *pJson)
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.SetSktEnForInitAuto)
		Ret = m_DllApiOpts.SetSktEnForInitAuto(pJson);
	return Ret;

}

BOOL CExternBurnApi::MesSetProgramResult(char *SiteSN, char *JsonResult)
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.SetProgramResult)
		Ret = m_DllApiOpts.SetProgramResult(SiteSN, JsonResult);
	return Ret;
}

BOOL CExternBurnApi::MesGetAdapterEn(char *SiteSN, UINT &AdpEn)
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.GetAdapterEn)
		Ret = m_DllApiOpts.GetAdapterEn(SiteSN, AdpEn);
	return Ret;
}

BOOL CExternBurnApi::MesSetAdapterEn(char *SiteSN, UINT AdpEn)
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.SetAdapterEn)
		Ret = m_DllApiOpts.SetAdapterEn(SiteSN, AdpEn);
	return Ret;
}


BOOL CExternBurnApi::MesGetReadyStatus(int nAutoSiteIdx)
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.GetReadyStatus)
		Ret = m_DllApiOpts.GetReadyStatus(nAutoSiteIdx);
	return Ret;
}

BOOL CExternBurnApi::MesClearReadyStatus(int nAutoSiteIdx)
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.ClearReadyStatus)
		Ret = m_DllApiOpts.ClearReadyStatus(nAutoSiteIdx);
	return Ret;
}