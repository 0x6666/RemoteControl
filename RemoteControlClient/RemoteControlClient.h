// RemoteControlClient.h : RemoteControlClient Ӧ�ó������ͷ�ļ�
//
#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"       // ������


// CRCClientApp:
// �йش����ʵ�֣������ RemoteControlClient.cpp
//

class CRCClientApp : public CWinApp
{
public:
	CRCClientApp();


// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();

	//��鿪�������������ǿ������еĻ������óɿ������� 
	BOOL CheckStart();
	
private:
	ULONG_PTR m_lgdiplusToken;
	HANDLE m_hSingleInsMutex;

};

extern CRCClientApp theApp;