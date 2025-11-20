#pragma once

#include "ComOpt.h"
//#include "MesSetting.h"
#include <vector>
using namespace std;

#define  MSG_UPDATA_SN (WM_USER+0x600) 
class CLongoodMESDlg;
//typedef struct {
//	std::vector<CString> strSN;
//}tSN;

class CHandleScan: public ComOpt
{
public:
	CHandleScan(HWND hWnd);
	~CHandleScan();
	bool OnComRecvPack(void* Para);
	void SendRet2Uart(UINT nAdpStatus); //
	void SendACK2Src(bool bRet);
	void AttachInfo(CLongoodMESDlg* pDlg);
	void test();
	HWND m_hParentWnd;
protected:
	/*BOOL IsCRCCorrect(CByteArray& pDataPck);*/
	INT WaitEvent(UINT mTimeOut);
	BOOL ClearEvent();
	BOOL IsEventReady();
	HANDLE m_hEvent;
	tUartEvent m_UartEvent;
private:
	CRITICAL_SECTION m_csModBus;
};

