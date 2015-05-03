///////////////////////////////////////////////////////////////
//
// FileName	: MainFrm.cpp  
// Creator	: ����
// Date		: 2013��2��27��, 20:10:26
// Comment	: Զ�ؿ��Ʒ���������ܵ����ʵ��
//
//////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RemoteControl.h"

#include "MainFrm.h"
#include "CtrlBar.h"
#include "RemoteControlDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_COMMAND(ID_CLIENT_LIST, &CMainFrame::OnClientList)
	ON_UPDATE_COMMAND_UI(ID_CLIENT_LIST, &CMainFrame::OnUpdateClientList)
	ON_MESSAGE(WM_NOTIFYION_MSG_S , OnNotifyIconMsg)
	ON_COMMAND(ID_EXIT, &CMainFrame::OnExit)
	ON_MESSAGE(WM_CLIENT_COUNT , OnClientCount)
	ON_MESSAGE(WM_FULLS_CREEN , OnFullScreen)
	ON_MESSAGE(WM_PUSHED_COUNT , OnPushedCount)
	ON_MESSAGE(WM_MESSAGE , OnMessage)
	ON_WM_MOVE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // ״̬��ָʾ��
	ID_CLIENT_CNT,
	ID_ONLINT_CLIENT_CNT,
	ID_PUSH_CLINT_CNT,
};


// CMainFrame ����/����

CMainFrame::CMainFrame()
: m_pClientPane(NULL)
, m_bFSStatusBarWasVisible(TRUE)
, m_bFSClientListWasVisible(TRUE)
{

}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	//��ʼ��״̬��
	if(FALSE == InitStatusBar())
		return -1;

	//�����ͻ������
	if (FALSE == CreateClientPane())
		return -1;


	//��ʼ������ͼ�����
	m_notifyIcon.cbSize = sizeof(NOTIFYICONDATA);
	m_notifyIcon.hWnd = this->m_hWnd;   
	m_notifyIcon.uID = ICON_INDEX_S;   
	m_notifyIcon.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP;   
	m_notifyIcon.uCallbackMessage = WM_NOTIFYION_MSG_S;//�Զ������Ϣ����   
	m_notifyIcon.hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);   
	::wcscpy(m_notifyIcon.szTip , _T("Զ�̼�ط�����"));//��Ϣ��ʾ��Ϊ���ƻ��������ѡ�   
	//�����������ͼ��
	::Shell_NotifyIcon(NIM_ADD , &m_notifyIcon );   


	//������ʼ���ͻ���״̬��ʱ��
	SetTimer(CHECK_CLIENT_STATE_TIME , HEARTBEAT_PACKETS_TIME , NULL);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ

	//ȥ���ĵ�����
	cs.style &=~ (LONG) FWS_ADDTOTITLE;

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
BOOL CMainFrame::CreateClientPane()
{
	CSize size(240 , 400);

	//�����ͻ������
	m_pClientPane = new CCtrlBar();
	if (!m_pClientPane->Create(this, IDD_PANE, CBRS_LEFT|CBRS_TOOLTIPS|CBRS_FLYBY,
		IDD_PANE ,size))
	{
		TRACE0("�����ͻ������ʧ��\n");
		delete m_pClientPane;
		m_pClientPane = NULL;
		return FALSE; 
	}

	m_pClientList = new CListCtrl();
	if (!m_pClientList->Create(LVS_SHOWSELALWAYS|LVS_SHAREIMAGELISTS|LVS_REPORT|WS_BORDER
		|WS_TABSTOP|WS_CHILD|WS_VISIBLE
		,CRect() , m_pClientPane , IDC_CLIENT_LIST))
	{
		TRACE0("�����ͻ����б�ؼ�ʧ��\n");
		return FALSE; 
	}
	//����һ���б������
	DWORD dwStyle = m_pClientList->GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;//ѡ��ĳ��ʹ���и���
	dwStyle |= LVS_EX_GRIDLINES;	//������
	m_pClientList->SetExtendedStyle(dwStyle);
	InitClientListCtrl();


	//�����豸�б��ͣ��
	m_pClientPane->SetBarStyle(CBRS_ALIGN_LEFT|CBRS_GRIPPER |\
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_pClientPane->EnableDocking(CBRS_ALIGN_LEFT|CBRS_ALIGN_RIGHT);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(m_pClientPane , CBRS_ALIGN_LEFT);
	ShowControlBar(m_pClientPane , TRUE, FALSE);

	return TRUE;
}

void CMainFrame::InitClientListCtrl()
{
	//��ʼ���б�ؼ�
	this->m_pClientList->SetExtendedStyle(LVS_EX_FULLROWSELECT);
	this->m_pClientList->InsertColumn(CLIENT_LIST_ID , _T("ID") , 0 , 30);
	this->m_pClientList->InsertColumn(CLIENT_LIST_STATU , _T("״̬") , 0 , 50);
	this->m_pClientList->InsertColumn(CLIENT_LIST_PUSH , _T("����") , 0 , 40);
	this->m_pClientList->InsertColumn(CLIENT_LIST_NAME , _T("�������") , 0 , 80);
	this->m_pClientList->InsertColumn(CLIENT_LIST_IP , _T("IP") , 0 , 100);
}


// CMainFrame ��Ϣ�������

BOOL CMainFrame::DestroyWindow()
{
	//�Ȼ��ǹرռ��ͻ���״̬��ʱ����
	KillTimer(CHECK_CLIENT_STATE_TIME);

	if (NULL != m_pClientList)
	{
		m_pClientList->DestroyWindow();
		delete m_pClientList;
		m_pClientList = NULL;
	}
	if (NULL != m_pClientPane)
	{
		m_pClientPane->DestroyWindow();
		delete m_pClientPane;
		m_pClientPane = NULL;
	}

	//ɾ������ͼ��
	::Shell_NotifyIcon(NIM_DELETE , &m_notifyIcon);

	return CFrameWnd::DestroyWindow();
}


void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	switch(nIDEvent)
	{
	case CHECK_CLIENT_STATE_TIME:
		{
			CRemoteControlDoc* pDoc = dynamic_cast<CRemoteControlDoc*>(GetActiveDocument());
			if (NULL != pDoc)
				pDoc->CheckClientState();
		}
		break;
	default:
		break;
	}

	CFrameWnd::OnTimer(nIDEvent);
}

void CMainFrame::OnClientList()
{
	ShowControlBar(m_pClientPane, !m_pClientPane->IsVisible(), FALSE);
}

void CMainFrame::OnUpdateClientList(CCmdUI *pCmdUI)
{
	if (m_pClientPane->IsVisible())
		pCmdUI->SetCheck(TRUE);
	else
		pCmdUI->SetCheck(FALSE);
}

LRESULT CMainFrame::OnNotifyIconMsg( WPARAM wParam , LPARAM lParam )
{
	if(wParam != ICON_INDEX_S)
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
			::TrackPopupMenu(p->m_hMenu , TPM_LEFTALIGN|TPM_BOTTOMALIGN , point.x , point.y , 0 , this->GetSafeHwnd() , NULL);
		}
		break;
	case WM_LBUTTONDBLCLK://˫������Ĵ���   
		this->ShowWindow(this->IsWindowVisible()?SW_HIDE:SW_SHOW);
		break;   
	}   
	return 0;
}

void CMainFrame::OnExit()
{
	OnClose();
}

BOOL CMainFrame::InitStatusBar()
{
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("δ�ܴ���״̬��\n");
		return FALSE;      // δ�ܴ���
	}

	CString strTemp;
	m_wndStatusBar.SetPaneInfo( 1 , ID_CLIENT_CNT , SBPS_NORMAL , 100);
	strTemp.LoadString(ID_CLIENT_CNT );
	m_wndStatusBar.SetPaneText(1 , strTemp);

	m_wndStatusBar.SetPaneInfo(2 , ID_ONLINT_CLIENT_CNT , SBPS_NORMAL , 100);
	strTemp.LoadString(ID_ONLINT_CLIENT_CNT );
	m_wndStatusBar.SetPaneText(2 , strTemp);

	m_wndStatusBar.SetPaneInfo(3 , ID_PUSH_CLINT_CNT , SBPS_NORMAL , 100);
	strTemp.LoadString(ID_PUSH_CLINT_CNT);
	m_wndStatusBar.SetPaneText(3 , strTemp);
	return TRUE;
}

LRESULT CMainFrame::OnClientCount( WPARAM wParam , LPARAM lParam )
{
	CString strTemp;
	int nIndex = 0;

	//�ͻ�������
	nIndex = this->m_wndStatusBar.CommandToIndex(ID_CLIENT_CNT);
	strTemp.LoadString(ID_CLIENT_CNT);
	strTemp.AppendFormat(_T("%d") , wParam);
	this->m_wndStatusBar.SetPaneText(nIndex , strTemp);

	//��������
	nIndex = this->m_wndStatusBar.CommandToIndex(ID_ONLINT_CLIENT_CNT);
	strTemp.LoadString(ID_ONLINT_CLIENT_CNT);
	strTemp.AppendFormat(_T("%d") , lParam);
	this->m_wndStatusBar.SetPaneText(nIndex , strTemp);

	return 1;
}

//�Ѿ������˵Ŀͻ������� 
LRESULT CMainFrame::OnPushedCount(WPARAM wParam , LPARAM lParam)
{
	CString strTemp;
	int nIndex = 0;

	//��������
	nIndex = this->m_wndStatusBar.CommandToIndex(ID_PUSH_CLINT_CNT);
	strTemp.LoadString(ID_PUSH_CLINT_CNT);
	strTemp.AppendFormat(_T("%d") , wParam);
	this->m_wndStatusBar.SetPaneText(nIndex , strTemp);

	return 1;
}

LRESULT CMainFrame::OnFullScreen( WPARAM wParam , LPARAM lParam )
{
	if (wParam)
	{
		//����״̬�� 
		m_bFSStatusBarWasVisible = m_wndStatusBar.IsWindowVisible();
		m_wndStatusBar.ShowWindow(SW_HIDE);

		//���ؿͻ����б�
		m_bFSClientListWasVisible = m_pClientPane->IsWindowVisible();
		ShowControlBar(m_pClientPane, FALSE, FALSE);

		//���ز˵��� 
		SetMenu(NULL); 

		// ������ǰ��λ����Ϣ 
		GetWindowRect(&m_rcFSPos); 

		// ȥ�������ڵı���� 
		ModifyStyle( 0 ,WS_POPUP );
		ModifyStyle(WS_CAPTION , 0);
		ModifyStyle(WS_BORDER , 0);
		ModifyStyle(WS_THICKFRAME , 0);

		// ȫ����ʾ 
		SetWindowPos(NULL , 0 , 0 , SCREEN_SIZE_W , SCREEN_SIZE_H , SWP_NOZORDER); 
	}else
	{
		//�ָ����ڱ��� 
		ModifyStyle( 0, WS_CAPTION );
		ModifyStyle( 0, WS_BORDER);
		ModifyStyle( 0, WS_THICKFRAME );

		//ע��,�˴�������WS_MAXIMIZE,����λ�û��Ϊ��һ�δ��ڵ�λ��
		ModifyStyle( 0, WS_OVERLAPPED |  FWS_ADDTOTITLE |WS_MINIMIZEBOX | WS_MAXIMIZEBOX );

		//�����Ҫ����ʾ������ 
		if(m_bFSStatusBarWasVisible) 
			m_wndStatusBar.ShowWindow(SW_SHOW); 

		//����Ҫ����ʾ�ͻ����б�
		if (m_bFSClientListWasVisible)
			ShowControlBar(m_pClientPane, TRUE, FALSE);

		//�ָ�������ǰ�Ĵ�С 
		MoveWindow(&m_rcFSPos); 

		//�ָ��˵��� 
		SetMenu( CMenu::FromHandle( ::LoadMenu( ::AfxGetApp()->m_hInstance ,MAKEINTRESOURCE( IDR_MAINFRAME))));
	}
	return 0;
}

void CMainFrame::OnMove(int x, int y)
{
	CFrameWnd::OnMove(x, y);

	//��Ϊÿ���ͻ�����ͼ����һ��������Ҫ���Ÿ����ڵ��ƶ����ƶ�
	//�����ڴ����ƶ�ʱ��Ҳ��Ҫ֪ͨ��ͼ��
	CView * active = GetActiveView();
	if(active != NULL)
		active->PostMessage(WM_MOVE , x , y);
}

LRESULT CMainFrame::OnMessage( WPARAM wParam , LPARAM lParam )
{
	if (wParam)
	{
		CString msg = *(CString*)wParam;
		int nIndex = 0;

		//��������
		nIndex = this->m_wndStatusBar.CommandToIndex(ID_SEPARATOR);
		this->m_wndStatusBar.SetPaneText(nIndex , msg);
	}
	return 1;
}
