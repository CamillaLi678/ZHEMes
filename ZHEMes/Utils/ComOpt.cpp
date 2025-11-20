#include "ComOpt.h"

#define TMSG_STARTRECDATA	(WM_USER+0x001)

#define PrintLog(_Level,fmt,...) \
	if(m_pLogMsg){\
	/*m_pLogMsg->PrintLog(_Level,fmt,__VA_ARGS__);*/\
	}

//void ComOpt::AttachLog(ILog*pILog)
//{
//	m_pILog = pILog;
//}

void ComOpt::ShowErrMsg( CString ErrMsgHead,DWORD ErrNum )
{
	LPTSTR lpMessageBuffer = NULL;
	CString strErr;
	//FormatMessage 将GetLastError函数得到的错误信息（这个错误信息是数字代号）转化成字符串信息的函数。
	if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,NULL,ErrNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR)&lpMessageBuffer,0,NULL )){
			strErr.Format(_T("COM Error: %s failure: (0x%08X) %s\r\n"),ErrMsgHead,ErrNum,(LPTSTR)lpMessageBuffer);
	}
	else{
		strErr.Format("COM Error: %s failure: (0x%08X)\r\n",ErrMsgHead,ErrNum);
	}
	if (lpMessageBuffer) 
		LocalFree( lpMessageBuffer ); // Free system buffer
	PrintLog(LOGLEVEL_ERR,(LPSTR)strErr);
	//AfxMessageBox(strErr,MB_OK|MB_ICONERROR);
}

INT ComOpt::SettingCom(tUartPara& UartPara)
{
	INT Ret=0;
	DCB dcb ; // 定义数据控制块结构
	dcb.DCBlength=sizeof(DCB);
	if(&UartPara!=&m_UartPara){///不是内部调用，需要再赋值
		memcpy(&m_UartPara,&UartPara,sizeof(tUartPara));
	}
	if(m_hCom==INVALID_HANDLE_VALUE){
		PrintLog(LOGLEVEL_ERR,"Please Open Com First");
		Ret=-1;
		return 0;
	}
	GetCommState(m_hCom, &dcb ) ; //读串口原来的参数设置
	dcb.BaudRate =m_UartPara.dwBaudRate; ///9600
	dcb.ByteSize =(BYTE)m_UartPara.wDataWidth; ///数据宽度 8
	dcb.Parity = (BYTE)m_UartPara.wParityMode;	///奇偶校验	ODDPARITY 	
	dcb.StopBits = (BYTE)m_UartPara.wStopWidth ; ///Stop位宽度 ONESTOPBIT 
	dcb.fBinary = TRUE ;
	dcb.fDsrSensitivity = FALSE;          
	dcb.fTXContinueOnXoff = FALSE;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
	dcb.fDtrControl = RTS_CONTROL_DISABLE;
	switch (m_UartPara.wFlowCtrl){
		case FLOWCTRL_NONE:
			{
				dcb.fOutxCtsFlow = FALSE;
				dcb.fOutxDsrFlow = FALSE;
				dcb.fOutX = FALSE;
				dcb.fInX = FALSE;
				break;
			}
		case FLOWCTRL_CTSRTS:
			{
				dcb.fOutxCtsFlow = TRUE;
				dcb.fOutxDsrFlow = FALSE;
				dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
				dcb.fOutX = FALSE;
				dcb.fInX = FALSE;
				break;
			}
		case FLOWCTRL_CTSDTR:
			{
				dcb.fOutxCtsFlow = TRUE;
				dcb.fOutxDsrFlow = FALSE;
				dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
				dcb.fOutX = FALSE;
				dcb.fInX = FALSE;
				break;
			}
		case FLOWCTRL_DSRRTS:
			{
				dcb.fOutxCtsFlow = FALSE;
				dcb.fOutxDsrFlow = TRUE;
				dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
				dcb.fOutX = FALSE;
				dcb.fInX = FALSE;
				break;
			}
		case FLOWCTRL_DSRDTR:
			{
				dcb.fOutxCtsFlow = FALSE;
				dcb.fOutxDsrFlow = TRUE;
				dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
				dcb.fOutX = FALSE;
				dcb.fInX = FALSE;
				break;
			}
		case FLOWCTRL_XONXOFF:
			{
				dcb.fOutxCtsFlow = FALSE;
				dcb.fOutxDsrFlow = FALSE;
				dcb.fOutX = TRUE;
				dcb.fInX = TRUE;
				dcb.XonChar = 0x11;
				dcb.XoffChar = 0x13;
				dcb.XoffLim = 0x800;
				dcb.XonLim = 0x200;
				break;
			}
	}
	if(SetCommState(m_hCom, &dcb )==0){//串口参数配置
		PrintLog(LOGLEVEL_ERR,"SetCommState failed");
		Ret=COMERR_SETUPCOMM;
	}
	return Ret;
}

INT  ComOpt::WorkThreadProc(MSG msg,void *Para)
{
	INT Ret=COMERR_OK;
	//ComOpt *pComOpt=(ComOpt*)Para;
	if(msg.message==TMSG_STARTRECDATA){

		while(1){
			Ret=/*pComOpt->*/CommWatchProc();
			if(Ret!=0){
				break;
			}
		}
	}
	return Ret;
}

INT ComOpt::ClearAcceptBuffer(void)
{
	try{
		if(m_hCom == INVALID_HANDLE_VALUE )
			return -1;
		PurgeComm(m_hCom,PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR| PURGE_RXCLEAR); 
	}
	catch(...)
	{
		return(1);
	}
	return(0);
}

INT ComOpt::OpenCom(INT nComPort,BOOL ShowErr)
{
	CMsgHandler<ComOpt, ComOpt> MsgHandler = MakeMsgHandler(this, &ComOpt::WorkThreadProc);

	INT Ret=COMERR_OK;
	CString strComPort;
	m_nUartPort = nComPort;
	if(m_nUartPort>9){
		strComPort.Format("\\\\.\\COM%d",m_nUartPort);
	}
	else{
		strComPort.Format("COM%d",m_nUartPort);
	}
	m_hCom =CreateFile(strComPort, GENERIC_READ | GENERIC_WRITE, // 允许读写
		0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL |FILE_FLAG_OVERLAPPED,NULL );
	if(m_hCom==INVALID_HANDLE_VALUE){
		if(ShowErr)
			ShowErrMsg("Open com",GetLastError());
		Ret=COMERR_OPENCOM;
		goto __end;
	}
	//m_pILog->PrintLog(LOGLEVEL_LOG, "+++++++++++OpenCom...");
	PrintLog(LOGLEVEL_LOG, "OpenCom...");
	//清空异步读写参数
	memset(&(m_OverlappedRead), 0, sizeof (OVERLAPPED));
	memset(&(m_OverlappedWrite), 0, sizeof (OVERLAPPED));

	if(SetCommMask(m_hCom, EV_RXCHAR|EV_TXEMPTY )==0){//设置事件驱动的类型
		Ret=COMERR_SETCOMMMASK;
		goto __end;
	}
	if(SetupComm(m_hCom, 1024,512)==0){//设置输入、输出缓冲区的大小
		PrintLog(LOGLEVEL_ERR,"SetupComm failed");
		Ret=COMERR_SETUPCOMM;
		goto __end;
	}
	if(PurgeComm(m_hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR| PURGE_RXCLEAR )==0){ //清干净输入、输出缓冲区
		PrintLog(LOGLEVEL_ERR,"PurgeComn failed");
		Ret=COMERR_SETUPCOMM;
		goto __end;	
	}

	COMMTIMEOUTS TimeOuts;
	TimeOuts.ReadIntervalTimeout=MAXDWORD; 
	// 把间隔超时设为最大，把总超时设为0将导致ReadFile立即返回并完成操作 
	TimeOuts.ReadTotalTimeoutMultiplier=0; //读时间系数 
	TimeOuts.ReadTotalTimeoutConstant=50; //读时间常量 
	TimeOuts.WriteTotalTimeoutMultiplier=50; //总超时=时间系数*要求读/写的字符数+时间常量 
	TimeOuts.WriteTotalTimeoutConstant=2000; 

	//获取信号句柄
	m_OverlappedRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if(m_OverlappedRead.hEvent==NULL){
		PrintLog(LOGLEVEL_ERR,"Create read event failed");
		Ret=COMERR_CREATEEVENT;
		goto __end;	
	}

	m_OverlappedWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if(m_OverlappedWrite.hEvent==NULL){
		PrintLog(LOGLEVEL_ERR,"Create write event failed");
		Ret=COMERR_CREATEEVENT;
		goto __end;	
	}
	SettingCom(m_UartPara);
	ClearAcceptBuffer();
	///创建线程，异步接收串口数据

	

	m_bIsThreadStop=FALSE;
	m_ComThread.SetMsgHandle(MsgHandler);
	m_ComThread.CreateThread();
	m_ComThread.PostMsg(TMSG_STARTRECDATA,0,0);

__end:
	return Ret;
}

INT ComOpt::OpenCom( BOOL ShowErr/*=TRUE*/ )
{
	if(m_nUartPort==0){
		return COMERR_OPENCOM;
	}
	return OpenCom(m_nUartPort,ShowErr);
}
INT ComOpt::CommWatchProc()
{	
	DWORD dwEvtMask=EV_RXCHAR|EV_TXEMPTY;
	DWORD dwErrorFlags,dwBytesRead;
	DWORD dwRes=0;
	INT i=0;
	BYTE DataBuf[512];
	INT PckSize=-1;
	if(m_bIsThreadStop==TRUE){
		return -2;
	}
	SetCommMask(m_hCom, EV_RXCHAR|EV_TXEMPTY );//有哪些串口事件需要监视？
	WaitCommEvent(m_hCom, &dwEvtMask,&m_OverlappedRead);// 等待串口通信事件的发生
	if ((dwEvtMask & EV_RXCHAR) == EV_RXCHAR){ // 缓冲区中有数据到达
		COMSTAT ComStat ; DWORD dwLength;
		ClearCommError(m_hCom, &dwErrorFlags, &ComStat ) ;
		dwLength = ComStat.cbInQue ; //输入缓冲区有多少数据
		if (dwLength > 0) {
			BOOL fReadStat ;
			fReadStat=ReadFile(m_hCom,DataBuf,dwLength,&dwBytesRead,&(m_OverlappedRead)); 
			if( !fReadStat ){
				//PrintLog(LOGLEVEL_ERR,"ReadFile dwBytesRead=%d",dwBytesRead);
				if( GetLastError() == ERROR_IO_PENDING ){ //重叠 I/O 操作在进行中
					dwRes=WaitForSingleObject(m_OverlappedRead.hEvent,3000); //等待，直到超时
					switch(dwRes){
						case WAIT_OBJECT_0: //读完成 
							if(GetOverlappedResult(m_hCom,&(m_OverlappedRead),&dwBytesRead,FALSE)==0){//错误
								return -2;
							}
							PrintLog(LOGLEVEL_ERR,"GetOverlappedResult dwBytesRead=%d",dwBytesRead);
							break;
						case WAIT_TIMEOUT: //超时
							return -1;
							break;
						default: //WaitForSingleObject 错误
							break;
					}
				}
			}
			else{
			}
			dwBytesRead=dwLength;
			if(m_bDumpPackEn){
				PrintLog(LOGLEVEL_WARNING,"GetPackFromUart:Bytes=%d",dwBytesRead);
				//DumpBuf(LOGLEVEL_WARNING,DataBuf,dwBytesRead,16);
			}

			PrintLog(LOGLEVEL_WARNING, "ScanComOpt::GetPackFromUart:Bytes=%d", dwBytesRead);

			if(dwBytesRead>0){
				for(i=0;i<(INT)dwBytesRead;i++){
					m_PackData.Add(DataBuf[i]);
					if (DataBuf[i] ==0x0D) {
						tUartEvent UartEvent;
						UartEvent.msg = MSG_PACKRECV;
						UartEvent.Data.Append(m_PackData);
						m_EventRecvPack(&UartEvent);
						m_PackData.RemoveAll();
						PckSize = -1;
						break;
					}
				}
			}
		}
	}
	return 0;
}

void ComOpt::ReInit()
{
	m_UartPara.dwRetryCnt = 0;
	m_UartPara.dwBaudRate=CBR_115200;
	m_UartPara.wParityMode=ODDPARITY;
	m_UartPara.wDataWidth=8;
	m_UartPara.wStopWidth=ONESTOPBIT;
	m_UartPara.wFlowCtrl=FLOWCTRL_NONE;
	m_hCom=INVALID_HANDLE_VALUE;
	m_bIsThreadStop = TRUE;
	memset(&m_OverlappedRead,0,sizeof(OVERLAPPED));
	memset(&m_OverlappedWrite,0,sizeof(OVERLAPPED));
	InitializeCriticalSection(&m_csCom);
}

ComOpt::ComOpt()
:m_nUartPort(0)
,m_bDumpPackEn(FALSE)
{

	ReInit();
}

ComOpt::ComOpt( INT nComPot )
:m_nUartPort(nComPot)
,m_bDumpPackEn(FALSE)
{
	ReInit();
}
INT ComOpt::CloseCom( void )
{
	INT Ret=0;
	if(m_hCom == INVALID_HANDLE_VALUE )
		return 0;
	SetCommMask(m_hCom ,NULL);
	CloseHandle(m_hCom);
	if( m_OverlappedRead.hEvent != NULL ){
		SetEvent(m_OverlappedRead.hEvent);
		CloseHandle( m_OverlappedRead.hEvent );
	}
	if( m_OverlappedWrite.hEvent != NULL ){
		SetEvent(m_OverlappedWrite.hEvent);
		CloseHandle( m_OverlappedWrite.hEvent );
	}
	m_hCom = INVALID_HANDLE_VALUE;
	m_bIsThreadStop=TRUE;
	m_ComThread.DeleteThread();	
	return Ret;
}

ComOpt::~ComOpt()
{
	CloseCom();
	DeleteCriticalSection(&m_csCom);
}

INT ComOpt::WriteCom( BYTE*pData,INT Size )
{
	INT BytesWrite=0;
	INT TryCnt=1;
	tUartEvent UartEvent;
	UartEvent.msg=MSG_PACKSEND;
	UartEvent.SerialIn(pData,Size);
	m_EventSendPack(&UartEvent);
	EnterCriticalSection(&m_csCom);
	if(m_bDumpPackEn){
		PrintLog(LOGLEVEL_WARNING,"Dump Uart Package To Be Sent");
		//DumpBuf(LOGLEVEL_WARNING,pData,Size,16);
	}
	try{
		if( m_hCom == INVALID_HANDLE_VALUE ){
			BytesWrite=-1; goto __end;
		}
		DWORD dwBytesWritten=Size;
		BOOL bWriteStat;
		COMSTAT ComStat;
		DWORD dwErrorFlags;

		ClearCommError(m_hCom,&dwErrorFlags,&ComStat);
		bWriteStat=WriteFile(m_hCom, pData, Size, &dwBytesWritten, &(m_OverlappedWrite));

		if(!bWriteStat){
			if(GetLastError()==ERROR_IO_PENDING){
				GetOverlappedResult(m_hCom,&(m_OverlappedWrite),&dwBytesWritten,TRUE); //等待直到发送完毕
			}
			else{
				dwBytesWritten=0;
			}
		}
		BytesWrite=(long)dwBytesWritten;
	}
	catch(...){
		PrintLog(LOGLEVEL_ERR,"Write Com Error");
		BytesWrite=-1;
	}
__end:
	LeaveCriticalSection(&m_csCom);
	return BytesWrite;
}




BOOL ComOpt::IsComOpen()
{
	if( m_hCom == INVALID_HANDLE_VALUE ){
		return FALSE;
	}
	else{
		return TRUE;
	}
}