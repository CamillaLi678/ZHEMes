#pragma once
class CWorkThread;
#include <vector>
#include <map>


#define  SITE_COUNT (4)
#define  MAX_SKTCNT (16)
#define  EACH_SITE_CHIP_NUM (4)

#define  MSG_STARTPGETRET (WM_USER+0x500)

#define  MSG_LIST_HANDLE  (WM_USER+0x510)

#define  MSG_QUERY_COM_SITE_FINISHED (WM_USER+0x511)
#define  MSG_QUERY_COM_ALL_FINISHED (WM_USER+0x512)
#define  MSG_UPDATE_SITE_NAME (WM_USER+0x513)
#define  MSG_UPDATE_PROGRAM_RESULT (WM_USER+0x514)
#define  MSG_UPDATE_PROGRAM_COUNT (WM_USER+0x515)
#define  MSG_UPDATE_AUTO_COUNT (WM_USER+0x516)
#define  MSG_DO_EXTERN_APP_WINDOW_HANDLE (WM_USER+0x517)
#define  MSG_DO_PROGRAM_FINISH (WM_USER+0x518)
#define  MSG_PROGRAM_END (WM_USER+0x519)

#define  MSG_UPDATECHECKSUM (WM_USER+0x003)
#define  MSG_STARTWORK (WM_USER+0x300)
#define  MSG_START_QUERYCOM (WM_USER+0x900)
#define  MSG_CHECK_TIMEOUT (WM_USER+0x700)

typedef struct {
	int TotalCnt; ///当前所有编程器生产总个数
	int PassCnt;  ///当前所有编程器成功总个数
	int FailCnt;  ///当前所有编程器失败总个数
	CString lastYieldJson;
	void ReInitCnt() {
		TotalCnt = 0;
		PassCnt = 0;
		FailCnt = 0;
	}
}tYieldSites;

typedef struct stdMesMessage
{
	std::string msg;
	std::string json;
}tStdMesMessage;

///主界面上的数据
class CMainData
{
public:
	CString strWorkOrder;   ///工单号
	CString strMaterialID;	///包装料号
	CString strAutoTask;	///自动化文件
	CString strProjPath;	///工程路径
	CString strChecksum;	///校验值
	CString strChecksumType;  ///校验值类型
	INT dwWorkOrderICNum;
	INT dwPackageICNum;
	INT dwExpectICNum;
	CString streMMCMID; ///母片管理号
	BOOL bUnderAdminMode; ///是否在管理员模式下？
	CMainData();
	~CMainData();
};


//烧录完成传递到主界面的信息
class  CProgramResultData
{
public:
	void Reset() {
		AutoUPH = _T("");
		AutoTotalCnt = 0;
		AutoPassCnt = 0;
		AutoFailCnt = 0;
		AutoRemoveCnt = 0;
		LastYieldChangeJson = _T("");
		DevTimeRun = _T("000000");
		DevTimeStop = _T("000000");
		LotEndTime = _T("");
		LotStartTime = _T("");
	}
	//自动机运行时长
	CString DevTimeRun;
	//自动机中途停止时长
	CString DevTimeStop;
	//自动机UPH
	CString AutoUPH;
	//自动机统计的总数量
	UINT AutoTotalCnt;
	//自动机统计的总成功数量
	UINT AutoPassCnt;
	//自动机统计的总失败数量
	UINT AutoFailCnt;
	//自动机统计的总移除数量
	UINT AutoRemoveCnt;
	//最后一次MultiAprog返回的YieldChange的Json消息
	CString LastYieldChangeJson;
	//任务开始时间,2023-09-09 18:18:18
	CString LotStartTime;
	//任务结束时间,2023-09-09 18:18:18
	CString LotEndTime;
};


enum {
	ePhaseInit = -1,
	ePhaseReady,
	ePhaseLaunchBurn,
	ePhaseWaitPowerOnResponse,
	ePhaseBurnning,
	ePhaseBurnOver,
	ePhasePowerOnFail,
	ePhaseRecvPowerOffOK,
};

enum {
	PROGRAM_RESULT_DOING = 0,
	PROGRAM_RESULT_OK = 1,
	PROGRAM_RESULT_NG = 2,
};

typedef struct
{
	volatile int nPhase; //0;准备就绪，可以开始进行烧录，1:正在烧录中, 2:等待上电回复阶段，3:上电完成了，进入实际烧录， 4:已经烧录完毕，5:上电失败的， 6;下电成功的
	int nSktBurnResult[4]; //每个Skt的烧录结果，-1；Init， 0:烧录失败， 1:烧录成功
	UINT nCurrEnSkt; //当前的使能的Skt，一个BYTE标识最大8个，每个bit位，代表一个Skt;
	int nPowerCmd; // -1; Init， 0; send cmd for pw on, 1; recv pw ok, 2; recv pw ng, 3; send result for pw off
	int nSiteIdx;
	int nRecvPowerResult;//收到的驱动上下电结果
	time_t startTime;//增加开始烧录时间
	time_t endTime; //增加结束烧录时间
	int nRecvJobResult;//收到超时的jobresult
	int nComPort[1];
	int nAttachIdx;//connect idx
	std::vector<CWorkThread*> vBurnWokThread;
	//CString strSiteSN;
	BOOL SiteEnvInit;
	BYTE SiteIdxAuto;
	UINT AdapterEn;
	UINT AdapterEnPut;
	CMutex* pMutexForChipReady;
	std::map<int,int> m_SktComMap;//座子位对应使用的COM口
	////////////////
}tBurnStatus;

typedef struct
{
	CString SiteAlias;
	CString SiteSn;
	INT SiteIdx;			///站点在MultiAprog的索引1-8
	INT SKTIdx;			///座子在MultiAprog的索引1-8
	CString strCom;
}tSiteInfo_Com;