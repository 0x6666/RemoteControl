///////////////////////////////////////////////////////////////
//
// FileName	: FileManageDlg.cpp 
// Creator	: 杨松
// Date		: ?2013年4月17日，12:23:32
// Comment	: 远程文件管理对话框类的实现
//
//////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RemoteControl.h"
#include "FileManageDlg.h"
#include "RemoteControlDoc.h"

enum ItemImageIndex{
	III_COMPUTER = 0,
	III_MY_DOC = 1,
	III_DESKTOP = 2,
	III_DISK = 3,
	III_DIR = 4,
	III_FILE = 5
};

// CDocManageDlg 对话框

IMPLEMENT_DYNAMIC(CFileManageDlg, CDialog)

CFileManageDlg::CFileManageDlg(const CString& ip , 
							 USHORT port , const CString& name , 
							 CRemoteControlDoc* pDoc , CWnd* pParent/* = NULL*/)
	: CDialog(CFileManageDlg::IDD, pParent)
	, m_strIP(ip)
	, m_strName(name)
	, m_nPort(port)
	, m_pDoc(pDoc)
	, m_hMyDoc(NULL)
	, m_hComputer(NULL)
	, m_hDesktop(NULL)
{


}

CFileManageDlg::~CFileManageDlg()
{
}

void CFileManageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILE_TREE, m_wndFileTree);
}


BEGIN_MESSAGE_MAP(CFileManageDlg, CDialog)
	ON_WM_SIZE()
	ON_NOTIFY(NM_CLICK, IDC_FILE_TREE, &CFileManageDlg::OnNMClickFileTree)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(NM_RCLICK, IDC_FILE_TREE, &CFileManageDlg::OnNMRClickFileTree)
	ON_WM_INITMENUPOPUP()
	ON_COMMAND(ID_FLUSH, &CFileManageDlg::OnFlush)
	ON_COMMAND(ID_DOWNLOAD, &CFileManageDlg::OnDownload)
	ON_UPDATE_COMMAND_UI(ID_FLUSH, &CFileManageDlg::OnUpdateFlush)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOAD, &CFileManageDlg::OnUpdateDownload)
	ON_COMMAND(ID_UPLOAD, &CFileManageDlg::OnUpload)
	ON_UPDATE_COMMAND_UI(ID_UPLOAD, &CFileManageDlg::OnUpdateUpload)
	ON_COMMAND(ID_DELETE, &CFileManageDlg::OnDelete)
	ON_UPDATE_COMMAND_UI(ID_DELETE, &CFileManageDlg::OnUpdateDelete)
END_MESSAGE_MAP()


// CDocManageDlg 消息处理程序

BOOL CFileManageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//设置对话框可以调整大小
	ModifyStyle( NULL , WS_THICKFRAME);
	//设置对话框标题
	this->SetWindowText(m_strName + _T("(") + m_strIP + _T(")"));

	{//树控件图标列表
	//这里创建的图标图表列表需要使用ILC_COLOR32而不是ILC_COLOR16
	//否则显示的图标会有一个黑边框
	//当然在图标文件中需要存在32色的图标层存在
	this->m_wndImageList.Create(20 , 20 , ILC_COLOR32|ILC_MASK , 10 , 1);
	HICON hIcon = AfxGetApp()->LoadIcon(IDI_COMPUTER);//计算机
	m_wndImageList.Add(hIcon);
	hIcon=AfxGetApp()->LoadIcon(IDI_MY_DOC);//我的文档
	m_wndImageList.Add(hIcon);
	hIcon=AfxGetApp()->LoadIcon(IDI_DT);//桌面
	m_wndImageList.Add(hIcon);
	hIcon=AfxGetApp()->LoadIcon(IDI_DISK);//磁盘
	m_wndImageList.Add(hIcon);
	hIcon=AfxGetApp()->LoadIcon(IDI_DIR);//文件夹
	m_wndImageList.Add(hIcon);
	hIcon=AfxGetApp()->LoadIcon(IDI_FILE);//文件
	m_wndImageList.Add(hIcon);
	this->m_wndFileTree.SetImageList(&m_wndImageList,TVSIL_NORMAL);
	}//树控件图标列表

	{//添加几个根节点
	CString strTmp;
	//我的文档
	strTmp.LoadString(IDS_MY_DOC);
	this->m_hMyDoc = this->m_wndFileTree.InsertItem(strTmp , III_MY_DOC , III_MY_DOC , TVI_ROOT);
	//我的文档
	strTmp.LoadString(IDS_DESKTOP);
	this->m_hDesktop = this->m_wndFileTree.InsertItem(strTmp , III_DESKTOP , III_DESKTOP , TVI_ROOT);
	//计算机
	strTmp.LoadString(IDS_COMPUTER);
	this->m_hComputer = this->m_wndFileTree.InsertItem(strTmp , III_COMPUTER , III_COMPUTER , TVI_ROOT);
	}//添加几个根节点

	//获取磁盘驱动器信息
	DEF_RCMSG(rcMsg , MT_GET_DRIVER_S);
	m_pDoc->SendRcMessage(m_strIP , m_nPort , &rcMsg);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CFileManageDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	//调整树控件的大小和位置
	if (IsWindow(m_wndFileTree.GetSafeHwnd()))
	{
		CRect cRc;
		this->GetClientRect(&cRc);
		this->m_wndFileTree.MoveWindow( 0 , 0 ,cRc.Width() , cRc.Height() , TRUE);
	}
}

void CFileManageDlg::OnNMClickFileTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	//当前鼠标位置
	CPoint pt;
	GetCursorPos(&pt);
	this->m_wndFileTree.ScreenToClient(&pt);
	UINT uFlag = 0;
	HTREEITEM hCurSel = m_wndFileTree.GetSelectedItem();
	HTREEITEM hItem   = m_wndFileTree.HitTest(pt, &uFlag);
	if(NULL == hItem) 
	{//没有选中项
		*pResult = 0;
		return ;
	}
	if(hCurSel != hItem)
		m_wndFileTree.SelectItem(hItem);

	//当前选择的文件/文件路径
	CString file = this->GetCurSel();
	GetAllFiles(file);

	*pResult = 0;
}

CString CFileManageDlg::GetCurSel()
{
	//是否是文件夹
	BOOL isDir = FALSE;

	int nImage,nSelImage;
	CString filePath = _T("");
	HTREEITEM hCurSel = this->m_wndFileTree.GetSelectedItem();
	
	m_wndFileTree.GetItemImage(hCurSel , nImage , nSelImage);
	if(nImage == III_COMPUTER)
	{//选择的是 计算机
		return  CString(DIR_COMPUTER) + _T("\\");
	}else if(nImage == III_DESKTOP)
	{//桌面
		return CString(DIR_DESKTOP) + _T("\\");
	}
	else if (nImage == III_MY_DOC)
	{//我的文档
		return CString(DIR_MY_DOC) + _T("\\");
	}
	if(nImage == III_DIR || nImage == III_DISK)
	{
		isDir = TRUE;
	}

	filePath = this->m_wndFileTree.GetItemText(hCurSel).Trim();
	if(filePath.IsEmpty())//不能为空
	{
		//AfxMessageBox(_T("请选择一个节点"));
		return _T("");
	}	
	
	//获取完整的路径
	hCurSel = this->m_wndFileTree.GetParentItem(hCurSel);
	while(hCurSel != this->m_hComputer
		&& hCurSel != this->m_hDesktop
		&& hCurSel != this->m_hMyDoc)
	{
		filePath = (m_wndFileTree.GetItemText(hCurSel).Trim() +_T("\\")+ filePath);
		hCurSel = this->m_wndFileTree.GetParentItem(hCurSel);
	}

	if(hCurSel == m_hDesktop)
	{//是在桌面
		filePath  = _T("\\") + filePath;
		filePath = DIR_DESKTOP + filePath;
	}else if (hCurSel == m_hMyDoc)
	{//在我的文档
		filePath  = _T("\\") + filePath;
		filePath = DIR_MY_DOC + filePath;
	}

	//是目录
	if (isDir)
		filePath += _T("\\");
	
	return filePath;
}

void CFileManageDlg::InsertFilePath( const CString& data )
{
	//路径
	CString strPath = data;
	HTREEITEM  hParent = m_hComputer;//要插入的属性父项

	if(strPath.Left(_tcslen(DIR_MY_DOC)) == DIR_MY_DOC)
	{//我的文档
		hParent = m_hMyDoc;
		strPath = strPath.Mid(_tcslen(DIR_MY_DOC)+1);
	}else if (strPath.Left(_tcslen(DIR_DESKTOP)) == DIR_DESKTOP)
	{//桌面
		hParent = m_hDesktop;
		strPath = strPath.Mid(_tcslen(DIR_DESKTOP)+1);
	}
	
	//最后一个  '\'以后的下标
	int last = strPath.ReverseFind('\\')+1;

	CString strInsert,temp;

	for(int i = 0 ;i != last && i >= 0 ; i = strPath.Find(_T('\\'),i) + 1 )
	{// i 指向这次要截取的第一个字母
		strInsert = strPath.Mid( i , strPath.Find(_T('\\') , i) - i  );//截取要查找的数据
		if(this->m_wndFileTree.ItemHasChildren(hParent))
		{//有子项  就找一下是不是包含了这个字符串
			HTREEITEM thisParent = hParent;
			hParent = this->m_wndFileTree.GetChildItem(hParent);//获得第一个子项
			temp = this->m_wndFileTree.GetItemText(hParent).Trim();
			while(temp != strInsert)
			{
				//获得下一个兄弟节点
				hParent = this->m_wndFileTree.GetNextSiblingItem(hParent);
				if(hParent)//存在
					temp = this->m_wndFileTree.GetItemText(hParent).Trim();
				else//不存在
				{
					//有序的插入
					if(thisParent == m_hComputer)
						hParent = this->m_wndFileTree.InsertItem(strInsert , III_DISK ,  III_DISK , thisParent , TVI_SORT);
					else
						hParent = this->m_wndFileTree.InsertItem(strInsert , III_DIR ,  III_DIR , thisParent , TVI_SORT);
					break;
				}
			}
		}
		else
		{//没有子项  就直接插入
			if (m_hComputer == hParent)
			{//是一个磁盘驱动器
				hParent = this->m_wndFileTree.InsertItem(strInsert , III_DISK , III_DISK , hParent,TVI_SORT);
			}else//是一个目录
				hParent = this->m_wndFileTree.InsertItem(strInsert , III_DIR , III_DIR , hParent,TVI_SORT);
		}
	}
	//将最后的文件名插入到控件离去
	strPath = strPath.Mid(last);
	if(!strPath.IsEmpty())
	{//是文件就在这里插入
		if(this->m_wndFileTree.ItemHasChildren(hParent))
		{//要查的文件夹中有文件 就要判断自己是否存在
			HTREEITEM thisParent = hParent;
			//获得第一个子项
			hParent = this->m_wndFileTree.GetChildItem(hParent);
			temp = this->m_wndFileTree.GetItemText(hParent).Trim();
			while(temp != strPath)
			{
				//获得下一个兄弟节点
				hParent = this->m_wndFileTree.GetNextSiblingItem(hParent);
				if(hParent)
				{//存在
					temp = this->m_wndFileTree.GetItemText(hParent).Trim();
					continue;
				}else//不存在
				{
					hParent = this->m_wndFileTree.InsertItem(strPath , III_FILE , III_FILE , thisParent,TVI_SORT);
					break;
				}
			}
		}else //要插入的文件夹中没有文件
		{
			hParent = this->m_wndFileTree.InsertItem(strPath , III_FILE , III_FILE , hParent,TVI_SORT);
		}
	}
	return ;
}


void CFileManageDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (pWnd != &m_wndFileTree)
		return ;

	CMenu	menu;

	//快捷菜单
	menu.LoadMenu(IDR_FILE_TREE_MENU);
	menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN|TPM_TOPALIGN , point.x , point.y , this , NULL);
}

void CFileManageDlg::OnNMRClickFileTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	//当前鼠标位置
	CPoint pt;
	GetCursorPos(&pt);
	this->m_wndFileTree.ScreenToClient(&pt);
	UINT uFlag = 0;
	HTREEITEM hCurSel = m_wndFileTree.GetSelectedItem();
	HTREEITEM hItem   = m_wndFileTree.HitTest(pt, &uFlag);
	if(NULL == hItem) 		
		return ;
	if(hCurSel != hItem)
		m_wndFileTree.SelectItem(hItem);
}
void CFileManageDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
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

void CFileManageDlg::OnFlush()
{
	//先删除删除所有的子节点
	HTREEITEM hCurSel = m_wndFileTree.GetSelectedItem();
	if (hCurSel)
		FlushItem(hCurSel);
}

void CFileManageDlg::OnUpdateFlush(CCmdUI *pCmdUI)
{
	HTREEITEM hCurSel = m_wndFileTree.GetSelectedItem();
	int nImage = 0;
	int nSelImage = 0;
	m_wndFileTree.GetItemImage(hCurSel , nImage , nSelImage);
	if (nImage == III_FILE)
	{
		pCmdUI->Enable(FALSE);
	}else
		pCmdUI->Enable(TRUE);
}

void CFileManageDlg::OnDownload()
{
	//当前选择的文件/文件路径
	CString strPath = this->GetCurSel();
	if (!strPath.IsEmpty())
	{
		m_pDoc->DownloadFile(strPath , m_strIP );
	}
}

void CFileManageDlg::OnUpdateDownload(CCmdUI *pCmdUI)
{
	HTREEITEM hCurSel = m_wndFileTree.GetSelectedItem();
	int nImage = 0;
	int nSelImage = 0;
	m_wndFileTree.GetItemImage(hCurSel , nImage , nSelImage);
	if (nImage == III_FILE)
	{
		pCmdUI->Enable(TRUE);
	}else
		pCmdUI->Enable(FALSE);
}

void CFileManageDlg::OnUpload()
{
	//当前选择的文件/文件路径
	CString strPath = this->GetCurSel();
	if (!strPath.IsEmpty())
	{
		m_pDoc->UploadFile(strPath , m_strIP );
	}
}

void CFileManageDlg::OnUpdateUpload(CCmdUI *pCmdUI)
{
	HTREEITEM hCurSel = m_wndFileTree.GetSelectedItem();
	int nImage = 0;
	int nSelImage = 0;
	m_wndFileTree.GetItemImage(hCurSel , nImage , nSelImage);
	if (nImage == III_FILE || nImage == III_COMPUTER)
	{
		pCmdUI->Enable(FALSE);
	}else
		pCmdUI->Enable(TRUE);
}

void CFileManageDlg::OnDelete()
{
	//当前选择的文件/文件路径
	CString strPath = this->GetCurSel();
	char* cmdBuf = NULL;
	PRCMSG_BUFF pMsg = NULL;
	int size = 0;

	{//字符编码转换
	::setlocale(LC_ALL , "");
	size = ::wcstombs(NULL , strPath , 0);
	cmdBuf = new char[sizeof(RCMSG_BUFF) - 1 + size + 1];
	pMsg = PRCMSG_BUFF(cmdBuf);
	::wcstombs(pMsg->buf , strPath , size);
	::setlocale(LC_ALL , "C");
	}//字符编码转换

	pMsg->buf[size] = '\0';
	pMsg->msgHead.type = MT_DELETE_FILE_S;
	pMsg->msgHead.size = sizeof(RCMSG_BUFF) - 1 + size + 1;
	//将命令发出去
	m_pDoc->SendRcMessage(m_strIP , m_nPort , pMsg);
	delete[] cmdBuf;
}

void CFileManageDlg::OnUpdateDelete(CCmdUI *pCmdUI)
{
	HTREEITEM hCurSel = m_wndFileTree.GetSelectedItem();
	int nImage = 0;
	int nSelImage = 0;
	m_wndFileTree.GetItemImage(hCurSel , nImage , nSelImage);
	if (nImage == III_DIR || nImage == III_FILE)
	{
		pCmdUI->Enable(TRUE);
	}else
		pCmdUI->Enable(FALSE);
}

BOOL CFileManageDlg::GetAllFiles( const CString& strPath )
{
	if(strPath.Right(1) == _T("\\"))//选中的是一个文件夹
	{
		//通知客户端需要获取制定目录的文件/目录列表
		char* cmdBuf = NULL;
		PRCMSG_BUFF pMsg = NULL;
		int size = 0;

		{//字符编码转换
		::setlocale(LC_ALL , "");
		size = ::wcstombs(NULL , strPath , 0);
		cmdBuf = new char[sizeof(RCMSG_BUFF) - 1 + size + 1];
		pMsg = PRCMSG_BUFF(cmdBuf);
		::wcstombs(pMsg->buf , strPath , size);
		::setlocale(LC_ALL , "C");
		}//字符编码转换

		pMsg->buf[size] = '\0';
		pMsg->msgHead.type = MT_GET_FILE_LIST_S;
		pMsg->msgHead.size = sizeof(RCMSG_BUFF) - 1 + size + 1;
		//将命令发出去
		m_pDoc->SendRcMessage(m_strIP , m_nPort , pMsg);
		delete[] cmdBuf;
		return TRUE;
	}
	return FALSE;
}

void CFileManageDlg::FlushDir( const CString& strDir )
{
	HTREEITEM item = GetItemHandle(strDir);
	if (item)
		FlushItem(item);
}

HTREEITEM CFileManageDlg::GetItemHandle( CString path )
{
	//路径
	CString strPath = path;
	HTREEITEM  hParent = m_hComputer;
	
	//去掉最后的“\\”
	if(strPath.Right(1) == _T("\\"))
		strPath = strPath.Mid(strPath.GetLength()-1);

	if(strPath == DIR_MY_DOC)
		return m_hMyDoc;
	else if (strPath == DIR_DESKTOP)
		return m_hDesktop;

	if(strPath.Left(_tcslen(DIR_MY_DOC)) == DIR_MY_DOC)
	{//我的文档
		hParent = m_hMyDoc;
		strPath = strPath.Mid(_tcslen(DIR_MY_DOC)+1);
	}else if (strPath.Left(_tcslen(DIR_DESKTOP)) == DIR_DESKTOP)
	{//桌面
		hParent = m_hDesktop;
		strPath = strPath.Mid(_tcslen(DIR_DESKTOP)+1);
	}


	int index = 0;
	CString node;
	HTREEITEM hChild = hParent;
	while(1)
	{
		if(0 == strPath.GetLength())//找完了？
			return hChild;

		index = strPath.Find(_T('\\'));
		if (index == -1)
		{
			node = strPath;
			strPath = _T("");
		}
		else
		{
			node = strPath.Left(index);
			strPath = strPath.Mid(index + 1);
		}
		if (m_wndFileTree.ItemHasChildren(hParent))
		{
			//第一个孩子节点
			hChild = m_wndFileTree.GetChildItem(hParent);
			if (m_wndFileTree.GetItemText(hChild) != node)
			{
				while(hChild)
				{
					hChild = m_wndFileTree.GetNextSiblingItem(hChild);
					if(NULL == hChild)
						return NULL;//没有匹配项
					if (m_wndFileTree.GetItemText(hChild) == node)
						break;//当前节点已经匹配
				}//while(hChild)
			}//
		}
		else
			return NULL;
	}//while(1)
	return NULL;
}

void CFileManageDlg::FlushItem( HTREEITEM item)
{
	if (m_wndFileTree.ItemHasChildren(item))
	{//删除所有的孩子节点
		HTREEITEM temp;
		do{
			temp = m_wndFileTree.GetChildItem(item);
			m_wndFileTree.DeleteItem(temp);
		}while(m_wndFileTree.ItemHasChildren(item));
	}

	//当前选择的文件/文件路径
	CString file = this->GetCurSel();
	if (file.Left(_tcslen(DIR_COMPUTER)) == DIR_COMPUTER)
	{//获得驱动器信息
		DEF_RCMSG(rcMsg , MT_GET_DRIVER_S);
		m_pDoc->SendRcMessage(m_strIP , m_nPort , &rcMsg);
	}else
		GetAllFiles(file);
}
