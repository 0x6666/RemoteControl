// MainFrm.cpp : CMainFrame ���ʵ��
//

#include "stdafx.h"
#include "RemoteControlClient.h"

#include "MainFrm.h"
#include "LetterRainDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_TIMER()
	ON_MESSAGE(WM_NOTIFYION_MSG_C , OnNotifyIconMsg)
	ON_COMMAND(ID_EXIT, &CMainFrame::OnExit)
	ON_COMMAND(ID_HANDS_UP, &CMainFrame::OnHandsUp)
	ON_MESSAGE( WM_CREATE_UPLOAD_THREAD , OnCreateUploadThread)
	ON_MESSAGE(WM_UPLOAD_LISTEN_PORT , OnUploadListenPort)
END_MESSAGE_MAP()


// CMainFrame ����/����

CMainFrame::CMainFrame()
: m_pMsgCenter(NULL)
, m_pBroadcastSocket(NULL)
, m_czHostName(NULL)
, m_pLetterRainDlg(NULL)
, m_uPort(0)
, m_bHeartbeatTimerRun(FALSE)
, m_pDlListenDes(NULL)
{
	//��������ͼ���߳�
	m_CCScreenThreadContext.hEvent = NULL;
	m_CCScreenThreadContext.lThreadState = 0;
	m_CCScreenThreadContext.uScreenPort = 0;
	m_CCScreenThreadContext.hThread = NULL;

	//CMD��ȡ���������߳�������
	m_cmdContext.hCmd = NULL;
	m_cmdContext.hWrite = NULL;
	m_cmdContext.hRead = NULL;
	m_cmdContext.hReadCMDThread = NULL;
	m_cmdContext.lReadCMDThreadState = 0;

	//�������ض�����ͼ���߳�������
	m_RecvPushedDesktopThreadContext.hThread = NULL;
	m_RecvPushedDesktopThreadContext.lThreadState = 0;
}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// ����һ����ͼ��ռ�ÿ�ܵĹ�����
	if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("δ�ܴ�����ͼ����\n");
		return -1;
	}

	m_pMsgCenter = new MsgCenter();
	if (FALSE == m_pMsgCenter->InitMsgCenter(this , CLIENT_MSG_PORT  , TRUE ))
	{//��ʹ��ָ���˿���һ�£����ʧ������������˿�
		if (FALSE == m_pMsgCenter->InitMsgCenter(this , 0 , TRUE ))
		{//��ʼ��ʧ��
			delete m_pMsgCenter;
			m_pMsgCenter = NULL;
			return -1;
		}
	}

	//��������� 
	char buf[MAX_PATH] = {0};
	if (gethostname(buf , MAX_PATH) == 0)
	{
		int len = strlen(buf);
		m_czHostName = new char[len + 1];
		strcpy(m_czHostName , buf);
	}

	//��ÿͻ��˼����Ķ˿�
	m_uPort = m_pMsgCenter->GetMsgPort();

	//��ʼ��ѯ�������㲥��ʱ��
	StartFindServerBroadcastTimer();


	//��ʼ������ͼ�����
	m_notifyIcon.cbSize = sizeof(NOTIFYICONDATA);
	m_notifyIcon.hWnd	= GetSafeHwnd();   
	m_notifyIcon.uID	= ICON_INDEX_C;   
	m_notifyIcon.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP;   
	m_notifyIcon.uCallbackMessage = WM_NOTIFYION_MSG_C;//�Զ������Ϣ����   
	m_notifyIcon.hIcon	= AfxGetApp()->LoadIcon(IDR_MAINFRAME);   
	::wcscpy(m_notifyIcon.szTip , _T("Զ�̼�ؿͻ���"));//��Ϣ��ʾ��Ϊ���ƻ��������ѡ�   
	//�����������ͼ��
	::Shell_NotifyIcon(NIM_ADD , &m_notifyIcon );   

	{//���ؼ����߳�
	m_pDlListenDes = new DownloadListrenDescripter();
	m_pDlListenDes->pWnd = this;
	InterlockedExchange(&(m_pDlListenDes->lThreadState) , 1);
	DWORD dwThread = 0;
	m_pDlListenDes->hThread = CreateThread(NULL , 0 , UploadListenerThread , m_pDlListenDes , CREATE_SUSPENDED , &dwThread);
	ResumeThread(m_pDlListenDes->hThread);
	}

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ

	cs.style &= ~WS_SYSMENU;//ȥ���˵���
	cs.style &= ~WS_CAPTION;//ȥ��������
	cs.style &= ~WS_BORDER;//�߿�
	cs.style &= ~WS_THICKFRAME;//�߿�
	cs.hMenu = NULL;
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	cs.cx = 300;
	cs.cy = 260;
	return TRUE;
}


// CMainFrame ���

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

// CMainFrame ��Ϣ�������

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// ������ǰ�Ƶ���ͼ����
	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// ����ͼ��һ�γ��Ը�����
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// ����ִ��Ĭ�ϴ���
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::DispatchMsg( const void* msg , CString ip, UINT port )
{
	if (NULL == msg)
		ASSERT(FALSE);

	switch(PRCMSG(msg)->type)
	{
	case MT_RECV_ERROR:
		{//��������ʧ��
			break;
		}
	case MT_REPEAT_FIND_SERVER_S://�رն�ʱ��
		OnRepeatFindServer(ip);
		break;
	case MT_SCREEN_PORT_S://�������Ѿ�׼�����˺��˽���screen����
		StartScreenCaptureThread( msg);
		break;
	case MT_SCREEN_QUALITY_S://����ץ��������
		OnRcScreenQuality(msg);	
		break;
	case MT_SCREEN_FLUENCY_S://����screen��������
		OnRcScreenFluency(msg);	
		break;
	case MT_STOP_SCREEN_CAPTURE_S://ֹͣץ��
		OnRcStopScreenCapture(); 
		break;
	case MT_SCREEN_CTRL_S:
		{
			//TODO: ����Ҫ�����������Ҫ���Ƶ�ǰ���������� 
		}
		break;
	case MT_MOUSE_EVENT_S://����¼�
		OnRcMouseEvent(msg);
		break;
	case MT_KETBD_EVENT_S://�����¼�
		OnRcKeybdEvent(msg);
		break;
	case MT_SCREEN_SIZE_S://����ץ���Ĵ�С
		OnRcScreenSize(msg); 
		break;
	case MT_PUSH_SERVER_DESKTOP_S://������Ҫ�������������
		OnRcPushServerDesktop();
		break;
	case MT_CALCEL_PUSH_DESKTOP_S://��Ҫȡ���Ե�ǰ�ͻ��˵�����
		OnRcCancelPushDesktop();
		break;
	case MT_EXIT_CLIENT_S://��Ҫ�˳��ͻ���
		OnRcExitClient();
		break;
	case MT_SHUTDOWN_CLIENT_S://�ػ�
		OnRcShutdown();
		break;
	case MT_RESTART_CLIENT_S://����
		OnRcRestart();
		break;
	case MT_LOGIN_OUT_CLIENT_S://ע��
		OnRcLoginOut();
		break;
	case MT_LETTER_RAIN_S://��Ļ��
		OnRcLetterRain(msg);
		break;
	case MT_START_CMD_S://����CMD
		if(FALSE == OnRcStartCmd())
		{//����cmd����ʧ��
			DEF_RCMSG(rcMsg , MT_START_CMD_FAILED_C);
			m_pMsgCenter->SendMsg(ip , SERVER_MSG_PORT , &rcMsg);
		}
		break;
	case MT_CMD_S://CMD����
		OnRcCMD(msg);
		break;
	case MT_SERVER_EXIT_S://����������
		OnRcServerExit();
		break;
	case MT_GET_FILE_LIST_S://��ȡ�ƶ�·�����ļ��б�
		OnRcGetFileList(msg);
		break;
	case MT_GET_DRIVER_S://��ȡ������
		OnRcGetDriver();
		break;
	case MT_DELETE_FILE_S://ɾ���ļ�
		OnRcDeleteFile(msg);
		break;
	case MT_RESUME_SCREEN_S://���¿�ʼ������Ļͼ��
		OnRcResumeScreen();
		break;
	case MT_PAUSE_SCREEN_S://��ͣ������Ļͼ��
		OnRcPauseScreen();
		break;
	default:
		ASSERT(FALSE);
	}
	return ;
}

BOOL CMainFrame::DestroyWindow()
{
	{//ֹͣ�����ϴ�
	InterlockedExchange(&(m_pDlListenDes->lThreadState) , 0);
	WaiteExitThread(m_pDlListenDes->hThread , 1000);
	CloseHandle(m_pDlListenDes->hThread);
	delete m_pDlListenDes;
	m_pDlListenDes= NULL;
	}

	//ֹͣ�ϴ��ļ��߳�
	StopUploadThreads();

	//ֹͣץ���߳�
	OnRcStopScreenCapture();
	//�������շ��������������߳�
	StopRecvServerPushDektopThread();

	if (NULL != m_pMsgCenter)
	{
		//�����ȷ�����Ϣ֪ͨһ�·�����
		DEF_RCMSG(rcMsg , MT_CONN_CLOSED_C);
		m_pMsgCenter->SendMsg(m_strServerIP , SERVER_MSG_PORT , &rcMsg);
		Sleep(0);

		m_pMsgCenter->Close();
		delete m_pMsgCenter;
		m_pMsgCenter = NULL;
	}

	//�رչ㲥��ʱ��
	StopFindServerBroadcast();

	//�ر���������ʱ��
	StopHeartbeatTimer();

	//ɾ������������ռ�
	if (NULL != m_czHostName)
		delete[] m_czHostName;

	//ֹͣ��ȡCMD�Ļ�������
	StopReadCMDThread();

	//ɾ������ͼ��
	::Shell_NotifyIcon(NIM_DELETE , &m_notifyIcon);

	return CFrameWnd::DestroyWindow();
}

void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	switch(nIDEvent)
	{
	case BROADCAST_TIMER:
		{//��ѯ�����
			int len = 0;
			if ((m_czHostName != NULL) && ((len = strlen(m_czHostName)) != 0))
			{
				//֪ͨ��������ǰ�ͻ��˵���Ϣ�˿ںͼ������
				char* msgBuf = new char[sizeof(RCMSG_BROADCAST_MSG) - 1 + len + 1];
				PRCMSG_BROADCAST_MSG pMsg = (PRCMSG_BROADCAST_MSG)msgBuf;
				pMsg->msgHead.size = sizeof(RCMSG_BROADCAST_MSG) - 1 + len + 1;
				pMsg->msgHead.type = MT_FIND_SERVER_BROADCAST_C;
				pMsg->port = m_uPort;
				memcpy(pMsg->name , m_czHostName , len);
				pMsg->name[len] = 0;
				m_pBroadcastSocket->SendTo( pMsg , pMsg->msgHead.size 
					, SERVER_MSG_PORT , NULL , 0 );
				delete [] msgBuf;
			}
			else
			{//�����������ǲ����ܵ�
				ASSERT(FALSE);
			}
		}
		break;
	case HEARTBEAT_TIMER://������
		{
			DEF_RCMSG(rcMsg , MT_HEARTBEAT_C);
			m_pMsgCenter->SendMsg(m_strServerIP , SERVER_MSG_PORT , &rcMsg);
		}
		break;
	default:
		ASSERT(FALSE);
	}

	CFrameWnd::OnTimer(nIDEvent);
}

void CMainFrame::StopFindServerBroadcast()
{
	if ( m_pBroadcastSocket != NULL)
	{
		KillTimer(BROADCAST_TIMER);
		m_pBroadcastSocket->Close();
		delete m_pBroadcastSocket;
		m_pBroadcastSocket = NULL;
	}
}

void CMainFrame::StartScreenCaptureThread( const void* msg )
{
	if (m_CCScreenThreadContext.lThreadState)
	{//�߳��������Ǿ��Ƚ���
		InterlockedExchange(&(m_CCScreenThreadContext.lThreadState ), 0);
		SetEvent(m_CCScreenThreadContext.hEvent);
		WaiteExitThread(m_CCScreenThreadContext.hThread, 2*1000);
		CloseHandle(m_CCScreenThreadContext.hEvent);
		CloseHandle(m_CCScreenThreadContext.hThread);
		m_CCScreenThreadContext.hThread = NULL;
		m_CCScreenThreadContext.hEvent	= NULL;
	}

	//��ʼ��ץ���߳�������
	m_CCScreenThreadContext.strServerIP = m_strServerIP;
	m_CCScreenThreadContext.uScreenPort = *((USHORT*)(PRCMSG_BUFF(msg)->buf));
	m_CCScreenThreadContext.sSocket		= 0;
	m_CCScreenThreadContext.pWnd		= this;
	m_CCScreenThreadContext.lScreenW	= SCREEN_SIZE_W;
	m_CCScreenThreadContext.lScreenH	= SCREEN_SIZE_H;
	m_CCScreenThreadContext.lFluency	= DEFAULT_FREQUENCY_LEVEL ;
	m_CCScreenThreadContext.lScreenQuality = DEFAULT_SCREEN_QUALITY;

	InterlockedExchange(&(m_CCScreenThreadContext.lThreadState ) , 1);
	m_CCScreenThreadContext.hEvent = CreateEvent(NULL , TRUE , TRUE , NULL);
	DWORD dwID = 0;
	m_CCScreenThreadContext.hThread = CreateThread(NULL , 0 , CaptureScreenThread , &m_CCScreenThreadContext , 0 , &dwID);

	//����һ��������Ϣ֪ͨ�Ѿ�
	DEF_RCMSG(rcMsg , MT_SCREEN_CAPTURE_START_C);
	m_pMsgCenter->SendMsg(m_strServerIP , SERVER_MSG_PORT , &rcMsg);
}

CString CMainFrame::GetServerIP()
{
	return m_strServerIP;
}

void CMainFrame::OnRcScreenQuality( const void* msg )
{
	PRCMSG_BUFF pMsg = PRCMSG_BUFF(msg);
	LONG q = *((LONG*)pMsg->buf);
	if ( q < 0)
		q = 0;
	else if (q > 100)
		q = 100;
	InterlockedExchange(&(m_CCScreenThreadContext.lScreenQuality) , q);
}

void CMainFrame::OnRcScreenFluency( const void* msg )
{
	PRCMSG_BUFF pMsg = PRCMSG_BUFF(msg);
	LONG f = *((LONG*)pMsg->buf);
	InterlockedExchange(&(m_CCScreenThreadContext.lFluency) , f);
}

void CMainFrame::OnRcStopScreenCapture()
{
	if(m_CCScreenThreadContext.lThreadState)
	{//ֹͣץ���߳�
		InterlockedExchange(&(m_CCScreenThreadContext.lThreadState) , 0);
		SetEvent(m_CCScreenThreadContext.hEvent);

		//ץ���߳�
		if(FALSE == ::WaiteExitThread(m_CCScreenThreadContext.hThread , 2*1000))
		{//�߳���ǿ�н�����
			if ((m_CCScreenThreadContext.sSocket != 0 )&&
				(m_CCScreenThreadContext.sSocket != INVALID_SOCKET) )
			{//��Ҫ�ر��׽���
				closesocket(m_CCScreenThreadContext.sSocket);
				m_CCScreenThreadContext.sSocket = 0;
			}
		}
		CloseHandle(m_CCScreenThreadContext.hEvent);
		m_CCScreenThreadContext.hEvent = NULL;
		CloseHandle(m_CCScreenThreadContext.hThread);
		m_CCScreenThreadContext.hThread = NULL;
	}
}

void CMainFrame::OnRcMouseEvent( const void* msg )
{
	PMOUSE_EVENT_MSG pMsg = PMOUSE_EVENT_MSG(msg);
	switch(pMsg->message)
	{
	case WM_MOUSEMOVE://    �ƶ����ʱ���� 
		MOUSE_TEST_CTRL_SHIFT_D(pMsg->param);
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE , pMsg->x , pMsg->y , 0 , 0 );
		MOUSE_TEST_CTRL_SHIFT_U(pMsg->param);
		break;
	case WM_LBUTTONDOWN://  ���������� 
		MOUSE_TEST_CTRL_SHIFT_D(pMsg->param);
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_MOVE , pMsg->x , pMsg->y , 0 , 0 );
		MOUSE_TEST_CTRL_SHIFT_U(pMsg->param);
		break;
	case WM_LBUTTONUP: //   �ͷ������� 
		MOUSE_TEST_CTRL_SHIFT_D(pMsg->param);
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTUP|MOUSEEVENTF_MOVE , pMsg->x , pMsg->y , 0 , 0 );
		MOUSE_TEST_CTRL_SHIFT_U(pMsg->param);
		break;
	case WM_LBUTTONDBLCLK://˫�������� 
		MOUSE_TEST_CTRL_SHIFT_D(pMsg->param);
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_MOVE , pMsg->x , pMsg->y , 0 , 0 );
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTUP|MOUSEEVENTF_MOVE ,   pMsg->x , pMsg->y , 0 , 0 );
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_MOVE , pMsg->x , pMsg->y , 0 , 0 );
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTUP|MOUSEEVENTF_MOVE ,   pMsg->x , pMsg->y , 0 , 0 );
		MOUSE_TEST_CTRL_SHIFT_U(pMsg->param);
		break;
	case WM_RBUTTONDOWN://  ��������Ҽ� 
		MOUSE_TEST_CTRL_SHIFT_D(pMsg->param);
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_RIGHTDOWN|MOUSEEVENTF_MOVE , pMsg->x , pMsg->y , 0 , 0 );
		MOUSE_TEST_CTRL_SHIFT_U(pMsg->param);
		break;
	case WM_RBUTTONUP://    �ͷ�����Ҽ� 
		MOUSE_TEST_CTRL_SHIFT_D(pMsg->param);
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_RIGHTUP|MOUSEEVENTF_MOVE ,   pMsg->x , pMsg->y , 0 , 0 );
		MOUSE_TEST_CTRL_SHIFT_U(pMsg->param);
		break;
	case WM_RBUTTONDBLCLK://˫������Ҽ� 
		MOUSE_TEST_CTRL_SHIFT_D(pMsg->param);
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_RIGHTDOWN|MOUSEEVENTF_MOVE , pMsg->x , pMsg->y , 0 , 0 );
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_RIGHTUP|MOUSEEVENTF_MOVE ,   pMsg->x , pMsg->y , 0 , 0 );
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_RIGHTDOWN|MOUSEEVENTF_MOVE , pMsg->x , pMsg->y , 0 , 0 );
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_RIGHTUP|MOUSEEVENTF_MOVE ,   pMsg->x , pMsg->y , 0 , 0 );
		MOUSE_TEST_CTRL_SHIFT_U(pMsg->param);
		break;
	case WM_MBUTTONDOWN://  ��������м� 
		MOUSE_TEST_CTRL_SHIFT_D(pMsg->param);
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MIDDLEDOWN|MOUSEEVENTF_MOVE , pMsg->x , pMsg->y , 0 , 0 );
		MOUSE_TEST_CTRL_SHIFT_U(pMsg->param);
		break;
	case WM_MBUTTONUP://�����ͷ�����м� 
		MOUSE_TEST_CTRL_SHIFT_D(pMsg->param);
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MIDDLEUP|MOUSEEVENTF_MOVE ,   pMsg->x , pMsg->y , 0 , 0 );
		MOUSE_TEST_CTRL_SHIFT_U(pMsg->param);
		break;
	case WM_MBUTTONDBLCLK://˫������м� 
		MOUSE_TEST_CTRL_SHIFT_D(pMsg->param);
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MIDDLEDOWN|MOUSEEVENTF_MOVE , pMsg->x , pMsg->y , 0 , 0 );
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MIDDLEUP|MOUSEEVENTF_MOVE ,   pMsg->x , pMsg->y , 0 , 0 );
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MIDDLEDOWN|MOUSEEVENTF_MOVE , pMsg->x , pMsg->y , 0 , 0 );
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MIDDLEUP|MOUSEEVENTF_MOVE ,   pMsg->x , pMsg->y , 0 , 0 );
		MOUSE_TEST_CTRL_SHIFT_U(pMsg->param);
		break;
	case WM_MOUSEWHEEL://�����ֹ��� 
		{
			MOUSE_TEST_CTRL_SHIFT_D(pMsg->param);
			short delta = HIWORD(pMsg->param);
			::mouse_event(MOUSEEVENTF_WHEEL , 0 , 0 , (DWORD)delta , 0 );
			MOUSE_TEST_CTRL_SHIFT_U(pMsg->param);
		}break;
	default:
		break;
	}
}

void CMainFrame::OnRcKeybdEvent( const void* msg )
{
	PKEYBD_EVENT_MSG pMsg = PKEYBD_EVENT_MSG(msg);
	switch(pMsg->message)
	{
	case WM_KEYDOWN://���̼��̰���
		{
			if ((pMsg->vKey != VK_CONTROL) && ( pMsg->vKey != VK_SHIFT) && (pMsg->vKey != VK_MENU))
			{
				if (pMsg->keyState & KS_CTRL) 
					::keybd_event(VK_CONTROL , 0 , 0 , 0);
				if (pMsg->keyState & KS_SHIFT)
					::keybd_event(VK_SHIFT , 0 , 0 , 0);
				if (pMsg->keyState & KS_ALT)  
					::keybd_event(VK_MENU , 0 , 0 , 0);
			}

			::keybd_event(pMsg->vKey , 0 , 0 , 0);

			if ((pMsg->vKey != VK_CONTROL) && ( pMsg->vKey != VK_SHIFT) && (pMsg->vKey != VK_MENU))
			{
				if (pMsg->keyState & KS_CTRL) 
					::keybd_event(VK_CONTROL , 0 , KEYEVENTF_KEYUP , 0);
				if (pMsg->keyState & KS_SHIFT)
					::keybd_event(VK_SHIFT , 0 , KEYEVENTF_KEYUP , 0);
				if (pMsg->keyState & KS_ALT)  
					::keybd_event(VK_MENU , 0 , KEYEVENTF_KEYUP , 0);
			}
		}break;
	case WM_KEYUP://���̰������ͷ�
		{
			if ((pMsg->vKey != VK_CONTROL) && ( pMsg->vKey != VK_SHIFT) && (pMsg->vKey != VK_MENU))
			{
				if (pMsg->keyState & KS_CTRL) 
					::keybd_event(VK_CONTROL , 0 , 0 , 0);
				if (pMsg->keyState & KS_SHIFT)
					::keybd_event(VK_SHIFT , 0 , 0 , 0);
				if (pMsg->keyState & KS_ALT)  
					::keybd_event(VK_MENU , 0 , 0 , 0);
			}
			::keybd_event(pMsg->vKey , 0 , KEYEVENTF_KEYUP , 0);
			if ((pMsg->vKey != VK_CONTROL) && ( pMsg->vKey != VK_SHIFT) && (pMsg->vKey != VK_MENU))
			{
				if (pMsg->keyState & KS_CTRL) 
					::keybd_event(VK_CONTROL , 0 , KEYEVENTF_KEYUP , 0);
				if (pMsg->keyState & KS_SHIFT)
					::keybd_event(VK_SHIFT , 0 , KEYEVENTF_KEYUP , 0);
				if (pMsg->keyState & KS_ALT)  
					::keybd_event(VK_MENU , 0 , KEYEVENTF_KEYUP , 0);
			}
		}break;
	default:
		break;
	}
}

void CMainFrame::OnRcScreenSize( const void* msg )
{
	PRCMSG_SCREEN_SIZE pMSg = PRCMSG_SCREEN_SIZE(msg);
	if (pMSg->x < 1) 
		pMSg->x = 1;
	if (pMSg->y < 1)
		pMSg->y = 1;

	//Ҫץ��ͼֻ���Ա�ScreenС
	if (SCREEN_SIZE_W > pMSg->x)
		InterlockedExchange(&(m_CCScreenThreadContext.lScreenW) , pMSg->x);
	if (SCREEN_SIZE_H > pMSg->y)
		InterlockedExchange(&(m_CCScreenThreadContext.lScreenH) , pMSg->y);
}

void CMainFrame::OnRcPushServerDesktop()
{
	//TODO: �����������͵������߳�
	if( 0 == m_RecvPushedDesktopThreadContext.lThreadState)
	{
		InterlockedExchange(&(m_RecvPushedDesktopThreadContext.lThreadState) , 1);
		DWORD dwThreadID = 0;
		m_RecvPushedDesktopThreadContext.pFrame = this;
		m_RecvPushedDesktopThreadContext.pScreenView = &m_wndView;
		m_RecvPushedDesktopThreadContext.hThread= CreateThread(NULL , 0 , RecvServerPushedDesktopThread , &m_RecvPushedDesktopThreadContext , 0 , &dwThreadID);
	}
	//���ͻ�ȡ�����ȷ����Ϣ
	DEF_RCMSG(rcMsg , MT_PUSH_SERVER_DESKTOP_C);
	m_pMsgCenter->SendMsg(m_strServerIP , SERVER_MSG_PORT , &rcMsg);

	//��������ʾ����
	ShowWindow(SW_SHOW);

	{//ȫ����ʾ 
	//ȥ��������
	ModifyStyle(WS_CAPTION , 0);

	::SetWindowPos(GetSafeHwnd() , HWND_TOPMOST , 0 , 0 , SCREEN_SIZE_W , SCREEN_SIZE_H  , SWP_SHOWWINDOW); 
	//�������
	::ShowCursor(FALSE);
	}//ȫ����ʾ 
}

void CMainFrame::StopRecvServerPushDektopThread()
{
	if ( 0 != m_RecvPushedDesktopThreadContext.lThreadState)
	{//ֹͣ�������������߳�
		InterlockedExchange(&(m_RecvPushedDesktopThreadContext.lThreadState) , 0);
		WaiteExitThread(m_RecvPushedDesktopThreadContext.hThread , 2*1000);
		CloseHandle(m_RecvPushedDesktopThreadContext.hThread);
		m_RecvPushedDesktopThreadContext.hThread = NULL;
	}
}

void CMainFrame::OnRcCancelPushDesktop()
{
	//��Ҫ�������������
	StopRecvServerPushDektopThread();

	DEF_RCMSG(rcMsg , MT_CALCEL_PUSH_DESKTOP_C);
	m_pMsgCenter->SendMsg(m_strServerIP , SERVER_MSG_PORT , &rcMsg);

	//���ش���
	ShowWindow(SW_HIDE);

	//��ʾ���
	::ShowCursor(FALSE);
}

void CMainFrame::OnRcExitClient()
{//�˳���ǰ����
	OnClose();
}

void CMainFrame::OnRcShutdown()
{
	if (FALSE == Shutdown(SD_SHUTDOWN))
	{//����ʧ��
		DEF_RCMSG(rcMsg , MT_SHUTDOWN_FALIED_C);
		m_pMsgCenter->SendMsg(m_strServerIP , SERVER_MSG_PORT , &rcMsg);
	}
}

void CMainFrame::OnRcRestart()
{
	if (FALSE == Shutdown(SD_RESTART))
	{//����ʧ��
		DEF_RCMSG(rcMsg , MT_RESTART_FALIED_C);
		m_pMsgCenter->SendMsg(m_strServerIP , SERVER_MSG_PORT , &rcMsg);
	}
}

void CMainFrame::OnRcLoginOut()
{
	if (FALSE == Shutdown(SD_LOGIN_OUT))
	{//����ʧ��
		DEF_RCMSG(rcMsg , MT_LOGIN_OUT_FALIED_C);
		m_pMsgCenter->SendMsg(m_strServerIP , SERVER_MSG_PORT , &rcMsg);
	}
}

void CMainFrame::OnRcLetterRain( const void* msg )
{
	if (1 == *((BYTE*)&(PRCMSG_BUFF(msg)->buf)))
	{//������Ļ��
		if(NULL != m_pLetterRainDlg)
			return ;//�Ѿ�����
		m_pLetterRainDlg = new CLetterRainDlg();
		m_pLetterRainDlg->Create(IDD_LETTER_RAIN_DLG , this);
		m_pLetterRainDlg->ShowWindow(SW_SHOW);
		UpdateWindow();
	}
	else//ֹͣ��Ļ��
		StopLetterRain();
}

BOOL CMainFrame::OnRcStartCmd()
{
	if (NULL != m_cmdContext.hRead)
	{//�Ѿ�������cmd�ӽ���
		return TRUE;
	}

	//������������ڸ�CMD��ȡ��������
	HANDLE hInnerRead = NULL;
	//������������ڸ�CMD���ִ������ĺ�Ļ�������
	HANDLE hInnerWrite = NULL;

	//BOOL res = FALSE;

	//��ʼ����cmd����ȫ������
	SECURITY_ATTRIBUTES  sa = {0};
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	
	//���������ܵ�
	if(FALSE == ::CreatePipe(&(m_cmdContext.hRead) , &hInnerWrite , &sa , NULL))
	{//�����ܵ���ʧ��
		return FALSE;		
	}
	if(FALSE == ::CreatePipe(&hInnerRead , &(m_cmdContext.hWrite) , &sa , NULL))
	{//�����ܵ���ʧ��
		::CloseHandle(m_cmdContext.hRead);
		m_cmdContext.hRead = NULL;
		::CloseHandle(hInnerWrite);
		hInnerWrite = NULL;
		return FALSE;		
	}

	STARTUPINFO si = {0};
	::GetStartupInfo(&si);
	//�ı����׼����Ϊ ����ܵ��Ķ�ȡ��  ��׼��� �ʹ��� Ϊ�ܵ�һ��д
	si.hStdInput   = hInnerRead;
	si.hStdError   = hInnerWrite;
	si.hStdOutput  = hInnerWrite;
	si.dwFlags     = STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
	//����CMD�ӽ��̴���
	si.wShowWindow = SW_HIDE;

	//������Ϣ
	PROCESS_INFORMATION ProInfo = {0};

	//��ȡcmd��·��
	CString strCmdPath = GetCMDPath();
	if (0 == strCmdPath.GetLength())
	{//��ȡ����cmd·��ʧ��
		::CloseHandle(m_cmdContext.hRead);
		m_cmdContext.hRead = NULL;
		::CloseHandle(m_cmdContext.hWrite);
		m_cmdContext.hWrite = NULL;
		::CloseHandle(hInnerRead);
		hInnerRead = NULL;
		::CloseHandle(hInnerWrite);
		hInnerWrite = NULL;
		return FALSE;
	}

	//�����ӽ���
	if(FALSE == ::CreateProcess(strCmdPath , NULL , NULL , NULL , TRUE 
		, NULL , NULL , NULL , &si , &ProInfo))
	{//�����ӽ���ʧ��
		::CloseHandle(m_cmdContext.hRead);
		m_cmdContext.hRead = NULL;
		::CloseHandle(m_cmdContext.hWrite);
		m_cmdContext.hWrite = NULL;
		::CloseHandle(hInnerRead);
		hInnerRead = NULL;
		::CloseHandle(hInnerWrite);
		hInnerWrite = NULL;
		return FALSE;
	}

	//�ӽ��̾��
	m_cmdContext.hCmd = ProInfo.hProcess;
	
	//�ر����߳̾��
	CloseHandle(ProInfo.hThread);

	//������ȡCMD�Ļ�������
	InterlockedExchange(&(m_cmdContext.lReadCMDThreadState) , 1);

	//�����IP��ַ
	m_cmdContext.strIP = m_strServerIP;

	DWORD dwThreadID = 0;
	m_cmdContext.hReadCMDThread = CreateThread(NULL , 0 , ReadCMDThread , &m_cmdContext , 0 , &dwThreadID);
	
	return TRUE;
}

CString CMainFrame::GetCMDPath()
{
	CString sEnvironmentName = _T("ComSpec");
	TCHAR cBuffer[256];

	// ��ȡ����������Ϣ
	DWORD dwRet = GetEnvironmentVariable(sEnvironmentName, cBuffer, 256);
	if (dwRet == 0)
	{//��ȡ��������ʧ��
		return CString();
	}
	else
	{//�Ѿ���ȷ��ȡ��cmd·��
		return CString(cBuffer);
	}
}

LRESULT CMainFrame::OnNotifyIconMsg(WPARAM wParam , LPARAM lParam)
{
	if(wParam != ICON_INDEX_C)
		return  1;//���Ǳ����������ͼ��

	switch(lParam)   
	{
	case WM_RBUTTONUP://�Ҽ�����ʱ������ݲ˵�
		{
			CMenu m , *p;
			CPoint point;
			m.LoadMenu(IDR_ICON_MENU);
			p = m.GetSubMenu(0);
			::GetCursorPos(&point);
			::TrackPopupMenu(p->m_hMenu , TPM_LEFTALIGN|TPM_BOTTOMALIGN , point.x , point.y , 0 , GetSafeHwnd() , NULL);
		}
		break;
	case WM_LBUTTONDBLCLK://˫������Ĵ���   
		//ShowWindow(IsWindowVisible()?SW_HIDE:SW_SHOW);
		break;   
	}   
	return 0;
}

void CMainFrame::OnExit()
{
	//�����Ҫ��ѯ�������Ƿ�ͬ���˳�
	OnClose();
}

void CMainFrame::OnHandsUp()
{
	//����
	DEF_RCMSG(rcMsg , MT_HANDS_UP_C);
	m_pMsgCenter->SendMsg(m_strServerIP , SERVER_MSG_PORT , &rcMsg);
}

void CMainFrame::OnRepeatFindServer( const CString& ip )
{
	m_strServerIP = ip;
	
	//�رղ��ҷ������Ĺ㲥
	StopFindServerBroadcast();

	//�����������㲥
	if (FALSE == m_bHeartbeatTimerRun)
	{
		m_bHeartbeatTimerRun = TRUE;
		SetTimer(HEARTBEAT_TIMER , HEARTBEAT_PACKETS_TIME/2 , NULL);
	}

	//���ͼ������ض˿�
	SendUploadPort();
}

void CMainFrame::StopHeartbeatTimer()
{
	if (TRUE == m_bHeartbeatTimerRun)
	{//��������ʱ����������
		KillTimer(HEARTBEAT_TIMER);
		m_bHeartbeatTimerRun = FALSE;
	}
}

void CMainFrame::OnRcCMD( const void* msg )
{
	PRCMSG_BUFF pMsg = PRCMSG_BUFF(msg);
	DWORD dwSize = 0;
	if(!::WriteFile(m_cmdContext.hWrite , pMsg->buf , ::strlen(pMsg->buf) , &dwSize , 0))
	{//ִ������ʧ��
		pMsg->msgHead.type = MT_CMD_FAILED_C;
		m_pMsgCenter->SendMsg(m_strServerIP , m_uPort , pMsg);
		return ;
	}
}

void CMainFrame::StopLetterRain()
{
	if(NULL != m_pLetterRainDlg)
	{//ֹͣ��Ļ��	
		m_pLetterRainDlg->DestroyWindow();
		delete m_pLetterRainDlg;
		m_pLetterRainDlg = NULL;
	}
}

void CMainFrame::StopReadCMDThread()
{
	if (m_cmdContext.lReadCMDThreadState != 0)
	{
		InterlockedExchange(&(m_cmdContext.lReadCMDThreadState), 0);
		WaiteExitThread(m_cmdContext.hReadCMDThread , 2*1000);
		CloseHandle(m_cmdContext.hReadCMDThread);
		m_cmdContext.hReadCMDThread = NULL;


		//�ر�cmd��д���
		CloseHandle(m_cmdContext.hRead);
		CloseHandle(m_cmdContext.hWrite);
		m_cmdContext.hRead  = NULL;
		m_cmdContext.hWrite = NULL;

		//����cmd����
		TerminateProcess( m_cmdContext.hCmd , 4 );
		CloseHandle(m_cmdContext.hCmd);
		m_cmdContext.hCmd = NULL;
	}
}

void CMainFrame::OnRcServerExit()
{
	//ֹͣץ���߳�
	OnRcStopScreenCapture();

	//ֹͣ���շ�������������
	StopRecvServerPushDektopThread();

	//ֹͣ��Ļ��
	StopLetterRain();

	//ֹͣ��ȡcmd���ݵ��߳�
	StopReadCMDThread();

	//ֹͣ����������
	StopHeartbeatTimer();
	
	//��ʼ��ѯ������ 
	StartFindServerBroadcastTimer();

}

void CMainFrame::StartFindServerBroadcastTimer()
{
	//���ڲ�ѯ�������Ĺ㲥�׽���
	if (NULL == m_pBroadcastSocket)
	{
		m_pBroadcastSocket = new CAsyncSocket();
		m_pBroadcastSocket->Create( 0 , SOCK_DGRAM);
		int roadcase = TRUE; 
		if(TRUE == m_pBroadcastSocket->SetSockOpt( SO_BROADCAST, &roadcase, sizeof(BOOL), SOL_SOCKET))
		{
			SetTimer(BROADCAST_TIMER , HEARTBEAT_PACKETS_TIME/2 , NULL);
		}
		else
		{//�����׽��ֹ㲥����ʧ��
			m_pBroadcastSocket->Close();
			delete m_pBroadcastSocket;
			m_pBroadcastSocket = NULL;
		}
	}
}

void CMainFrame::OnRcGetFileList( const void* msg )
{
	m_strEnumPath = _T("");
	m_strEnumPath += PRCMSG_BUFF(msg)->buf;
	//ֱ�ӿ���һ���߳���ö�����е��ļ�
	DWORD dwThreadID = 0;
	HANDLE hThread = CreateThread(NULL , 0 , EnumFileThread , this , 0 , &dwThreadID);
	CloseHandle(hThread);
}

CString CMainFrame::GetEnumPath()
{
	return m_strEnumPath;
}

void CMainFrame::OnRcGetDriver()
{
	TCHAR buf[65] = {0};
	TCHAR* pTemp = buf;
	char msgBuf[sizeof(RCMSG_BUFF) - 1 +4] = {0};
	PRCMSG_BUFF pMsg = PRCMSG_BUFF(msgBuf);
	pMsg->msgHead.size = sizeof(RCMSG_BUFF) - 1 +4;
	pMsg->msgHead.type = MT_FILE_PATH_C;
	if(GetLogicalDriveStrings(64 , pTemp))
	{
		while(*pTemp)
		{
			pMsg->buf[0] = (char)(pTemp[0]);
			pMsg->buf[1] = ':';
			pMsg->buf[2] = '\\';
			m_pMsgCenter->SendMsg(m_strServerIP , SERVER_MSG_PORT , pMsg);
			pTemp += _tcslen(pTemp);
			pTemp++;
		}
	}
}

void CMainFrame::OnRcDeleteFile( const void* msg )
{
	PRCMSG_BUFF pMsg = PRCMSG_BUFF(msg);
	CString strPath;
	strPath += pMsg->buf;
	UINT type = 0;
	strPath = RevertPath(strPath , type);

	if(FALSE == DeleteFileOrDir(strPath))
	{//ɾ���ļ�ʧ��
		pMsg->msgHead.type = MT_DELETE_FILE_FAILED_C;
	}else{
		pMsg->msgHead.type = MT_DELETE_FILE_SUCCESS_C;
	}
	//����һ��������Ϣ
	m_pMsgCenter->SendMsg(m_strServerIP , SERVER_MSG_PORT , pMsg);
}

LRESULT CMainFrame::OnCreateUploadThread( WPARAM wParam , LPARAM lParam )
{
	PUploadDescripter pudes = new UploadDescripter();
	pudes->sSocket = (SOCKET)wParam;
	InterlockedExchange(&(pudes->lThreadState ), 1);
	pudes->pWnd = this;
	m_listUploadDescripter.push_back(pudes);

	{//�����߳�
	DWORD dwThreadID = 0;
	HANDLE handle = CreateThread(NULL , 0 , DownUploadFileThread_C , pudes , CREATE_SUSPENDED , &dwThreadID);
	pudes->hThread = handle;
	ResumeThread(handle);
	}

	return 0;
}

void CMainFrame::StopUploadThreads()
{
	while (!m_listUploadDescripter.empty())
	{
		PUploadDescripter pudes =  m_listUploadDescripter.front();
		m_listUploadDescripter.pop_front();
		InterlockedExchange(&(pudes->lThreadState ), 0);
		WaiteExitThread(pudes->hThread , 500);
		CloseHandle(pudes->hThread);
		pudes->hThread = NULL;
		delete pudes;
	}
}

LRESULT CMainFrame::OnUploadListenPort( WPARAM wParam , LPARAM lParam )
{
	m_pDlListenDes->uPort = (USHORT)wParam;
	SendUploadPort();
	return 0;
}

void CMainFrame::SendUploadPort() 
{
	char msgBuf[sizeof(RCMSG_BUFF) - 1 + sizeof(USHORT)] = {0};
	PRCMSG_BUFF pMsg = (PRCMSG_BUFF)msgBuf;
	pMsg->msgHead.size = sizeof(RCMSG_BUFF) - 1 + sizeof(USHORT);
	pMsg->msgHead.type = MT_LISTEN_DOWNLOAD_PORT_C;
	*((USHORT*)(pMsg->buf)) = m_pDlListenDes->uPort;
	m_pMsgCenter->SendMsg(m_strServerIP , SERVER_MSG_PORT , pMsg);
}

void CMainFrame::OnRcPauseScreen()
{
	if (m_CCScreenThreadContext.lThreadState == 1)//�����е�����¾���ͣ
		InterlockedExchange(&(m_CCScreenThreadContext.lThreadState), 2);
}

void CMainFrame::OnRcResumeScreen()
{
	//����ͣ������
	if (m_CCScreenThreadContext.lThreadState == 2){
		InterlockedExchange(&(m_CCScreenThreadContext.lThreadState) , 1);
		SetEvent(m_CCScreenThreadContext.hEvent);
	}
}
