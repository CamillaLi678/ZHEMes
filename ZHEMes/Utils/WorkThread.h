#pragma once

#include <process.h>
#include <afxcmn.h>
#include <windef.h>


typedef  unsigned int (WINAPI* FnWorkThreadProc)(void* args);

class CMsgHandlerBase
{
public:
	CMsgHandlerBase(void* pObject, void* pFn) :m_pObject(pObject), m_pFn(pFn) {};  ///一般构造函数
	CMsgHandlerBase(const CMsgHandlerBase& rhs)///拷贝构造函数
	{ 
		m_pObject = rhs.m_pObject;
		m_pFn = rhs.m_pFn;
	}
	virtual ~CMsgHandlerBase() {};
	virtual CMsgHandlerBase* Copy() = 0;
	INT operator()(MSG msg, void *Para)
	{
		return Invoke(msg, Para);
	}
protected:
	virtual INT  Invoke(MSG msg, void *Para) = 0;
	void* GetObj() { return m_pObject; }
private:
	void* m_pObject;
	void* m_pFn;
};

template <class O, class T>
class CMsgHandler :public CMsgHandlerBase
{
public:
	typedef INT (T::*FnMsgHandler)(MSG msg, void *Para);
	CMsgHandler(O*pObject, FnMsgHandler pFn) :CMsgHandlerBase(pObject,&pFn), m_pFn(pFn){}   ///注意这个地方的复制
	CMsgHandler(const CMsgHandler& rhs) : CMsgHandlerBase(rhs) { m_pFn = rhs.m_pFn; }
	CMsgHandler* Copy() ///利用拷贝构造函数重新获取一个堆中的实例
	{
		return new CMsgHandler(*this);
	}
protected:
	virtual INT  Invoke(MSG msg, void *Para)
	{
		O*pObject = (O*)GetObj();
		return (pObject->*m_pFn)(msg, Para);
	}
private:
	FnMsgHandler m_pFn;
};

template <class O, class T>
CMsgHandler<O, T> MakeMsgHandler(O* pObject, INT(T::*pfnMsgHandler)(MSG msg, void *Para))
{
	return CMsgHandler<O, T>(pObject, pfnMsgHandler);
}



class CWorkThread
{
public:
	CWorkThread(void);
	INT CreateThread();
	INT SetMsgHandle(CMsgHandlerBase &MsgHandler)
	{
		if (m_MsgHandler != NULL) {
			delete m_MsgHandler;
		}
		m_MsgHandler = MsgHandler.Copy();
		return 0;
	};
	BOOL PostMsg(UINT Msg,WPARAM wParam,LPARAM lParam);
	BOOL ExitThread();
	BOOL DeleteThread();
	BOOL IsThreadRun();
	static UINT WINAPI  WorkThreadProc(void* args);
public:
	virtual ~CWorkThread(void);

protected:
	BOOL CWorkThread::EnEvent();
	INT CWorkThread::HandleMsg(MSG Msg);

private:
	HANDLE m_hEvent;
	UINT m_dwThreadID;
	HANDLE m_hThread;
	void *m_Para;
	CMsgHandlerBase*m_MsgHandler;
};


