#include "stdafx.h"
#include "ComTool.h"

//-------------------------------------------------------------------------------------
//Description:
// This function maps a character string to a new utf8 character string
//
//Parameters:
// lpcszStr: [in] Pointer to the character string to be converted
// lpszStr: [out] Pointer to a buffer that receives the translated string.
// dwSize: [in] Size of the lpszStr buffer
// SizeUsed: [out] Size of the buffer be used when translate successfully   ///多少个字节单位包含一个字节0结尾
//
//Return Values:
// 1: Succeed
// 0: Failed
// -1：Need More Buffer
//
//Example:
// MByteToUtf8(szA,szUTF8,sizeof(szUTF8)/sizeof(szUTF8[0]),SizeUsed);
//---------------------------------------------------------------------------------------
INT MByteToUtf8(LPCSTR lpcszStr, LPSTR lpszStr, DWORD dwSize, DWORD& SizeUsed)
{
	INT Ret = 1;
	DWORD dwMinSize;
	LPWSTR lpwszStr = NULL;
	///MBSC->Unicode
	dwMinSize = MultiByteToWideChar(CP_ACP, 0, lpcszStr, -1, NULL, 0);///先转Unicode
	lpwszStr = new WCHAR[dwMinSize];
	if (!lpwszStr) {
		Ret = 0; goto __end;
	}
	MultiByteToWideChar(CP_ACP, 0, lpcszStr, -1, lpwszStr, dwMinSize);

	///Unicode->UTF8
	dwMinSize = WideCharToMultiByte(CP_UTF8, NULL, lpwszStr, -1, NULL, 0, NULL, FALSE);
	if (dwSize < dwMinSize) {
		Ret = -1; goto __end;
	}
	SizeUsed = WideCharToMultiByte(CP_UTF8, NULL, lpwszStr, -1, lpszStr, dwSize, NULL, FALSE);

__end:
	if (lpwszStr) {
		delete[] lpwszStr;
	}
	return Ret;
}


//-------------------------------------------------------------------------------------
//Description:
// This function maps a utf8  character string to a new utf8 character string
//
//Parameters:
// lpcszStr: [in] Pointer to the character string to be converted
// lpszStr: [out] Pointer to a buffer that receives the translated string.
// dwSize: [in] Size of the lpszStr buffer
// SizeUsed: [out] Size of the buffer be used when translate successfully   ///多少个字节单位包含一个字节0结尾

//
//Return Values:
// 1: Succeed
// 0: Failed
// -1：Need More Buffer
//
//Example:
// Utf8ToMByte(szUTF8,szA,sizeof(szA)/sizeof(szA[0]),SizeUsed);
//---------------------------------------------------------------------------------------
INT Utf8ToMByte(LPCSTR lpcszStr, LPSTR lpszStr, DWORD dwSize, DWORD& SizeUsed)
{
	INT Ret = 1;
	DWORD dwMinSize;
	LPWSTR lpwszStr = NULL;
	///Utf8-->Unicode
	dwMinSize = MultiByteToWideChar(CP_UTF8, 0, lpcszStr, -1, NULL, 0);
	lpwszStr = new WCHAR[dwMinSize];
	if (!lpwszStr) {
		Ret = 0; goto __end;
	}
	MultiByteToWideChar(CP_UTF8, 0, lpcszStr, -1, lpwszStr, dwMinSize);

	///Unicode->MBCS
	dwMinSize = WideCharToMultiByte(CP_OEMCP, NULL, lpwszStr, -1, NULL, 0, NULL, FALSE);
	if (dwSize < dwMinSize) {
		Ret = -1; goto __end;
	}
	SizeUsed = WideCharToMultiByte(CP_OEMCP, NULL, lpwszStr, -1, lpszStr, dwSize, NULL, FALSE);

__end:
	if (lpwszStr) {
		delete[] lpwszStr;
	}
	return Ret;
}

//-------------------------------------------------------------------------------------
//Description:
// This function maps a character string to a wide-character (Unicode) string
//
//Parameters:
// lpcszStr: [in] Pointer to the character string to be converted
// lpwszStr: [out] Pointer to a buffer that receives the translated string.
// dwSize: [in] Size of the buffer
// SizeUsed: [out] Size of the buffer be used when translate successfully   ///多少个2个字节单位包含2个字节的0结尾
//
//Return Values:
// 1: Succeed
// 0: Failed
// -1：Need More Buffer
//
//Example:
// MByteToWChar(szA,szW,sizeof(szW)/sizeof(szW[0]),SizeUsed);
//---------------------------------------------------------------------------------------
INT MByteToWChar(LPCSTR lpcszStr, LPWSTR lpwszStr, DWORD dwSize, DWORD& SizeUsed)
{
	// Get the required size of the buffer that receives the Unicode
	// string.
	DWORD dwMinSize;
	dwMinSize = MultiByteToWideChar(CP_ACP, 0, lpcszStr, -1, NULL, 0);
	if (dwSize < dwMinSize) {
		return -1;
	}
	// Convert headers from ASCII to Unicode.
	SizeUsed = MultiByteToWideChar(CP_ACP, 0, lpcszStr, -1, lpwszStr, dwMinSize);
	return 1;
}


//-------------------------------------------------------------------------------------
//Description:
// This function maps a wide-character string to a new character string
//
//Parameters:
// lpcwszStr: [in] Pointer to the character string to be converted
// lpszStr: [out] Pointer to a buffer that receives the translated string.
// dwSize: [in] Size of the buffer
// SizeUsed: [out] Size of the buffer be used when translate successfully   ///多少个字节单位包含一个字节0结尾

//
//Return Values:
// 1: Succeed
// 0: Failed
// -1：Need More Buffer
//
//Example:
// MByteToWChar(szW,szA,sizeof(szA)/sizeof(szA[0]),SizeUsed);
//---------------------------------------------------------------------------------------
INT WCharToMByte(LPCWSTR lpcwszStr, LPSTR lpszStr, DWORD dwSize, DWORD& SizeUsed)
{
	DWORD dwMinSize;
	dwMinSize = WideCharToMultiByte(CP_ACP, NULL, lpcwszStr, -1, NULL, 0, NULL, FALSE);
	if (dwSize < dwMinSize) {
		return -1;
	}
	SizeUsed = WideCharToMultiByte(CP_ACP, NULL, lpcwszStr, -1, lpszStr, dwSize, NULL, FALSE);
	return 1;
}


CString GetCurTime(char Seperator)
{
	CString strTime;
	/*CTime CurTime;
	CurTime = CTime::GetCurrentTime();//获取当前系统时间
	strTime.Format("%04d%02d%02d_%02d%c%02d%c%02d", CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetHour(),
		Seperator, CurTime.GetMinute(), Seperator, CurTime.GetSecond());*/
	SYSTEMTIME st;
	GetLocalTime(&st);
	strTime.Format("%04d%02d%02d_%02d%c%02d%c%02d_%03d", st.wYear, st.wMonth, st.wDay, st.wHour,
		Seperator, st.wMinute, Seperator, st.wSecond, st.wMilliseconds);
	return strTime;
}

CString GetCurrentPath(void)
{
	TCHAR szFilePath[MAX_PATH + 1];
	TCHAR *pPos = NULL;
	CString str_url;
	GetModuleFileName(NULL, szFilePath, MAX_PATH);
	pPos = _tcsrchr(szFilePath, _T('\\'));
	if (pPos != NULL) {
		pPos[0] = 0;//删除文件名，只获得路径
		str_url = CString(szFilePath);
	}
	else {
		pPos = _tcsrchr(szFilePath, _T('/'));
		if (pPos == NULL) {
			str_url = "";
		}
		else {
			str_url = CString(szFilePath);
		}
	}
	return str_url;
}

CString GetModulePath(HMODULE hModule)
{
	TCHAR szFilePath[MAX_PATH + 1];
	TCHAR *pPos = NULL;
	CString str_url;
	GetModuleFileName(hModule, szFilePath, MAX_PATH);
	pPos = _tcsrchr(szFilePath, _T('\\'));
	if (pPos != NULL) {
		pPos[0] = 0;//删除文件名，只获得路径
		str_url = CString(szFilePath);
	}
	else {
		pPos = _tcsrchr(szFilePath, _T('/'));
		if (pPos == NULL) {
			str_url = "";
		}
		else {
			str_url = CString(szFilePath);
		}
	}
	return str_url;
}

CString GetFolderNameFromRelative(CString strFolderPath)
{
	INT i;
	INT Pos = 0;
	CString FolderName = "";
	char* strP = strFolderPath.GetBuffer();
	for (i = 0; i < strFolderPath.GetLength(); ++i, strP++) {
		if (*strP == '.' || *strP == '/' || *strP == '\\') {
			Pos++;
		}
		else {
			break;
		}
	}
	FolderName= strFolderPath.Mid(Pos, strFolderPath.GetLength());
	return FolderName;
}

CString GetFileNameWithSuffix(const CString & strFilePath)
{
	CString FileName = "";
	INT Pot = strFilePath.ReverseFind('.');
	INT Slash = strFilePath.ReverseFind('\\');

	if (Slash != -1 && Pot != -1) {
		FileName = strFilePath.Mid(Slash + 1, 256);
	}
	else {
		Slash = strFilePath.ReverseFind('/');
		if (Slash != -1 && Pot != -1) {
			FileName = strFilePath.Mid(Slash + 1, 256);
		}
		else {
			FileName = strFilePath;///全部是文件名
		}
	}
	return FileName;
}

CString GetFileName(const CString &strFilePath)
{
	CString FileName = "";
	INT Pot = strFilePath.ReverseFind('.');
	INT Slash = strFilePath.ReverseFind('\\');
	if (Slash != -1 && Pot != -1) {
		FileName = strFilePath.Mid(Slash + 1, Pot - Slash - 1);
	}
	else {
		if (Slash == -1) {
			Slash = strFilePath.ReverseFind('/');
			if (Slash != -1 && Pot != -1) {
				FileName = strFilePath.Mid(Slash + 1, Pot - Slash - 1);
			}
		}
		else {
			FileName = strFilePath;///全部是文件名
		}
	}
	return FileName;
}

/**
convert the hex to string
hex: the pointer where the digital stored
hexlen : the length of hex
str_len : return the length of the string buffer, end with '\0'
return :
the pointer of the string ,记得需要外部自行调用delete释放内存

example
hex=F2E32EC4
hexlen=4
return : F2E32EC4
str_len:9
**/
UCHAR* Hex2Str(UCHAR*hex, int hexlen, int *str_len)
{
	int i, ret = 0;
	unsigned char* tmpstr = NULL;
	if (hex == NULL || hexlen == 0 || str_len == NULL) {
		return NULL;
	}

	*str_len = hexlen * 2 + 1;
	tmpstr = new unsigned char[*str_len];
	if (tmpstr == NULL) {
		ret = -1;
		goto __end;
	}

	for (i = 0; i < hexlen; i++) {
		if ((hex[i] >> 4) >= 0xA) {//high 4 bits
			tmpstr[i * 2] = (hex[i] >> 4) + 'A' - 0xA;
		}
		else {
			tmpstr[i * 2] = (hex[i] >> 4) + '0';
		}

		if ((hex[i] & 0xF) >= 0xA) {///low 4 bits
			tmpstr[i * 2 + 1] = (hex[i] & 0xF) + 'A' - 0xA;
		}
		else {
			tmpstr[i * 2 + 1] = (hex[i] & 0xF) + '0';
		}
	}

	tmpstr[*str_len - 1] = 0;

__end:
	if (ret != 0) {
		if (tmpstr != NULL)
			delete[]tmpstr;

		tmpstr = NULL;
		*str_len = 0;
	}
	return tmpstr;
}

void Split(CString source, CStringArray& dest, CString division)
{
	dest.RemoveAll();
	int pos = 0;
	while (-1 != pos) {
		CString tmp = source.Tokenize(division, pos);
		if (!tmp.IsEmpty()) {
			dest.Add(tmp);
		}
	}
}

CString MByteStrToUtf8CStr(CString source) {
	CString ret = _T("");
	DWORD len = source.GetLength()*3;
	char *pUTF8Data = (char *)malloc(len);
	if (pUTF8Data == NULL) {
		return ret;
	}
	memset(pUTF8Data, 0, len);
	if (MByteToUtf8((LPCTSTR)source.GetBuffer(), pUTF8Data, len, len) != 1) {
		delete pUTF8Data;
		pUTF8Data = NULL;
		return ret;
	}
	ret.Format("%s", pUTF8Data);
	delete pUTF8Data;
	pUTF8Data = NULL;
	return ret;
}