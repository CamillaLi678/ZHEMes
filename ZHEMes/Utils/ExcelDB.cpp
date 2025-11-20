#include "stdafx.h"
#include "ExcelDB.h"
#include <afxdb.h>
#include <odbcinst.h>



CExcelDB::CExcelDB()
{
}

CString GetExcelDriver()
{
	char szBuf[2001];
	WORD cbBufMax = 2000;
	WORD cbBufOut;
	char *pszBuf = szBuf;
	CString sDriver;

	// 获取已安装驱动的名称(涵数在odbcinst.h里)
	if (!SQLGetInstalledDrivers(szBuf, cbBufMax, &cbBufOut)) {
		return "";
	}

	// 检索已安装的驱动是否有Excel...
	do{
		if (strstr(pszBuf, "Excel") != 0){
			//发现 !
			sDriver = CString(pszBuf);
			break;
		}
		pszBuf = strchr(pszBuf, '\0') + 1;
	} while (pszBuf[1] != '\0');

	return sDriver;
}


int CExcelDB::ReadExcel(CString strExcelPath,CString strWorkOrderID,CString strMaterialID,CString&strErrMsg,std::vector<tExcelRecord>&vRecord)
{
	int Ret = 0;
	int j;
	BOOL RtnCall;
	CDatabase database;
	CString sSql;
	CString sDriver;
	CString sDsn;
	CString vExtItemName[MAXEXT_ITEMNUM] = {"Item1","Item2","Item3","Item4"}; ///
	
	CString sFile = strExcelPath; // 将被读取的Excel文件名
	vRecord.clear();
	// 检索是否安装有Excel驱动 "Microsoft Excel Driver (*.xls)" 
	sDriver = GetExcelDriver();
	if (sDriver.IsEmpty()){
		// 没有发现Excel驱动
		strErrMsg.Format("%s","没有安装Excel驱动!");
		Ret = -1; 
		goto __end;
	}


	// 创建进行存取的字符串，此操作只用于打开excel文件 不具备创建新文件的功能，如果不存在该文件则报错。
	sDsn.Format("ODBC;DRIVER={%s};DSN='';DBQ=%s", sDriver, sFile);
	try{
		RtnCall=database.Open(NULL, false, false, sDsn);// 打开数据库(既Excel文件)
		if (RtnCall == FALSE) {
			strErrMsg.Format("%s", "Excel表格打开失败!");
			Ret = -1;
			goto __end;
		}
		CRecordset recset(&database);//创建用于操作数据文件中记录的对象

		sSql.Format("SELECT * FROM [Sheet2$]");// 设置其他Item的名称
		RtnCall = recset.Open(CRecordset::forwardOnly, sSql, CRecordset::readOnly);
		if (RtnCall == TRUE) {
			int i = 0;
			for (j = 0; j < MAXEXT_ITEMNUM; j++) {
				recset.GetFieldValue((short)i++, vExtItemName[j]);
			}
		}
		recset.Close();

		//sSql.Format("SELECT * FROM [Sheet1$] where 工单号='%s'", strWorkOrderID);// 设置读取的查询语句.
		sSql.Format("SELECT * FROM [Sheet1$] where 工单号='%s' AND 包装号='%s'", strWorkOrderID, strMaterialID);
		//特别注意此处的[Sheet1$]格式，其中Sheet1是demo.xls文件中的一个数据表，
		//普通excel文件中往往不具备表结构，而通过odbc创建的excel文件具备表结构，
		//这就是网上大多数无法用odbc打开普通excel文件的原因所在（
		//将相应的数据表设置会具备表功能的结构也可以通过直接使用Sheet1打开，具体方法百度即可），
		//直接使用[Sheet1$]这种格式即忽略了文件中是否具有表结构，直接将指定的表名所对应的表当作数据表进行读取操作。

		recset.Open(CRecordset::forwardOnly, sSql, CRecordset::readOnly);
		while (!recset.IsEOF()){
			//读取Excel内部数值
			int i = 0;
			tExcelRecord OneRecord;
			CString strProjAutoCreate; ///自动合成工程文件
			CString strAutoTask;//自动化任务数据文件名称
			recset.GetFieldValue((short)i++, OneRecord.strWorkOrder);//读取某行的第0列单元格数据
			recset.GetFieldValue((short)i++, OneRecord.strPackageNo);//读取某行的第1列单元格数据	
			recset.GetFieldValue((short)i++, strProjAutoCreate);
			if (strProjAutoCreate == "是") {
				OneRecord.bProjAutoCreate = TRUE;
			}
			else {
				OneRecord.bProjAutoCreate = FALSE;
			}
			recset.GetFieldValue((short)i++, OneRecord.strProgFile);
			recset.GetFieldValue((short)i++, OneRecord.strTemplateFile);
			recset.GetFieldValue((short)i++, OneRecord.strProjFile);
			recset.GetFieldValue((short)i++, OneRecord.strChecksumFrom);
			recset.GetFieldValue((short)i++, OneRecord.strChecksumType);
			recset.GetFieldValue((short)i++, OneRecord.strChecksum);
			recset.GetFieldValue((short)i++, OneRecord.strAutoTask);

			for (int j = 0; j < MAXEXT_ITEMNUM; ++j) {
				CString strItemValue;
				tExcelExtItem ExtItem;
				recset.GetFieldValue((short)i++, ExtItem.strItemValue);
				ExtItem.strItemName = vExtItemName[j];
				if (ExtItem.strItemValue != "" || ExtItem.strItemName != "") {///只有两个都不为空的是否才需要显示，否则后续界面不显示
					OneRecord.vExtItem.push_back(ExtItem);
				}
			}
			OneRecord.IsFromExcel = TRUE;
			vRecord.push_back(OneRecord);
			recset.MoveNext();// 移到下一行
		}
		recset.Close();
		
		// 关闭数据库
		database.Close();

	}
	catch(CDBException*e){
		// 数据库操作产生异常时...
		strErrMsg.Format("EXCEL数据库错误:%s",e->m_strError);
		Ret = -1;
	}
	catch (...) {
		strErrMsg.Format("EXCEL数据库发生未知异常");
		Ret = -1;
	}

__end:
	return Ret;
}

CExcelDB::~CExcelDB()
{
}
