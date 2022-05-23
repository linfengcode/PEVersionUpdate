
// PEVersionUpdateDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "PEVersionUpdate.h"
#include "PEVersionUpdateDlg.h"
#include "afxdialogex.h"

#include "VersionUpdater.h"
#include <string>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPEVersionUpdateDlg 对话框



CPEVersionUpdateDlg::CPEVersionUpdateDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PEVERSIONUPDATE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPEVersionUpdateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_TYPE, m_cbList);
}

BEGIN_MESSAGE_MAP(CPEVersionUpdateDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_OPEN_BTN, &CPEVersionUpdateDlg::OnOpenFilePathBtn)
	ON_BN_CLICKED(IDOK, &CPEVersionUpdateDlg::OnVersionUpdateBtn)
END_MESSAGE_MAP()


// CPEVersionUpdateDlg 消息处理程序

BOOL CPEVersionUpdateDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_cbList.ShowWindow(FALSE);
	m_cbList.AddString(_T("gcad2023"));
	m_cbList.AddString(_T("加密"));

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CPEVersionUpdateDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CPEVersionUpdateDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//////////////////////////////////////////////////////////////////////////
bool SetPEResource(const char* exepath, const char* type,
	const char* name, const std::string& value,
	int language = 0)
{
	HANDLE hexe = BeginUpdateResource((exepath), FALSE);
	if (!hexe)
		return false;

	BOOL r = UpdateResource(hexe, type, name, language,
		(LPVOID)value.c_str(), value.size());

	BOOL er = EndUpdateResource(hexe, FALSE);

	return (r && er);
}

//拼接字符串测试
bool appendInfo(const char* filePath)
{
	std::string str = "\n1 VERSIONINFO\n";
	str += "FILEVERSION 5,6,7,8\n";
	str += "PRODUCTVERSION 2,2,2,1\n";
	str += "FILEOS 0x40004\n";
	str += "FILETYPE 0x1\n";
	str += "{\n";
	str += "BLOCK \"StringFileInfo\"\n";
	str += "{";
	str += "BLOCK \"080404b0\"\n";
	str += "{\n";
	str += "VALUE \"CompanyName\",\"TODO:\"\n";
	str += "VALUE \"FileDescription\",\"MFC\"\n";
	str += "VALUE \"FileVersion\",\"1.2.2.1\"\n";
	str += "VALUE \"InternalName\",\"MFC.exe\"\n";
	str += "VALUE \"LegalCopyright\",\"TODO\"\n";
	str += "VALUE \"OriginalFilename\",\"MFC.exe\"\n";
	str += "VALUE \"ProductName\",\"TODO\"\n";
	str += "VALUE \"ProductVersion\",\"2.0.0.1\"\n";
	str += "}\n";
	str += "}\n";
	str += "BLOCK \"VarFileInfo\"\n";
	str += "{\n";
	str += "VALUE \"Translation\",0x0804 0x04B0\n";
	str += "}\n";
	str += "}\n";

	bool bSucess = SetPEResource(filePath, RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO), str, 0);
	return bSucess;
}

void CPEVersionUpdateDlg::OnOpenFilePathBtn()
{
	// TODO: 在此添加控件通知处理程序代码

	// 设置过滤器   
	TCHAR szFilter[] = _T("可执行程序(*.exe)|*.exe|Dll文件(*.dll)|*.dll|所有文件(*.*)|*.*||");
	// 构造打开文件对话框   
	CFileDialog fileDlg(TRUE, _T("exe"), NULL, 0, szFilter);
	CString strFilePath;

	// 显示打开文件对话框   
	if (IDOK == fileDlg.DoModal())
	{
		// 如果点击了文件对话框上的“打开”按钮，则将选择的文件路径显示到编辑框里   
		strFilePath = fileDlg.GetPathName();
		SetDlgItemText(IDC_OPEN_EDIT, strFilePath);
	}
}

void CPEVersionUpdateDlg::OnVersionUpdateBtn()
{
	// TODO: 在此添加控件通知处理程序代码

	CString strPath;
	GetDlgItemText(IDC_OPEN_EDIT, strPath);
	std::string strTmp = strPath.GetString();

	bool bSucess = updatePeInfo(strTmp.c_str());
	//bool bAppend = appendInfo(strTmp.c_str());
	if (bSucess)
	{
		MessageBox(_T("修改成功"));
	}
	else
	{
		MessageBox(_T("修改失败"));
	}
}

//调用一个类进行数据重组
bool CPEVersionUpdateDlg::updatePeInfo(const char* filePath)
{
	CVersionUpdater vu;
	if (vu.Open(filePath))
	{
		if (vu.VersionInfo == NULL) vu.VersionInfo = new CVersionUpdater::CVersionInfo;
		if (vu.VersionInfo->StringFileInfo == NULL) vu.VersionInfo->StringFileInfo = new CVersionUpdater::CStringFileInfo;
		if (vu.VersionInfo->StringFileInfo->Children.GetUpperBound() < 0) vu.VersionInfo->StringFileInfo->Children.Add(new CVersionUpdater::CStringTable);
		for (int i = 0; i <= vu.VersionInfo->StringFileInfo->Children.GetUpperBound(); ++i)
		{
			CVersionUpdater::CStringTable* st = vu.VersionInfo->StringFileInfo->Children.GetAt(i);

			st->SetFileDescription(_T("My 文件描述"));
			st->SetProductName(_T("My 产品名称"));
			st->SetProductVersion(_T("My 产品版本"));
			st->SetLegalCopyright(_T("My 版权"));
			st->SetLegalTrademarks(_T("My 合法商标"));
			st->SetOriginalFilename(_T("My 原始文件名"));

			st->SetFileVersion(_T("4,3,2,1"));
			/*
			st->SetComments(_T("My Com"));
			st->SetCompanyName(_T("My有限公司"));
			st->SetInternalName(_T("My 正式名称"));
			st->SetPrivateBuild(_T("My build"));
			st->SetSpecialBuild(_T("My spBuild"));
			*/
		}
	}
	bool bSucess = vu.Update();
	//bool bMainUpdate = updateMainVersion(filePath, "1.2.3.4");
	return (bSucess);
}

struct
{
	WORD wLanguage;
	WORD wCodePage;
} *lpTranslate;
//只更新主版本号
bool CPEVersionUpdateDlg::updateMainVersion(const char* filePath, const char* strVersion)
{
	if (filePath == "" || strVersion == "")
		return false;

	std::string strTmp(strVersion);

	WORD ver[4] = { 0 };// 分别对应0.0.0.0
	size_t ide = -1;
	int idx = 0;
	strTmp += '.';
	while ((ide = strTmp.find(_T('.'))) != size_t(-1) && ide < 4)
	{
		strTmp[ide] = 0;
		ver[idx] = atoi(strTmp.c_str());
		strTmp = &strTmp[ide + 1];
		idx++;
	}

	if (strTmp.length() > 0 && ide < 4)
	{
		ver[idx] = atoi(strTmp.c_str());
	}

	DWORD dwHandle = 0;
	DWORD dwSize = 0;
	dwSize = GetFileVersionInfoSize(filePath, &dwHandle);
	if (0 >= dwSize)
		return false;

	LPBYTE lpBuffer = new BYTE[dwSize];
	memset(lpBuffer, 0, dwSize);
	if (GetFileVersionInfo(filePath, dwHandle, dwSize, lpBuffer) != FALSE)
	{
		UINT uTemp = 0;
		if (VerQueryValue(lpBuffer, _T("\\VarFileInfo\\Translation"), (LPVOID*)&lpTranslate, &uTemp) != FALSE)
		{
			// 修改版本信息 
			LPVOID lpFixedBuf = NULL;
			DWORD dwFixedLen = 0;
			if (FALSE != VerQueryValue(lpBuffer, _T("\\"), &lpFixedBuf, (PUINT)&dwFixedLen))
			{
				VS_FIXEDFILEINFO* pFixedInfo = (VS_FIXEDFILEINFO*)lpFixedBuf;
				pFixedInfo->dwFileVersionMS = MAKELONG(ver[1], ver[0]);
				pFixedInfo->dwFileVersionLS = MAKELONG(ver[3], ver[2]);
			}
		}
		HANDLE hResource = BeginUpdateResource(filePath, FALSE);
		if (NULL != hResource)
		{
			// 更新  
			if (UpdateResource(hResource, RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO), lpTranslate->wLanguage, lpBuffer, dwSize) != FALSE)
			{
				if (EndUpdateResource(hResource, FALSE) == FALSE)
					return false;
			}
		}
	}
	return true;
}
