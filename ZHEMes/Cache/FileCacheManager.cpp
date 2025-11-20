#include "stdafx.h"
#include "FileCacheManager.h"
#include "ILog.h"
#include "ComFunc.h"
#include "ComTool.h"
#include <direct.h>

#define SAVE_FOLDER "Cache"

CFileCacheManager::CFileCacheManager() {

}

CFileCacheManager::~CFileCacheManager() {
	DeInit();
}

void CFileCacheManager::Init(ILog *log) {

	DeInit();

	if (log == NULL) {
		return;
	}

	if (!m_bInited) {
		m_pILog = log;
		m_bInited = TRUE;

		m_strDateCacheFolder.Format("%s\\%s", ::GetCurrentPath(), SAVE_FOLDER);
		if (!CComFunc::IsDirExist(m_strDateCacheFolder)) {
			_mkdir(m_strDateCacheFolder);
		}
	}
}

void CFileCacheManager::DeInit() {
	if (m_bInited) {
		m_bInited = FALSE;
	}
}

void CFileCacheManager::WriteMsgToLog(CString fullFileName, CString&strMsg)
{
	if (!m_bInited) {
		return;
	}
	CFile writeFile;
	if (writeFile.Open(fullFileName, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone, NULL) == FALSE) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "Open File %s Error", fullFileName); 
		return;
	}
	if (writeFile.m_hFile != CFile::hFileNull) {
		writeFile.Write(strMsg, strMsg.GetLength());
	}
	if (writeFile.m_hFile != CFile::hFileNull) {
		writeFile.Flush();
		writeFile.Close();
	}
}

INT CFileCacheManager::SaveResult2File(CString filename, CString content) {
	if (!m_bInited) {
		return -1;
	}
	m_FileMutex.Lock();
	CString fullFileName = _T("");
	fullFileName.Format("%s\\%s", m_strDateCacheFolder, filename);
	WriteMsgToLog(fullFileName, content);
	m_FileMutex.Unlock();
	return 0;
}

INT CFileCacheManager::RemoveResultFile(CString filename)
{
	if (!m_bInited) {
		return -1;
	}
	CString fullFileName = _T("");
	fullFileName.Format("%s\\%s", m_strDateCacheFolder, filename);
	m_FileMutex.Lock();
	if (PathFileExists(fullFileName)) {
		remove(fullFileName);
		m_pILog->PrintLog(LOGLEVEL_LOG, "Remove File: %s", filename);
	}
	m_FileMutex.Unlock();
	return 0;
}

INT CFileCacheManager::GetCacheFileList(CString strExtName, std::map<CString, CString> &ResultMap)
{
	if (!m_bInited) {
		return -1;
	}
	std::vector<CString> v_Files;
	v_Files.clear();
	ScanFiles(m_strDateCacheFolder, strExtName, "", v_Files);
	for (UINT i = 0; i < v_Files.size(); i++) {
		CString FilePath = v_Files[i];
		CString FileName = CComFunc::GetFileNameWithSuffix(FilePath);

		CString strBuffer = _T("");
		CString strLine = _T("");
		CFile File;
		BYTE *pData = NULL;
		INT Size = 0;
		try {
			if (File.Open(FilePath, CFile::modeRead | CFile::shareDenyNone) == FALSE) {
				m_pILog->PrintLog(LOGLEVEL_ERR, "Open Fail %s", FilePath);
				continue;
			}
			Size = (INT)File.GetLength();
			pData = new BYTE[Size + 1];
			if (!pData) {
				if (File.m_hFile != CFile::hFileNull) {
					File.Close();
				}
				continue;
			}
			memset(pData, 0, Size + 1);
			File.Read(pData, Size);

			strBuffer.Format("%s", (pData));

			if (File.m_hFile != CFile::hFileNull) {
				File.Close();
			}
			if (pData) {
				delete[] pData;
			}
		}
		catch (...) {
			if (File.m_hFile != CFile::hFileNull) {
				File.Close();
			}
			if (pData) {
				delete[] pData;
			}
			m_pILog->PrintLog(LOGLEVEL_ERR, "file operate Fail %s", FilePath);
			continue;
		}

		m_pILog->PrintLog(LOGLEVEL_LOG, "GetLocalResultList FilePath:%s ReadString:%s", FilePath, strBuffer);
		ResultMap[FileName] = strBuffer;
	}
	return 0;
}

BOOL CFileCacheManager::ScanFiles(CString& dir, CString strExtName, CString strPrefix, std::vector<CString> &v_Files)
{
	if (!m_bInited) {
		return -1;
	}
	CFileFind ff;
	HANDLE file;
	CString path;
	CString strMsg;
	CString strSerchDir = dir;
	CString strSNFileDir = dir;
	WIN32_FIND_DATA pNextInfo;
	if (strSerchDir.Right(1) != "\\") {
		strSerchDir += "\\";
	}

	strSerchDir += "*.*";
	v_Files.clear();
	file = FindFirstFile(strSerchDir, &pNextInfo);
	if (file == INVALID_HANDLE_VALUE) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "Error when FindFirstFile strSerchDir: %s", strSerchDir);
		return FALSE;
	}
	while (FindNextFile(file, &pNextInfo)) {
		//m_pILog->PrintLog(LOGLEVEL_LOG, "FindNextFile cFileName: %s", pNextInfo.cFileName);
		if (pNextInfo.cFileName[0] == '.')
			continue;
		path.Format("%s\\%s", strSNFileDir, pNextInfo.cFileName);
		CString strExt = CComFunc::GetFileExt(path);
		CString strFileName = pNextInfo.cFileName;
		m_pILog->PrintLog(LOGLEVEL_LOG, "FindNextFile strExt: %s strExtName:%s strFileName:%s strPrefix:%s", strExt, strExtName, strFileName, strPrefix);
		if (strExt.CompareNoCase(strExtName) == 0) {//符合条件大小的才能进入
			m_pILog->PrintLog(LOGLEVEL_LOG, "FindNextFile push_back path:%s", path);
			v_Files.push_back(path);
		}
	}
	//m_pILog->PrintLog(LOGLEVEL_LOG, "ScanFiles End: %s", dir);
	return TRUE;
}