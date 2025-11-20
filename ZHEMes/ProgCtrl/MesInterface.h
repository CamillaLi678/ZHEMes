#pragma once

#include "LogEdit.h"
#include "MesCommon.h"
#include <vector>

class CMesInterface
{
	// 单例模式
public:
	~CMesInterface();
	static CMesInterface& getInstance()   // 注意调用此函数一定要使用 & 接收返回值，因为 拷贝构造函数已经被禁止
	{
		static CMesInterface _centralcache; // 懒汉式    // 利用了 C++11 magic static 特性，确保此句并发安全
		return _centralcache;
	}
	void Init(ILog *log, MesUserInfo userInfo, CString baseUrl, MesInterfaceInfo interfaceInfo);
	void DeInit();
	INT GetMesLoginToServer(CString User , CString PassWord);
	INT GetTokenFromServer();
	MesInfo getSelectMesInfo(std::vector<MesInfo> items);
	MesInfo GetMesRecord(CString workOrder, CString materialID, CString strCurExec="Program");
	MesInfo GetACMesRecord(CString workOrder, CString materialID, CString strCurExec = "Program");
	
	// 新增：设置额外参数（箱单条码、批号、机台编号、操作员）
	void SetExtraParams(CString boxSN, CString batNo, CString rsNo, CString wkNo);
	
	INT CommitProgramerInfo2Mes(CString strJson);
	//直接发送json内容
	INT CommitProgramRetJson2Mes(CString strJson);
	INT CommitProgramRet2Mes(CString strLastJson);
	INT CommitProgramRet2ACMes(CString strLastJson);
	INT CommitTaskInfo2Mes(CString timeStart, CString timeEnd, CString timeRun, CString timeStop);
	INT CommitAlarmInfo2Mes(CString alarmCode, CString alarmMsg, CString alarmcStartTime, CString alarmKillTime, int alarmFlag);
private:
	CMesInterface();
	CMesInterface(const CMesInterface&) = delete;
	CMesInterface& operator=(const CMesInterface&) = delete;

	ILog *m_pILog;
	BOOL m_bInited;
	BOOL m_bLocal;
	BOOL m_bServerTset;
	//user info
	MesUserInfo mUserInfo;
	CString m_strToken;
	//url info
	CString m_strBaseUrl;
	CString m_strGetLoginUrl;
	CString m_strGetTokenUrl;
	CString m_strGetMesRecordUrl;
	CString m_strSendTaskInfoUrl;
	CString m_strSendAlarmInfoUrl;
	CString m_strSendProgrammerInfoUrl;
	CString m_strSendProgResultUrl;
	//other
	CString m_strDeviceName;
	CString m_strAutoTaskFolder;
	CString m_strProgFileFolder;
	CString m_strStationId;
	CString m_strFacotryId;//厂商ID
	CString m_strDeviceId;//资产编码ID

	INT nOrderID;  ////工单数据表ID

	//workinfo
	MesInfo m_mesInfo;
	
	// 新增：额外参数
	CString m_strBoxSN;      // 箱单条码 (box_sn) - USB扫码枪输入
	CString m_strBatNo;      // 芯片批号 (bat_no) - MES返回或用户输入
	CString m_strRsNo;       // 机台编号 (rs_no)
	CString m_strWkNo;       // 操作员账号 (wk_no) - 登录用户名
};
