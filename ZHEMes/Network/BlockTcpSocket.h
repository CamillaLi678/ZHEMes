#ifndef _STSOCKET_H_
#define _STSOCKET_H_

#include <WinSock2.h>
#include <afxmt.h>

#define SIO_RCVALL            _WSAIOW(IOC_VENDOR,1)
#define SIO_RCVALL_MCAST      _WSAIOW(IOC_VENDOR,2)
#define SIO_RCVALL_IGMPMCAST _WSAIOW(IOC_VENDOR,3)
#define SIO_KEEPALIVE_VALS    _WSAIOW(IOC_VENDOR,4)
#define SIO_ABSORB_RTRALERT   _WSAIOW(IOC_VENDOR,5)
#define SIO_UCAST_IF          _WSAIOW(IOC_VENDOR,6)
#define SIO_LIMIT_BROADCASTS _WSAIOW(IOC_VENDOR,7)
#define SIO_INDEX_BIND        _WSAIOW(IOC_VENDOR,8)
#define SIO_INDEX_MCASTIF     _WSAIOW(IOC_VENDOR,9)
#define SIO_INDEX_ADD_MCAST   _WSAIOW(IOC_VENDOR,10)
#define SIO_INDEX_DEL_MCAST   _WSAIOW(IOC_VENDOR,11)

struct tcp_keepalive {
	u_long onoff;    // 是否开启 keepalive
	u_long keepalivetime; //// 多长时间（ ms ）没有数据就开始 send 心跳包
	u_long keepaliveinterval;  // 每隔多长时间（ ms ） send 一个心跳包，
};

class BlockTcpSocket
{
public:
	BlockTcpSocket();
	~BlockTcpSocket();
	///初始化Socket环境，需要先被调用
	static BOOL InitSocket();
	///释放Socket环境，最后调用
	static BOOL UninitSocket();
	static BOOL IsWinSktRun;

	BOOL Create(INT nSktType=SOCK_STREAM,INT nSktProtocal=IPPROTO_TCP);
	BOOL Close();
	
	BOOL Bind(UINT nPort,CString strIP);
	BOOL Listen(INT BackLog);
	BOOL Accept(BlockTcpSocket& rContSocket,SOCKADDR* lpSockAddr = NULL, int* lpSockAddrLen = NULL);

	BOOL Connect(CString strIP,UINT nPort);
	
	BOOL SetSockOpt(int nOptionName, const void* lpOptionValue,
		int nOptionLen, int nLevel = SOL_SOCKET);
	BOOL SetSockOptKeepAlive(ULONG AliveTime,ULONG AliveInterval);

	/*****************
	接收数据，成功返回实际发送字节数
	返回0表示Socket关闭
	失败返回-1;
	此函数自带协议功能，会根据协议包中的字段确认接受的数据长度
	*********************/
	INT Receive(BYTE*pData,INT Size);
	/*****************
	发送数据，成功返回实际发送字节数
	失败返回-1;
	*********************/
	INT Send(BYTE*pData,INT Size);
	CString GetErrMsg();
	INT GetLastError();

	/*****************
	接收数据，成功返回实际发送字节数
	返回0表示Socket关闭
	失败返回-1;
	此函数接收裸数据
	Size表示希望接受的数据字节数
	*********************/
	INT RawReceive(BYTE*pData,INT Size);

	void AttachHandle(SOCKET Sock){m_sock=Sock;}
	SOCKET FromHandle(){return m_sock;}
private:
	CMutex m_SktMutex;
	BOOL Socket(INT nSktType,INT nSktProtocal);
	SOCKET m_sock;
	CString m_strErrMsg;
};
#endif