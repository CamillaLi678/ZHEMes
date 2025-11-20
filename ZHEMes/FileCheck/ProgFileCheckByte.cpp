#include "stdafx.h"
#include "ProgFileCheckByte.h"


CProgFileCheckByte::CProgFileCheckByte()
{
}


CProgFileCheckByte::~CProgFileCheckByte()
{
}

INT CProgFileCheckByte::CheckFile(CString strFilePath, CString strDesiredValue)
{
	INT Ret = 0;
	INT64 FileLen, Offset = 0, BytesRead, BytesRtn;
	BYTE *pData = NULL;
	INT TmpBufsize = 2 * 1024 * 1024;
	UCHAR *pStrHex = NULL;
	CFile File;
	UINT ByteSum = 0;
	UINT Checksumdesired;
	CString strDesired;
	pData = new BYTE[TmpBufsize];
	if (!pData) {
		Ret = -1;
		goto __end;
	}

	if (File.Open(strFilePath, CFile::modeRead, NULL) == FALSE) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "烧录档案打开错误，请确认档案路径是否存在并没有被其他进程打开\r\n");
		m_pILog->PrintLog(LOGLEVEL_ERR, "烧录档案:%s\r\n", strFilePath);
		Ret = -1; goto __end;
	}

	FileLen = File.GetLength();
	while (Offset < FileLen) {
		BytesRead = FileLen - Offset;
		if (BytesRead > TmpBufsize) {
			BytesRead = TmpBufsize;
		}

		BytesRtn = File.Read(pData, (UINT)BytesRead);
		if (BytesRtn != BytesRead) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "烧录档案读取错误,希望读取0x%X字节，实际返回0x%X字节\r\n", (UINT)BytesRead, BytesRtn);
			Ret = -1; goto __end;
		}
		for (int i = 0; i < BytesRead; ++i) {
			ByteSum += pData[i];
		}
		Offset += BytesRead;
	}

	strDesired = strDesiredValue.Trim("0x");
	sscanf(strDesired.GetBuffer(), "%X", &Checksumdesired);
	if (Checksumdesired != ByteSum) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "烧录档案Bytesum值比对错误\r\n");
		m_pILog->PrintLog(LOGLEVEL_ERR, "期望值:0x%08X\r\n",Checksumdesired);
		m_pILog->PrintLog(LOGLEVEL_ERR, "实际值:0x%08X\r\n", ByteSum);
		Ret = -1; goto __end;
	}


__end:

	if (pData) {
		delete[] pData;
	}
	if (File.m_hFile != CFile::hFileNull) {
		File.Close();
	}
	return Ret;
}
