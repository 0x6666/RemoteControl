///////////////////////////////////////////////////////////////
//
// FileName	: CMDDlg.cpp 
// Creator	: 杨松
// Date		: 2013年5月8日，13:24:47
// Comment	: CMD对话框类实现
//
//////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "RemoteControl.h"
#include "CMDDlg.h"
#include "RemoteControlDoc.h"
#include <locale.h>


// CCMDDlg 对话框

IMPLEMENT_DYNAMIC(CCMDDlg, CDialog)

CCMDDlg::CCMDDlg(const CString& ip , USHORT port ,const CString& name 
				 ,CRemoteControlDoc* pDoc, PCMD_CFG_V cmdv,  CWnd* pParent /*=NULL*/)
	: CDialog(CCMDDlg::IDD, pParent)
	, m_strIP(ip)
	, m_strName(name)
	, m_pBkBrush(NULL)
	, m_pFont(NULL)
	, m_nLastCount(0)
	, m_pDoc(pDoc)
	, m_uPort(port)
{
	m_cmdCfgValue = *cmdv;
}

CCMDDlg::~CCMDDlg()
{
}

void CCMDDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ET_CMD_VIEW, m_wndCMDView);
}


BEGIN_MESSAGE_MAP(CCMDDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_COMMAND(ID_COPY, &CCMDDlg::OnCopy)
	ON_UPDATE_COMMAND_UI(ID_COPY, &CCMDDlg::OnUpdateCopy)
	ON_COMMAND(ID_CUT, &CCMDDlg::OnCut)
	ON_UPDATE_COMMAND_UI(ID_CUT, &CCMDDlg::OnUpdateCut)
	ON_COMMAND(ID_PAST, &CCMDDlg::OnPast)
	ON_UPDATE_COMMAND_UI(ID_PAST, &CCMDDlg::OnUpdatePast)
	ON_WM_INITMENUPOPUP()
END_MESSAGE_MAP()


// CCMDDlg 消息处理程序

BOOL CCMDDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//设置对话框可以调整大小
	ModifyStyle( NULL , WS_THICKFRAME);

	//设置对话框标题
	SetWindowText(m_strName + _T("(") + m_strIP + _T(")"));

	CEdit* pEdit = dynamic_cast<CEdit*>(GetDlgItem(IDC_ET_CMD_VIEW));
	ASSERT(pEdit != NULL);

	m_pFont = new CFont();
	//字体
	LOGFONT lf = {0};
	CString name;
	lf.lfCharSet = m_cmdCfgValue.nCMD_FontCharset;
	lf.lfHeight = m_cmdCfgValue.nCMD_FontSize;
	name += m_cmdCfgValue.czCMD_FontFaceName;
	wcscpy(lf.lfFaceName , name);
	m_pFont->CreateFontIndirect(&lf);
	pEdit->SetFont( m_pFont );

	m_pBkBrush = new CBrush();
	m_pBkBrush->CreateSolidBrush(m_cmdCfgValue.nCMD_BackgroundColor);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CCMDDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	
	//调整编辑区域的大小和位置
	if (IsWindow(m_wndCMDView.GetSafeHwnd()))
	{
		CRect cRc;
		GetClientRect(&cRc);
		m_wndCMDView.MoveWindow( 0 , 0 ,cRc.Width() , cRc.Height() , FALSE);
	}
}

void CCMDDlg::AddCMDLineData( CString strCMDline )
{
	//不重复显示命令
	if((FALSE == m_strCMD.IsEmpty()) && 
		(strCMDline.Left(m_strCMD.GetLength()) == m_strCMD))
		strCMDline = (_T("\r") + strCMDline.Mid(m_strCMD.GetLength()));

	//将数据显示出来
	m_wndCMDView.SetSel( m_wndCMDView.GetWindowTextLength() , -1 , 0 );
	m_wndCMDView.ReplaceSel(strCMDline);
	m_wndCMDView.SetSel( m_wndCMDView.GetWindowTextLength() , -1 , 0 );


	CString str;
	m_wndCMDView.GetWindowText(str);



	//一条命令执行完毕?
	if(strCMDline.Right(1) == _T(">"))
	{
		if(m_strCMD.IsEmpty())
		{//还没有执行过任何命令
			//修改版权信息
			CString strInfo = _T("");
			m_wndCMDView.GetWindowText(strInfo);
			int nIndex = strInfo.Find(_T(":\\"));
			strInfo = _T("中南林业科技大学2009级软件工程毕业设计\r\nPC机远程监控系统的开发——杨松.\r\n\r\n") + strInfo.Mid(nIndex-1);
			m_wndCMDView.SetWindowText(strInfo);
			m_wndCMDView.SetSel(strInfo.GetLength() , -1 , 0 );
		}
	}
	m_nLastCount = m_wndCMDView.GetWindowTextLength();
}

HBRUSH CCMDDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// TODO:  在此更改 DC 的任何属性
	if(nCtlColor == CTLCOLOR_EDIT)
	{
		pDC->SetTextColor(m_cmdCfgValue.nCMD_FontColor);
		pDC->SetBkColor(m_cmdCfgValue.nCMD_BackgroundColor);
		return static_cast <HBRUSH>(m_pBkBrush->GetSafeHandle());
	}

	//没有必要的时候还是返回原有的画刷
	//否则会出问题的，例如右键显示的菜单无法显示
	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL CCMDDlg::DestroyWindow()
{
	//删除字体
	m_pFont->DeleteObject();
	delete m_pFont;
	
	//删除背景刷子
	m_pBkBrush->DeleteObject();
	delete m_pBkBrush;

	return CDialog::DestroyWindow();
}

BOOL CCMDDlg::PreTranslateMessage(MSG* pMsg)
{
	if (GetFocus() != &m_wndCMDView)
	{//只有在编辑区的消息才处理
		return CDialog::PreTranslateMessage(pMsg);
	}

	if((pMsg->message == WM_KEYDOWN))
	{//键盘消息
		if(pMsg->wParam == VK_RETURN)
		{//按下的是回车键
			if(FALSE == ExecuteCMD())
			{//执行命令失败 或无需回显
				return TRUE;
			}

			//是清屏的话
			if(!m_strCMD.CompareNoCase(_T("cls")))
			{
				//等待线程处理完毕
				Sleep(50);

				CString strInfo;
				m_wndCMDView.GetWindowText(strInfo);
				int nIndex = strInfo.Find(_T(":\\"));
				strInfo = _T("\r\n") + strInfo.Mid(nIndex - 1);
				m_wndCMDView.SetWindowText(strInfo);
				//光标放到最后
				m_wndCMDView.SetSel(strInfo.GetLength() , -1 , 0 );
				m_nLastCount = strInfo.GetLength();
			}
			return TRUE;
		}

		//处理 退格 和delete键
		int nCount = m_nLastCount;
		int n = 0;
		int m = 0;
		m_wndCMDView.GetSel( n , m );

		if(pMsg->wParam == VK_BACK)
		{//退格
			if (n == m)
			{//没有选中任何数据
				if(nCount >= n)
					return TRUE;
			}else
			{//选中了数据
				if(nCount > n)
					return TRUE;
			}
		}

		if(pMsg->wParam == VK_DELETE)
		{//delete键
			if(nCount > n)
				return TRUE;
		}

// 		if(pMsg->wParam == VK_UP)
// 		{//上键的
// 			//向上的按键的处理
// 			CString cmd = GetDocument()->GetCmdByUpKey();
// 
// 			////////将数据上传
// 			GetEditCtrl().SetSel(GetDocument()->m_strInfo.GetLength(),-1,0);
// 			GetEditCtrl().ReplaceSel(cmd);
// 			//GetEditCtrl().GetWindowTextW(GetDocument()->m_strInfo);
// 			//GetDocument()->m_nLastCount=GetDocument()->m_strInfo.GetLength();
// 			return TRUE;
// 		}
// 		if(pMsg->wParam==VK_DOWN)
// 		{////向下的按键的处理
// 			////向上的按键的处理
// 			CString cmd = GetDocument()->GetCmdByDownKey();
// 
// 			////////将数据上传
// 			GetEditCtrl().SetSel(GetDocument()->m_strInfo.GetLength(),-1,0);
// 			GetEditCtrl().ReplaceSel(cmd);
// 			//GetEditCtrl().GetWindowTextW(GetDocument()->m_strInfo);
// 			//GetDocument()->m_nLastCount=GetDocument()->m_strInfo.GetLength();
// 			return TRUE;
// 		}
	}
	//处理 字符键
	if(pMsg->message == WM_CHAR)
	{
		int nCount = m_nLastCount;
		int n = 0;
		int m = 0;
		m_wndCMDView.GetSel( n , m );

		if(nCount > n)
			return TRUE;

	}

	if (pMsg->message == WM_RBUTTONUP)
	{//鼠标右键  弹出环境菜单 
		CPoint	point;
		CMenu	menu;
		::GetCursorPos(&point);

		//快捷菜单
		menu.LoadMenu(IDR_CMD_MENU);
		menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN|TPM_TOPALIGN , 
			point.x , point.y , this , NULL);
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CCMDDlg::ExecuteCMD()
{
	if (FALSE == PreExecuteCMD())
	{//命令已经执行完了，不需要在处理了
		return FALSE;
	}

	char* cmdBuf = NULL;
	PRCMSG_BUFF pMsg = NULL;
	int size = 0;
	{//字符编码转换
	::setlocale(LC_ALL , "");
	size = ::wcstombs(NULL , m_strCMD , 0);
	cmdBuf = new char[sizeof(RCMSG_BUFF) - 1 + size + 2];
	pMsg = PRCMSG_BUFF(cmdBuf);
	::wcstombs(pMsg->buf , m_strCMD , size);
	::setlocale(LC_ALL , "C");
	}//字符编码转换

	pMsg->buf[size] = '\n';
	pMsg->buf[size+1] = '\0';
	pMsg->msgHead.type = MT_CMD_S;
	pMsg->msgHead.size = sizeof(RCMSG_BUFF) - 1 + size + 2;
	//将命令发出去
	m_pDoc->SendRcMessage(m_strIP , m_uPort , pMsg);
	delete[] cmdBuf;

	return TRUE;
}

void CCMDDlg::CMDFailed( const void* msg )
{
	
}

void CCMDDlg::OnCopy()
{
	m_wndCMDView.Copy();
}

void CCMDDlg::OnUpdateCopy(CCmdUI *pCmdUI)
{
	int n;
	int m;
	m_wndCMDView.GetSel(n , m);
	if (m == n)
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->Enable(TRUE);
}

void CCMDDlg::OnCut()
{
	m_wndCMDView.Cut();
}

void CCMDDlg::OnUpdateCut(CCmdUI *pCmdUI)
{
	int n;
	int m;
	m_wndCMDView.GetSel(n , m);

	if (n == m)//没有选择任何东西不可以剪切
	{
		pCmdUI->Enable(FALSE);
		return ;
	}
	if (m_nLastCount > n)
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->Enable(TRUE);
}

void CCMDDlg::OnPast()
{
	m_wndCMDView.Paste();
}

void CCMDDlg::OnUpdatePast(CCmdUI *pCmdUI)
{
	int n;
	int m;
	m_wndCMDView.GetSel(n , m);

// 	if (m_nLastCount > n)
// 		pCmdUI->Enable(FALSE);
// 	else
	if (m_nLastCount <= n && OpenClipboard())
	{
		if (IsClipboardFormatAvailable(CF_TEXT))
		{//粘贴板里面的是文本数据
			pCmdUI->Enable(TRUE);
			CloseClipboard();
			return ;
		}
		CloseClipboard();	
	}
	pCmdUI->Enable(FALSE);
}

void CCMDDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
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

void CCMDDlg::ChangeCfgValue( PCMD_CFG_V cmdValue )
{
	m_cmdCfgValue = *cmdValue;
	if (NULL != m_pFont)
	{
		//字体
		LOGFONT lf = {0};
		lf.lfCharSet = cmdValue->nCMD_FontCharset;
		lf.lfHeight = cmdValue->nCMD_FontSize;
		//字体名字
		CString name;
		name += cmdValue->czCMD_FontFaceName;
		wcscpy(lf.lfFaceName , name);
		m_pFont->DeleteObject();
		m_pFont->CreateFontIndirect(&lf);
		m_wndCMDView.SetFont(m_pFont);
	}

	//背景颜色
	if (NULL != m_pBkBrush)
	{
		m_pBkBrush->DeleteObject();
		m_pBkBrush->CreateSolidBrush(m_cmdCfgValue.nCMD_BackgroundColor);
	}

	Invalidate();
}

BOOL CCMDDlg::PreExecuteCMD()
{
	CString strInfo;
	m_wndCMDView.GetWindowText( strInfo );
	m_strCMD = strInfo.Mid( m_nLastCount );
	m_strCMD.Trim();

	CString temp;
	if((strInfo.Mid(m_nLastCount-1 , 1) == _T(">")) && !m_strCMD.CompareNoCase(_T("exit")))
	{//退出
		//TODO 做exit的预处理
		temp.LoadString(IDS_NOT_EXE_CMD);
		temp.Replace(STR_RP , m_strCMD);

		CString strDir;
		int nIndex = strInfo.ReverseFind(_T('\n'));
		strDir = strInfo.Mid(nIndex-1 , m_wndCMDView.GetWindowTextLength() - nIndex - m_strCMD.GetLength() + 1);
		temp = _T("\r\n") + temp + _T("\r\n");
		temp += strDir;
		m_wndCMDView.SetSel(strInfo.GetLength() , -1 , 0 );
		m_wndCMDView.ReplaceSel(temp);
		m_wndCMDView.SetSel(m_wndCMDView.GetWindowTextLength(), -1 , 0 );
		m_nLastCount = m_wndCMDView.GetWindowTextLength();
		return FALSE;
	}
	
	if(!m_strCMD.CompareNoCase(_T("cls")))
	{//清屏
		CString strDir;
		int nIndex = strInfo.ReverseFind(_T('\n'));
		strDir = strInfo.Mid(nIndex-1 , m_wndCMDView.GetWindowTextLength() - nIndex - m_strCMD.GetLength() + 1);
		m_wndCMDView.SetWindowText(strDir);
		m_wndCMDView.SetSel(m_wndCMDView.GetWindowTextLength(), -1 , 0 );
		m_nLastCount = m_wndCMDView.GetWindowTextLength();
		return FALSE;
	}

	temp = m_strCMD.Left(6);
	temp.Trim();
	if(!temp.CompareNoCase(_T("title")))
	{//修改标题
		SetWindowText(m_strCMD.Mid(5));
		return FALSE;
	}

	return TRUE;
}
