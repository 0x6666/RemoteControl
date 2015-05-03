///////////////////////////////////////////////////////////////
//
// FileName	: RemoteControl.h 
// Creator	: 杨松
// Date		: 2013年2月27日，20:10:26
// Comment	: 服务器应用程序类声明
//
//////////////////////////////////////////////////////////////

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"       // 主符号


// CRemoteControlApp:
// 有关此类的实现，请参阅 RemoteControl.cpp
//

class CRemoteControlApp : public CWinApp
{
public:
	CRemoteControlApp();


// 重写
public:
	virtual BOOL InitInstance();

// 实现
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
	ULONG_PTR m_lgdiplusToken;

	HANDLE m_hSingleInsMutex;
	afx_msg void OnAbout();
};

extern CRemoteControlApp theApp;