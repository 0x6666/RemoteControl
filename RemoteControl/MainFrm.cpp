///////////////////////////////////////////////////////////////
//
// FileName	: MainFrm.cpp  
// Creator	: 杨松
// Date		: 2013年2月27日, 20:10:26
// Comment	: 远控控制服务器主框架的类的实现
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
	ID_SEPARATOR,           // 状态行指示器
	ID_CLIENT_CNT,
	ID_ONLINT_CLIENT_CNT,
	ID_PUSH_CLINT_CNT,
};


// CMainFrame 构造/析构

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

	//初始化状态栏
	if(FALSE == InitStatusBar())
		return -1;

	//创建客户端面板
	if (FALSE == CreateClientPane())
		return -1;


	//初始化托盘图标变量
	m_notifyIcon.cbSize = sizeof(NOTIFYICONDATA);
	m_notifyIcon.hWnd = this->m_hWnd;   
	m_notifyIcon.uID = ICON_INDEX_S;   
	m_notifyIcon.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP;   
	m_notifyIcon.uCallbackMessage = WM_NOTIFYION_MSG_S;//自定义的消息名称   
	m_notifyIcon.hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);   
	::wcscpy(m_notifyIcon.szTip , _T("远程监控服务器"));//信息提示条为“计划任务提醒”   
	//在托盘区添加图标
	::Shell_NotifyIcon(NIM_ADD , &m_notifyIcon );   


	//启动开始检查客户端状态定时器
	SetTimer(CHECK_CLIENT_STATE_TIME , HEARTBEAT_PACKETS_TIME , NULL);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	//去掉文档标题
	cs.style &=~ (LONG) FWS_ADDTOTITLE;

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
BOOL CMainFrame::CreateClientPane()
{
	CSize size(240 , 400);

	//创建客户端面板
	m_pClientPane = new CCtrlBar();
	if (!m_pClientPane->Create(this, IDD_PANE, CBRS_LEFT|CBRS_TOOLTIPS|CBRS_FLYBY,
		IDD_PANE ,size))
	{
		TRACE0("创建客户端面板失败\n");
		delete m_pClientPane;
		m_pClientPane = NULL;
		return FALSE; 
	}

	m_pClientList = new CListCtrl();
	if (!m_pClientList->Create(LVS_SHOWSELALWAYS|LVS_SHAREIMAGELISTS|LVS_REPORT|WS_BORDER
		|WS_TABSTOP|WS_CHILD|WS_VISIBLE
		,CRect() , m_pClientPane , IDC_CLIENT_LIST))
	{
		TRACE0("创建客户端列表控件失败\n");
		return FALSE; 
	}
	//设置一下列表的属性
	DWORD dwStyle = m_pClientList->GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮
	dwStyle |= LVS_EX_GRIDLINES;	//网格线
	m_pClientList->SetExtendedStyle(dwStyle);
	InitClientListCtrl();


	//设置设备列表可停靠
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
	//初始化列表控件
	this->m_pClientList->SetExtendedStyle(LVS_EX_FULLROWSELECT);
	this->m_pClientList->InsertColumn(CLIENT_LIST_ID , _T("ID") , 0 , 30);
	this->m_pClientList->InsertColumn(CLIENT_LIST_STATU , _T("状态") , 0 , 50);
	this->m_pClientList->InsertColumn(CLIENT_LIST_PUSH , _T("推送") , 0 , 40);
	this->m_pClientList->InsertColumn(CLIENT_LIST_NAME , _T("计算机名") , 0 , 80);
	this->m_pClientList->InsertColumn(CLIENT_LIST_IP , _T("IP") , 0 , 100);
}


// CMainFrame 消息处理程序

BOOL CMainFrame::DestroyWindow()
{
	//先还是关闭检查客户端状态定时器吧
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

	//删除托盘图标
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
			::TrackPopupMenu(p->m_hMenu , TPM_LEFTALIGN|TPM_BOTTOMALIGN , point.x , point.y , 0 , this->GetSafeHwnd() , NULL);
		}
		break;
	case WM_LBUTTONDBLCLK://双击左键的处理   
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
		TRACE0("未能创建状态栏\n");
		return FALSE;      // 未能创建
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

	//客户端数量
	nIndex = this->m_wndStatusBar.CommandToIndex(ID_CLIENT_CNT);
	strTemp.LoadString(ID_CLIENT_CNT);
	strTemp.AppendFormat(_T("%d") , wParam);
	this->m_wndStatusBar.SetPaneText(nIndex , strTemp);

	//在线数量
	nIndex = this->m_wndStatusBar.CommandToIndex(ID_ONLINT_CLIENT_CNT);
	strTemp.LoadString(ID_ONLINT_CLIENT_CNT);
	strTemp.AppendFormat(_T("%d") , lParam);
	this->m_wndStatusBar.SetPaneText(nIndex , strTemp);

	return 1;
}

//已经推送了的客户端数量 
LRESULT CMainFrame::OnPushedCount(WPARAM wParam , LPARAM lParam)
{
	CString strTemp;
	int nIndex = 0;

	//在线数量
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
		//隐藏状态栏 
		m_bFSStatusBarWasVisible = m_wndStatusBar.IsWindowVisible();
		m_wndStatusBar.ShowWindow(SW_HIDE);

		//隐藏客户端列表
		m_bFSClientListWasVisible = m_pClientPane->IsWindowVisible();
		ShowControlBar(m_pClientPane, FALSE, FALSE);

		//隐藏菜单栏 
		SetMenu(NULL); 

		// 保存以前的位置信息 
		GetWindowRect(&m_rcFSPos); 

		// 去除主窗口的标题等 
		ModifyStyle( 0 ,WS_POPUP );
		ModifyStyle(WS_CAPTION , 0);
		ModifyStyle(WS_BORDER , 0);
		ModifyStyle(WS_THICKFRAME , 0);

		// 全屏显示 
		SetWindowPos(NULL , 0 , 0 , SCREEN_SIZE_W , SCREEN_SIZE_H , SWP_NOZORDER); 
	}else
	{
		//恢复窗口标题 
		ModifyStyle( 0, WS_CAPTION );
		ModifyStyle( 0, WS_BORDER);
		ModifyStyle( 0, WS_THICKFRAME );

		//注意,此处不能用WS_MAXIMIZE,否则位置会变为第一次窗口的位置
		ModifyStyle( 0, WS_OVERLAPPED |  FWS_ADDTOTITLE |WS_MINIMIZEBOX | WS_MAXIMIZEBOX );

		//如果需要，显示工具栏 
		if(m_bFSStatusBarWasVisible) 
			m_wndStatusBar.ShowWindow(SW_SHOW); 

		//若需要则显示客户端列表
		if (m_bFSClientListWasVisible)
			ShowControlBar(m_pClientPane, TRUE, FALSE);

		//恢复窗口以前的大小 
		MoveWindow(&m_rcFSPos); 

		//恢复菜单栏 
		SetMenu( CMenu::FromHandle( ::LoadMenu( ::AfxGetApp()->m_hInstance ,MAKEINTRESOURCE( IDR_MAINFRAME))));
	}
	return 0;
}

void CMainFrame::OnMove(int x, int y)
{
	CFrameWnd::OnMove(x, y);

	//因为每个客户端视图的有一个窗口需要随着父窗口的移动而移动
	//所以在窗口移动时，也需要通知试图类
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

		//在线数量
		nIndex = this->m_wndStatusBar.CommandToIndex(ID_SEPARATOR);
		this->m_wndStatusBar.SetPaneText(nIndex , msg);
	}
	return 1;
}
