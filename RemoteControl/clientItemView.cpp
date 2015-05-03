///////////////////////////////////////////////////////////////
//
// FileName	: clientItemView.cpp
// Creator	: ����
// Date		: 2013��2��27��, 23:39:40
// Comment	: �ͻ��˼�����ͼ��ʵ��
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

// ClientItemView ��Ϣ�������

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
	{//�������ǻ���һЩ�򵥵���ʾ��
		CRect rc;
		GetClientRect(&rc);
		CString strInfo;
		strInfo.LoadString(IDS_CLOSED);
		strInfo = strInfo + _T("  ") + m_pClientDescripter->mName;
		strInfo = strInfo + _T("  ") + m_pClientDescripter->mIP;
		dc.DrawText(strInfo , rc , DT_CENTER|DT_VCENTER);
	}
	
	if (m_bDrawEdge)
	{//���Ʊ߿�
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
	
	//���������������ݵ��߳�
	m_PrintScreenThreadContext.hThreadEvent = CreateEvent(NULL , TRUE , TRUE , NULL);
	InterlockedExchange(&(m_PrintScreenThreadContext.lThreadState) , 1);
	DWORD dwID = 0;
	m_PrintScreenThreadContext.hThread = CreateThread(NULL , 0 , PrintScreenThread , &m_PrintScreenThreadContext , 0 , &dwID);
}

LRESULT ClientItemView::OnGetAScreenPort(WPARAM wParam,LPARAM lParam)
{
	//������������screen�˿ڷ��͵��ͻ���
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
		{//�ͻ����Ѿ�׼������ץ���߳�
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
	{//��ʾ����
		DEF_RCMSG(rcMsg , MT_RESUME_SCREEN_S);
		this->m_pDoc->SendRcMessage(m_pClientDescripter->mIP , m_pClientDescripter->mPort, &rcMsg);
		if ((NULL != m_pCmdTable) && (IsWindow(m_pCmdTable->GetSafeHwnd())))
		{//��ʾΪ����ģʽ
			m_pCmdTable->ShowAsIndexLabel();
		}
	}
	else
	{//���ش���
		DEF_RCMSG(rcMsg , MT_PAUSE_SCREEN_S);
		this->m_pDoc->SendRcMessage(m_pClientDescripter->mIP , m_pClientDescripter->mPort, &rcMsg);
		if ((NULL != m_pCmdTable) && (IsWindow(m_pCmdTable->GetSafeHwnd())))
		{//�����������
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
		//����һ����Ϣ֪ͨ�ͻ���ֹͣץ��
		DEF_RCMSG(rcMsg , MT_STOP_SCREEN_CAPTURE_S);
		m_pDoc->SendRcMessage(m_pClientDescripter->mIP , m_pClientDescripter->mPort , &rcMsg);

		InterlockedExchange(&(m_PrintScreenThreadContext.lThreadState) , 0);
		SetEvent(m_PrintScreenThreadContext.hThreadEvent);

		//�ȹر�socket
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
	{//ò�ƿ���������
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
	//ץ������
	LONG q = (LONG)wParam;
	//��Ϣ����
	char buf[sizeof(RCMSG_BUFF) - 1 + sizeof(LONG)] = {0};
	PRCMSG_BUFF pMsg = (PRCMSG_BUFF)buf;
	//������Ϣ����
	pMsg->msgHead.type = MT_SCREEN_QUALITY_S;
	//��Ϣ��С
	pMsg->msgHead.size = sizeof(RCMSG_BUFF) - 1 + sizeof(LONG);
	//����ץ������ֵ����Ϣ����
	memcpy(pMsg->buf , &q , sizeof(q));
	//������Ϣ
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
	//��׽����������������Ϣ
	if(this->m_bScreenCtrled)
	{
		switch(pMsg->message)
		{
		case WM_LBUTTONDOWN:	//���������� 
		case WM_RBUTTONDOWN:	//��������Ҽ� 
			SetFocus();
		case WM_LBUTTONUP:		//�ͷ������� 
		case WM_LBUTTONDBLCLK:	//˫�������� 
		case WM_RBUTTONUP:		//�ͷ�����Ҽ� 
		case WM_RBUTTONDBLCLK:	//˫������Ҽ� 
		case WM_MBUTTONDOWN:	//��������м� 
		case WM_MBUTTONUP:		//�ͷ�����м� 
		case WM_MBUTTONDBLCLK:	//˫������м� 
		case WM_MOUSEWHEEL:		//�����ֹ��� 
		case WM_MOUSEMOVE:		//�ƶ����ʱ���� 
			{
				//��굱ǰ�Ĺ��λ��
				CPoint point;
				::GetCursorPos(&point);

				//ItemView�ľ�������
				CRect viewRect;
				::GetWindowRect(this->GetSafeHwnd() , &viewRect);

				//����¼�ʱ  �����겻�ڵ�ǰ����ʱ�����������Ϣ
				if(!viewRect.PtInRect(point))
					break;

				//��������ItemView��λ��
				::ScreenToClient(this->GetSafeHwnd() , &point);
				//��һ�������Ϣ
				MOUSE_EVENT_MSG msg = {0};
				msg.msgHead.type	= MT_MOUSE_EVENT_S;
				msg.msgHead.size	= sizeof(MOUSE_EVENT_MSG);
				msg.x				= point.x * 65536 / viewRect.Width(); //ת�����������ϵͳ��x
				msg.y				= point.y * 65535 / viewRect.Height();//ת�����������ϵͳ��y
				msg.message			= pMsg->message;
				msg.param			= pMsg->wParam;
				
				//����Ϣ���͵����ض�
				m_pDoc->SendRcMessage(m_pClientDescripter->mIP , m_pClientDescripter->mPort , &msg);
			}
			break;
		case WM_KEYDOWN:		//���̼��̰���
		case WM_KEYUP:			//���̰������ͷ�   
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

	//��ͼ����
	this->GetClientRect(&m_rcScreenViewRect);
	if(m_bDrawEdge)
		m_rcScreenViewRect.DeflateRect( 2 , 2 , 2 , 2 );
}

void ClientItemView::Dropped()
{//�ͻ�������
	StopScreenCaptureThread();
	//�����ػ�һ�°�
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
	{//״̬û�иı�
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
	{//ֻ���ڿɼ�������²ŵ���λ��
		m_pCmdTable->PostMessage(WM_MOVE , 0 , 0);
	}
}
