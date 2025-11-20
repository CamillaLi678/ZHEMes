#pragma once
class IStatus
{
public:
	IStatus();
	virtual ~IStatus();
	virtual void PrintStatus(INT Level,CString strMsg)=0;
};

