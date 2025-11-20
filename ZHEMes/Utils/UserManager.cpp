#include "stdafx.h"
#include "UserManager.h"
#include "DlgUserManager.h"
#include "AES/ChkSum.h"
#include "AES/AES.h"
#include "ComTool.h"
#include <vector>
#include <fstream>

UINT8 ENCRYPT_KEY[16] = { 0xF1,0x02,0xB7,0xA4,0x78,0x21,0x82,0xC9,0x04,0x95,0xA1,0xC3,0xB5,0x34,0x2B,0xE1 };

CUserManager::CUserManager() {

}

CUserManager::~CUserManager() {
	DeInit();
}

void CUserManager::Init(ILog *log, const char *username, const char *password, const char * role) {

	DeInit();
	if (!m_bInited) {
		m_pILog = log;
		m_configFilePath.Format("%s\\user.bin", GetCurrentPath());
		m_strUserName = "";
		m_bInited = TRUE;
		Load();
		/*AddUser(username, password, role);
		AddRole(role, ADMIN_PERMISSION);*/
	}
}

void CUserManager::DeInit() {
	if (m_bInited) {
		m_bInited = FALSE;
		m_strUserName = _T("");
	}
}

INT CUserManager::GetUserCnt()
{
	INT UserCnt = 0;
	if (!m_bInited) {
		return -1;
	}
	for (UINT i = 0; i < MAX_ITEM_INFO; i++) {
		if (m_UserList.m_users[i].m_UserName[0] != 0) {
			UserCnt++;
		}
	}

	return UserCnt;
}

INT CUserManager::GetUserList(std::vector<UserInfo> &userList)
{
	userList.clear();
	if (!m_bInited) {
		return -1;
	}
	for (UINT i = 0; i < MAX_ITEM_INFO; i++) {
		if (GetUserRole().CompareNoCase("Admin") ==0){
			if (m_UserList.m_users[i].m_UserName[0] != 0) {
				userList.push_back(m_UserList.m_users[i]);
			}
		}
		else{
			if (m_UserList.m_users[i].m_UserName[0] != 0) {
				if (strcmp((const char *)m_UserList.m_users[i].m_UserName, m_strUserName) == 0 ){
					userList.push_back(m_UserList.m_users[i]);
				}
			}

		}

	}
	return 0;
}

INT CUserManager::AddUser(const char *username, const char *password, const char * role) {
	INT Ret = -1;
	if (!m_bInited) {
		return Ret;
	}
	if (username == NULL || password == NULL) {
		return Ret;
	}

	for (UINT i = 0; i < MAX_ITEM_INFO; i++) {
		if (strcmp((const char *)m_UserList.m_users[i].m_UserName, username) == 0) {
			//m_pILog->PrintLog(LOGLEVEL_LOG, "AddUser Fail：Have same user:%s", username);
			return Ret;
		}
	}

	UINT i = 0;
	for (i = 0; i < MAX_ITEM_INFO; i++) {
		if (m_UserList.m_users[i].m_UserName[0] == 0) {
			strcpy((char *)m_UserList.m_users[i].m_UserName, username);
			strcpy((char *)m_UserList.m_users[i].m_UserPassword, password);
			strcpy((char *)m_UserList.m_users[i].m_Role, role);
			Ret = 0;
			m_pILog->PrintLog(LOGLEVEL_LOG, "AddUser：%s", username);
			break;
		}
	}

	return Ret;
}

UINT CUserManager::CheckUser(const char *username, const char *password) {
	INT8 role[64];
	for (UINT i = 0; i < MAX_ITEM_INFO; i++) {
		if (strcmp((const char *)m_UserList.m_users[i].m_UserName, username) == 0 &&
			strcmp((const char *)m_UserList.m_users[i].m_UserPassword, password) == 0) {
			strcpy((char *)role, (char *)m_UserList.m_users[i].m_Role);
			m_strUserName = username;
			return 1;
		}
	}
	return 0;
}

INT CUserManager::HavePermission(const char *username, UINT permission) {
	for (UINT i = 0; i < MAX_ITEM_INFO; i++) {
		if (strcmp((const char *)m_UserList.m_users[i].m_UserName, username) == 0) {
			if ((GetPermission((const char *)m_UserList.m_users[i].m_Role) & permission) > 0) {
				return 1;
			}
			else {
				return 0;
			}
		}
	}
	return -1;
}

CString CUserManager::GetUserRole() {
	for (UINT i = 0; i < MAX_ITEM_INFO; i++) {
		if (strcmp((const char *)m_UserList.m_users[i].m_UserName, m_strUserName) == 0) {
			return (const char *)m_UserList.m_users[i].m_Role;
		}
	}
	return 0;
}

UINT64 CUserManager::GetPermission(const char * role) {
	for (UINT i = 0; i < MAX_ITEM_INFO; i++) {
		if (strcmp((const char *)m_UserList.m_roles[i].m_Role, role) == 0) {
			return m_UserList.m_roles[i].m_Permission;
		}
	}
	return 0;
}

INT CUserManager::UpdateUser(const char *username, const char *password, const char * role) {
	INT Ret = -1;
	if (!m_bInited) {
		return Ret;
	}
	for (UINT i = 0; i < MAX_ITEM_INFO; i++) {
		if (strcmp((const char *)m_UserList.m_users[i].m_UserName, username) == 0) {
			//strcpy((char *)m_UserList.m_users[i].m_UserName, username);
			if (password != NULL){
				strcpy((char *)m_UserList.m_users[i].m_UserPassword, password);
			}

			strcpy((char *)m_UserList.m_users[i].m_Role, role);
			Ret = 0;
			m_pILog->PrintLog(LOGLEVEL_LOG, "UpdateUser：%s", username);
			break;
		}
	}
	return Ret;
}


INT CUserManager::RemoveUser(const char *username) {
	INT Ret = -1;
	if (!m_bInited) {
		return Ret;
	}
	if (username == NULL) {
		return Ret;
	}
	for (UINT i = 0; i < MAX_ITEM_INFO; i++) {
		if (strcmp((const char *)m_UserList.m_users[i].m_UserName, username) == 0) {
			strcpy((char *)m_UserList.m_users[i].m_UserName, "");
			strcpy((char *)m_UserList.m_users[i].m_UserPassword, "");
			strcpy((char *)m_UserList.m_users[i].m_Role, "");
			Ret = 0;
			m_pILog->PrintLog(LOGLEVEL_LOG, "RemoveUser：%s", username);
			break;
		}
	}
	return Ret;
}


INT CUserManager::GetRoleList(std::vector<RoleInfo> &roleList) {
	roleList.clear();
	if (!m_bInited) {
		return -1;
	}
	for (UINT i = 0; i < MAX_ITEM_INFO; i++) {
		if (m_UserList.m_roles[i].m_Role[0] != 0) {
			roleList.push_back(m_UserList.m_roles[i]);
		}
	}
	return 0;
}
INT CUserManager::AddRole(const char *role, UINT64 permission) {
	INT Ret = -1;
	if (!m_bInited) {
		return Ret;
	}
	if (role == NULL || permission == 0) {
		return Ret;
	}
	for (UINT i = 0; i < MAX_ITEM_INFO; i++) {
		if (strcmp((const char *)m_UserList.m_roles[i].m_Role, role) == 0) {
			//m_pILog->PrintLog(LOGLEVEL_LOG, "AddRole Fail：Have same role:%s", role);
			return Ret;
		}
	}

	UINT i = 0;
	for (i = 0; i < MAX_ITEM_INFO; i++) {
		if (m_UserList.m_roles[i].m_Role[0] == 0) {
			strcpy((char *)m_UserList.m_roles[i].m_Role, role);
			m_UserList.m_roles[i].m_Permission = permission;
			Ret = 0;
			m_pILog->PrintLog(LOGLEVEL_LOG, "AddRole：%s", role);
			break;
		}
	}

	return Ret;
}
INT CUserManager::UpdateRole(const char *role, UINT64 permission) {
	INT Ret = -1;
	if (!m_bInited) {
		return Ret;
	}
	for (UINT i = 0; i < MAX_ITEM_INFO; i++) {
		if (strcmp((const char *)m_UserList.m_roles[i].m_Role, role) == 0) {
			//strcpy((char *)m_UserList.m_roles[i].m_Role, role);
			m_UserList.m_roles[i].m_Permission = permission;
			Ret = 0;
			m_pILog->PrintLog(LOGLEVEL_LOG, "UpdateRole：%s", role);
			break;
		}
	}
	return Ret;
}
INT CUserManager::RemoveRole(const char *role) {
	INT Ret = -1;
	if (!m_bInited) {
		return Ret;
	}
	if (role == NULL) {
		return Ret;
	}
	for (UINT i = 0; i < MAX_ITEM_INFO; i++) {
		if (strcmp((const char *)m_UserList.m_users[i].m_Role, role) == 0) {
			m_pILog->PrintLog(LOGLEVEL_ERR, "RemoveRole Fail as user have this role：%s", role);
			return Ret;
		}
	}
	for (UINT i = 0; i < MAX_ITEM_INFO; i++) {
		if (strcmp((const char *)m_UserList.m_roles[i].m_Role, role) == 0) {
			strcpy((char *)m_UserList.m_roles[i].m_Role, "");
			m_UserList.m_roles[i].m_Permission = 0;
			Ret = 0;
			m_pILog->PrintLog(LOGLEVEL_LOG, "RemoveRole：%s", role);
			break;
		}
	}
	return Ret;
}

INT CUserManager::Load() {
	CFile inFile;
	INT Ret = -1;
	INT nCodeResult = 0;
	UINT BlockSize = BLOCKSIZE + 0x10;
	UserInfoList userInfo;
	UINT readLen = 0;
	CHKINFO chkInfo;
	UINT8 *inBuffer = NULL;
	UINT8 *outBuffer = NULL;

	if (!m_bInited) {
		return Ret;
	}
	UINT userInfolen = sizeof(UserInfoList);
	UINT userInfoHeaderlen = sizeof(UserInfoHeader);
	if (!inFile.Open(m_configFilePath, CFile::modeRead | CFile::typeBinary)) {
		//m_pILog->PrintLog(LOGLEVEL_ERR, "Load Open Fail：%s", m_configFilePath);
		goto _end;
	}

	readLen = inFile.Read((char*)&userInfo, userInfoHeaderlen);
	if (readLen != userInfoHeaderlen) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "Load Read Fail：%d", readLen);
		goto _end;
	}

	if (!userInfo.isValidHeader()) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "Load InValid Header");
		goto _end;
	}

	memset(&chkInfo, 0, sizeof(CHKINFO));


	inBuffer = new UINT8[BlockSize];
	if (inBuffer == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "Load new Fail");
		goto _end;
	}
	outBuffer = new UINT8[BlockSize];
	if (outBuffer == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "Load new Fail");
		goto _end;
	}
	// 加密文件

	memset(inBuffer, 0, BlockSize);
	memset(outBuffer, 0, BlockSize);
	INT len = inFile.Read((char*)inBuffer, BlockSize);
	if (len == 0) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "Load Read Fail");
		goto _end;
	}

	nCodeResult = aesDecryptUseOutBuff(ENCRYPT_KEY, sizeof(ENCRYPT_KEY), inBuffer, len, outBuffer, BlockSize);
	if (nCodeResult > 0) {
		Crc32CalcSubRoutine(&chkInfo, (UINT8*)inBuffer, len);
		memcpy((char *)&userInfo + userInfoHeaderlen, (char *)outBuffer, nCodeResult);
	}


	Crc32GetChkSum(&chkInfo);
	UINT crc32 = (UINT)chkInfo.chksum;
	if (crc32 != userInfo.m_header.m_CRC) {
		m_pILog->PrintLog(LOGLEVEL_LOG, "Load different crc32");
		goto _end;
	}

	memcpy(&m_UserList, &userInfo, sizeof(UserInfoList));
	Ret = 0;

_end:
	if (inBuffer) {
		delete inBuffer;
		inBuffer = NULL;
	}
	if (outBuffer) {
		delete outBuffer;
		outBuffer = NULL;
	}
	if (inFile.m_hFile != CFile::hFileNull) {
		inFile.Close();
	}
	return Ret;
}

INT CUserManager::Save() {
	std::ofstream outFile(m_configFilePath, std::ios::binary);
	INT Ret = -1;
	INT nCodeResult = 0;

	//写头
	UINT userInfolen = sizeof(UserInfoList);
	UINT userInfoHeaderlen = sizeof(UserInfoHeader);
	CHKINFO chkInfo;
	UINT8 *inBuffer = NULL;
	UINT8 *outBuffer = NULL;

	if (!m_bInited) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "Save Not Inited");
		return Ret;
	}
	//跳过m_Magic,m_CRC，开始计算CRC
	memset(&chkInfo, 0, sizeof(CHKINFO));
	outFile.write((char*)&m_UserList.m_header, userInfoHeaderlen);

	ULONG nBufferOffset = userInfoHeaderlen;
	INT outLen = BLOCKSIZE * 3;
	outBuffer = new UINT8[BLOCKSIZE * 3];
	if (outBuffer == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "Save New Fail");
		goto _end;
	}
	// 加密文件

	memset(outBuffer, 0, outLen);
	nCodeResult = aesEncryptUseOutBuff(ENCRYPT_KEY, sizeof(ENCRYPT_KEY), (UINT8 *)m_UserList.m_users, userInfolen - userInfoHeaderlen, outBuffer, outLen);
	if (nCodeResult <= 0) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "Save aesEncrypt Fail");
		goto _end;
	}
	outFile.write((char*)outBuffer, nCodeResult);
	nBufferOffset += nCodeResult;
	Crc32CalcSubRoutine(&chkInfo, outBuffer, nCodeResult);

	//跳过m_Magic，重新写入CRC
	Crc32GetChkSum(&chkInfo);
	m_UserList.m_header.m_CRC = (UINT32)chkInfo.chksum;
	outFile.seekp(12, outFile.beg);
	outFile.write((char*)&m_UserList.m_header.m_CRC, 4);

	Ret = nBufferOffset;
_end:
	if (outBuffer) {
		delete outBuffer;
		outBuffer = NULL;
	}
	return Ret;
}

void CUserManager::Dump() {
	if (!m_bInited) {
		return;
	}
	for (UINT i = 0; i < MAX_ITEM_INFO; i++) {
		if (m_UserList.m_users[i].m_UserName[0] != 0) {
			m_pILog->PrintLog(LOGLEVEL_LOG, "用户信息：%s %s %s", m_UserList.m_users[i].m_UserName,
				m_UserList.m_users[i].m_UserPassword, m_UserList.m_users[i].m_Role);

		}
	}
}

void CUserManager::Test() {

	Dump();
	
	//RemoveUser("123");
	Dump();

	//INT Ret = AddUser("123", "456", ROLE_LOGIN);
	//Ret = AddUser("123123", "456456", ROLE_SETTING);
	//Ret = AddUser("123", "456", ROLE_USER);
	Save();

	//UpdateUser("123123", "456", ROLE_SETTING);
	Dump();
	Save();

}