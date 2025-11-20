#pragma once

///自动化任务数据类
class CAutoTaskData
{
public:
	CAutoTaskData();
	~CAutoTaskData();

	CString strAutoMachineType;
	CString strChipName;
	CString strAdapterName;
	CString strAdapterData;
	CString strTrayData;
	CString strTapeData;
	INT Lot;
	BOOL bTapeIn;
	INT XPos;
	INT YPos;
};

