#pragma once

#include "ILog.h"
#include <vector>


#define CHARSET_MULTICHAR (0)   //多字节字符集
#define CHARSET_UTF8 (1)		//UTF8
#define CHARSET_UNICODE (2)		//UniCode

///Delphi 中对应BOOL的应该是integer 而不是boolean
typedef BOOL (_stdcall *FnGetCharset)(void);
typedef BOOL (_stdcall *FnInit)(char*strJsonFile);
typedef BOOL (_stdcall *FnDeinit)(void);
typedef BOOL (_stdcall *FnGetErrMessage)(char*strMsg, int Size);
typedef BOOL (_stdcall *FnCheckOperator)(char* OperatorID, char* Password, char*strJson, int Size);
typedef BOOL (_stdcall *FnGetWorkOrderRecord)(char*WorkOrder, char*MaterialNo, char*strJsonRecord, int Size);
typedef BOOL (_stdcall *FnSendProgrammerInfo)(char*strProgsInfoJson);
typedef BOOL (_stdcall *FnSendStatus)(char*strStatusJson);
typedef BOOL (_stdcall *FnSendReport)(char*strReportJson);
typedef BOOL(_stdcall *FnGetEMMCMIDInfo)(char*WorkOrder, char*MaterialNo, char*EMMCMID, char*strMIDInfo, int Size);
typedef struct tagIMesDll {
	FnGetCharset pfnGetCharset;
	FnInit pfnInit;
	FnDeinit pfnDeinit;
	FnGetErrMessage pfnGetErrMessage;
	FnCheckOperator pfnCheckOperator;
	FnGetWorkOrderRecord pfnGetWorkOrderRecord;
	FnSendProgrammerInfo pfnSendProgrammerInfo;
	FnSendStatus pfnSendStatus;
	FnSendReport pfnSendReport;
	FnGetEMMCMIDInfo pfnGetEMMCMIDInfo;
}tIMesDll;

class IMesAccess
{
public:
	IMesAccess();
	virtual ~IMesAccess();
	void AttachLog(ILog*pILog);
	virtual BOOL AttachDll(CString strDllPath, BOOL bSet);
	virtual BOOL DetachDll();
	virtual INT  GetCharset();
	virtual BOOL Init(CString strJsonFile);
	virtual BOOL Deinit(void);
	virtual BOOL GetErrMessage(CString&strErrMsg,CString*pErrCode=NULL);
	virtual BOOL CheckOperator(CString Operator, CString Password, CString &strJson);
	virtual BOOL GetWorkOrderRecord(CString strWorkOrder, CString strMaterialNo, CString&strJsonRecode);
	virtual BOOL SendProgrammerInfo(CString strProgsInfoJson);
	virtual BOOL SendStatus(CString strStatusJson);
	virtual BOOL SendReport(CString strReportJson);
	virtual BOOL SendReport(std::string strReportJson);
	virtual BOOL GetEMMCMIDInfo(CString strWorkOrder, CString strMaterialNo, CString strEMMCMID, CString&strJsonRecode);

protected:
	INT m_CharSet; ///0为UTF8,1为UNICODE
	tIMesDll m_IMesDll;
	ILog*m_pILog;
	CString m_strErrMsg;
	CString m_errCode;
};

