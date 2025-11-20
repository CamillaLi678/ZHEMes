#pragma once
#include "ILog.h"
////烧录档案确认接口
class IProgFileCheck
{
public:
	IProgFileCheck();
	virtual ~IProgFileCheck();

	void AttachILog(ILog*pLog) { m_pILog = pLog; }
	///确认档案是否正确
	virtual INT CheckFile(CString strFilePath, CString strDesiredValue)=0;
protected:
	ILog *m_pILog;
};


IProgFileCheck *GetProgFileCheckFactory(CString CheckerName);
void PutProgFileCheckFactory(IProgFileCheck *pProgFileCheck);

