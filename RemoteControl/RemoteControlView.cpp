///////////////////////////////////////////////////////////////
//
// FileName	: RemoteControlView.cpp
// Creator	: ����
// Date		: 2013��2��27��, 20:10:26
// Comment	: Զ�ؿ��Ʒ������ĵ���ͼ�ṹ�е���ͼ���ʵ��
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

// CRemoteControlView ����/����

CRemoteControlView::CRemoteControlView()
/*_APS_NEXT_CONTROL_VALUE�Ǳ��������ɵ���һ���ؼ���ID
  ��Ϊ�ͻ�����ͼ�Ƕ�̬��������ҪID�Ҳ����ظ�������������
  ��m_iLastClientID����Ϊ_APS_NEXT_CONTROL_VALUE��ֵ����
  ̬���ã�ÿ����ӻ���ɾ���˿ؼ����ֶ����ã�
*/
: m_iLastClientID(1037)
, m_pFullViewItem(NULL)
, m_curRowIndex(0)
, m_iRowCount(2)  //2
, m_iColumnCount(3)    //3
{
	// TODO: �ڴ˴���ӹ������
}

CRemoteControlView::~CRemoteControlView()
{
}

BOOL CRemoteControlView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ

	return CView::PreCreateWindow(cs);
}

// CRemoteControlView ����

void CRemoteControlView::OnDraw(CDC* /*pDC*/)
{
	CRemoteControlDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: �ڴ˴�Ϊ����������ӻ��ƴ���
}


// CRemoteControlView ���

#ifdef _DEBUG
void CRemoteControlView::AssertValid() const
{
	CView::AssertValid();
}

void CRemoteControlView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CRemoteControlDoc* CRemoteControlView::GetDocument() const // �ǵ��԰汾��������
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CRemoteControlDoc)));
	return (CRemoteControlDoc*)m_pDocument;
}

#endif //_DEBUG

BOOL CRemoteControlView::AddNewClient( PClientDescripter clientDes )
{
	if (NULL == clientDes)
		return FALSE;//ò�Ʋ���������
	
	for (ClientViewList::iterator it = m_lstClient.begin() 
		; it != m_lstClient.end() ; ++it)
	{
		if ((*it)->GetClientIP() == clientDes->mIP)//�ͻ����Ѿ�������
			return FALSE;
	}

	ClientItemView* item = new ClientItemView( clientDes/*name , strIP , port */, GetDocument());
	if(FALSE == item->Create( this , ++m_iLastClientID ))
	{//����ʧ��
		delete item;
		--m_iLastClientID;
		return FALSE;
	}
	else
	{//�����ɹ�
		m_lstClient.push_back(item);
		AdjustItemViewPosition(FALSE);

		//�����������Ĺ�����Χ
		AdjustScrollSize();
	}
	return TRUE;
}

// CRemoteControlView ��Ϣ�������

void CRemoteControlView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	AdjustItemViewPosition();
}

void CRemoteControlView::DispatchMsg( const CString& ip, USHORT port, const void* msg )
{
	//����Ϣ�������͵�ָ���Ŀͻ�����ͼ
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
	{//�Ѿ���full view��,�Ǿ���������
		m_pFullViewItem->SetDrawEdge(TRUE);
		m_pFullViewItem = NULL;
		AdjustItemViewPosition(FALSE);
	}else
	{//����full view
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

	//����������Χ
	AdjustScrollSize();

	this->Invalidate(TRUE);
	return 0;
}

void CRemoteControlView::AdjustItemViewPosition(BOOL keepFull /*= TRUE*/)
{
	if (keepFull && m_pFullViewItem)
	{//����ԭ�е�fullView
		//�ȵ���������Χ
		AdjustScrollSize();
	
		CRect rc;
		this->GetClientRect(&rc);
		m_pFullViewItem->SetWindowPos(NULL , 0 , 0 , rc.Width() , rc.Height() , SWP_SHOWWINDOW);
	}
	else
	{//����Ҫ����ԭ�е�fullView
		
		if (NULL != m_pFullViewItem)
		{//ԭ������ȫ����
			m_pFullViewItem->SetDrawEdge(TRUE);
			m_pFullViewItem = NULL;
		}

		ClientViewList::iterator it = m_lstClient.begin();
		int height = 0;
		int width = 0;
		GetItenSize( width , height);
		int i = 0;

		//������ǰ�����ص�
		for ( i = 0 ; it != m_lstClient.end() && 
			(i < m_curRowIndex * m_iColumnCount) ; ++it , ++i) 
		{//������ʾ�ľ͵���һ��λ��
			(*it)->ShowWindow(SW_HIDE);
		}

		for ( i = 0 ; it != m_lstClient.end() &&
			(i != (m_iRowCount * m_iColumnCount)) ;
			++it , ++i ) 
		{//������ʾ�ľ͵���һ��λ��
			(*it)->MoveWindow((i % m_iColumnCount) * width , 
				(/*(*/i / m_iColumnCount/*) / m_iRowCount*/) * height ,
				width , height ,TRUE);
			(*it)->ShowWindow(SW_SHOW);
		}

		//�����Ҳ����Ҫ��ʾ��
		for ( ; it != m_lstClient.end() ; ++it ) 
		{//������ʾ�ľ͵���һ��λ��
			(*it)->ShowWindow(SW_HIDE);
		}

		//�ȵ���������Χ
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
		{//�ҵ����ƶ��Ŀͻ�����ͼ
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

	//�ӿͻ�����ͼ������ɾ��ָ���Ŀͻ���
	for(ClientViewList::iterator it = m_lstClient.begin() ; it != m_lstClient.end() ; ++it)
	{
		if ((*it)->GetClientIP() == strIP)
		{//�ҵ����ƶ��Ŀͻ�����ͼ
			ClientItemView* view = *it;
			if (m_pFullViewItem == view)
			{//��ȫ����
				m_pFullViewItem  = NULL;
			}
			view->DestroyWindow();
			delete view;
			m_lstClient.remove(view);
			break;
		}
	}

	if ((0 != m_curRowIndex) && (GetRowCount() - m_curRowIndex) < m_iRowCount)
	{//�����ص��ǵ�ǰ��ʾ����û������
		--m_curRowIndex;
		this->SetScrollPos(SB_VERT , m_curRowIndex , TRUE);
	}

	//����������ͼ��λ��
	AdjustItemViewPosition(FALSE);

	return 0;
}

LRESULT CRemoteControlView::OnFullScreen( WPARAM wParam , LPARAM lParam )
{
	ASSERT(wParam != NULL);
	CString ip = *((CString*)wParam);
	if ((NULL != m_pFullViewItem) && (m_pFullViewItem->GetClientIP() == ip))
	{//�Ѿ���full view��
		return 0;
	}else
	{//����full view
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
			{//�����Ĳ���Ҫ��ʾ�Ķ�����
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
	{//ȫ��ʱ��Ҫ���ع�����
		this->ShowScrollBar(SB_VERT , FALSE);
	}
	else
	{//��ȫ��
		if((int)m_lstClient.size() > (m_iRowCount * m_iColumnCount))
		{//��Ҫ��ʾ������
			//�����������Ĵ�С
			int rCnt = GetRowCount();
			rCnt -= (m_iRowCount - 1);
			this->SetScrollRange(SB_VERT , 0 , rCnt-1);
			this->ShowScrollBar(SB_VERT , TRUE);
		}
		else
		{//����Ҫ��ʾ������
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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	switch (nSBCode)
	{
	case SB_BOTTOM:
	case SB_TOP:
	case SB_ENDSCROLL:
		break;
	case SB_LINEDOWN:
	case SB_PAGEDOWN:
		{
			int rCnt = GetRowCount();//����
			if (rCnt > m_curRowIndex + 1)
			{//�������¹�
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
			{//�������¹�
				--m_curRowIndex;
				AdjustItemViewPosition(TRUE);
				this->SetScrollPos(SB_VERT , m_curRowIndex , TRUE);
			}
		}
		break;
	case SB_THUMBPOSITION:
		break;
	case SB_THUMBTRACK:
		{//�϶����ƶ�λ��
			if (m_curRowIndex != nPos)
			{//λ���е���
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
