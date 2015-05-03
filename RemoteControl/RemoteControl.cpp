///////////////////////////////////////////////////////////////
//
// FileName	: RemoteControl.cpp
// Creator	: ����
// Date		: 2013��2��27�գ�20:10:26
// Comment	: ������Ӧ�ó�����ʵ��
//
//////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RemoteControl.h"
#include "MainFrm.h"

#include "RemoteControlDoc.h"
#include "RemoteControlView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CRemoteControlApp

BEGIN_MESSAGE_MAP(CRemoteControlApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CRemoteControlApp::OnAppAbout)
	// �����ļ��ı�׼�ĵ�����
// 	ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
// 	ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
ON_COMMAND(ID_ABOUT, &CRemoteControlApp::OnAbout)
END_MESSAGE_MAP()


// CRemoteControlApp ����

CRemoteControlApp::CRemoteControlApp()
:m_hSingleInsMutex(NULL)
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CRemoteControlApp ����

CRemoteControlApp theApp;


// CRemoteControlApp ��ʼ��

BOOL CRemoteControlApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()�����򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);


	m_hSingleInsMutex = CreateMutex(NULL , FALSE , _T("__REMOUTE_CTRL_SERVER_SINGLE_INS_METEX__"));
	if ( NULL != m_hSingleInsMutex)
	{
		if (ERROR_ALREADY_EXISTS == GetLastError())
		{//�Ѿ�������

			CString strMsg;
			CString strCapture;
			strCapture.LoadString(IDS_NOTIFY);
			strMsg.LoadString(IDS_APP_IS_RUNNING);
			MessageBox(NULL , strMsg , strCapture , MB_OK|MB_ICONWARNING);
			
			CloseHandle(m_hSingleInsMutex);
			return FALSE;
		}
	}
	else
	{//ֱ�Ӵ���������ʧ�ܣ���
		return FALSE;
	}



	CWinApp::InitInstance();

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
	LoadStdProfileSettings(0);  // ���ر�׼ INI �ļ�ѡ��(���� MRU)
	// ע��Ӧ�ó�����ĵ�ģ�塣�ĵ�ģ��
	// �������ĵ�����ܴ��ں���ͼ֮�������
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CRemoteControlDoc),
		RUNTIME_CLASS(CMainFrame),       // �� SDI ��ܴ���
		RUNTIME_CLASS(CRemoteControlView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);



	// ������׼������DDE�����ļ�������������
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);


	// ��������������ָ����������
	// �� /RegServer��/Register��/Unregserver �� /Unregister ����Ӧ�ó����򷵻� FALSE��
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// Ψһ��һ�������ѳ�ʼ���������ʾ����������и���
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// �������к�׺ʱ�ŵ��� DragAcceptFiles
	//  �� SDI Ӧ�ó����У���Ӧ�� ProcessShellCommand ֮����
	return TRUE;
}



// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// �������жԻ����Ӧ�ó�������
void CRemoteControlApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CRemoteControlApp ��Ϣ�������


int CRemoteControlApp::ExitInstance()
{
	//����GPI+��
	GdiplusShutdown(m_lgdiplusToken);

	return CWinApp::ExitInstance();
}

void CRemoteControlApp::OnAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}
