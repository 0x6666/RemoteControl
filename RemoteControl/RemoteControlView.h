///////////////////////////////////////////////////////////////
//
// FileName	: RemoteControlView.h 
// Creator	: 杨松
// Date		: 2013年2月27日, 20:10:26
// Comment	: 远控控制服务器文档视图结构中的视图类的声明
//
//////////////////////////////////////////////////////////////


#pragma once

class ClientItemView;


class CRemoteControlView : public CView
{
protected: // 仅从序列化创建
	CRemoteControlView();
	DECLARE_DYNCREATE(CRemoteControlView)

// 属性
public:
	CRemoteControlDoc* GetDocument() const;

	//所有客户端视图
	typedef std::list<ClientItemView*>  ClientViewList;
	ClientViewList m_lstClient;

	//最后一个客户端视图的ID
	UINT m_iLastClientID;
	//全屏显示的ClientItemView
	ClientItemView* m_pFullViewItem;
	
	//客户端视图可显示的行数和列数
	int m_iRowCount;
	int m_iColumnCount;

	//当前显示的第一行行号
	int m_curRowIndex;

// 操作
public:
	//添加一个客户端监视窗口
	BOOL AddNewClient(PClientDescripter clientDes);
	//调整各个ItemView的位置
	//param
	//		keepFull	是否保持原有的
	void AdjustItemViewPosition(BOOL keepFull = TRUE);

	//销毁所有需要销毁的数据
	void DestroyView();

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// 实现
public:
	virtual ~CRemoteControlView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnItemViewDBClick(WPARAM wParam , LPARAM lParam);
	
	//全屏显示
	afx_msg LRESULT OnFullScreen(WPARAM wParam , LPARAM lParam);
	
	//////////////////////////////////////////////////////////////////////////
	//停止监视一个客户端
	//param
	//		wParam	客户端的ip地址
	afx_msg LRESULT OnStopMonitoringClient(WPARAM wParam , LPARAM lParam);

	void DispatchMsg( const CString& ip, USHORT port, const void* msg );
	

	//////////////////////////////////////////////////////////////////////////
	//一个客户端下线
	//param
	//		ip	客户端ip地址
	void ClientDropped( const CString& ip );

	//初始化
	virtual void OnInitialUpdate();

	//调整滚动条滚动范围
	void AdjustScrollSize();

	//获得每一个客户端视图的大小
	void GetItenSize(int& w , int& h);

	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	//获得总行数
	int GetRowCount();
	afx_msg void OnMove(int x, int y);
};

#ifndef _DEBUG  // RemoteControlView.cpp 中的调试版本
inline CRemoteControlDoc* CRemoteControlView::GetDocument() const
   { return reinterpret_cast<CRemoteControlDoc*>(m_pDocument); }
#endif

