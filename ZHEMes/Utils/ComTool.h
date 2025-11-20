#pragma once
#include "stdafx.h"

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
INT MByteToUtf8(LPCSTR lpcszStr, LPSTR lpszStr, DWORD dwSize, DWORD& SizeUsed);

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
INT Utf8ToMByte(LPCSTR lpcszStr, LPSTR lpszStr, DWORD dwSize, DWORD& SizeUsed);


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
INT MByteToWChar(LPCSTR lpcszStr, LPWSTR lpwszStr, DWORD dwSize, DWORD& SizeUsed);


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
INT WCharToMByte(LPCWSTR lpcwszStr, LPSTR lpszStr, DWORD dwSize, DWORD& SizeUsed);
CString GetCurrentPath(void);
CString GetCurTime(char Seperator = ':');
CString GetModulePath(HMODULE hModule);

CString GetFileName(const CString &strFilePath);///给定一个全路径，返回文件名称，不包含后缀
CString GetFolderNameFromRelative(CString strFolderPath); ///去掉前面的相对路径，获取

CString GetFileNameWithSuffix(const CString	& strFilePath);///给定一个全路径，返回文件名称，包含后缀


///返回指向字符串的指针，需要用delete[]释放
UCHAR* Hex2Str(UCHAR*hex, int hexlen, int *str_len);///Hex2str

//==================================================================
//函数名： Split
//功能：   Split a cstring to cstring array. "1234 5678" to ["1234","5678"]
//输入参数：
//返回值：
//==================================================================
void Split(CString source, CStringArray& dest, CString division);

//==================================================================
//函数名： MByteStrToUtf8CStr
//功能：   MByte CString To Utf8 CString
//输入参数：
//返回值：
//==================================================================
CString MByteStrToUtf8CStr(CString source);