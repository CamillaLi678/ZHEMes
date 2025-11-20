#pragma once
#include "IProgFileCheck.h"
class CProgFileCheckByte :
	public IProgFileCheck
{
public:
	CProgFileCheckByte();
	~CProgFileCheckByte();
	///确认档案是否正确
	INT CheckFile(CString strFilePath, CString strDesiredValue);
};

