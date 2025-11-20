#include "stdafx.h"
#include "IProgFileCheck.h"
#include "ProgFileCheckMD5.h"
#include "ProgFileCheckByte.h"

IProgFileCheck::IProgFileCheck()
{
}


IProgFileCheck::~IProgFileCheck()
{
}

IProgFileCheck * GetProgFileCheckFactory(CString CheckerName)
{
	IProgFileCheck *pProgFile = NULL;
	if (CheckerName.CompareNoCase("MD5")==0){	
		pProgFile = new CProgFileCheckMD5();
	}
	else if (CheckerName.CompareNoCase("Byte") == 0) {
		pProgFile = new CProgFileCheckByte();
	}
	return  pProgFile;
}

void PutProgFileCheckFactory(IProgFileCheck *pProgFileCheck)
{
	if (pProgFileCheck) {
		delete pProgFileCheck;
	}
}
