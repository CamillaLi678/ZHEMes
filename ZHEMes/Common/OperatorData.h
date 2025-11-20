#pragma once
class COperatorData
{
public:
	COperatorData();
	~COperatorData();

	CString m_strOperator; ///操作员ID
	UINT m_Level;   ///操作等级
	BOOL m_bCheckAuthPass;	///确定权限通过。
	BOOL m_bAuthCancel; ///操作认证取消
};

