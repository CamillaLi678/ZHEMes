#include "stdafx.h"
#include "ACMesAccess.h"
#include "ComTool.h"

#define CHECK_INTERFACE(_FName) \
do{if (m_IMesDll.pfn##_FName == NULL) {m_pILog->PrintLog(LOGLEVEL_ERR, "未实现接口: %s\r\n", "MES_"#_FName); }\
} while (0);

BOOL CACMesAccess::AttachDll(CString strDllPath,BOOL bSet)
{
	BOOL Ret = TRUE;
	if (bSet == FALSE) {
		m_strDllPath.Format("%s\\%s", ::GetModulePath(NULL), strDllPath);
	}
	else {
		m_strDllPath = strDllPath;
	}
	hLib = LoadLibrary(m_strDllPath); //
	if (hLib == NULL) {
		DWORD ErrNo = GetLastError();
		m_pILog->PrintLog(LOGLEVEL_ERR, "打开%s出错，错误码:%d\r\n", strDllPath, ErrNo);
		Ret =FALSE; goto __end;
	}

	m_IMesDll.pfnGetCharset = (FnGetCharset)GetProcAddress(hLib, _T("MES_GetCharset"));
	m_IMesDll.pfnInit = (FnInit)GetProcAddress(hLib, _T("MES_Init"));
	m_IMesDll.pfnDeinit = (FnDeinit)GetProcAddress(hLib, _T("MES_Deinit"));
	m_IMesDll.pfnCheckOperator = (FnCheckOperator)GetProcAddress(hLib, _T("MES_CheckOperator"));
	m_IMesDll.pfnGetWorkOrderRecord = (FnGetWorkOrderRecord)GetProcAddress(hLib, _T("MES_GetWorkOrderRecord"));
	m_IMesDll.pfnGetErrMessage = (FnGetErrMessage)GetProcAddress(hLib, _T("MES_GetErrMessage"));
	m_IMesDll.pfnSendProgrammerInfo = (FnSendProgrammerInfo)GetProcAddress(hLib, _T("MES_SendProgrammerInfo"));
	m_IMesDll.pfnSendReport = (FnSendProgrammerInfo)GetProcAddress(hLib, _T("MES_SendReport"));
	m_IMesDll.pfnSendStatus = (FnSendProgrammerInfo)GetProcAddress(hLib, _T("MES_SendStatus"));
	m_IMesDll.pfnGetEMMCMIDInfo = (FnGetEMMCMIDInfo)GetProcAddress(hLib, _T("MES_GetEMMCMIDInfo"));

	if (m_IMesDll.pfnGetCharset == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "MESAccess动态库没有导出MES_GetCharset函数,默认使用多字节字符集编码方式\r\n");
		m_CharSet = CHARSET_MULTICHAR;
	}
	else {
		INT Charset = m_IMesDll.pfnGetCharset();
		if (Charset == 1) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "MESAccess动态库约定使用UTF8编码\r\n");
			m_CharSet = CHARSET_UTF8;
		}
		else if(Charset == 2){
			m_pILog->PrintLog(LOGLEVEL_ERR, "MESAccess动态库约定使用Unicode编码\r\n");
			m_CharSet = CHARSET_UNICODE;
		}
		else if (Charset == 0) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "MESAccess动态库约定使用多字节字符集编码\r\n");
			m_CharSet = CHARSET_MULTICHAR;
		}
		else{
			m_pILog->PrintLog(LOGLEVEL_ERR, "MESAccess动态库设定的字符集类型为 %d 暂不支持，请确认\r\n", Charset);
			Ret = -1; goto __end;
		}
	}

	if (m_IMesDll.pfnInit == NULL
		|| m_IMesDll.pfnDeinit == NULL
		|| m_IMesDll.pfnCheckOperator == NULL
		|| m_IMesDll.pfnGetWorkOrderRecord == NULL
		|| m_IMesDll.pfnGetErrMessage == NULL
		|| m_IMesDll.pfnSendProgrammerInfo == NULL
		|| m_IMesDll.pfnSendReport == NULL
		|| m_IMesDll.pfnSendStatus == NULL
		|| m_IMesDll.pfnGetEMMCMIDInfo == NULL
		) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "MESAccess动态库有部分函数未实现，请确认\r\n", strDllPath);
		CHECK_INTERFACE(Init);
		CHECK_INTERFACE(Deinit);
		CHECK_INTERFACE(CheckOperator);
		CHECK_INTERFACE(GetWorkOrderRecord);
		CHECK_INTERFACE(GetErrMessage);
		CHECK_INTERFACE(SendProgrammerInfo);
		CHECK_INTERFACE(SendReport);
		CHECK_INTERFACE(SendStatus);
		CHECK_INTERFACE(GetEMMCMIDInfo);
		Ret = -1; goto __end;
	}

__end:
	return Ret;
}

BOOL CACMesAccess::DetachDll()
{
	if (hLib != NULL) {
		FreeLibrary(hLib);
		hLib = NULL;
	}
	return TRUE;
}

CACMesAccess::CACMesAccess()
{
}


CACMesAccess::~CACMesAccess()
{
}
