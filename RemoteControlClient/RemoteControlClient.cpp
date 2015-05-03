// RemoteControlClient.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "RemoteControlClient.h"
#include "MainFrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CRCClientApp

BEGIN_MESSAGE_MAP(CRCClientApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CRCClientApp::OnAppAbout)
END_MESSAGE_MAP()


// CRCClientApp 构造

CRCClientApp::CRCClientApp()
:m_hSingleInsMutex(NULL)
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CRCClientApp 对象

CRCClientApp theApp;


// CRCClientApp 初始化

BOOL CRCClientApp::InitInstance()
{
	CWinApp::InitInstance();

	m_hSingleInsMutex = CreateMutex(NULL , FALSE , _T("__REMOUTE_CTRL_CLIENT_SINGLE_INS_METEX__"));
	if ( NULL != m_hSingleInsMutex)
	{
		if (ERROR_ALREADY_EXISTS == GetLastError())
		{//已经存在了
			CloseHandle(m_hSingleInsMutex);
			return FALSE;
		}
	}
	else
	{//直接创建互斥量失败？！
		return FALSE;
	}
	
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&m_lgdiplusToken, &gdiplusStartupInput, NULL);

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	//检查/设置开机启动项
	CheckStart();


	// 若要创建主窗口，此代码将创建新的框架窗口
	// 对象，然后将其设置为应用程序的主窗口对象
	CMainFrame* pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;
	// 创建并加载框架及其资源
	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);


	// 唯一的一个窗口已初始化，因此显示它并对其进行更新
	//这里跟新一下就行了 ，暂不需要显示
	//pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
	// 仅当具有后缀时才调用 DragAcceptFiles
	//  在 SDI 应用程序中，这应在 ProcessShellCommand 之后发生
	return TRUE;
}


// CRCClientApp 消息处理程序



// 用于运行对话框的应用程序命令
void CRCClientApp::OnAppAbout()
{
}


// CRCClientApp 消息处理程序


int CRCClientApp::ExitInstance()
{
	CloseHandle(m_hSingleInsMutex);
	m_hSingleInsMutex = NULL;

	// TODO: 在此添加专用代码和/或调用基类
	GdiplusShutdown(m_lgdiplusToken);

	//确保光标光标显示
	::ShowCursor(TRUE);

	return CWinApp::ExitInstance();
}

BOOL CRCClientApp::CheckStart()
{
	//打开相关的键
	HKEY hKey;

	CString keyName( _T("CRCClient"));
	//获取当前程序的路径
	CString strPath;
	::GetModuleFileName(NULL , strPath.GetBufferSetLength(MAX_PATH) , MAX_PATH);
	strPath.ReleaseBuffer();

	if(::RegOpenKey(HKEY_LOCAL_MACHINE , _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run") , &hKey))
	{//打开注册表失败
		return FALSE;
	}
	
	DWORD dwType = REG_SZ;
	DWORD dwCount = MAX_PATH + 1;
	BYTE pBuf[MAX_PATH+1] = {0};
	if(ERROR_SUCCESS == ::RegQueryValueEx(hKey , keyName , 0 , &dwType , pBuf , &dwCount))
	{//查找成功  键存在
		CString strTemp;
		strTemp += (LPCTSTR)pBuf;
		if (0 == strTemp.CompareNoCase(strPath))
		{//开机启动键值正确 
			::RegCloseKey(hKey);
			return TRUE;
		}
	}

	//讲述写入注册表
	if(::RegSetValueEx(hKey , keyName , 0 , REG_SZ,
		(BYTE*)strPath.GetBuffer(),strPath.GetLength()*2))
	{//操作失败
		::RegCloseKey(hKey);
		return FALSE;
	}

	::RegCloseKey(hKey);
	return TRUE;
}
