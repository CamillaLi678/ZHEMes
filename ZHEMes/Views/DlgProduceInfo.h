#pragma once


// CDlgProduceInfo 对话框

class CDlgProduceInfo : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgProduceInfo)

public:
	CDlgProduceInfo(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgProduceInfo();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGPRODUCEINFO };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void SetProduceInfo(CString strProduceInfo);
private:
	CString m_strInfo;
};
