
// PEVersionUpdateDlg.h: 头文件
//

#pragma once


// CPEVersionUpdateDlg 对话框
class CPEVersionUpdateDlg : public CDialogEx
{
// 构造
public:
	CPEVersionUpdateDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PEVERSIONUPDATE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnOpenFilePathBtn();
	afx_msg void OnVersionUpdateBtn();

public:
	bool updatePeInfo(const char* filePath);
	bool updateMainVersion(const char* filePath, const char* strVersion);

	CComboBox m_cbList;
};
