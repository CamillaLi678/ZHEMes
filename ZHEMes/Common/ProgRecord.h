#pragma once
///编程过程中用到的记录
#include "ExcelDB.h"
#include "MesCommon.h"

typedef struct {
	CString strOperatorID;
	CString strWorkOrder;  ///工单号，来自界面
	CString strAutoTaskPath; ///自动化路径
	CString strProjPath;

	CString strMaterialID; ///包装料号，来自界面
	BOOL bIsEMMC; ///是否是eMMC
	CString streMMCMID; ///母片管理号
	
	LONG ExpertIDNum; ///最后芯片烧录数量
	
	BOOL bNeedCompareProjChecksum; ///需要比对工程的校验值
	CString ProjChecksumProvider; ///工程校验值提供者可为MES或EXCEL
	CString ProjChecksumType; ///工程文件校验值类型

	BOOL bNeedComapreFileChecksum; ///需要比对文件的校验值
	CString FileChecksumProvider; ///工程校验值提供者,可为MES或EXCEL
	CString FileChecksumType;///烧录文件校验值类型
	
	
	BOOL bProjSelDirect; ///工程文件是否来自直接选择，如果来自直接选择则不进行校验值比对，需要客户自行比对。

	CString m_WorkOrderICNum;  //工单数量
	CString strProgFilePath; //烧录档案路径
	CString strProgFileChecksum; //烧录档案校验值
	CString strProjChecksum; //工程校验值
	CString strTempFilePath; //模板路径
	CString strFWPath;//模组烧录档案

	CString strChipName;
	CString strSoftVer;

}tDestRecord;///最后决定烧录的信息

class CProgRecord
{
public:
	CProgRecord();
	~CProgRecord();
	CString SerialRecordInfo();
	void ResetDestRecord();
	void ResetExcelRecord();
	void ResetMesRecord();
	void ResetEMMCMIDRecord();
	tExcelRecord ExcelRecord;
	tMesRecord MesRecord;
	tDestRecord DestRecord;
	tEMMCMIDRecord EMMCMIDRecord;
};

