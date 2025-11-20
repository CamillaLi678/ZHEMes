#pragma once

#define AUTOTASK_CMD3 (3)
#define AUTOTASK_CMD2 (2)

#define MSG_UPDATA_SETTING_CONFIG	(WM_USER+0x304) //更新设置里的值

class CSetting
{
public:
	CSetting();
	~CSetting();

	CString strProgFileFolder; ///烧录档案存放文件夹
	CString strProjTemplateFolder; ///工程模板存放文件夹 
	CString strProjSaveFolder;  ///工程存放文件夹
	
	CString strACServerFolder;///服务器路径
	CString strCurExec; ///当前执行的命令
	CString strAutoTaskFolder;///自动化数据存放文件夹
	CString strReportFolder; ///生产报告保存文件夹

	CString strWorkOrder; //工单
	CString strOperator;//操作员
	double MaxProduceValue; //上限值
	CString strAutoTaskFileExt;///自动化任务数据文件后缀名
	CString strWorkStationID;///工位
	UINT nAutoTaskLoadCmd; ////自动化任务数据加载时采用的命令
	int nElectricInsertCheck; //余料检测
	int nCheckDay;
	int nLocalServer;
	int nServerTest;    
	int nDoHttpTest;//是否进行HTTP测试

	CString strProgramMode;//烧录模式
	CString strMesWordMode; //工作模式
	CString strProjectMode;//工程模式
	CString strAutoMode;//自动模式
	CString strAutomaticType;//自动机类型
	CString strWebServiceInterface;
	CString strReportURL; //上报烧录结果的url；

	CString strModuleName;

	int nEnPrint; //打印轩辕
	CString strPrintIP;
	int nPrintPort;

	int nEnBarPrint; //是否开启BarPrint
	 
	int  nReInitIndex;
	int nStartIndex;
	int nSyslogEnable;
	int nTotalNum;
	int nProgramIndex;

	int nScanComPort;

	CString strItemNum;
	CString strSoftVer;
	CString strManufactor;
	CString strIcNum;
	CString strDeviceName;
	CString strSoftMainVer;

	CString strAccount;
	CString strPwd;
	CString strUseCode;
	CString strServiceproviderid;

	CString strUfsWriterFolder;

	///设置配置的保存路径
	BOOL SetJsonPath(CString strJsonPath);
	BOOL Save();
	BOOL Load();
	CString GetErrMsg() {
		return m_strErrMsg;
	};
private:
	CString m_strSettingJsonPath;
	CString m_strErrMsg;
};

