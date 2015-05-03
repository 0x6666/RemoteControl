// MainFrm.cpp : CMainFrame 类的实现
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


// CMainFrame 构造/析构

CMainFrame::CMainFrame()
: m_pMsgCenter(NULL)
, m_pBroadcastSocket(NULL)
, m_czHostName(NULL)
, m_pLetterRainDlg(NULL)
, m_uPort(0)
, m_bHeartbeatTimerRun(FALSE)
, m_pDlListenDes(NULL)
{
	//发送桌面图像线程
	m_CCScreenThreadContext.hEvent = NULL;
	m_CCScreenThreadContext.lThreadState = 0;
	m_CCScreenThreadContext.uScreenPort = 0;
	m_CCScreenThreadContext.hThread = NULL;

	//CMD读取回显数据线程上下文
	m_cmdContext.hCmd = NULL;
	m_cmdContext.hWrite = NULL;
	m_cmdContext.hRead = NULL;
	m_cmdContext.hReadCMDThread = NULL;
	m_cmdContext.lReadCMDThreadState = 0;

	//接收主控端桌面图像线程上下文
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

	// 创建一个视图以占用框架的工作区
	if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("未能创建视图窗口\n");
		return -1;
	}

	m_pMsgCenter = new MsgCenter();
	if (FALSE == m_pMsgCenter->InitMsgCenter(this , CLIENT_MSG_PORT  , TRUE ))
	{//先使用指定端口试一下，如果失败则在用随机端口
		if (FALSE == m_pMsgCenter->InitMsgCenter(this , 0 , TRUE ))
		{//初始化失败
			delete m_pMsgCenter;
			m_pMsgCenter = NULL;
			return -1;
		}
	}

	//获得主机名 
	char buf[MAX_PATH] = {0};
	if (gethostname(buf , MAX_PATH) == 0)
	{
		int len = strlen(buf);
		m_czHostName = new char[len + 1];
		strcpy(m_czHostName , buf);
	}

	//获得客户端监听的端口
	m_uPort = m_pMsgCenter->GetMsgPort();

	//开始查询服务器广播定时器
	StartFindServerBroadcastTimer();


	//初始化托盘图标变量
	m_notifyIcon.cbSize = sizeof(NOTIFYICONDATA);
	m_notifyIcon.hWnd	= GetSafeHwnd();   
	m_notifyIcon.uID	= ICON_INDEX_C;   
	m_notifyIcon.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP;   
	m_notifyIcon.uCallbackMessage = WM_NOTIFYION_MSG_C;//自定义的消息名称   
	m_notifyIcon.hIcon	= AfxGetApp()->LoadIcon(IDR_MAINFRAME);   
	::wcscpy(m_notifyIcon.szTip , _T("远程监控客户端"));//信息提示条为“计划任务提醒”   
	//在托盘区添加图标
	::Shell_NotifyIcon(NIM_ADD , &m_notifyIcon );   

	{//下载监听线程
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
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	cs.style &= ~WS_SYSMENU;//去掉菜单栏
	cs.style &= ~WS_CAPTION;//去掉标题栏
	cs.style &= ~WS_BORDER;//边框
	cs.style &= ~WS_THICKFRAME;//边框
	cs.hMenu = NULL;
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	cs.cx = 300;
	cs.cy = 260;
	return TRUE;
}


// CMainFrame 诊断

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

// CMainFrame 消息处理程序

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// 将焦点前移到视图窗口
	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// 让视图第一次尝试该命令
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// 否则，执行默认处理
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::DispatchMsg( const void* msg , CString ip, UINT port )
{
	if (NULL == msg)
		ASSERT(FALSE);

	switch(PRCMSG(msg)->type)
	{
	case MT_RECV_ERROR:
		{//接收数据失败
			break;
		}
	case MT_REPEAT_FIND_SERVER_S://关闭定时器
		OnRepeatFindServer(ip);
		break;
	case MT_SCREEN_PORT_S://服务器已经准备好了好了接收screen数据
		StartScreenCaptureThread( msg);
		break;
	case MT_SCREEN_QUALITY_S://设置抓屏的质量
		OnRcScreenQuality(msg);	
		break;
	case MT_SCREEN_FLUENCY_S://设置screen的流畅度
		OnRcScreenFluency(msg);	
		break;
	case MT_STOP_SCREEN_CAPTURE_S://停止抓屏
		OnRcStopScreenCapture(); 
		break;
	case MT_SCREEN_CTRL_S:
		{
			//TODO: 还需要处理服务器需要控制当前机器的命令 
		}
		break;
	case MT_MOUSE_EVENT_S://鼠标事件
		OnRcMouseEvent(msg);
		break;
	case MT_KETBD_EVENT_S://键盘事件
		OnRcKeybdEvent(msg);
		break;
	case MT_SCREEN_SIZE_S://调整抓屏的大小
		OnRcScreenSize(msg); 
		break;
	case MT_PUSH_SERVER_DESKTOP_S://服务器要推送桌面过来了
		OnRcPushServerDesktop();
		break;
	case MT_CALCEL_PUSH_DESKTOP_S://需要取消对当前客户端的推送
		OnRcCancelPushDesktop();
		break;
	case MT_EXIT_CLIENT_S://需要退出客户端
		OnRcExitClient();
		break;
	case MT_SHUTDOWN_CLIENT_S://关机
		OnRcShutdown();
		break;
	case MT_RESTART_CLIENT_S://重启
		OnRcRestart();
		break;
	case MT_LOGIN_OUT_CLIENT_S://注销
		OnRcLoginOut();
		break;
	case MT_LETTER_RAIN_S://字幕雨
		OnRcLetterRain(msg);
		break;
	case MT_START_CMD_S://启动CMD
		if(FALSE == OnRcStartCmd())
		{//创建cmd进程失败
			DEF_RCMSG(rcMsg , MT_START_CMD_FAILED_C);
			m_pMsgCenter->SendMsg(ip , SERVER_MSG_PORT , &rcMsg);
		}
		break;
	case MT_CMD_S://CMD命令
		OnRcCMD(msg);
		break;
	case MT_SERVER_EXIT_S://服务器离线
		OnRcServerExit();
		break;
	case MT_GET_FILE_LIST_S://获取制定路径的文件列表
		OnRcGetFileList(msg);
		break;
	case MT_GET_DRIVER_S://获取驱动器
		OnRcGetDriver();
		break;
	case MT_DELETE_FILE_S://删除文件
		OnRcDeleteFile(msg);
		break;
	case MT_RESUME_SCREEN_S://重新开始发送屏幕图像
		OnRcResumeScreen();
		break;
	case MT_PAUSE_SCREEN_S://暂停发送屏幕图像
		OnRcPauseScreen();
		break;
	default:
		ASSERT(FALSE);
	}
	return ;
}

BOOL CMainFrame::DestroyWindow()
{
	{//停止监听上传
	InterlockedExchange(&(m_pDlListenDes->lThreadState) , 0);
	WaiteExitThread(m_pDlListenDes->hThread , 1000);
	CloseHandle(m_pDlListenDes->hThread);
	delete m_pDlListenDes;
	m_pDlListenDes= NULL;
	}

	//停止上传文件线程
	StopUploadThreads();

	//停止抓屏线程
	OnRcStopScreenCapture();
	//结束接收服务器推送桌面线程
	StopRecvServerPushDektopThread();

	if (NULL != m_pMsgCenter)
	{
		//还是先发个消息通知一下服务器
		DEF_RCMSG(rcMsg , MT_CONN_CLOSED_C);
		m_pMsgCenter->SendMsg(m_strServerIP , SERVER_MSG_PORT , &rcMsg);
		Sleep(0);

		m_pMsgCenter->Close();
		delete m_pMsgCenter;
		m_pMsgCenter = NULL;
	}

	//关闭广播定时器
	StopFindServerBroadcast();

	//关闭心跳包定时器
	StopHeartbeatTimer();

	//删除主机名缓存空间
	if (NULL != m_czHostName)
		delete[] m_czHostName;

	//停止读取CMD的回显数据
	StopReadCMDThread();

	//删除托盘图标
	::Shell_NotifyIcon(NIM_DELETE , &m_notifyIcon);

	return CFrameWnd::DestroyWindow();
}

void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	switch(nIDEvent)
	{
	case BROADCAST_TIMER:
		{//查询服务机
			int len = 0;
			if ((m_czHostName != NULL) && ((len = strlen(m_czHostName)) != 0))
			{
				//通知服务器当前客户端的消息端口和计算机名
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
			{//理论上这里是不可能的
				ASSERT(FALSE);
			}
		}
		break;
	case HEARTBEAT_TIMER://心跳包
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
	{//线程在运行那就先结束
		InterlockedExchange(&(m_CCScreenThreadContext.lThreadState ), 0);
		SetEvent(m_CCScreenThreadContext.hEvent);
		WaiteExitThread(m_CCScreenThreadContext.hThread, 2*1000);
		CloseHandle(m_CCScreenThreadContext.hEvent);
		CloseHandle(m_CCScreenThreadContext.hThread);
		m_CCScreenThreadContext.hThread = NULL;
		m_CCScreenThreadContext.hEvent	= NULL;
	}

	//初始化抓屏线程上下文
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

	//发送一个回馈消息通知已经
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
	{//停止抓屏线程
		InterlockedExchange(&(m_CCScreenThreadContext.lThreadState) , 0);
		SetEvent(m_CCScreenThreadContext.hEvent);

		//抓屏线程
		if(FALSE == ::WaiteExitThread(m_CCScreenThreadContext.hThread , 2*1000))
		{//线程是强行结束的
			if ((m_CCScreenThreadContext.sSocket != 0 )&&
				(m_CCScreenThreadContext.sSocket != INVALID_SOCKET) )
			{//需要关闭套接字
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
	case WM_MOUSEMOVE://    移动鼠标时发生 
		MOUSE_TEST_CTRL_SHIFT_D(pMsg->param);
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE , pMsg->x , pMsg->y , 0 , 0 );
		MOUSE_TEST_CTRL_SHIFT_U(pMsg->param);
		break;
	case WM_LBUTTONDOWN://  按下鼠标左键 
		MOUSE_TEST_CTRL_SHIFT_D(pMsg->param);
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_MOVE , pMsg->x , pMsg->y , 0 , 0 );
		MOUSE_TEST_CTRL_SHIFT_U(pMsg->param);
		break;
	case WM_LBUTTONUP: //   释放鼠标左键 
		MOUSE_TEST_CTRL_SHIFT_D(pMsg->param);
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTUP|MOUSEEVENTF_MOVE , pMsg->x , pMsg->y , 0 , 0 );
		MOUSE_TEST_CTRL_SHIFT_U(pMsg->param);
		break;
	case WM_LBUTTONDBLCLK://双击鼠标左键 
		MOUSE_TEST_CTRL_SHIFT_D(pMsg->param);
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_MOVE , pMsg->x , pMsg->y , 0 , 0 );
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTUP|MOUSEEVENTF_MOVE ,   pMsg->x , pMsg->y , 0 , 0 );
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_MOVE , pMsg->x , pMsg->y , 0 , 0 );
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTUP|MOUSEEVENTF_MOVE ,   pMsg->x , pMsg->y , 0 , 0 );
		MOUSE_TEST_CTRL_SHIFT_U(pMsg->param);
		break;
	case WM_RBUTTONDOWN://  按下鼠标右键 
		MOUSE_TEST_CTRL_SHIFT_D(pMsg->param);
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_RIGHTDOWN|MOUSEEVENTF_MOVE , pMsg->x , pMsg->y , 0 , 0 );
		MOUSE_TEST_CTRL_SHIFT_U(pMsg->param);
		break;
	case WM_RBUTTONUP://    释放鼠标右键 
		MOUSE_TEST_CTRL_SHIFT_D(pMsg->param);
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_RIGHTUP|MOUSEEVENTF_MOVE ,   pMsg->x , pMsg->y , 0 , 0 );
		MOUSE_TEST_CTRL_SHIFT_U(pMsg->param);
		break;
	case WM_RBUTTONDBLCLK://双击鼠标右键 
		MOUSE_TEST_CTRL_SHIFT_D(pMsg->param);
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_RIGHTDOWN|MOUSEEVENTF_MOVE , pMsg->x , pMsg->y , 0 , 0 );
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_RIGHTUP|MOUSEEVENTF_MOVE ,   pMsg->x , pMsg->y , 0 , 0 );
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_RIGHTDOWN|MOUSEEVENTF_MOVE , pMsg->x , pMsg->y , 0 , 0 );
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_RIGHTUP|MOUSEEVENTF_MOVE ,   pMsg->x , pMsg->y , 0 , 0 );
		MOUSE_TEST_CTRL_SHIFT_U(pMsg->param);
		break;
	case WM_MBUTTONDOWN://  按下鼠标中键 
		MOUSE_TEST_CTRL_SHIFT_D(pMsg->param);
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MIDDLEDOWN|MOUSEEVENTF_MOVE , pMsg->x , pMsg->y , 0 , 0 );
		MOUSE_TEST_CTRL_SHIFT_U(pMsg->param);
		break;
	case WM_MBUTTONUP://　　释放鼠标中键 
		MOUSE_TEST_CTRL_SHIFT_D(pMsg->param);
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MIDDLEUP|MOUSEEVENTF_MOVE ,   pMsg->x , pMsg->y , 0 , 0 );
		MOUSE_TEST_CTRL_SHIFT_U(pMsg->param);
		break;
	case WM_MBUTTONDBLCLK://双击鼠标中键 
		MOUSE_TEST_CTRL_SHIFT_D(pMsg->param);
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MIDDLEDOWN|MOUSEEVENTF_MOVE , pMsg->x , pMsg->y , 0 , 0 );
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MIDDLEUP|MOUSEEVENTF_MOVE ,   pMsg->x , pMsg->y , 0 , 0 );
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MIDDLEDOWN|MOUSEEVENTF_MOVE , pMsg->x , pMsg->y , 0 , 0 );
		::mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MIDDLEUP|MOUSEEVENTF_MOVE ,   pMsg->x , pMsg->y , 0 , 0 );
		MOUSE_TEST_CTRL_SHIFT_U(pMsg->param);
		break;
	case WM_MOUSEWHEEL://鼠标滚轮滚动 
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
	case WM_KEYDOWN://键盘键盘按键
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
	case WM_KEYUP://键盘按键的释放
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

	//要抓的图只可以比Screen小
	if (SCREEN_SIZE_W > pMSg->x)
		InterlockedExchange(&(m_CCScreenThreadContext.lScreenW) , pMSg->x);
	if (SCREEN_SIZE_H > pMSg->y)
		InterlockedExchange(&(m_CCScreenThreadContext.lScreenH) , pMSg->y);
}

void CMainFrame::OnRcPushServerDesktop()
{
	//TODO: 启动接收推送的桌面线程
	if( 0 == m_RecvPushedDesktopThreadContext.lThreadState)
	{
		InterlockedExchange(&(m_RecvPushedDesktopThreadContext.lThreadState) , 1);
		DWORD dwThreadID = 0;
		m_RecvPushedDesktopThreadContext.pFrame = this;
		m_RecvPushedDesktopThreadContext.pScreenView = &m_wndView;
		m_RecvPushedDesktopThreadContext.hThread= CreateThread(NULL , 0 , RecvServerPushedDesktopThread , &m_RecvPushedDesktopThreadContext , 0 , &dwThreadID);
	}
	//发送获取桌面的确认消息
	DEF_RCMSG(rcMsg , MT_PUSH_SERVER_DESKTOP_C);
	m_pMsgCenter->SendMsg(m_strServerIP , SERVER_MSG_PORT , &rcMsg);

	//在这里显示窗口
	ShowWindow(SW_SHOW);

	{//全屏显示 
	//去掉标题栏
	ModifyStyle(WS_CAPTION , 0);

	::SetWindowPos(GetSafeHwnd() , HWND_TOPMOST , 0 , 0 , SCREEN_SIZE_W , SCREEN_SIZE_H  , SWP_SHOWWINDOW); 
	//隐藏鼠标
	::ShowCursor(FALSE);
	}//全屏显示 
}

void CMainFrame::StopRecvServerPushDektopThread()
{
	if ( 0 != m_RecvPushedDesktopThreadContext.lThreadState)
	{//停止接收桌面推送线程
		InterlockedExchange(&(m_RecvPushedDesktopThreadContext.lThreadState) , 0);
		WaiteExitThread(m_RecvPushedDesktopThreadContext.hThread , 2*1000);
		CloseHandle(m_RecvPushedDesktopThreadContext.hThread);
		m_RecvPushedDesktopThreadContext.hThread = NULL;
	}
}

void CMainFrame::OnRcCancelPushDesktop()
{
	//需要结束桌面的推送
	StopRecvServerPushDektopThread();

	DEF_RCMSG(rcMsg , MT_CALCEL_PUSH_DESKTOP_C);
	m_pMsgCenter->SendMsg(m_strServerIP , SERVER_MSG_PORT , &rcMsg);

	//隐藏窗口
	ShowWindow(SW_HIDE);

	//显示光标
	::ShowCursor(FALSE);
}

void CMainFrame::OnRcExitClient()
{//退出当前程序
	OnClose();
}

void CMainFrame::OnRcShutdown()
{
	if (FALSE == Shutdown(SD_SHUTDOWN))
	{//操作失败
		DEF_RCMSG(rcMsg , MT_SHUTDOWN_FALIED_C);
		m_pMsgCenter->SendMsg(m_strServerIP , SERVER_MSG_PORT , &rcMsg);
	}
}

void CMainFrame::OnRcRestart()
{
	if (FALSE == Shutdown(SD_RESTART))
	{//操作失败
		DEF_RCMSG(rcMsg , MT_RESTART_FALIED_C);
		m_pMsgCenter->SendMsg(m_strServerIP , SERVER_MSG_PORT , &rcMsg);
	}
}

void CMainFrame::OnRcLoginOut()
{
	if (FALSE == Shutdown(SD_LOGIN_OUT))
	{//操作失败
		DEF_RCMSG(rcMsg , MT_LOGIN_OUT_FALIED_C);
		m_pMsgCenter->SendMsg(m_strServerIP , SERVER_MSG_PORT , &rcMsg);
	}
}

void CMainFrame::OnRcLetterRain( const void* msg )
{
	if (1 == *((BYTE*)&(PRCMSG_BUFF(msg)->buf)))
	{//开启字幕雨
		if(NULL != m_pLetterRainDlg)
			return ;//已经存在
		m_pLetterRainDlg = new CLetterRainDlg();
		m_pLetterRainDlg->Create(IDD_LETTER_RAIN_DLG , this);
		m_pLetterRainDlg->ShowWindow(SW_SHOW);
		UpdateWindow();
	}
	else//停止字幕雨
		StopLetterRain();
}

BOOL CMainFrame::OnRcStartCmd()
{
	if (NULL != m_cmdContext.hRead)
	{//已经创建了cmd子进程
		return TRUE;
	}

	//辅助句柄，用于给CMD读取输入命令
	HANDLE hInnerRead = NULL;
	//辅助句柄，用于给CMD输出执行命令的后的回显数据
	HANDLE hInnerWrite = NULL;

	//BOOL res = FALSE;

	//初始化安cmd进程全描述符
	SECURITY_ATTRIBUTES  sa = {0};
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	
	//创建匿名管道
	if(FALSE == ::CreatePipe(&(m_cmdContext.hRead) , &hInnerWrite , &sa , NULL))
	{//创建管道以失败
		return FALSE;		
	}
	if(FALSE == ::CreatePipe(&hInnerRead , &(m_cmdContext.hWrite) , &sa , NULL))
	{//创建管道以失败
		::CloseHandle(m_cmdContext.hRead);
		m_cmdContext.hRead = NULL;
		::CloseHandle(hInnerWrite);
		hInnerWrite = NULL;
		return FALSE;		
	}

	STARTUPINFO si = {0};
	::GetStartupInfo(&si);
	//改变其标准输入为 输入管道的读取出  标准输出 和错误 为管道一的写
	si.hStdInput   = hInnerRead;
	si.hStdError   = hInnerWrite;
	si.hStdOutput  = hInnerWrite;
	si.dwFlags     = STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
	//隐藏CMD子进程窗口
	si.wShowWindow = SW_HIDE;

	//进程信息
	PROCESS_INFORMATION ProInfo = {0};

	//获取cmd的路径
	CString strCmdPath = GetCMDPath();
	if (0 == strCmdPath.GetLength())
	{//获取环境cmd路径失败
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

	//创建子进程
	if(FALSE == ::CreateProcess(strCmdPath , NULL , NULL , NULL , TRUE 
		, NULL , NULL , NULL , &si , &ProInfo))
	{//创建子进程失败
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

	//子进程句柄
	m_cmdContext.hCmd = ProInfo.hProcess;
	
	//关闭主线程句柄
	CloseHandle(ProInfo.hThread);

	//创建读取CMD的回显数据
	InterlockedExchange(&(m_cmdContext.lReadCMDThreadState) , 1);

	//服务端IP地址
	m_cmdContext.strIP = m_strServerIP;

	DWORD dwThreadID = 0;
	m_cmdContext.hReadCMDThread = CreateThread(NULL , 0 , ReadCMDThread , &m_cmdContext , 0 , &dwThreadID);
	
	return TRUE;
}

CString CMainFrame::GetCMDPath()
{
	CString sEnvironmentName = _T("ComSpec");
	TCHAR cBuffer[256];

	// 读取环境变量信息
	DWORD dwRet = GetEnvironmentVariable(sEnvironmentName, cBuffer, 256);
	if (dwRet == 0)
	{//获取环境变量失败
		return CString();
	}
	else
	{//已经正确获取了cmd路径
		return CString(cBuffer);
	}
}

LRESULT CMainFrame::OnNotifyIconMsg(WPARAM wParam , LPARAM lParam)
{
	if(wParam != ICON_INDEX_C)
		return  1;//不是本程序的托盘图标

	switch(lParam)   
	{
	case WM_RBUTTONUP://右键起来时弹出快捷菜单
		{
			CMenu m , *p;
			CPoint point;
			m.LoadMenu(IDR_ICON_MENU);
			p = m.GetSubMenu(0);
			::GetCursorPos(&point);
			::TrackPopupMenu(p->m_hMenu , TPM_LEFTALIGN|TPM_BOTTOMALIGN , point.x , point.y , 0 , GetSafeHwnd() , NULL);
		}
		break;
	case WM_LBUTTONDBLCLK://双击左键的处理   
		//ShowWindow(IsWindowVisible()?SW_HIDE:SW_SHOW);
		break;   
	}   
	return 0;
}

void CMainFrame::OnExit()
{
	//这个需要查询服务器是否同意退出
	OnClose();
}

void CMainFrame::OnHandsUp()
{
	//举手
	DEF_RCMSG(rcMsg , MT_HANDS_UP_C);
	m_pMsgCenter->SendMsg(m_strServerIP , SERVER_MSG_PORT , &rcMsg);
}

void CMainFrame::OnRepeatFindServer( const CString& ip )
{
	m_strServerIP = ip;
	
	//关闭查找服务器的广播
	StopFindServerBroadcast();

	//启动心跳包广播
	if (FALSE == m_bHeartbeatTimerRun)
	{
		m_bHeartbeatTimerRun = TRUE;
		SetTimer(HEARTBEAT_TIMER , HEARTBEAT_PACKETS_TIME/2 , NULL);
	}

	//发送监听下载端口
	SendUploadPort();
}

void CMainFrame::StopHeartbeatTimer()
{
	if (TRUE == m_bHeartbeatTimerRun)
	{//心跳包定时器好在运行
		KillTimer(HEARTBEAT_TIMER);
		m_bHeartbeatTimerRun = FALSE;
	}
}

void CMainFrame::OnRcCMD( const void* msg )
{
	PRCMSG_BUFF pMsg = PRCMSG_BUFF(msg);
	DWORD dwSize = 0;
	if(!::WriteFile(m_cmdContext.hWrite , pMsg->buf , ::strlen(pMsg->buf) , &dwSize , 0))
	{//执行命令失败
		pMsg->msgHead.type = MT_CMD_FAILED_C;
		m_pMsgCenter->SendMsg(m_strServerIP , m_uPort , pMsg);
		return ;
	}
}

void CMainFrame::StopLetterRain()
{
	if(NULL != m_pLetterRainDlg)
	{//停止字幕雨	
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


		//关闭cmd读写句柄
		CloseHandle(m_cmdContext.hRead);
		CloseHandle(m_cmdContext.hWrite);
		m_cmdContext.hRead  = NULL;
		m_cmdContext.hWrite = NULL;

		//结束cmd进程
		TerminateProcess( m_cmdContext.hCmd , 4 );
		CloseHandle(m_cmdContext.hCmd);
		m_cmdContext.hCmd = NULL;
	}
}

void CMainFrame::OnRcServerExit()
{
	//停止抓屏线程
	OnRcStopScreenCapture();

	//停止接收服务器推送桌面
	StopRecvServerPushDektopThread();

	//停止字幕雨
	StopLetterRain();

	//停止读取cmd数据的线程
	StopReadCMDThread();

	//停止发送心跳包
	StopHeartbeatTimer();
	
	//开始查询服务器 
	StartFindServerBroadcastTimer();

}

void CMainFrame::StartFindServerBroadcastTimer()
{
	//用于查询服务器的广播套接字
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
		{//设置套接字广播属性失败
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
	//直接开启一个线程来枚举所有的文件
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
	{//删除文件失败
		pMsg->msgHead.type = MT_DELETE_FILE_FAILED_C;
	}else{
		pMsg->msgHead.type = MT_DELETE_FILE_SUCCESS_C;
	}
	//发送一个反馈消息
	m_pMsgCenter->SendMsg(m_strServerIP , SERVER_MSG_PORT , pMsg);
}

LRESULT CMainFrame::OnCreateUploadThread( WPARAM wParam , LPARAM lParam )
{
	PUploadDescripter pudes = new UploadDescripter();
	pudes->sSocket = (SOCKET)wParam;
	InterlockedExchange(&(pudes->lThreadState ), 1);
	pudes->pWnd = this;
	m_listUploadDescripter.push_back(pudes);

	{//创建线程
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
	if (m_CCScreenThreadContext.lThreadState == 1)//在运行的情况下就暂停
		InterlockedExchange(&(m_CCScreenThreadContext.lThreadState), 2);
}

void CMainFrame::OnRcResumeScreen()
{
	//在暂停就重启
	if (m_CCScreenThreadContext.lThreadState == 2){
		InterlockedExchange(&(m_CCScreenThreadContext.lThreadState) , 1);
		SetEvent(m_CCScreenThreadContext.hEvent);
	}
}
