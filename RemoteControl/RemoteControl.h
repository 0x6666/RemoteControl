///////////////////////////////////////////////////////////////
//
// FileName	: RemoteControl.h 
// Creator	: ����
// Date		: 2013��2��27�գ�20:10:26
// Comment	: ������Ӧ�ó���������
//
//////////////////////////////////////////////////////////////

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"       // ������


// CRemoteControlApp:
// �йش����ʵ�֣������ RemoteControl.cpp
//

class CRemoteControlApp : public CWinApp
{
public:
	CRemoteControlApp();


// ��д
public:
	virtual BOOL InitInstance();

// ʵ��
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
	ULONG_PTR m_lgdiplusToken;

	HANDLE m_hSingleInsMutex;
	afx_msg void OnAbout();
};

extern CRemoteControlApp theApp;