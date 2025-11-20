#pragma once
#include "IMesAccess.h"
class CACMesAccess :
	public IMesAccess
{
public:
	virtual BOOL AttachDll(CString strDllPath, BOOL bSet);
	virtual BOOL DetachDll();
	CACMesAccess();
	~CACMesAccess();

private:
	CString m_strDllPath;
	HINSTANCE hLib;
};

