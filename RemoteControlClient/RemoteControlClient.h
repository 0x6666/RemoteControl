// RemoteControlClient.h : RemoteControlClient 应用程序的主头文件
//
#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"       // 主符号


// CRCClientApp:
// 有关此类的实现，请参阅 RemoteControlClient.cpp
//

class CRCClientApp : public CWinApp
{
public:
	CRCClientApp();


// 重写
public:
	virtual BOOL InitInstance();

// 实现

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();

	//检查开机启动项，如果不是开机运行的话就设置成开机运行 
	BOOL CheckStart();
	
private:
	ULONG_PTR m_lgdiplusToken;
	HANDLE m_hSingleInsMutex;

};

extern CRCClientApp theApp;