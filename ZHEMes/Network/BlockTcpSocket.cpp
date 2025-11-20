#include "stdafx.h"
#include "BlockTcpSocket.h"

BlockTcpSocket::BlockTcpSocket()
:m_sock(INVALID_SOCKET)
,m_strErrMsg("")
{

}

BlockTcpSocket::~BlockTcpSocket()
{
	Close();
}

BOOL BlockTcpSocket::IsWinSktRun = FALSE;

BOOL BlockTcpSocket::InitSocket()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	IsWinSktRun=FALSE;
	wVersionRequested = MAKEWORD( 1, 1 );
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		return FALSE;
	}
	if ( LOBYTE( wsaData.wVersion ) != 1 ||
		HIBYTE( wsaData.wVersion ) != 1 ) {
			WSACleanup( );
			return FALSE; 
	}
	IsWinSktRun=TRUE;
	return TRUE;
}

BOOL BlockTcpSocket::UninitSocket()
{
	if(IsWinSktRun==TRUE){
		if(WSACleanup()==0){
			IsWinSktRun=FALSE;
			return TRUE;
		}
		return FALSE;
	}
	else{
		return TRUE;
	}
}



BOOL BlockTcpSocket::Socket(INT nSktType,INT nSktProtocal)
{
	ULONG ul;
	BOOL Ret=TRUE;
	m_sock = socket(AF_INET,nSktType,nSktProtocal);
	if(m_sock!=INVALID_SOCKET){
		ul = 0;
		if(ioctlsocket(m_sock, FIONBIO, &ul)!=0){ //设置为阻塞模式
			Ret=FALSE;
		}
	}
	return Ret;
}

BOOL BlockTcpSocket::Create(INT nSktType,INT nSktProtocal)
{
	BOOL Ret=TRUE;
	Close();
	if(Socket(nSktType,nSktProtocal)==FALSE){
		Ret=FALSE;
	}
	return Ret;
}

BOOL BlockTcpSocket::Bind(UINT nPort,CString strIP)
{
	BOOL Ret=TRUE;
	struct sockaddr_in     servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(strIP);
	servaddr.sin_port = htons(nPort);

	if(bind(m_sock,(struct sockaddr*)&servaddr, sizeof(servaddr))!=0){
		m_strErrMsg.Format("Bind Socket Fail, ErrCode=%d",GetLastError());
		Ret=FALSE;
	}
	return Ret;
}

BOOL BlockTcpSocket::Listen(INT BackLog)
{
	BOOL Ret=TRUE;
	if(listen(m_sock,BackLog)!=0){
		m_strErrMsg.Format("Listen Socket Fail, ErrCode=%d",GetLastError());
		Ret=FALSE;
	}
	return Ret;
}

BOOL BlockTcpSocket::Accept(BlockTcpSocket& rContSocket,SOCKADDR* lpSockAddr, int* lpSockAddrLen)
{
	BOOL Ret=TRUE;
	SOCKET skt=accept(m_sock,lpSockAddr,lpSockAddrLen);
	if(skt!=INVALID_SOCKET){
		rContSocket.AttachHandle(skt);
	}
	else{
		Ret=FALSE;
	}
	return Ret;
}

BOOL BlockTcpSocket::SetSockOpt(int nOptionName, const void* lpOptionValue,
				int nOptionLen, int nLevel)
{
	BOOL Ret=TRUE;
	if(m_sock!=INVALID_SOCKET){
		if(setsockopt(m_sock,nLevel,nOptionName,(const char*)lpOptionValue,nOptionLen)!=0){
			Ret=FALSE;
		}
	}
	else{
		Ret=FALSE;
	}

	return Ret;
}

INT BlockTcpSocket::Receive(BYTE*pData,INT Size )
{
	INT Rtn=0;
	INT  iRecvLen;
	m_SktMutex.Lock();
	if (m_sock==INVALID_SOCKET) {
		m_strErrMsg.Format("NET ParamterError m_sock=0x%X,bytes=%d",(UINT)m_sock,Size);
		Rtn=-1; goto __end;
	}
	iRecvLen = recv(m_sock, (char*)pData, Size, 0);
	if (iRecvLen == 0) {///Socket被关闭
		m_strErrMsg.Format("Receive Fail, Socket Close");
		Rtn=0; goto __end;
	}
	else if(iRecvLen<0){
		m_strErrMsg.Format("Receive Fail, ErrCode=%d",WSAGetLastError());
		Rtn=-1; goto __end;
	}
	
	Rtn= iRecvLen;

__end:
	m_SktMutex.Unlock();
	return Rtn;
}

///返回接收的字节数
INT BlockTcpSocket::RawReceive(BYTE*pData,INT Size)
{
	INT Rtn=0;
	INT  iRecvLen;
	INT uiCompleted = 0;
	INT uiPkgSize   = Size; //至少要接收5个字节
	m_SktMutex.Lock();
	if (m_sock==INVALID_SOCKET ) {
		m_strErrMsg.Format("NET ParamterError m_sock=0x%X",(UINT)m_sock);
		Rtn=-1; goto __end;
	}

	while (uiCompleted < uiPkgSize) {
		iRecvLen = recv(m_sock, (char*)pData+uiCompleted, uiPkgSize-uiCompleted, 0);
		if (iRecvLen == 0) {///Socket被关闭
			m_strErrMsg.Format("Receive Fail, Socket Close");		
			Rtn=0; goto __end;
		}
		else if(iRecvLen<0){
			m_strErrMsg.Format("Receive Fail, ErrCode=%d",WSAGetLastError());		
			Rtn=-1; goto __end;
		}
		uiCompleted += iRecvLen;
	}
	Rtn=uiCompleted;

__end:
	m_SktMutex.Unlock();
	return Rtn;
}

//设置心跳包，AliveTime为ms，多少ms没数据就发送心跳包
//AliveInterval 两个心跳包发送的时间间隔
BOOL BlockTcpSocket::SetSockOptKeepAlive(ULONG AliveTime,ULONG AliveInterval)
{
	BOOL Ret=TRUE;
	tcp_keepalive live,liveout; 
	live.keepaliveinterval=AliveInterval; ///每隔2s发送一个心跳包     
	live.keepalivetime=AliveTime; //30s没有就发送心跳包  
	live.onoff=TRUE;
	BOOL bKeepAlive = TRUE;
	int iRet = setsockopt(m_sock,SOL_SOCKET,SO_KEEPALIVE,(char *)&bKeepAlive,sizeof(bKeepAlive));
	if(iRet == 0){
		DWORD dw;      
		if(WSAIoctl(m_sock,SIO_KEEPALIVE_VALS,&live,sizeof(live),&liveout,sizeof(liveout),&dw,NULL,NULL)== SOCKET_ERROR){   
			Ret=FALSE;
		}
	}
	else{
		Ret=FALSE;
	}
	return Ret;
}

BOOL BlockTcpSocket::Connect(CString strIP,UINT nPort)
{
	BOOL Ret=TRUE;

	if(m_sock==INVALID_SOCKET){
		Ret=Create();
		if(Ret==FALSE){
			return Ret;
		}
	}
	struct sockaddr_in serv_addr;

	//以服务器地址填充结构serv_addr
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(strIP);
	serv_addr.sin_port = htons(nPort);
	int error = -1;
	int len = sizeof(int);
	//timeval tm;
	//fd_set set;
	//unsigned long ul = 1;
	//ioctlsocket(sockfd, FIONBIO, &ul); //设置为非阻塞模式
	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	setsockopt(m_sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));

	bool ret = false;
	if( connect(m_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
		Ret=FALSE;
	}
	return Ret;
}

BOOL BlockTcpSocket::Close()
{
	if (m_sock!=INVALID_SOCKET) {
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}
	return TRUE;
}

INT BlockTcpSocket::Send(BYTE *pData,INT Size)
{
	INT Rtn=0;
	INT  iSendLen;
	INT uiCompleted = 0;
	m_SktMutex.Lock();
	if (m_sock==INVALID_SOCKET) {
		m_strErrMsg.Format("NET ParamterError m_sock=0x%X",(UINT)m_sock);
		Rtn=-1; goto __end;
	}
	
	while (uiCompleted < Size) {
		iSendLen = send(m_sock, (char*)pData+uiCompleted, Size-uiCompleted, 0);
		if (iSendLen<0) {
			m_strErrMsg.Format("Send Data Fail, ErrCode=%d",WSAGetLastError());
			Rtn=-1; goto __end;
		}
		uiCompleted += iSendLen;
	}
	Rtn=uiCompleted;

__end:
	m_SktMutex.Unlock();
	return Rtn;
}


CString BlockTcpSocket::GetErrMsg()
{
	return m_strErrMsg;
}

INT BlockTcpSocket::GetLastError()
{
	return WSAGetLastError();
}

