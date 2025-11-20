#pragma once
#include "IProgFileCheck.h"
class CProgFileCheckMD5 :
	public IProgFileCheck
{
public:
	CProgFileCheckMD5();
	~CProgFileCheckMD5();

	///确认档案是否正确
	INT CheckFile(CString strFilePath, CString strDesiredValue);
};

