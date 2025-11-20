#pragma once

#include "LogEdit.h"
#include <vector>

#define USERINFO_VERSION "V1.0.4_230420"
#define BLOCKSIZE 128*1024  //AES-128分组长度为16字节
#define ENCRYPT_VESION 2
#define MAX_ITEM_INFO (16)


enum {
	FW_FILE_BIN = 1,
	FW_FILE_INTEL_HEX = 2,
};

struct UserInfo {
	INT8 m_Role[64];
	INT8 m_UserName[64];
	INT8 m_UserPassword[64];
};

struct RoleInfo {
	UINT64 m_Permission;
	INT8 m_Role[64];
};

struct UserInfoHeader {
	UINT8 m_Magic[4];
	UINT  m_Vesion;
	UINT  m_UserCnt;
	UINT  m_CRC;
};

struct UserInfoList {
public:
	UserInfoList() {
		memcpy(m_header.m_Magic, "ACSE", 4);
		m_header.m_Vesion = ENCRYPT_VESION;
		m_header.m_UserCnt = MAX_ITEM_INFO;
		m_header.m_CRC = 0;
		for (UINT i = 0; i < MAX_ITEM_INFO; i++) {
			memset(&m_users[i], 0, sizeof(UserInfo));
		}
		for (UINT i = 0; i < MAX_ITEM_INFO; i++) {
			memset(&m_roles[i], 0, sizeof(RoleInfo));
		}
	}
	~UserInfoList() {

	}
	BOOL isValidHeader() {
		if (m_header.m_Magic[0] != 'A' || m_header.m_Magic[1] != 'C' || m_header.m_Magic[2] != 'S' || m_header.m_Magic[3] != 'E') {
			return FALSE;
		}
		return TRUE;
	}
	UserInfoHeader m_header;
	UserInfo m_users[MAX_ITEM_INFO];
	RoleInfo m_roles[MAX_ITEM_INFO];
};

/* 此单件只负责保存和管理用户内容
 * 不负责password加密，role的管理
 */
class CUserManager
{
public:
	~CUserManager();
	static CUserManager& getInstance()   // 注意调用此函数一定要使用 & 接收返回值，因为 拷贝构造函数已经被禁止
	{
		static CUserManager _centralcache; // 懒汉式    // 利用了 C++11 magic static 特性，确保此句并发安全
		return _centralcache;
	}
	void Init(ILog *log, const char *username, const char *password, const char * role);
	void DeInit();
	INT Save();
	void Dump();
	//返回User的permission
	UINT CheckUser(const char *username, const char *password);
	INT HavePermission(const char *username, UINT permission);
	INT GetUserList(std::vector<UserInfo> &userList);
	INT GetUserCnt();
	INT AddUser(const char *username, const char *password, const char * role);
	INT UpdateUser(const char *username, const char *password, const char * role);
	INT RemoveUser(const char *username);
	CString GetUserRole();//获取当前登录的用户角色
	UINT64 GetPermission(const char * role);
	INT GetRoleList(std::vector<RoleInfo> &roleList);
	INT AddRole(const char *role, UINT64 permission);
	INT UpdateRole(const char *role, UINT64 permission);
	INT RemoveRole(const char *role);
	void Test();
private:
	CUserManager();
	CUserManager(const CUserManager&) = delete;
	CUserManager& operator=(const CUserManager&) = delete;
	INT Load();

	ILog *m_pILog;
	BOOL m_bInited;
	UserInfoList m_UserList;
	CString m_configFilePath;
	CString m_strUserName;
};
