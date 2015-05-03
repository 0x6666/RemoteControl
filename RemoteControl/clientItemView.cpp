///////////////////////////////////////////////////////////////
//
// FileName	: clientItemView.cpp
// Creator	: 杨松
// Date		: 2013年2月27日, 23:39:40
// Comment	: 客户端监视视图类实现
//
//////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "RemoteControl.h"
#include "clientItemView.h"
#include "RemoteControlDoc.h"
#include "MainFrm.h"
#include "CMDDlg.h"
#include "CMDTable.h"


// ClientItemView

IMPLEMENT_DYNAMIC(ClientItemView, CWnd)

ClientItemView::ClientItemView(PClientDescripter clientDes ,CRemoteControlDoc* pDoc)
: m_pClientDescripter(clientDes)//m_strIP(ip)
//, m_strName(name)
, m_pDoc(pDoc)
, m_pCmdTable(NULL)
, m_bScreenCtrled(FALSE)
, m_bLetterRain(FALSE)
, m_bDrawEdge(TRUE)
{
	m_PrintScreenThreadContext.lThreadState = 0;
	m_PrintScreenThreadContext.hThread = NULL;
	m_PrintScreenThreadContext.hThreadEvent = NULL;
}

ClientItemView::~ClientItemView()
{
}


BEGIN_MESSAGE_MAP(ClientItemView, CWnd)
	ON_WM_PAINT()
	ON_MESSAGE(WM_GET_A_SCREEN_PORT , OnGetAScreenPort)
	ON_MESSAGE(WM_SCREEN_SOCKET_ERR , OnCreatScreenSockErr)
	ON_MESSAGE(WM_SCREEN_THREAD_EXIT , OnScreenThreadExit)
	ON_MESSAGE(WM_WAIT_FOR_CAPTURE_TIMEOUT , OnWaitForCaptureTimeOut)
	ON_MESSAGE(WM_CHANGE_SCREEN_AUALITY , OnChangeScreenQuality)
	ON_MESSAGE(WM_CHANGE_SCREEN_FLUENCY , OnChangeScreenFluency)
	ON_MESSAGE(WM_SCREEN_CTRL , OnScreenCtrl)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SHOWWINDOW()
	ON_WM_CREATE()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_COMMAND(ID_STOP_CTRL, &ClientItemView::OnStopCtrl)
	ON_COMMAND(ID_EXIT_CLIENT, &ClientItemView::OnExitClient)
	ON_COMMAND(ID_SHUTDWON_CLIENT, &ClientItemView::OnShutdwonClient)
	ON_COMMAND(ID_RESARTE_CLIENT, &ClientItemView::OnResarteClient)
	ON_COMMAND(ID_LOGIN_OUT, &ClientItemView::OnLoginOut)
	ON_COMMAND(ID_LETTER_RAIN, &ClientItemView::OnLetterRain)
	ON_MESSAGE(WM_CMD_CLICKED , OnCMDClicked)
	ON_MESSAGE(WM_FULLS_CREEN , OnFullScreen)
	ON_MESSAGE(WM_DOC_MANAGE_CLICKED , OnDocManage)
	ON_UPDATE_COMMAND_UI(ID_LETTER_RAIN, &ClientItemView::OnUpdateLetterRain)
	ON_WM_MOVE()
END_MESSAGE_MAP()

// ClientItemView 消息处理程序

int ClientItemView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_pCmdTable = new CCMDTable(this , m_pClientDescripter->mIndex);
	m_pCmdTable->Create(IDD_CMDTABLE , this);
	m_pCmdTable->UpdateWindow();
	return 0;
}

void ClientItemView::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	if (0 == m_PrintScreenThreadContext.lThreadState)
	{//不在线是绘制一些简单的提示语
		CRect rc;
		GetClientRect(&rc);
		CString strInfo;
		strInfo.LoadString(IDS_CLOSED);
		strInfo = strInfo + _T("  ") + m_pClientDescripter->mName;
		strInfo = strInfo + _T("  ") + m_pClientDescripter->mIP;
		dc.DrawText(strInfo , rc , DT_CENTER|DT_VCENTER);
	}
	
	if (m_bDrawEdge)
	{//绘制边框
		CPen Pen;
		Pen.CreatePen(PS_SOLID, 2, RGB(0 , 0 , 0) );
		CPen* pOld = dc.SelectObject( &Pen );

		CRect rc;
		this->GetClientRect(&rc);
		rc.DeflateRect( 1 , 1 , 0 , 0 );
		dc.Rectangle(rc );
		dc.SelectObject(pOld);
	}
}

const CString& ClientItemView::GetClientIP(void)
{
	return m_pClientDescripter->mIP;
}

BOOL ClientItemView::Create(/* RECT &rc,*/CWnd *pParent , UINT id)
{
	BOOL bRes;
	bRes = CWnd::Create(_T("STATIC") , 	NULL , WS_CHILD|SS_NOTIFY , CRect(0 , 0 , 100 , 100) , pParent , id);

	if (TRUE == bRes)
		InitItemView();

	return bRes;
}

void ClientItemView::InitItemView()
{
	m_PrintScreenThreadContext.pWnd = this;
	
	//创建接收桌面数据的线程
	m_PrintScreenThreadContext.hThreadEvent = CreateEvent(NULL , TRUE , TRUE , NULL);
	InterlockedExchange(&(m_PrintScreenThreadContext.lThreadState) , 1);
	DWORD dwID = 0;
	m_PrintScreenThreadContext.hThread = CreateThread(NULL , 0 , PrintScreenThread , &m_PrintScreenThreadContext , 0 , &dwID);
}

LRESULT ClientItemView::OnGetAScreenPort(WPARAM wParam,LPARAM lParam)
{
	//将服务器接收screen端口发送到客户端
	USHORT port = wParam;
	char msgBuf[sizeof(RCMSG_BUFF) - 1 + sizeof(USHORT)] = {0};
	RCMSG_BUFF* pMsg = (RCMSG_BUFF*)msgBuf;
	pMsg->msgHead.size = sizeof(RCMSG_BUFF) - 1 + sizeof(USHORT);
	pMsg->msgHead.type = MT_SCREEN_PORT_S;
	memcpy(pMsg->buf , &port , sizeof(USHORT) );
	m_pDoc->SendRcMessage(m_pClientDescripter->mIP , m_pClientDescripter->mPort , pMsg );
	return 0;
}

const CRect& ClientItemView::GetScreenViewRect()
{
	return m_rcScreenViewRect;
}

LRESULT ClientItemView::OnCreatScreenSockErr( WPARAM wParam,LPARAM lParam )
{
	return 0;
}

LRESULT ClientItemView::OnScreenThreadExit( WPARAM wParam,LPARAM lParam )
{
	return 0;
}

void ClientItemView::OnRcMessage( USHORT port, const void* msg )
{
	switch(PRCMSG(msg)->type)
	{
	case MT_SCREEN_CAPTURE_START_C:
		{//客户端已经准备好了抓屏线程
			if (0 == m_PrintScreenThreadContext.lThreadState)
				InterlockedExchange(&(m_PrintScreenThreadContext.lThreadState) , 1);
			SetEvent(m_PrintScreenThreadContext.hThreadEvent);			
		}
		break;
	default:
		ASSERT(FALSE);
	}	
}

LRESULT ClientItemView::OnWaitForCaptureTimeOut( WPARAM wParam,LPARAM lParam )
{
	StopScreenCaptureThread();

	CString strMsg;
	strMsg.LoadString(IDS_WAITE_FOR_CAPTURE_TIMEOUT);
	strMsg.Replace(STR_RP , m_pClientDescripter->mName);
	AfxMessageBox(strMsg);
	return 0;
}

void ClientItemView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (FALSE == m_bScreenCtrled)
		GetParent()->SendMessage(WM_ITEM_VIEW_SBCLICK , (WPARAM)&(m_pClientDescripter->mIP) , 0);
	CWnd::OnLButtonDblClk(nFlags, point);
}

void ClientItemView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CWnd::OnShowWindow(bShow, nStatus);
	if (bShow)
	{//显示窗口
		DEF_RCMSG(rcMsg , MT_RESUME_SCREEN_S);
		this->m_pDoc->SendRcMessage(m_pClientDescripter->mIP , m_pClientDescripter->mPort, &rcMsg);
		if ((NULL != m_pCmdTable) && (IsWindow(m_pCmdTable->GetSafeHwnd())))
		{//显示为索引模式
			m_pCmdTable->ShowAsIndexLabel();
		}
	}
	else
	{//隐藏窗口
		DEF_RCMSG(rcMsg , MT_PAUSE_SCREEN_S);
		this->m_pDoc->SendRcMessage(m_pClientDescripter->mIP , m_pClientDescripter->mPort, &rcMsg);
		if ((NULL != m_pCmdTable) && (IsWindow(m_pCmdTable->GetSafeHwnd())))
		{//隐藏命令面板
			m_pCmdTable->ShowWindow(SW_HIDE);
		}
	}
	
}

BOOL ClientItemView::DestroyWindow()
{
	StopScreenCaptureThread();
	if (m_pCmdTable)
	{
		m_pCmdTable->DestroyWindow();
		delete m_pCmdTable;
		m_pCmdTable = NULL;
	}

	return CWnd::DestroyWindow();
}

void ClientItemView::StopScreenCaptureThread() 
{
	if (m_PrintScreenThreadContext.lThreadState)
	{
		//发送一个消息通知客户端停止抓屏
		DEF_RCMSG(rcMsg , MT_STOP_SCREEN_CAPTURE_S);
		m_pDoc->SendRcMessage(m_pClientDescripter->mIP , m_pClientDescripter->mPort , &rcMsg);

		InterlockedExchange(&(m_PrintScreenThreadContext.lThreadState) , 0);
		SetEvent(m_PrintScreenThreadContext.hThreadEvent);

		//先关闭socket
		if ((m_PrintScreenThreadContext.sSocket != NULL)&&
			(m_PrintScreenThreadContext.sSocket != INVALID_SOCKET))
		{
			closesocket(m_PrintScreenThreadContext.sSocket);
			m_PrintScreenThreadContext.sSocket = 0;
		}

		WaiteExitThread(m_PrintScreenThreadContext.hThread , 2*1000);
		CloseHandle(m_PrintScreenThreadContext.hThreadEvent);
		m_PrintScreenThreadContext.hThreadEvent = NULL;
		CloseHandle(m_PrintScreenThreadContext.hThread);
		m_PrintScreenThreadContext.hThread = NULL;
	}
}

void ClientItemView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_pCmdTable->IsShowAsLabel())
	{//貌似可以隐藏了
		CRect rc;
		CPoint p = point;
		m_pCmdTable->GetClientRect(&rc);
		m_pCmdTable->ClientToScreen(&rc);
		this->ClientToScreen(&p);
		if (!rc.PtInRect(p))
		{
			//m_pCmdTable->SmoothHideWindow();
			m_pCmdTable->ShowAsIndexLabel();
		}
	}

	CWnd::OnMouseMove(nFlags, point);
}

LRESULT ClientItemView::OnChangeScreenQuality( WPARAM wParam,LPARAM lParam )
{
	//抓屏质量
	LONG q = (LONG)wParam;
	//消息缓存
	char buf[sizeof(RCMSG_BUFF) - 1 + sizeof(LONG)] = {0};
	PRCMSG_BUFF pMsg = (PRCMSG_BUFF)buf;
	//设置消息类型
	pMsg->msgHead.type = MT_SCREEN_QUALITY_S;
	//消息大小
	pMsg->msgHead.size = sizeof(RCMSG_BUFF) - 1 + sizeof(LONG);
	//拷贝抓屏质量值到消息缓存
	memcpy(pMsg->buf , &q , sizeof(q));
	//发送消息
	m_pDoc->SendRcMessage(m_pClientDescripter->mIP , m_pClientDescripter->mPort , pMsg);
	return 0;
}

LRESULT ClientItemView::OnChangeScreenFluency( WPARAM wParam,LPARAM lParam )
{
	LONG f = (LONG)wParam;
	f = IndexToFrequency(f);
	char buf[sizeof(RCMSG_BUFF) - 1 + sizeof(LONG)] = {0};
	PRCMSG_BUFF pMsg = (PRCMSG_BUFF)buf;
	pMsg->msgHead.type = MT_SCREEN_FLUENCY_S;
	pMsg->msgHead.size = sizeof(RCMSG_BUFF) - 1 + sizeof(LONG);
	memcpy(pMsg->buf , &f , sizeof(f));
	m_pDoc->SendRcMessage(m_pClientDescripter->mIP , m_pClientDescripter->mPort, pMsg);
	return 0;
}

LRESULT ClientItemView::OnScreenCtrl( WPARAM wParam,LPARAM lParam )
{
	m_bScreenCtrled = wParam?TRUE : FALSE;
	RCMSG_BUFF rcMsg = {0};
	rcMsg.msgHead.type = MT_SCREEN_CTRL_S;
	rcMsg.msgHead.size = sizeof(RCMSG_BUFF);
	rcMsg.buf[0] = (char)m_bScreenCtrled;
	m_pDoc->SendRcMessage(m_pClientDescripter->mIP , m_pClientDescripter->mPort , &rcMsg);

	return 0;
}

BOOL ClientItemView::PreTranslateMessage(MSG* pMsg)
{
	//扑捉控制桌面控制相关消息
	if(this->m_bScreenCtrled)
	{
		switch(pMsg->message)
		{
		case WM_LBUTTONDOWN:	//按下鼠标左键 
		case WM_RBUTTONDOWN:	//按下鼠标右键 
			SetFocus();
		case WM_LBUTTONUP:		//释放鼠标左键 
		case WM_LBUTTONDBLCLK:	//双击鼠标左键 
		case WM_RBUTTONUP:		//释放鼠标右键 
		case WM_RBUTTONDBLCLK:	//双击鼠标右键 
		case WM_MBUTTONDOWN:	//按下鼠标中键 
		case WM_MBUTTONUP:		//释放鼠标中键 
		case WM_MBUTTONDBLCLK:	//双击鼠标中键 
		case WM_MOUSEWHEEL:		//鼠标滚轮滚动 
		case WM_MOUSEMOVE:		//移动鼠标时发生 
			{
				//鼠标当前的光标位置
				CPoint point;
				::GetCursorPos(&point);

				//ItemView的矩形区域
				CRect viewRect;
				::GetWindowRect(this->GetSafeHwnd() , &viewRect);

				//鼠标事件时  如果光标不在当前区域时不发送相关消息
				if(!viewRect.PtInRect(point))
					break;

				//光标相对于ItemView的位置
				::ScreenToClient(this->GetSafeHwnd() , &point);
				//定一个鼠标消息
				MOUSE_EVENT_MSG msg = {0};
				msg.msgHead.type	= MT_MOUSE_EVENT_S;
				msg.msgHead.size	= sizeof(MOUSE_EVENT_MSG);
				msg.x				= point.x * 65536 / viewRect.Width(); //转换到鼠标坐标系统的x
				msg.y				= point.y * 65535 / viewRect.Height();//转换到鼠标坐标系统的y
				msg.message			= pMsg->message;
				msg.param			= pMsg->wParam;
				
				//将消息发送到被控端
				m_pDoc->SendRcMessage(m_pClientDescripter->mIP , m_pClientDescripter->mPort , &msg);
			}
			break;
		case WM_KEYDOWN:		//键盘键盘按键
		case WM_KEYUP:			//键盘按键的释放   
			{
				KEYBD_EVENT_MSG msg = {0};
				msg.msgHead.size = sizeof(KEYBD_EVENT_MSG);
				msg.msgHead.type = MT_KETBD_EVENT_S;
				msg.message = pMsg->message;
				msg.vKey = pMsg->wParam;
				msg.keyState = 0;
				if (KEY_DOWN(VK_CONTROL))
					msg.keyState |= KS_CTRL;
				if (KEY_DOWN(VK_SHIFT))
					msg.keyState |= KS_SHIFT;
				if (KEY_DOWN(VK_MENU))
					msg.keyState |= KS_ALT;
				this->m_pDoc->SendRcMessage(m_pClientDescripter->mIP , m_pClientDescripter->mPort, &msg);
			}
			break;
		default:
			break;
		}
	}

	return CWnd::PreTranslateMessage(pMsg);
}

void ClientItemView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	RCMSG_SCREEN_SIZE sizeMsg = {0};
	sizeMsg.msgHead.size = sizeof(RCMSG_SCREEN_SIZE);
	sizeMsg.msgHead.type = MT_SCREEN_SIZE_S;
	sizeMsg.x = cx;
	sizeMsg.y = cy;
	m_pDoc->SendRcMessage(m_pClientDescripter->mIP , m_pClientDescripter->mPort, &sizeMsg);

	//绘图区域
	this->GetClientRect(&m_rcScreenViewRect);
	if(m_bDrawEdge)
		m_rcScreenViewRect.DeflateRect( 2 , 2 , 2 , 2 );
}

void ClientItemView::Dropped()
{//客户端下线
	StopScreenCaptureThread();
	//还是重绘一下吧
	this->PostMessage(WM_PAINT , 0 , 0);
}

void ClientItemView::OnStopCtrl()
{
	StopScreenCaptureThread();
	this->GetParent()->PostMessage(WM_MONITORINT_CLIENT , (WPARAM)&(m_pClientDescripter->mIP) , 0);
}

void ClientItemView::OnExitClient()
{
	DEF_RCMSG(rcMsg , MT_EXIT_CLIENT_S);
	this->m_pDoc->SendRcMessage(m_pClientDescripter->mIP , m_pClientDescripter->mPort, &rcMsg);
}

void ClientItemView::OnShutdwonClient()
{
	DEF_RCMSG(rcMsg , MT_SHUTDOWN_CLIENT_S);
	this->m_pDoc->SendRcMessage(m_pClientDescripter->mIP , m_pClientDescripter->mPort, &rcMsg);	
}

void ClientItemView::OnResarteClient()
{
	DEF_RCMSG(rcMsg , MT_RESTART_CLIENT_S);
	this->m_pDoc->SendRcMessage(m_pClientDescripter->mIP , m_pClientDescripter->mPort , &rcMsg);	
}

void ClientItemView::OnLoginOut()
{
	DEF_RCMSG(rcMsg , MT_LOGIN_OUT_CLIENT_S);
	this->m_pDoc->SendRcMessage(m_pClientDescripter->mIP , m_pClientDescripter->mPort, &rcMsg);
}

void ClientItemView::OnLetterRain()
{
	if (TRUE == m_bLetterRain)
		m_bLetterRain = FALSE;
	else
		m_bLetterRain = TRUE;
	RCMSG_BUFF rcMsg = {0};
	rcMsg.msgHead.size = sizeof(RCMSG_BUFF);
	rcMsg.msgHead.type = MT_LETTER_RAIN_S;
	*((BYTE*)&(rcMsg.buf))= m_bLetterRain ? 1 : 0;
	this->m_pDoc->SendRcMessage(m_pClientDescripter->mIP , m_pClientDescripter->mPort , &rcMsg);
}

void ClientItemView::OnUpdateLetterRain(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bLetterRain);
}

LRESULT ClientItemView::OnCMDClicked( WPARAM wParam,LPARAM lParam )
{
	m_pDoc->CreateCMDDlg(m_pClientDescripter->mIP);
	return 0;
}

BOOL ClientItemView::SetDrawEdge( BOOL drawEdge )
{
	if (m_bDrawEdge == drawEdge)
	{//状态没有改变
		return drawEdge;
	}

	BOOL old = m_bDrawEdge;
	m_bDrawEdge = drawEdge;
	this->SendMessage(WM_PAINT , 0 , 0);

	return old;
}

LRESULT ClientItemView::OnFullScreen( WPARAM wParam,LPARAM lParam )
{
	if (FALSE == m_bScreenCtrled)
		GetParent()->SendMessage(WM_FULLS_CREEN , (WPARAM)&(m_pClientDescripter->mIP) , 0);
	AfxGetMainWnd()->SendMessage(WM_FULLS_CREEN , wParam , 0);
	return 0;
}

LRESULT ClientItemView::OnDocManage( WPARAM wParam,LPARAM lParam )
{
	m_pDoc->CreateDocManageDlg(m_pClientDescripter->mIP);
	return 0;
}

void ClientItemView::OnMove(int x, int y)
{
	CWnd::OnMove(x, y);

	if ((NULL != m_pCmdTable) 
		&& (IsWindow(m_pCmdTable->GetSafeHwnd())) &&
		m_pCmdTable->IsWindowVisible())
	{//只有在可见的情况下才调整位置
		m_pCmdTable->PostMessage(WM_MOVE , 0 , 0);
	}
}
