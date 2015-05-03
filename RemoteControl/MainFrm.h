///////////////////////////////////////////////////////////////
//
// FileName	: MainFrm.h  
// Creator	: 杨松
// Date		: 2013年2月27日, 20:10:26
// Comment	: 远控控制服务器主框架的类的声明
//
//////////////////////////////////////////////////////////////


#pragma once

class CCtrlBar;

class CMainFrame : public CFrameWnd
{
	
protected: // 仅从序列化创建
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// 属性
public:
	//客户端列表
	CCtrlBar*	m_pClientPane;
	CListCtrl*	m_pClientList;


// 操作
public:

private:
	//创建客户端列表面板
	BOOL CreateClientPane();
	void InitClientListCtrl();
	
// 重写
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 实现
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // 控件条嵌入成员
	CStatusBar  m_wndStatusBar;
	
	//托盘图标
	NOTIFYICONDATA m_notifyIcon;

	//全屏之前是否显示状态栏
	BOOL m_bFSStatusBarWasVisible;
	//全屏之前是否显示客户端列表
	BOOL m_bFSClientListWasVisible;
	//全屏之前窗口的位置
	CRect m_rcFSPos;

// 生成的消息映射函数
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	//初始化状态栏 
	BOOL InitStatusBar();

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL DestroyWindow();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClientList();
	afx_msg void OnUpdateClientList(CCmdUI *pCmdUI);

	//托盘图标发过来的消息
	afx_msg LRESULT OnNotifyIconMsg(WPARAM wParam , LPARAM lParam);
	afx_msg void OnExit();

	//改变状态栏客户端以及在线数量显示
	afx_msg LRESULT OnClientCount(WPARAM wParam , LPARAM lParam);
	
	//已经推送了的客户端数量 
	afx_msg LRESULT OnPushedCount(WPARAM wParam , LPARAM lParam);

	//在状态栏体是一个消息
	//param
	//	wParam	(CString*) 要提示的消息
	afx_msg LRESULT OnMessage(WPARAM wParam , LPARAM lParam);

	//全屏 
	afx_msg LRESULT OnFullScreen(WPARAM wParam , LPARAM lParam);
	afx_msg void OnMove(int x, int y);
};


