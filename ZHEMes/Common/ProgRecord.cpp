#include "stdafx.h"
#include "ProgRecord.h"


CProgRecord::CProgRecord()
{
}


CProgRecord::~CProgRecord()
{
}

//CString CProgRecord::PrintMesRecord(ExcelRecord*pExcelRecord,CString&Out)
//{
//
//}

CString CProgRecord::SerialRecordInfo()
{
	CString strMsg;
	CString Output;
	strMsg.Format("\r\n=======================生产任务信息========================\r\n");
	Output += strMsg;
	strMsg.Format("工单号:%s\r\n", ExcelRecord.strWorkOrder);
	Output += strMsg;
	strMsg.Format("包装号:%s\r\n", ExcelRecord.strPackageNo);
	Output += strMsg;
	if (ExcelRecord.IsFromExcel == TRUE) {
		strMsg.Format("==================以下信息来源于EXCEL配置==================\r\n");
		Output += strMsg;
		strMsg.Format("是否自动合成工程:%s\r\n",ExcelRecord.bProjAutoCreate?"是":"否");
		Output += strMsg;
		if (ExcelRecord.strProgFile) {
			strMsg.Format("烧录档案名称:%s\r\n", ExcelRecord.strProgFile);
			Output += strMsg;
		}
		if (ExcelRecord.strTemplateFile) {
			strMsg.Format("工程模板名称:%s\r\n", ExcelRecord.strTemplateFile);
			Output += strMsg;
		}
		if (ExcelRecord.strProjFile) {
			strMsg.Format("工程文件名称:%s\r\n", ExcelRecord.strProjFile);
			Output += strMsg;
		}
		if (ExcelRecord.strChecksumFrom) {
			strMsg.Format("校验值来源:%s\r\n", ExcelRecord.strChecksumFrom);
			Output += strMsg;
		}
		if (ExcelRecord.strChecksumType) {
			strMsg.Format("校验值类型:%s\r\n", ExcelRecord.strChecksumType);
			Output += strMsg;
		}
		if (ExcelRecord.strChecksum) {
			strMsg.Format("校验值:%s\r\n", ExcelRecord.strChecksum);
			Output += strMsg;
		}

		if (ExcelRecord.strAutoTask) {
			strMsg.Format("自动化任务数据文件名称:%s\r\n", ExcelRecord.strAutoTask);
			Output += strMsg;
		}
		if (ExcelRecord.vExtItem.size() > 0) {
			int k;
			strMsg.Format("======扩展项信息======\r\n");
			Output += strMsg;
			for (k = 0; k < (INT)ExcelRecord.vExtItem.size(); k++) {
				strMsg.Format("%s : %s\r\n", ExcelRecord.vExtItem[k].strItemName,ExcelRecord.vExtItem[k].strItemValue);
				Output += strMsg;
			}
		}
	}
	if (MesRecord.bMesEnable) {
		strMsg.Format("\r\n==================以下信息来源于MES 配置==================\r\n");
		Output += strMsg;
		
		strMsg.Format("工单生产芯片数:%.0f\r\n", MesRecord.WorkOrderLotNumber);
		Output += strMsg;
		
		strMsg.Format("包装号对应芯片数:%.0f\r\n", MesRecord.PackageNumber);
		Output += strMsg;

		strMsg.Format("芯片类型:%s\r\n", MesRecord.bIseMMC ? "eMMC" : "非eMMC");
		Output += strMsg;

		if (MesRecord.strProgFile) {
			strMsg.Format("烧录档案所在路径:%s\r\n", MesRecord.strProgFile);
			Output += strMsg;
		}
		if (MesRecord.strChecksumFrom) {
			strMsg.Format("校验值来源:%s\r\n", MesRecord.strChecksumFrom);
			Output += strMsg;
		}
		if (MesRecord.strChecksumType) {
			strMsg.Format("校验值类型:%s\r\n", MesRecord.strChecksumType);
			Output += strMsg;
		}
		if (MesRecord.strChecksum) {
			strMsg.Format("校验值:%s\r\n", MesRecord.strChecksum);
			Output += strMsg;
		}

		if (MesRecord.bProjAutoCreateSet) {
			strMsg.Format("是否自动合成工程:%s\r\n", MesRecord.bProjAutoCreate ? "是" : "否");
			Output += strMsg;
		}

		if (MesRecord.bstrTemplateFileSet) {
			strMsg.Format("工程模板名称:%s\r\n", MesRecord.strTemplateFile);
			Output += strMsg;
		}

		if (MesRecord.bstrProjFileSet) {
			strMsg.Format("工程文件名称:%s\r\n", MesRecord.strProjFile);
			Output += strMsg;
		}

		if (MesRecord.bstrProjectChecksumSet) {
			strMsg.Format("工程文件校验值:%s\r\n", MesRecord.strProjectChecksum);
			Output += strMsg;
		}

		if (MesRecord.bstrAutoTaskSet) {
			strMsg.Format("自动化任务数据文件名称:%s\r\n", MesRecord.strAutoTask);
			Output += strMsg;
		}



		if (MesRecord.vExtItem.size() > 0) {
			int k;
			strMsg.Format("======扩展项信息======\r\n");
			Output += strMsg;
			for (k = 0; k < (INT)MesRecord.vExtItem.size(); k++) {
				strMsg.Format("%s : %s\r\n", MesRecord.vExtItem[k].strItemName, MesRecord.vExtItem[k].strItemValue);
				Output += strMsg;
			}
		}
		strMsg.Format("===========================================================\r\n");
		Output += strMsg;
	}
	else {
		strMsg.Format("===============MES未使能，没有信息可提供===================\r\n");
		Output += strMsg;
	}
	strMsg.Format("备注:如果MES和EXCEL都指定了烧录档案，则以MES为准.\r\n");
	Output += strMsg;
	strMsg.Format("MES接口返回的烧录档案可能位于服务器上，如果不能从服务器上正常下载,\r\n则可以通过先保存到本地ProgFile文件夹下，然后通过EXCEL指定.\r\n");
	Output += strMsg;
	strMsg.Format("如果MES和EXCEL都指定了校验值信息，则以MES为准\r\n");
	Output += strMsg;
	return Output;
}

void CProgRecord::ResetDestRecord()
{
	DestRecord.strProjPath = "";
	DestRecord.ExpertIDNum = 0; ///最后芯片烧录数量
	DestRecord.strAutoTaskPath=""; ///自动化路径
	DestRecord.bNeedCompareProjChecksum = FALSE; ///需要比对工程的校验值
	DestRecord.ProjChecksumType = ""; ///工程文件校验值类型
	DestRecord.strProjChecksum = "";	////工程的校验值
	DestRecord.ProjChecksumProvider = "";
	DestRecord.bNeedComapreFileChecksum = FALSE; ///需要比对文件的校验值
	DestRecord.FileChecksumType = "";///烧录文件校验值类型
	DestRecord.strProgFileChecksum = "";	////烧录文件的校验值
	DestRecord.FileChecksumProvider = "";
}

void CProgRecord::ResetExcelRecord()
{
	ExcelRecord.IsFromExcel = FALSE;
	ExcelRecord.strWorkOrder = "";	///工单号
	ExcelRecord.strPackageNo = ""; ///包装号
	ExcelRecord.bProjAutoCreate=FALSE; ///自动合成工程文件
	ExcelRecord.strProgFile= ""; ///烧录档案名称
	ExcelRecord.strTemplateFile = ""; //工程模板名称
	ExcelRecord.strProjFile = ""; //工程文件名称
	ExcelRecord.strChecksumFrom = ""; //校验值来源
	ExcelRecord.strChecksumType=""; //校验值类型
	ExcelRecord.strChecksum = ""; ///校验值
	ExcelRecord.strAutoTask = "";//自动化任务数据文件名称
	ExcelRecord.vExtItem.clear();
}

void CProgRecord::ResetMesRecord()
{
	MesRecord.strWorkOrder="";
	MesRecord.WorkOrderLotNumber=0;
	MesRecord.strPackageNo = ""; ///包装号
	MesRecord.PackageNumber=0; ///包装数
	MesRecord.strProgFile = ""; ///烧录档案位置
	MesRecord.strChecksumFrom = ""; ///校验值来源
	MesRecord.strChecksumType = ""; ///校验值类型
	MesRecord.strChecksum = ""; ///校验值
	MesRecord.bIseMMC=FALSE; ///是否是eMMC

	MesRecord.bProjAutoCreateSet = FALSE;
	MesRecord.bstrProjectChecksumSet = FALSE;
	MesRecord.bstrTemplateFileSet = FALSE;
	MesRecord.bstrAutoTaskSet = FALSE;
	MesRecord.bstrProjFileSet = FALSE;

	MesRecord.bProjAutoCreate=FALSE; ///自动合成工程文件
	MesRecord.strTemplateFile=""; //工程模板名称
	MesRecord.strProjFile=""; //工程文件名称
	MesRecord.strProjectChecksum=""; ///校验值
	MesRecord.strAutoTask="";//自动化任务数据文件名称
	MesRecord.vExtItem.clear();
}

void CProgRecord::ResetEMMCMIDRecord() {
	EMMCMIDRecord.strMID = "";
	EMMCMIDRecord.LimitCnt = 0;
	EMMCMIDRecord.CurrentCnt = 0;

}
