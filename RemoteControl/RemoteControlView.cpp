///////////////////////////////////////////////////////////////
//
// FileName	: RemoteControlView.cpp
// Creator	: 杨松
// Date		: 2013年2月27日, 20:10:26
// Comment	: 远控控制服务器文档视图结构中的视图类的实现
//
//////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RemoteControl.h"

#include "RemoteControlDoc.h"
#include "RemoteControlView.h"
#include "clientItemView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CRemoteControlView

IMPLEMENT_DYNCREATE(CRemoteControlView, CView)

BEGIN_MESSAGE_MAP(CRemoteControlView, CView)
	ON_WM_SIZE()
	ON_MESSAGE(WM_ITEM_VIEW_SBCLICK , OnItemViewDBClick)
	ON_MESSAGE(WM_FULLS_CREEN , OnFullScreen)
	ON_MESSAGE(WM_MONITORINT_CLIENT , OnStopMonitoringClient)
	ON_WM_NCCALCSIZE()
	ON_WM_VSCROLL()
	ON_WM_MOVE()
END_MESSAGE_MAP()

// CRemoteControlView 构造/析构

CRemoteControlView::CRemoteControlView()
/*_APS_NEXT_CONTROL_VALUE是编译器生成的下一个控件的ID
  因为客户端视图是动态创建，需要ID且不能重复，所以在这里
  将m_iLastClientID设置为_APS_NEXT_CONTROL_VALUE的值（动
  态设置，每次添加或者删除了控件后手动设置）
*/
: m_iLastClientID(1037)
, m_pFullViewItem(NULL)
, m_curRowIndex(0)
, m_iRowCount(2)  //2
, m_iColumnCount(3)    //3
{
	// TODO: 在此处添加构造代码
}

CRemoteControlView::~CRemoteControlView()
{
}

BOOL CRemoteControlView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CRemoteControlView 绘制

void CRemoteControlView::OnDraw(CDC* /*pDC*/)
{
	CRemoteControlDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
}


// CRemoteControlView 诊断

#ifdef _DEBUG
void CRemoteControlView::AssertValid() const
{
	CView::AssertValid();
}

void CRemoteControlView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CRemoteControlDoc* CRemoteControlView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CRemoteControlDoc)));
	return (CRemoteControlDoc*)m_pDocument;
}

#endif //_DEBUG

BOOL CRemoteControlView::AddNewClient( PClientDescripter clientDes )
{
	if (NULL == clientDes)
		return FALSE;//貌似参数有问题
	
	for (ClientViewList::iterator it = m_lstClient.begin() 
		; it != m_lstClient.end() ; ++it)
	{
		if ((*it)->GetClientIP() == clientDes->mIP)//客户端已经存在了
			return FALSE;
	}

	ClientItemView* item = new ClientItemView( clientDes/*name , strIP , port */, GetDocument());
	if(FALSE == item->Create( this , ++m_iLastClientID ))
	{//创建失败
		delete item;
		--m_iLastClientID;
		return FALSE;
	}
	else
	{//创建成功
		m_lstClient.push_back(item);
		AdjustItemViewPosition(FALSE);

		//调整滚动条的滚动范围
		AdjustScrollSize();
	}
	return TRUE;
}

// CRemoteControlView 消息处理程序

void CRemoteControlView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	AdjustItemViewPosition();
}

void CRemoteControlView::DispatchMsg( const CString& ip, USHORT port, const void* msg )
{
	//将消息派送派送到指定的客户端视图
	ClientViewList::iterator it = m_lstClient.begin();
	for ( ; it != m_lstClient.end() ; ++it ) 
	{
		if ((*it)->GetClientIP() == ip)
		{
			(*it)->OnRcMessage( port, msg );
			return ;
		}
	}
}

LRESULT CRemoteControlView::OnItemViewDBClick( WPARAM wParam , LPARAM lParam )
{
	ASSERT(wParam != NULL);
	CString ip = *((CString*)wParam);
	if(!IsWindow(m_pFullViewItem->GetSafeHwnd()))
		m_pFullViewItem = NULL;

	if ((NULL != m_pFullViewItem) && (m_pFullViewItem->GetClientIP() == ip))
	{//已经是full view了,那就重新排列
		m_pFullViewItem->SetDrawEdge(TRUE);
		m_pFullViewItem = NULL;
		AdjustItemViewPosition(FALSE);
	}else
	{//设置full view
		for ( ClientViewList::iterator it = m_lstClient.begin() ;
			it != m_lstClient.end() ; ++it ) 
		{
			if ((*it)->GetClientIP() == ip)
			{
				m_pFullViewItem = *it;
				m_pFullViewItem->SetDrawEdge(FALSE);
				CRect rc;
				this->GetClientRect(&rc);
				m_pFullViewItem->MoveWindow(0 , 0 , rc.Width() , rc.Height() , TRUE);
				m_pFullViewItem->ShowWindow(SW_SHOW);
				//m_pFullViewItem->SetWindowPos(NULL , 0 , 0 , rc.Width() , rc.Height() , SWP_SHOWWINDOW);
			}else{
				(*it)->ShowWindow(SW_HIDE);
			}
		}
	}

	//调整滚动范围
	AdjustScrollSize();

	this->Invalidate(TRUE);
	return 0;
}

void CRemoteControlView::AdjustItemViewPosition(BOOL keepFull /*= TRUE*/)
{
	if (keepFull && m_pFullViewItem)
	{//保持原有的fullView
		//先调整滚动范围
		AdjustScrollSize();
	
		CRect rc;
		this->GetClientRect(&rc);
		m_pFullViewItem->SetWindowPos(NULL , 0 , 0 , rc.Width() , rc.Height() , SWP_SHOWWINDOW);
	}
	else
	{//不需要保持原有的fullView
		
		if (NULL != m_pFullViewItem)
		{//原来就是全屏的
			m_pFullViewItem->SetDrawEdge(TRUE);
			m_pFullViewItem = NULL;
		}

		ClientViewList::iterator it = m_lstClient.begin();
		int height = 0;
		int width = 0;
		GetItenSize( width , height);
		int i = 0;

		//先跳过前面隐藏的
		for ( i = 0 ; it != m_lstClient.end() && 
			(i < m_curRowIndex * m_iColumnCount) ; ++it , ++i) 
		{//可以显示的就调整一下位置
			(*it)->ShowWindow(SW_HIDE);
		}

		for ( i = 0 ; it != m_lstClient.end() &&
			(i != (m_iRowCount * m_iColumnCount)) ;
			++it , ++i ) 
		{//可以显示的就调整一下位置
			(*it)->MoveWindow((i % m_iColumnCount) * width , 
				(/*(*/i / m_iColumnCount/*) / m_iRowCount*/) * height ,
				width , height ,TRUE);
			(*it)->ShowWindow(SW_SHOW);
		}

		//后面的也不需要显示了
		for ( ; it != m_lstClient.end() ; ++it ) 
		{//可以显示的就调整一下位置
			(*it)->ShowWindow(SW_HIDE);
		}

		//先调整滚动范围
		AdjustScrollSize();
	}
}

void CRemoteControlView::DestroyView()
{
	for (ClientViewList::iterator it = m_lstClient.begin() ; it != m_lstClient.end() ; ++it)
	{
		ClientItemView* pView = *it;
		pView->DestroyWindow();
		delete pView;
	//	it = m_lstClient.begin();
	}
	m_lstClient.clear();
}

void CRemoteControlView::ClientDropped( const CString& ip )
{
	for(ClientViewList::iterator it = m_lstClient.begin() ; it != m_lstClient.end() ; ++it)
	{
		if ((*it)->GetClientIP() == ip)
		{//找到了制定的客户端视图
			(*it)->Dropped();
			break;
		}
	}
}

LRESULT CRemoteControlView::OnStopMonitoringClient( WPARAM wParam , LPARAM lParam )
{
	CString strIP = *((CString*)wParam);
	CRemoteControlDoc* pDoc = GetDocument();
	pDoc->StopMonitoring(strIP);

	//从客户端视图链表中删除指定的客户端
	for(ClientViewList::iterator it = m_lstClient.begin() ; it != m_lstClient.end() ; ++it)
	{
		if ((*it)->GetClientIP() == strIP)
		{//找到了制定的客户端视图
			ClientItemView* view = *it;
			if (m_pFullViewItem == view)
			{//是全屏的
				m_pFullViewItem  = NULL;
			}
			view->DestroyWindow();
			delete view;
			m_lstClient.remove(view);
			break;
		}
	}

	if ((0 != m_curRowIndex) && (GetRowCount() - m_curRowIndex) < m_iRowCount)
	{//有隐藏但是当前显示区域没有填满
		--m_curRowIndex;
		this->SetScrollPos(SB_VERT , m_curRowIndex , TRUE);
	}

	//调整各个视图的位置
	AdjustItemViewPosition(FALSE);

	return 0;
}

LRESULT CRemoteControlView::OnFullScreen( WPARAM wParam , LPARAM lParam )
{
	ASSERT(wParam != NULL);
	CString ip = *((CString*)wParam);
	if ((NULL != m_pFullViewItem) && (m_pFullViewItem->GetClientIP() == ip))
	{//已经是full view了
		return 0;
	}else
	{//设置full view
		for ( ClientViewList::iterator it = m_lstClient.begin() ;
			it != m_lstClient.end() ; ++it ) 
		{
			if ((*it)->GetClientIP() == ip)
			{
				m_pFullViewItem = *it;
				m_pFullViewItem->SetDrawEdge(FALSE);
				CRect rc;
				this->GetClientRect(&rc);
				m_pFullViewItem->MoveWindow(0 , 0 , rc.Width() , rc.Height() , TRUE);
				m_pFullViewItem->ShowWindow(SW_SHOW);
				//m_pFullViewItem->SetWindowPos(NULL , 0 , 0 , rc.Width() , rc.Height() , SWP_SHOWWINDOW);
			}
			else
			{//其他的不需要显示的都隐藏
				(*it)->ShowWindow(SW_HIDE);
			}
		}
	}
	this->Invalidate(TRUE);
	return 0;
}

void CRemoteControlView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	SCROLLINFO si = {0};
	si.cbSize = sizeof(SCROLLINFO); 
	si.fMask = SIF_PAGE ; 
	si.nPage = 1;
	this->SetScrollInfo(SB_VERT , &si , TRUE);
}

void CRemoteControlView::AdjustScrollSize()
{
	if (NULL != m_pFullViewItem)
	{//全屏时需要隐藏滚动条
		this->ShowScrollBar(SB_VERT , FALSE);
	}
	else
	{//非全屏
		if((int)m_lstClient.size() > (m_iRowCount * m_iColumnCount))
		{//需要显示滚动条
			//计算滚动区域的大小
			int rCnt = GetRowCount();
			rCnt -= (m_iRowCount - 1);
			this->SetScrollRange(SB_VERT , 0 , rCnt-1);
			this->ShowScrollBar(SB_VERT , TRUE);
		}
		else
		{//不需要显示滚动条
			this->ShowScrollBar(SB_VERT , FALSE);
		}
	}
}

void CRemoteControlView::GetItenSize( int& w , int& h )
{
	CRect rc;
	this->GetClientRect(&rc);

	h = rc.Height() / m_iRowCount;
	w = rc.Width() / m_iColumnCount;
}

void CRemoteControlView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	switch (nSBCode)
	{
	case SB_BOTTOM:
	case SB_TOP:
	case SB_ENDSCROLL:
		break;
	case SB_LINEDOWN:
	case SB_PAGEDOWN:
		{
			int rCnt = GetRowCount();//行数
			if (rCnt > m_curRowIndex + 1)
			{//还可以下滚
				++m_curRowIndex;
				AdjustItemViewPosition(TRUE);
				this->SetScrollPos(SB_VERT , m_curRowIndex , TRUE);
			}
		}
		break;
	case SB_LINEUP:
	case SB_PAGEUP:
		{
			if (0 < m_curRowIndex )
			{//还可以下滚
				--m_curRowIndex;
				AdjustItemViewPosition(TRUE);
				this->SetScrollPos(SB_VERT , m_curRowIndex , TRUE);
			}
		}
		break;
	case SB_THUMBPOSITION:
		break;
	case SB_THUMBTRACK:
		{//拖动到制定位置
			if (m_curRowIndex != nPos)
			{//位置有调整
				m_curRowIndex = nPos;
				AdjustItemViewPosition(TRUE);
				this->SetScrollPos(SB_VERT , m_curRowIndex , TRUE);
			}
		}
		break;
	default:
		ASSERT(FALSE);
	}

	CView::OnVScroll(nSBCode, nPos, pScrollBar);
}

int CRemoteControlView::GetRowCount()
{
	int rCnt = m_lstClient.size();
	rCnt = (rCnt / m_iRowCount) + ((rCnt % m_iRowCount)? 1 : 0);
	return rCnt;
}

void CRemoteControlView::OnMove(int x, int y)
{
	CView::OnMove(x, y);

	for (ClientViewList::iterator it = m_lstClient.begin() ; it != m_lstClient.end(); ++it)
	{
		(*it)->PostMessage(WM_MOVE , 0 , 0);
	}
}
