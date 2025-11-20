#ifndef _COMOPT_H_
#define _COMOPT_H_

#include "WorkThread.h"
#include "Delegate.h"
//#include "LogMsg.h"
#include <afxcmn.h>
//#include "LogFile.h"
#include "ILog.h"


#define FLOWCTRL_NONE		(0)
#define FLOWCTRL_CTSRTS		(1)
#define FLOWCTRL_CTSDTR		(2)
#define FLOWCTRL_DSRRTS		(3)
#define FLOWCTRL_DSRDTR		(4)
#define FLOWCTRL_XONXOFF	(5)

#define COMEVENT_DATARECV	(1)

#define MSG_PACKSEND	(0x10000)
#define MSG_PACKRECV	(0x20000)

///V1.1需要支持
#define FUNCCODE_GETPROVER  (0x63) ///请求告知协议版本
#define FUNCCODE_QUERYRDYEXT  (0x68) ///请求告知IC是否放好,并返回放置情况

///V1.0需要支持
#define FUNCCODE_QUERYEN	(0x64)		///请求告知使能
#define FUNCCODE_QUERYRDY	(0x65)		///请求告知IC是否放好
#define FUNCCODE_SETRDY		(0x66)		///告知IC烧录结果
#define FUNCCODE_SETINITRDY	(0x67)		///告知站点已经初始化

#define FUNCCODE_EXCP		(0x80)			///异常响应位

typedef struct tUartEvent
{
	UINT msg;			///消息类型 MSG_PACKSEND etc
	UINT ErrCode;		///错误码，0表示没有错误
	CByteArray Data;
	void ReInit(){
		ErrCode=0;
		msg=0;
		Data.RemoveAll();
	}
	tUartEvent(){
		ReInit();
	};
	tUartEvent(const tUartEvent&ComEvent){
		ReInit();
		ErrCode=ComEvent.ErrCode;
		msg=ComEvent.msg;
		Data.Append(ComEvent.Data);
	}
	tUartEvent& operator=(const tUartEvent&ComEvent){
		ReInit();
		ErrCode=ComEvent.ErrCode;
		msg=ComEvent.msg;
		Data.Append(ComEvent.Data);
		return *this;
	}
	void SerialIn(BYTE*pData,INT Size){
		INT i;
		for(i=0;i<Size;++i){
			Data.Add(pData[i]);
		}
	}
}tUartvent;

enum eUartComErr
{
	COMERR_OK=0,
	COMERR_FAIL=-1,
	COMERR_OPENCOM=-2,
	COMERR_SETCOMMMASK=-3,
	COMERR_SETUPCOMM=-4,
	COMERR_CREATEEVENT=-5,
	COMERR_EVENT=-6,				///同步事件创建失败
	COMERR_CRC=-7,					///CRC校验码错误
	COMERR_TIMEOUT=-8,			///超时
	COMERR_WRITECOM=-9,		///写数据出错
	COMERR_ERRPCK=-10,			///错误的包
	COMERR_EXEFAIL=-11,			///服务器端执行错误
	COMERR_UNKNOWNFUNCCODE=-12,	///未定义的功能码
};

typedef struct{
	UINT dwUartPort;
	UINT dwBaudRate;		///波特率	 etc CBR_9600
	WORD wParityMode;		///奇偶校验	 etc NOPARITY
	WORD wStopWidth;		///Stop位宽  etc ONESTOPBIT
	WORD wDataWidth;		///数据位宽  etc 8
	WORD wFlowCtrl;			///流控		 etc FLOWCTRL_NONE
	UINT dwTimeout;
	UINT dwRetryCnt;
	UINT dwRetryTimeWait;
}tUartPara;

typedef struct{
	CByteArray PackData;
}tCOMPack;

typedef struct {
	CString strSN;
}tSN;

class ComOpt
{
public:
	ComOpt();
	ComOpt(INT nComPot);
	virtual ~ComOpt();
	INT OpenCom(INT nComPort,BOOL ShowErr=TRUE);
	INT OpenCom(BOOL ShowErr=TRUE);
	INT CloseCom(void);
	INT SettingCom(tUartPara& ComPara);
	INT WriteCom(BYTE*pData,INT Size);
	BOOL IsComOpen();
	INT GetComPort(){return (INT)m_nUartPort;};

	INT CommWatchProc();
	/*static INT WINAPI */int WorkThreadProc(MSG msg,void *Para);
	/*void AttachLog(ILog*pILog);*/
	/*void SetLogMsg(CLogMsg*pLogMsg) { m_pLogMsg = pLogMsg; }
	CLogMsg*m_pLogMsg;*/

	void SetLogMsg(ILog* pLogMsg) { m_pLogMsg = pLogMsg; }
	ILog* m_pLogMsg;
	
protected:
	void ShowErrMsg( CString ErrMsgHead,DWORD ErrNum );
	INT ClearAcceptBuffer(void);
	void ReInit();
	BOOL m_bDumpPackEn;
	tUartPara m_UartPara;
private:
	CByteArray m_PackData;
	CRITICAL_SECTION m_csCom;
	CWorkThread m_ComThread;
	volatile BOOL m_bIsThreadStop;
	OVERLAPPED m_OverlappedRead;
	OVERLAPPED m_OverlappedWrite;
	HANDLE m_hCom;
	INT m_nUartPort;	///串口号
	
public:
	CEventSource m_EventRecvPack;	
	CEventSource m_EventSendPack;
	/*ILog*m_pILog;*/
};


#endif