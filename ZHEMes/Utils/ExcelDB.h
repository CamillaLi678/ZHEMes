#pragma once
#include "stdafx.h"
#include <afxdb.h> //数据库头文件
#include <odbcinst.h>//odbc头文件
#include <vector>

#define MAXEXT_ITEMNUM (4)
typedef struct {
	CString strItemName;
	CString strItemValue;
}tExcelExtItem;

typedef struct {
	BOOL IsFromExcel;  ///是否来自与Excel，为TRUE表示后面数据来自EXCEL，为FALSE表示只是同步MES
	CString strWorkOrder;	///工单号
	CString strPackageNo; ///包装号
	BOOL bProjAutoCreate; ///自动合成工程文件
	CString strProgFile; ///烧录档案名称
	CString strTemplateFile; //工程模板名称
	CString strProjFile; //工程文件名称
	CString strChecksumFrom; //校验值来源
	CString strChecksumType; //校验值类型
	CString strChecksum; ///校验值
	CString strAutoTask;//自动化任务数据文件名称
	std::vector<tExcelExtItem> vExtItem;
}tExcelRecord;



class CExcelDB
{
public:
	CExcelDB();
	virtual ~CExcelDB();
	int ReadExcel(CString strExcelPath, CString strWorkOrderID, CString strMaterialID,CString&strErrMsg,std::vector<tExcelRecord>&vRecord);
};

