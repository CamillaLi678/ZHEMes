#include "StdAfx.h"
#include "StdMesApi.h"

CStdMesApi::CStdMesApi(CString strDllPath)
:m_strDllPath(strDllPath)
{
	memset(&m_DllApiOpts,0,sizeof(tDllApiOpts));
	save_hInstance=NULL;
	hLib=NULL;
}

CStdMesApi::CStdMesApi()
:m_strDllPath("")
{
	memset(&m_DllApiOpts,0,sizeof(tDllApiOpts));
	save_hInstance=NULL;
	hLib=NULL;
}

CStdMesApi::~CStdMesApi(void)
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
	

INT CStdMesApi::OpenDllCom()
{
	INT Ret=0;
	hLib = LoadLibrary(m_strDllPath); //
	if (hLib==NULL){
		DWORD ErrNo=GetLastError();
		AfxMessageBox("Open StdMes.dll Failed, ErrorCode=%d",ErrNo);
		Ret=-1;
		return Ret;
	}

	//m_DllApiOpts.StartService = (FnStartService)GetProcAddress(hLib,_T("AC_StartService"));
	ExportFunc(GetCmdErrorCode);
	ExportFunc(LoadSNC);
	ExportFunc(StartService);
	ExportFunc(StartServiceSimple);
	ExportFunc(StopService);
	ExportFunc(CheckService);
	ExportFunc(CreateProject);
	ExportFunc(GetChecksum);
	ExportFunc(LoadProject);
	ExportFunc(StartProject);
	ExportFunc(StopProject);
	ExportFunc(GetResult);
	ExportFunc(GetStatus);
	ExportFunc(GetStatus_Json);
	ExportFunc(GetProgrammerInfo);
	ExportFunc(GetProgrammerInfo_Json);
	ExportFunc(GetDllVersion);
	ExportFunc(GetVersion);
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
	ExportFunc(GetProjectInfo_Json);
	ExportFunc(SetMsgHandle);
	ExportFunc(GetChecksumExt_Json);
	return Ret;
}

INT CStdMesApi::CloseDllCom()
{
	INT Ret=0;
	if (hLib!=NULL){
		//if (MesCheckService() == TRUE) {
			Ret = MesStopService();///先关闭Service,有可能之前已经打开，但是没有正确关闭
		//}
		FreeLibrary(hLib);
		hLib=NULL;
	}
	if(save_hInstance!=NULL){
		save_hInstance=NULL;
	}
	return Ret;
}

BOOL CStdMesApi::MesStartService()
{
	BOOL Ret=FALSE;
	if (m_DllApiOpts.StartService) {
		Ret = m_DllApiOpts.StartService();
	}
	else {
		AfxMessageBox("请先导出StdMes.dll的函数",MB_OK|MB_ICONERROR);
	}
	return Ret;
}

BOOL CStdMesApi::MesStartServiceSimple()
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.StartServiceSimple)
		Ret = m_DllApiOpts.StartServiceSimple();
	return Ret;
}

BOOL CStdMesApi::MesStopService()
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.StopService)
		Ret=m_DllApiOpts.StopService();
	return Ret;
}

BOOL CStdMesApi::MesCheckService()
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.CheckService)
		Ret = m_DllApiOpts.CheckService();
	return Ret;
}

BOOL CStdMesApi::MesCreateProject( char *ProjectFile, char* ChipInfo,char*DataFile,int Quantity, int FuncMode )
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.CreateProject)
		Ret=m_DllApiOpts.CreateProject(ProjectFile,ChipInfo,DataFile,Quantity,FuncMode);
	return Ret;
}

INT CStdMesApi::MesCreateProjectByTemplate(char *ProjectFile,char*strJsonPara)
{
	INT Ret = 0;
	if (m_DllApiOpts.CreateProject)
		Ret = m_DllApiOpts.CreateProjectByTemplate(ProjectFile,strJsonPara);
	return Ret;
}

BOOL CStdMesApi::MesGetChecksum( char *Buffer )
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.GetChecksum)
		Ret=m_DllApiOpts.GetChecksum(Buffer);
	return Ret;
}

BOOL CStdMesApi::MesLoadProject( char* ProjectFile )
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.LoadProject)
		Ret=m_DllApiOpts.LoadProject(ProjectFile);
	return Ret;
}

BOOL CStdMesApi::MesLoadProjectWithLot( char *ProjectFile, char* FuncMode, int Quantity )
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.LoadProjectWithLot)
		Ret=m_DllApiOpts.LoadProjectWithLot(ProjectFile,FuncMode,Quantity);
	return Ret;
}

BOOL CStdMesApi::MesStartProject()
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.StartProject)
		Ret=m_DllApiOpts.StartProject();
	return Ret;
}

BOOL CStdMesApi::MesStopProject()
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.StopProject)
		Ret=m_DllApiOpts.StopProject();
	return Ret;
}

BOOL CStdMesApi::MesGetResult( char *Buffer, int Size )
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.GetResult)
		Ret=m_DllApiOpts.GetResult(Buffer,Size);
	return Ret;
}

BOOL CStdMesApi::MesGetStatus( char *Buffer, int Size )
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.GetStatus)
		Ret=m_DllApiOpts.GetStatus(Buffer,Size);
	return Ret;
}

BOOL CStdMesApi::MesGetDllVersion(char *Buffer)
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.GetDllVersion)
		Ret = m_DllApiOpts.GetDllVersion(Buffer);
	return Ret;
}

BOOL CStdMesApi::MesGetVersion(char *Buffer, int Size)
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.GetVersion)
		Ret = m_DllApiOpts.GetVersion(Buffer, Size);
	return Ret;
}

BOOL CStdMesApi::MesGetProgrammerInfo( char *Buffer,int Size )
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.GetProgrammerInfo)
		Ret=m_DllApiOpts.GetProgrammerInfo(Buffer,Size);
	return Ret;
}

BOOL CStdMesApi::MesGetStatus_Json( char *Buffer, int Size )
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.GetStatus_Json)
		Ret=m_DllApiOpts.GetStatus_Json(Buffer,Size);
	return Ret;
}

BOOL CStdMesApi::MesGetReport_Json( char *Buffer, int Size, int*pSizeNeed)
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.GetReport_Json)
		Ret=m_DllApiOpts.GetReport_Json(Buffer,Size,pSizeNeed);
	return Ret;
}

BOOL CStdMesApi::MesGetProgrammerInfo_Json( char *Buffer,int Size )
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.GetProgrammerInfo_Json)
		Ret=m_DllApiOpts.GetProgrammerInfo_Json(Buffer,Size);
	return Ret;
}

int CStdMesApi::MesSetMsgHandle(fnMsgHandle MsgHandle, void*Para)
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.SetMsgHandle)
		Ret = m_DllApiOpts.SetMsgHandle(MsgHandle, Para);
	return Ret;
}


BOOL CStdMesApi::MesSetStartMode( int Mode )
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.SetStartMode)
		Ret=m_DllApiOpts.SetStartMode(Mode);
	return Ret;
}

BOOL CStdMesApi::MesSetProjectParameter( char*name,char*value )
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.SetProjectParameter)
		Ret=m_DllApiOpts.SetProjectParameter(name,value);
	return Ret;
}

BOOL CStdMesApi::MesGetProjectInfo_Json(char *Buffer, int Size, int *pSizeNeed)
{
	BOOL Ret = 0;
	if (m_DllApiOpts.GetProjectInfo_Json)
		Ret = m_DllApiOpts.GetProjectInfo_Json(Buffer, Size, pSizeNeed);
	return Ret;
}

BOOL CStdMesApi::MesLoadPartitionTable( char*PartTblFile )
{
	BOOL Ret=FALSE;
	if(m_DllApiOpts.LoadPartitionTable)
		Ret=m_DllApiOpts.LoadPartitionTable(PartTblFile);
	return Ret;
}

BOOL CStdMesApi::MesLoadTaskDataToAutomatic(UINT32 TaskType, char* strTaskData)
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.LoadTaskDataToAutomatic)
		Ret = m_DllApiOpts.LoadTaskDataToAutomatic(TaskType, strTaskData);
	return Ret;
}

BOOL CStdMesApi::MesSendCmdToAutomatic(UINT32 TaskType, char* strCmd, char *respData, UINT32 len)
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.SendCmdToAutomatic)
		Ret = m_DllApiOpts.SendCmdToAutomatic(TaskType, strCmd, respData, len);
	return Ret;
}

BOOL CStdMesApi::MesLoadSNC(char* strSNCPath, int ReInitIndex, int StartIndex, int TotalNum, int ProgramIndex)
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.LoadSNC)
		Ret = m_DllApiOpts.LoadSNC(strSNCPath, ReInitIndex, StartIndex, TotalNum, ProgramIndex);
	return Ret;

}

BOOL CStdMesApi::MesGetCmdErrorCode(char *Buffer, int Size)
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.GetCmdErrorCode)
		Ret = m_DllApiOpts.GetCmdErrorCode(Buffer, Size);
	return Ret;
}

BOOL CStdMesApi::MesGetChecksumExt_Json(char *Buffer, int Size)
{
	BOOL Ret = FALSE;
	if (m_DllApiOpts.GetChecksumExt_Json)
		Ret = m_DllApiOpts.GetChecksumExt_Json(Buffer, Size);
	return Ret;

}