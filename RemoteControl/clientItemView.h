///////////////////////////////////////////////////////////////
//
// FileName	: clientItemView.h 
// Creator	: 杨松
// Date		: 2013年2月27日, 23:39:40
// Comment	: 客户端监视视图类声明
//
//////////////////////////////////////////////////////////////

#pragma once

// ClientItemView
class CRemoteControlDoc;
class CCMDTable;
class CCMDDlg;
struct ClientDescripter;

class ClientItemView : public CWnd
{
	DECLARE_DYNAMIC(ClientItemView)

public:
	ClientItemView( ClientDescripter* clientDes ,CRemoteControlDoc* pDoc );
	virtual ~ClientItemView();

	//客户端视图创建
	BOOL Create( CWnd *pParent , UINT id);

	//获得绘制远程screen的矩形
	const CRect& GetScreenViewRect();
	
	//获取客户端的ip地址
	const CString& GetClientIP(void);

	//客户端发来的消息函数处理函数
	void OnRcMessage( USHORT port, const void* msg );

	//停止监视客户端
	void StopScreenCaptureThread();

	//客户端下线
	void Dropped();

	//设置是否需要绘制边框
	BOOL SetDrawEdge(BOOL drawEdge);

	//Windows消息预处理
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	//窗口销毁
	virtual BOOL DestroyWindow();

	//////////////////////////////////////////////////////////////////////////
	//初始化ItemView
	void InitItemView();
protected:
	DECLARE_MESSAGE_MAP()

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	//当前view的绘制消息响应函数
	afx_msg void OnPaint();

	//当前view大小改变，需要通知客户端调整发送过来的图像大小
	afx_msg void OnSize(UINT nType, int cx, int cy);
	
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	//鼠标左键双击
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

	//当前view的显示消息处理函数
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	
	//////////////////////////////////////////////////////////////////////////
	//已经获得本地的screen端口
	//param
	//		wParam	端口
	afx_msg LRESULT OnGetAScreenPort(WPARAM wParam,LPARAM lParam);
	
	//套接字出错
	afx_msg LRESULT OnCreatScreenSockErr(WPARAM wParam,LPARAM lParam);
	
	//线程退出
	afx_msg LRESULT OnScreenThreadExit(WPARAM wParam,LPARAM lParam);
	
	//等待客户端发来ScreenPort响应消息失败
	afx_msg LRESULT OnWaitForCaptureTimeOut(WPARAM wParam,LPARAM lParam);
	
	//////////////////////////////////////////////////////////////////////////
	//调整抓屏质量
	//param
	//		wparam	质量
	afx_msg LRESULT OnChangeScreenQuality(WPARAM wParam,LPARAM lParam);
	
	//////////////////////////////////////////////////////////////////////////
	//调整流畅度
	//param
	//		wparam	流畅度
	afx_msg LRESULT OnChangeScreenFluency(WPARAM wParam,LPARAM lParam);

	//////////////////////////////////////////////////////////////////////////
	//跟换对对客户端的控制状态
	//param
	//		wParam	是否需要控制
	afx_msg LRESULT OnScreenCtrl(WPARAM wParam,LPARAM lParam);
	
	//点击了CMD图标
	afx_msg LRESULT OnCMDClicked(WPARAM wParam,LPARAM lParam);
	
	//全屏 
	afx_msg LRESULT OnFullScreen(WPARAM wParam,LPARAM lParam);
	
	//文件管理
	afx_msg LRESULT OnDocManage(WPARAM wParam,LPARAM lParam);

	//停止监视
	afx_msg void OnStopCtrl();
	
	//退出客户端
	afx_msg void OnExitClient();
	
	//客户端关机
	afx_msg void OnShutdwonClient();
	
	//重启被控端
	afx_msg void OnResarteClient();
	
	//注销被控端
	afx_msg void OnLoginOut();
	
	//字母雨屏蔽被控端桌面
	afx_msg void OnLetterRain();
	
	//字母雨菜单跟新
	afx_msg void OnUpdateLetterRain(CCmdUI *pCmdUI);
	
	//窗口被移动了
	afx_msg void OnMove(int x, int y);

private:

	//客户端描述符
	ClientDescripter* m_pClientDescripter;

	//服务器的文档视图结构的文档指针
	CRemoteControlDoc* m_pDoc;

	//接收和显示被控端桌面图像数据的线程上下文
	PrintScreenThreadContext m_PrintScreenThreadContext;

	//命令面板
	CCMDTable* m_pCmdTable;
	
	//是否控制桌面
	BOOL	m_bScreenCtrled;
	
	//字幕雨
	BOOL m_bLetterRain;

	//是否需要绘制边框
	BOOL m_bDrawEdge;

	//绘制图像区域的矩形
	CRect m_rcScreenViewRect;
};
