///////////////////////////////////////////////////////////////
//
// FileName	: RemoteControlView.h 
// Creator	: ����
// Date		: 2013��2��27��, 20:10:26
// Comment	: Զ�ؿ��Ʒ������ĵ���ͼ�ṹ�е���ͼ�������
//
//////////////////////////////////////////////////////////////


#pragma once

class ClientItemView;


class CRemoteControlView : public CView
{
protected: // �������л�����
	CRemoteControlView();
	DECLARE_DYNCREATE(CRemoteControlView)

// ����
public:
	CRemoteControlDoc* GetDocument() const;

	//���пͻ�����ͼ
	typedef std::list<ClientItemView*>  ClientViewList;
	ClientViewList m_lstClient;

	//���һ���ͻ�����ͼ��ID
	UINT m_iLastClientID;
	//ȫ����ʾ��ClientItemView
	ClientItemView* m_pFullViewItem;
	
	//�ͻ�����ͼ����ʾ������������
	int m_iRowCount;
	int m_iColumnCount;

	//��ǰ��ʾ�ĵ�һ���к�
	int m_curRowIndex;

// ����
public:
	//���һ���ͻ��˼��Ӵ���
	BOOL AddNewClient(PClientDescripter clientDes);
	//��������ItemView��λ��
	//param
	//		keepFull	�Ƿ񱣳�ԭ�е�
	void AdjustItemViewPosition(BOOL keepFull = TRUE);

	//����������Ҫ���ٵ�����
	void DestroyView();

// ��д
public:
	virtual void OnDraw(CDC* pDC);  // ��д�Ի��Ƹ���ͼ
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// ʵ��
public:
	virtual ~CRemoteControlView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// ���ɵ���Ϣӳ�亯��
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnItemViewDBClick(WPARAM wParam , LPARAM lParam);
	
	//ȫ����ʾ
	afx_msg LRESULT OnFullScreen(WPARAM wParam , LPARAM lParam);
	
	//////////////////////////////////////////////////////////////////////////
	//ֹͣ����һ���ͻ���
	//param
	//		wParam	�ͻ��˵�ip��ַ
	afx_msg LRESULT OnStopMonitoringClient(WPARAM wParam , LPARAM lParam);

	void DispatchMsg( const CString& ip, USHORT port, const void* msg );
	

	//////////////////////////////////////////////////////////////////////////
	//һ���ͻ�������
	//param
	//		ip	�ͻ���ip��ַ
	void ClientDropped( const CString& ip );

	//��ʼ��
	virtual void OnInitialUpdate();

	//����������������Χ
	void AdjustScrollSize();

	//���ÿһ���ͻ�����ͼ�Ĵ�С
	void GetItenSize(int& w , int& h);

	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	//���������
	int GetRowCount();
	afx_msg void OnMove(int x, int y);
};

#ifndef _DEBUG  // RemoteControlView.cpp �еĵ��԰汾
inline CRemoteControlDoc* CRemoteControlView::GetDocument() const
   { return reinterpret_cast<CRemoteControlDoc*>(m_pDocument); }
#endif

