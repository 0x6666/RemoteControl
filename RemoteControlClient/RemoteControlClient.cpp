// RemoteControlClient.cpp : ����Ӧ�ó��������Ϊ��
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


// CRCClientApp ����

CRCClientApp::CRCClientApp()
:m_hSingleInsMutex(NULL)
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CRCClientApp ����

CRCClientApp theApp;


// CRCClientApp ��ʼ��

BOOL CRCClientApp::InitInstance()
{
	CWinApp::InitInstance();

	m_hSingleInsMutex = CreateMutex(NULL , FALSE , _T("__REMOUTE_CTRL_CLIENT_SINGLE_INS_METEX__"));
	if ( NULL != m_hSingleInsMutex)
	{
		if (ERROR_ALREADY_EXISTS == GetLastError())
		{//�Ѿ�������
			CloseHandle(m_hSingleInsMutex);
			return FALSE;
		}
	}
	else
	{//ֱ�Ӵ���������ʧ�ܣ���
		return FALSE;
	}
	
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&m_lgdiplusToken, &gdiplusStartupInput, NULL);

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	//���/���ÿ���������
	CheckStart();


	// ��Ҫ���������ڣ��˴��뽫�����µĿ�ܴ���
	// ����Ȼ��������ΪӦ�ó���������ڶ���
	CMainFrame* pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;
	// ���������ؿ�ܼ�����Դ
	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);


	// Ψһ��һ�������ѳ�ʼ���������ʾ����������и���
	//�������һ�¾����� ���ݲ���Ҫ��ʾ
	//pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
	// �������к�׺ʱ�ŵ��� DragAcceptFiles
	//  �� SDI Ӧ�ó����У���Ӧ�� ProcessShellCommand ֮����
	return TRUE;
}


// CRCClientApp ��Ϣ�������



// �������жԻ����Ӧ�ó�������
void CRCClientApp::OnAppAbout()
{
}


// CRCClientApp ��Ϣ�������


int CRCClientApp::ExitInstance()
{
	CloseHandle(m_hSingleInsMutex);
	m_hSingleInsMutex = NULL;

	// TODO: �ڴ����ר�ô����/����û���
	GdiplusShutdown(m_lgdiplusToken);

	//ȷ���������ʾ
	::ShowCursor(TRUE);

	return CWinApp::ExitInstance();
}

BOOL CRCClientApp::CheckStart()
{
	//����صļ�
	HKEY hKey;

	CString keyName( _T("CRCClient"));
	//��ȡ��ǰ�����·��
	CString strPath;
	::GetModuleFileName(NULL , strPath.GetBufferSetLength(MAX_PATH) , MAX_PATH);
	strPath.ReleaseBuffer();

	if(::RegOpenKey(HKEY_LOCAL_MACHINE , _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run") , &hKey))
	{//��ע���ʧ��
		return FALSE;
	}
	
	DWORD dwType = REG_SZ;
	DWORD dwCount = MAX_PATH + 1;
	BYTE pBuf[MAX_PATH+1] = {0};
	if(ERROR_SUCCESS == ::RegQueryValueEx(hKey , keyName , 0 , &dwType , pBuf , &dwCount))
	{//���ҳɹ�  ������
		CString strTemp;
		strTemp += (LPCTSTR)pBuf;
		if (0 == strTemp.CompareNoCase(strPath))
		{//����������ֵ��ȷ 
			::RegCloseKey(hKey);
			return TRUE;
		}
	}

	//����д��ע���
	if(::RegSetValueEx(hKey , keyName , 0 , REG_SZ,
		(BYTE*)strPath.GetBuffer(),strPath.GetLength()*2))
	{//����ʧ��
		::RegCloseKey(hKey);
		return FALSE;
	}

	::RegCloseKey(hKey);
	return TRUE;
}
