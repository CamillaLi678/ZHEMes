#pragma once

#include <map>
#include <vector>

class ILog;
/*
	实现烧录过程中烧录结果的文件缓存，缓存内容格式自定，以后就可以扫描什么内容
	用于与MES系统上传过程出现异常，下次启动，扫描后上传MES
*/
class CFileCacheManager {
	// 单例模式
public:
	~CFileCacheManager();
	static CFileCacheManager& getInstance()   // 注意调用此函数一定要使用 & 接收返回值，因为 拷贝构造函数已经被禁止
	{
		static CFileCacheManager _centralcache; // 懒汉式    // 利用了 C++11 magic static 特性，确保此句并发安全
		return _centralcache;
	}
	//添加缓存文件内容
	INT SaveResult2File(CString filename, CString content);
	//删除缓存文件内容
	INT RemoveResultFile(CString filename);
	//返回缓存的文件列表
	INT GetCacheFileList(CString strExtName, std::map<CString, CString> &ResultMap);
	void Init(ILog *log);
	void DeInit();
private:
	CFileCacheManager();
	CFileCacheManager(const CFileCacheManager&) = delete;
	CFileCacheManager& operator=(const CFileCacheManager&) = delete;
	BOOL ScanFiles(CString& dir, CString strExtName, CString strPrefix, std::vector<CString> &v_Files);

	INT AttachFile(CString strFilePath);
	void WriteMsgToLog(CString fullFileName, CString&strMsg);
	INT DetachFile();

	ILog *m_pILog;
	BOOL m_bInited;
	CMutex m_FileMutex;
	CString m_strDateCacheFolder;

};