#pragma once
#include <vector>
typedef struct tMesInfo {
	INT errCode;
	CString workOrder;				//工单号
	CString materialID;				//工单料
	LONG expectICNum;				//工单期望数量，小于未完成数量
	LONG workOrderICNum;			//工单总数量
	LONG workOrderCompletedICNum;	//工单已经完成数量
	LONG RemainICNum;				////工单剩余数量
	CString curExec;				//当前Program，还是Verify
	CString projPath;				//MES返回工程文件apr地址，可能只是相对地址
	CString projChecksum;			//工程文件apr的checksum
	CString projVersion;			//工程文件版本号
	CString Model;
	CString ProductId;
	CString ProductName;
	CString Lotcode;
	CString Datecode;
	CString Manufacturer;
	CString LibraryName;
	CString chipName;				//芯片
	CString autoTaskFilePath;		//MES返回自动化文件tsk地址，可能只是相对地址
}MesInfo;

typedef struct tMesInterfaceInfo {
	CString tskFolder;				//自动化任务文件文件夹
	CString aprFolder;				//工程文件文件夹
	CString stationId;				//工作站点名
	CString factoryId;				//厂商名
	CString deviceId;				//资产编码ID
	BOOL bLocal;					//是否本地调试
	BOOL bServerTset;              //本地MES测试
}MesInterfaceInfo;

typedef struct tTcpMesMbtInfo {

	CString strIp;    // TCP IP地址
	CString strPort;  // TCP 端口号
	CString strTimeValue;// 时间
	CString strRandom;  // 随机码

	CString strValue;  // 内容
	CString strEquiID; // 设备定义的ID，具体是啥再问
	CString strStationID; // 设备ID ，与上面那个区别，需要再问

}MBTMesInfo;


typedef struct tClientRecvInfo {
	CString tMessageName;
	CString tTranSaction;
	CString tMessageID;
	CString tReplSubjectName;
	CString tEquimentId;
	CString tReturnCode;
	CString tReturnMessage;

	tClientRecvInfo() :tMessageName("Test"), tTranSaction("Test"), tMessageID("Test"), tReplSubjectName("Test"), tEquimentId("Test"), tReturnCode("01"), tReturnMessage("Test") {};
}tRecvInfo;


struct MesUserInfo {
	CString useCode;				//
	CString providerId;				//
	CString account;				//账号
	CString password;				//密码
};

typedef struct {
	CString strItemName;
	CString strItemValue;
}tMesExtItem;

typedef struct
{
	BOOL bMesEnable; ///MES??·?????
	CString strWorkOrder;
	double    WorkOrderLotNumber;

	CString strPackageNo; ///°ü×°??
	double    PackageNumber; ///°ü×°??

	CString strProgFile; ///??????°?????

	CString strChecksumFrom; ///???é??????
	CString strChecksumType; ///???é???à??
	CString strChecksum; ///???é??
	BOOL bIseMMC; ///??·???eMMC

				  ///V2.1°?±???????×?????MES?????¨?á????
				  ///??·??è?¨
	BOOL bProjAutoCreateSet;
	BOOL bstrTemplateFileSet;
	BOOL bstrProjFileSet;
	BOOL bstrProjectChecksumSet;
	BOOL bstrAutoTaskSet;

	///?è?¨??
	BOOL bProjAutoCreate; ///×????????¤??????
	CString strTemplateFile; //?¤????°?????
	CString strProjFile; //?¤??????????

	CString strAutoTask;//×?????????????????????

	CString strProgFileChecksum; //??????°????é??
	CString strProjectChecksum; ///?¤?????é??
	CString strProjTemplatePath;  //??°??·??
	CString strAutoTaskPath; //×??????????????·??
	CString strProgFilePath; //??????°??·??
	CString strSwap; //??·?swap 0:·??? 1:??
	CString strSNCPath; //snc???·??

	std::vector<tMesExtItem> vExtItem;
}tMesRecord;

typedef struct
{
	CString strMID;
	int LimitCnt;
	int CurrentCnt;
}tEMMCMIDRecord;
