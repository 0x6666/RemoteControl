///////////////////////////////////////////////////////////////
//
// FileName	: CMDTable.cpp 
// Creator	: 杨松
// Date		: 2013年3月15日, 16:09:32
// Comment	: 客户端监视视图控制面类的实现
//
//////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "RemoteControl.h"
#include "CMDTable.h"
#include "clientItemView.h"


// CCMDTable 对话框

#define AROUND_SPACE	10
#define CENTER_SPACE	20
#define CMD_ITEM_WIDTH	30
#define MAX_WIDTH		(CMD_ITEM_WIDTH * 5 + CENTER_SPACE * 4 + AROUND_SPACE * 2)
#define MAX_HIEGTH		(CMD_ITEM_WIDTH + AROUND_SPACE*2 )

//#define INDEX_LABEL_SIZE 20
#define INDEX_LABEL_H	15


IMPLEMENT_DYNAMIC(CCMDTable, CDialog)

CCMDTable::CCMDTable(ClientItemView* pParent , int id)
	: CDialog(CCMDTable::IDD, NULL)
	, m_bTracking(FALSE)
	, m_pParent(pParent)
	, m_nFullHieght(0)
	, m_bIsFullScreen(FALSE)
	, m_bShowAsLabel(TRUE)
	, m_iClientID(id)
{

}

CCMDTable::~CCMDTable()
{
}

void CCMDTable::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TERMINAL, m_wndTerminal);
	DDX_Control(pDX, IDC_DESKTOP, m_wndDesktop);
	DDX_Control(pDX, IDC_DOC_MANAGER, m_wndDocManager);
	DDX_Control(pDX, IDC_SHUTDOWN, m_wndShutdwon);
	DDX_Control(pDX, IDC_DESKTOP_CTRL, m_wndDesktopCtrl);
	DDX_Control(pDX, IDC_FLUENCY_LABEL, m_wndFluencyLabel);
	DDX_Control(pDX, IDC_ET_FLUENCY, m_wndFluencyEdit);
	DDX_Control(pDX, IDC_SP_FLUENCY, m_wndFluencySpin);
	DDX_Control(pDX, IDC_ST_QUALITY, m_wndQualityLabel);
	DDX_Control(pDX, IDC_SLIDE_QUALITY, m_wndQualitySlide);
	DDX_Control(pDX, IDC_ST_QUALITY_NUM, m_wndQualityNum);
	DDX_Control(pDX, IDC_FULL_SCREEN, m_wndFullScrenn);
	DDX_Control(pDX, IDC_INDEX_LABEL, m_wndIndexLebel);
}


BEGIN_MESSAGE_MAP(CCMDTable, CDialog)
	ON_STN_CLICKED(IDC_DESKTOP, &CCMDTable::OnStnClickedDesktop)
	ON_STN_CLICKED(IDC_DOC_MANAGER, &CCMDTable::OnStnClickedDocManager)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_DESKTOP_CTRL, &CCMDTable::OnBnClickedDesktopCtrl)
	ON_STN_CLICKED(IDC_SHUTDOWN, &CCMDTable::OnStnClickedShutdown)
	ON_STN_CLICKED(IDC_TERMINAL, &CCMDTable::OnStnClickedTerminal)
	ON_STN_CLICKED(IDC_FULL_SCREEN, &CCMDTable::OnStnClickedFullScreen)
	ON_WM_INITMENUPOPUP()
	ON_WM_MOVE()
	ON_STN_CLICKED(IDC_INDEX_LABEL, &CCMDTable::OnStnClickedIndexLabel)
END_MESSAGE_MAP()

int CCMDTable::GetMaxWiWidth()
{
	return MAX_WIDTH;
}

void CCMDTable::SmoothShowWindow()
{
	CRect parentRc;
	m_pParent->GetWindowRect(&parentRc);
	int x = parentRc.left + 10/*(parentRc.Width() - MAX_WIDTH) / 2*/;
	int y = parentRc.top + 10;
// 	for (int i = 1 ; i <= MAX_HIEGTH ; ++i )
// 	{
		SetWindowPos(NULL , x , y , MAX_WIDTH , MAX_HIEGTH/*i*/ , SWP_SHOWWINDOW);
//	}

	{//显示需要显示的控件
	m_wndShutdwon.ShowWindow(SW_SHOW);
	m_wndDesktopCtrl.ShowWindow(SW_SHOW);
	m_wndFluencyLabel.ShowWindow(SW_SHOW);
	m_wndFluencyEdit.ShowWindow(SW_SHOW);
	m_wndFluencySpin.ShowWindow(SW_SHOW);
	m_wndQualityLabel.ShowWindow(SW_SHOW);
	m_wndQualitySlide.ShowWindow(SW_SHOW);
	m_wndQualityNum.ShowWindow(SW_SHOW);
	m_wndTerminal.ShowWindow(SW_SHOW);
	m_wndDesktop.ShowWindow(SW_SHOW);
	m_wndDocManager.ShowWindow(SW_SHOW);
	m_wndFullScrenn.ShowWindow(SW_SHOW);
	}//隐藏不需要显示的控件

	//隐藏不需要显示的控件 
	m_wndIndexLebel.ShowWindow(SW_HIDE);

	m_bShowAsLabel = FALSE;
}
void CCMDTable::ShowAsIndexLabel()
{
	CRect parentRc;
	m_pParent->GetWindowRect(&parentRc);
	int x = parentRc.left + 10;
	int y = parentRc.top + 10;
	SetWindowPos(NULL , x , y , m_szLabelSize.cx , m_szLabelSize.cy, SWP_SHOWWINDOW);

	{//隐藏不需要显示的控件
	m_wndShutdwon.ShowWindow(SW_HIDE);
	m_wndDesktopCtrl.ShowWindow(SW_HIDE);
	m_wndFluencyLabel.ShowWindow(SW_HIDE);
	m_wndFluencyEdit.ShowWindow(SW_HIDE);
	m_wndFluencySpin.ShowWindow(SW_HIDE);
	m_wndQualityLabel.ShowWindow(SW_HIDE);
	m_wndQualitySlide.ShowWindow(SW_HIDE);
	m_wndQualityNum.ShowWindow(SW_HIDE);
	m_wndTerminal.ShowWindow(SW_HIDE);
	m_wndDesktop.ShowWindow(SW_HIDE);
	m_wndDocManager.ShowWindow(SW_HIDE);
	m_wndFullScrenn.ShowWindow(SW_HIDE);
	}//隐藏不需要显示的控件

	//显示需要显示的控件 
	m_wndIndexLebel.ShowWindow(SW_SHOW);

	m_bShowAsLabel = TRUE;
}

// void CCMDTable::SmoothHideWindow()
// {
// 	CRect parentRc;
// 	m_pParent->GetWindowRect(&parentRc);
// 	int x = parentRc.left + (parentRc.Width() - MAX_WIDTH) / 2;
// 	int y = parentRc.top +10;
// 	// 	for (int i = MAX_HIEGTH ; i >= 0 ; --i )
// 	// 	{
// 	SetWindowPos(NULL , x , y , MAX_WIDTH , /*i*/0 , SWP_SHOWWINDOW);
// 	//	}
// 	ShowWindow(SW_HIDE);
// }

// CCMDTable 消息处理程序

void CCMDTable::OnStnClickedTerminal()
{
	m_pParent->SendMessage(WM_CMD_CLICKED , 0 , 0 );
}

void CCMDTable::OnStnClickedDesktop()
{
	CRect parentRc;
	GetClientRect(&parentRc);

	if (parentRc.Height() == m_nFullHieght)
	{//该缩小了
// 		for (int i = parentRc.Height() ; i >= MAX_HIEGTH ; --i )
// 		{
			SetWindowPos(NULL , 0 , 0 , MAX_WIDTH , MAX_HIEGTH/*i*/ , SWP_NOMOVE|SWP_SHOWWINDOW);
//		}
	}
	else if(parentRc.Height() == MAX_HIEGTH)
	{//需要放大
// 		for (int i = parentRc.Height() ; i <= m_nFullHieght ; ++i )
// 		{
			SetWindowPos(NULL , 0 , 0 , MAX_WIDTH , m_nFullHieght/*i */, SWP_NOMOVE|SWP_SHOWWINDOW);
//		}
	}
}

void CCMDTable::OnStnClickedDocManager()
{
	m_pParent->SendMessage(WM_DOC_MANAGE_CLICKED , 0 , 0 );
}

BOOL CCMDTable::OnInitDialog()
{
	CDialog::OnInitDialog();

	//添加窗口分层属性
	SetWindowLong(GetSafeHwnd() , GWL_EXSTYLE ,
		GetWindowLong(GetSafeHwnd() , GWL_EXSTYLE)^WS_EX_LAYERED);
	//设置窗口透明效果
	SetLayeredWindowAttributes(/*GetSafeHwnd() ,*/ 0 , 200 , LWA_ALPHA);
	
	{//设置索引
	CString temp;
	temp.Format(_T("%d") , m_iClientID);
	m_szLabelSize = m_wndIndexLebel.GetDC()->GetTextExtent(temp);
	m_szLabelSize.cx += 6;
	//m_szLabelSize.cy += 4;
	m_wndIndexLebel.SetWindowText(temp);
	::MoveWindow(m_wndIndexLebel.GetSafeHwnd() , 0 , 0 , m_szLabelSize.cx , m_szLabelSize.cy, FALSE);
	}//设置索引

	{//调整主控制面板个控件的位置
	::MoveWindow(GetSafeHwnd() , 10 , 0 , MAX_WIDTH , MAX_HIEGTH , FALSE);
	::MoveWindow(m_wndTerminal.GetSafeHwnd()	, AROUND_SPACE , AROUND_SPACE , CMD_ITEM_WIDTH , CMD_ITEM_WIDTH , FALSE);
	::MoveWindow(m_wndDesktop.GetSafeHwnd()		, AROUND_SPACE + CMD_ITEM_WIDTH	  +   CENTER_SPACE , AROUND_SPACE , CMD_ITEM_WIDTH , CMD_ITEM_WIDTH , FALSE);
	::MoveWindow(m_wndDocManager.GetSafeHwnd()	, AROUND_SPACE + CMD_ITEM_WIDTH*2 + 2*CENTER_SPACE , AROUND_SPACE , CMD_ITEM_WIDTH , CMD_ITEM_WIDTH , FALSE);
	::MoveWindow(m_wndShutdwon.GetSafeHwnd()	, AROUND_SPACE + CMD_ITEM_WIDTH*3 + 3*CENTER_SPACE , AROUND_SPACE , CMD_ITEM_WIDTH , CMD_ITEM_WIDTH , FALSE);
	::MoveWindow(m_wndFullScrenn.GetSafeHwnd()	, AROUND_SPACE + CMD_ITEM_WIDTH*4 + 4*CENTER_SPACE , AROUND_SPACE , CMD_ITEM_WIDTH , CMD_ITEM_WIDTH , FALSE);
	}

	{//桌面控制
	CRect rc;
	int x = AROUND_SPACE;
	int y = MAX_HIEGTH + 1;
	m_wndDesktopCtrl.SetWindowPos(NULL , x  , y   , 0 , 0 , SWP_NOSIZE|SWP_SHOWWINDOW);
	m_wndDesktopCtrl.GetClientRect(&rc);
	x += (rc.Width() + 25);
	m_wndFluencyLabel.SetWindowPos(NULL , x	, y+1 , 0 , 0 , SWP_NOSIZE|SWP_SHOWWINDOW);
	m_wndFluencyLabel.GetClientRect(&rc);
	x += (rc.Width() + 5);
	m_wndFluencyEdit.SetWindowPos(NULL , x	, y-2 , 0 , 0 , SWP_NOSIZE|SWP_SHOWWINDOW);
	m_wndFluencyEdit.GetClientRect(&rc);
	x += (rc.Width() + 4);
	m_wndFluencySpin.SetWindowPos(NULL , x	, y-3 , 0 , 0 , SWP_NOSIZE|SWP_SHOWWINDOW);

	m_wndFluencyEdit.GetClientRect(&rc);
	y += (rc.Height() + 8);
	x = AROUND_SPACE;
	m_wndQualityLabel.SetWindowPos(NULL , x	, y+5 , 0 , 0 , SWP_NOSIZE|SWP_SHOWWINDOW);
	m_wndQualityLabel.GetClientRect(&rc);
	x += (rc.Width() + 2);
	m_wndQualitySlide.SetWindowPos(NULL , x	, y , 0 , 0 , SWP_NOSIZE|SWP_SHOWWINDOW);
	m_wndQualitySlide.GetClientRect(&rc);
	x += (rc.Width() + 2);
	m_wndQualityNum.SetWindowPos(NULL , x	, y+5 , 0 , 0 , SWP_NOSIZE|SWP_SHOWWINDOW);
	m_nFullHieght = y + rc.Height() + AROUND_SPACE;

	//质量的范围
	m_wndQualitySlide.SetRange(0 , 100 , FALSE);
	m_wndQualitySlide.SetPos(DEFAULT_SCREEN_QUALITY);
	CString strTemp;
	strTemp.Format(_T("%d") , DEFAULT_SCREEN_QUALITY);
	m_wndQualityNum.SetWindowText(strTemp);

	//流畅度相关控件初始化
	strTemp.Format(_T("%d") , FrequencyToIndex(DEFAULT_FREQUENCY_LEVEL));
	m_wndFluencyEdit.SetWindowText(strTemp);

	m_wndFluencySpin.SetBuddy(&m_wndFluencyEdit);
	m_wndFluencySpin.SetBase(0);
	m_wndFluencySpin.SetPos(FrequencyToIndex(DEFAULT_FREQUENCY_LEVEL));
	m_wndFluencySpin.SetRange(1 , 7);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CCMDTable::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar == (CScrollBar*)&m_wndQualitySlide)
	{
		int pos = m_wndQualitySlide.GetPos();
		CString strTemp;
		strTemp.Format(_T("%d") , pos);
		m_wndQualityNum.SetWindowText(strTemp);

		//给发送消息了
		m_pParent->PostMessage(WM_CHANGE_SCREEN_AUALITY , pos , 0);
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CCMDTable::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar == (CScrollBar*)&m_wndFluencySpin)
	{
		CString strTemp;
		strTemp.Format( _T("%d") , (int)nPos);
		m_wndFluencySpin.GetBuddy()->SetWindowText(strTemp);
		//给发送消息了
		m_pParent->PostMessage(WM_CHANGE_SCREEN_FLUENCY , nPos , 0);
	}
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}
void CCMDTable::OnBnClickedDesktopCtrl()
{
	BOOL  isChecked = m_wndDesktopCtrl.GetCheck();
	m_pParent->PostMessage(WM_SCREEN_CTRL , isChecked , 0);
}

void CCMDTable::OnStnClickedShutdown()
{
	CPoint	point;
	CMenu	menu;

	::GetCursorPos(&point);
	//快捷菜单
	menu.LoadMenu(IDR_CMD_SHUTDOWN);
	menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN|TPM_TOPALIGN , point.x , point.y , m_pParent , NULL);
}

void CCMDTable::OnStnClickedFullScreen()
{
	if (m_bIsFullScreen)
	{//取消全屏
		m_bIsFullScreen = FALSE;
		//m_wndFullScrenn.SetIcon(AfxGetApp()->LoadIcon(IDI_NO_FULL_SCREEN));
	}else
	{//全屏
		m_bIsFullScreen = TRUE;
		//m_wndFullScrenn.SetIcon(AfxGetApp()->LoadIcon(IDI_FULL_SCREEN));
	}
	m_pParent->PostMessage(WM_FULLS_CREEN , m_bIsFullScreen , 0);
}
void CCMDTable::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	CDialog::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
	
	/*/////////////////////////////////////////////////////////////////////////
	* 在下拉菜单显示的时候, WM_INITMENUPOPUP消息被先发送以显示菜单项。
	* MFC CFrameWnd::OnInitMenuPopup 函数遍历菜单项并为每个菜单项调用更新命令处
	* 理函数（如果有的话）.菜单的外观被更新以反映它的状态（启用/禁用,选择/取消选
	* 择）更新用户界面机制在基于对话框的应用程序中不能工作，因为CDialog没有
	* OnInitMenuPopup 处理函数，而使用CWnd's 默认处理函数,该函数没有为菜单项调用
	* 更新命令处理函数。
	* 以下代码源自MSDN http://support.microsoft.com/kb/242577
	/////////////////////////////////////////////////////////////////////////*/

	ASSERT(pPopupMenu != NULL);
	// Check the enabled state of various menu items.

	CCmdUI state;
	state.m_pMenu = pPopupMenu;
	ASSERT(state.m_pOther == NULL);
	ASSERT(state.m_pParentMenu == NULL);

	// Determine if menu is popup in top-level menu and set m_pOther to
	// it if so (m_pParentMenu == NULL indicates that it is secondary popup).
	HMENU hParentMenu;
	if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)
		state.m_pParentMenu = pPopupMenu;    // Parent == child for tracking popup.
	else if ((hParentMenu = ::GetMenu(m_hWnd)) != NULL)
	{
		CWnd* pParent = this;
		// Child windows don't have menus--need to go to the top!
		if (pParent != NULL &&
			(hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)
		{
			int nIndexMax = ::GetMenuItemCount(hParentMenu);
			for (int nIndex = 0; nIndex < nIndexMax; nIndex++)
			{
				if (::GetSubMenu(hParentMenu, nIndex) == pPopupMenu->m_hMenu)
				{
					// When popup is found, m_pParentMenu is containing menu.
					state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
					break;
				}
			}
		}
	}

	state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
		state.m_nIndex++)
	{
		state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
		if (state.m_nID == 0)
			continue; // Menu separator or invalid cmd - ignore it.

		ASSERT(state.m_pOther == NULL);
		ASSERT(state.m_pMenu != NULL);
		if (state.m_nID == (UINT)-1)
		{
			// Possibly a popup menu, route to first item of that popup.
			state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
			if (state.m_pSubMenu == NULL ||
				(state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
				state.m_nID == (UINT)-1)
			{
				continue;       // First item of popup can't be routed to.
			}
			state.DoUpdate(this, TRUE);   // Popups are never auto disabled.
		}
		else
		{
			// Normal menu item.
			// Auto enable/disable if frame window has m_bAutoMenuEnable
			// set and command is _not_ a system command.
			state.m_pSubMenu = NULL;
			state.DoUpdate(this, FALSE);
		}

		// Adjust for menu deletions and additions.
		UINT nCount = pPopupMenu->GetMenuItemCount();
		if (nCount < state.m_nIndexMax)
		{
			state.m_nIndex -= (state.m_nIndexMax - nCount);
			while (state.m_nIndex < nCount &&
				pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
			{
				state.m_nIndex++;
			}
		}
		state.m_nIndexMax = nCount;
	}
}

void CCMDTable::OnMove(int x, int y)
{
	CDialog::OnMove(x, y);

	//当窗口移动后将显示为索引lebel
	if (IsWindow(m_wndIndexLebel.GetSafeHwnd()))
		ShowAsIndexLabel();
}


void CCMDTable::OnStnClickedIndexLabel()
{
	SmoothShowWindow();
}

BOOL CCMDTable::IsShowAsLabel()
{
	return m_bShowAsLabel;
}

