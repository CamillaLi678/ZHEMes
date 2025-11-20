#include "stdafx.h"
#include "ProgFileCheckMD5.h"
#include "MD5.hpp"
#include "ComTool.h"

CProgFileCheckMD5::CProgFileCheckMD5()
{
}


CProgFileCheckMD5::~CProgFileCheckMD5()
{
}

INT CProgFileCheckMD5::CheckFile(CString strFilePath, CString strDesiredValue)
{
	INT Ret = 0;
	MD5_CTX md5;
	INT64 FileLen,Offset=0,BytesRead,BytesRtn;
	BYTE *pData = NULL;
	INT TmpBufsize = 2 * 1024 * 1024;
	INT strLen;
	UCHAR *pStrHex = NULL;
	CFile File;
	unsigned char decrypt[16]; ///16个字节的输出结果
	memset(&md5, 0, sizeof(MD5_CTX));
	memset(decrypt, 0, 16);
	pData = new BYTE[TmpBufsize];
	if (!pData) {
		Ret = -1;
		goto __end;
	}
	MD5Init(&md5);

	if (File.Open(strFilePath, CFile::modeRead, NULL) == FALSE) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "烧录档案打开错误，请确认档案路径是否存在并没有被其他进程打开\r\n");
		m_pILog->PrintLog(LOGLEVEL_ERR, "烧录档案:%s\r\n",strFilePath);
		Ret = -1;goto __end;
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
		MD5Update(&md5, pData, (unsigned int)BytesRtn);//只是个中间步骤
		Offset += BytesRead;
	}

	MD5Final(&md5, decrypt);//16字节的输出结果

	pStrHex=Hex2Str(decrypt, 16, &strLen);
	if (pStrHex) {
		CString strMD5;
		strMD5.Format("%s", pStrHex);
		if (strMD5.CompareNoCase(strDesiredValue) != 0) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "烧录档案MD5值比对错误\r\n");
			m_pILog->PrintLog(LOGLEVEL_ERR, "期望值:%s\r\n",strDesiredValue.MakeUpper());
			m_pILog->PrintLog(LOGLEVEL_ERR, "实际值:%s\r\n", strMD5.MakeUpper());
			Ret = -1; goto __end;
		}
		else {
			m_pILog->PrintLog(LOGLEVEL_LOG, "烧录档案MD5值比对成功:%s\r\n",strDesiredValue.MakeUpper());
		}
	}
	else {
		m_pILog->PrintLog(LOGLEVEL_ERR, "MD5值Hex转Str错误\r\n");
		Ret = -1; goto __end;
	}

__end:
	if (pStrHex) {
		delete[] pStrHex;
	}
	if (pData) {
		delete[] pData;
	}
	if (File.m_hFile != CFile::hFileNull) {
		File.Close();
	}
	return Ret;
}
