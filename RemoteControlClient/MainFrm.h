// MainFrm.h : CMainFrame 类的接口
//


#pragma once

#include "ChildView.h"
class CLetterRainDlg;
typedef std::list<PUploadDescripter> UploadDesList;

class CMainFrame : public CFrameWnd ,public Dispatcher
{
	
public:
	CMainFrame();
protected: 
	DECLARE_DYNAMIC(CMainFrame)

private:
	//服务器ip地址
	CString m_strServerIP;
	//消息中心
	MsgCenter* m_pMsgCenter;
	//用查找服务器的广播套接字
	CAsyncSocket* m_pBroadcastSocket;
	//心跳包广播是否在运行
	BOOL m_bHeartbeatTimerRun;
	//当前计算机的主机名
	char* m_czHostName;
	//当前客户端监听的端口 
	USHORT m_uPort;

	//抓屏线程上下文
	CaptureScreenThreadContext m_CCScreenThreadContext;
	
	//接收服务器推送过来的桌面图像数据线程上下文
	RecvPushedDesktopThreadContext m_RecvPushedDesktopThreadContext;

	//字幕雨对话框
	CLetterRainDlg* m_pLetterRainDlg;

	//CMD线程上下文
	CMDContext m_cmdContext;

	//托盘图标
	NOTIFYICONDATA m_notifyIcon;

	//需要枚举的文件夹路径
	CString m_strEnumPath;

	//监听线程描述符
	PDownloadListrenDescripter m_pDlListenDes;

	//上传线程描述符
	UploadDesList m_listUploadDescripter;

// 操作
public:

	//获得枚举路径
	CString GetEnumPath();

	//停止读取cmd数据线程
	void StopReadCMDThread();


// 重写
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

	//////////////////////////////////////////////////////////////////////////
	//消息派送函数，派送的是服务器发送过来的消息
	//param
	//		msg		消息数据(RCMSG)
	//		ip		服务器ip地址
	//		port	服务器发消息的端口
	virtual void DispatchMsg(const void* msg , CString ip, UINT port);

// 实现
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	CChildView    m_wndView;

// 生成的消息映射函数
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	void StartFindServerBroadcastTimer();

	afx_msg void OnSetFocus(CWnd *pOldWnd);
	DECLARE_MESSAGE_MAP()
public:
	//窗口销毁
	virtual BOOL DestroyWindow();
	//定时器回调函数
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	//托盘图标消息
	afx_msg LRESULT OnNotifyIconMsg(WPARAM wParam , LPARAM lParam);
	//托盘图标消息 退出 
	afx_msg void OnExit();
	//项服务器发送一个举手的消息
	afx_msg void OnHandsUp();

	//创建上传文件线程
	//wparam	SOCKET*	已经连接了的套接字
	afx_msg LRESULT OnCreateUploadThread(WPARAM wParam , LPARAM lParam);

	//获得了上传监听端口
	//param
	//wParam	USHORT	端口
	afx_msg LRESULT OnUploadListenPort(WPARAM wParam , LPARAM lParam);

	//发送上传监听端口
	void SendUploadPort();

	//停止查询服务器的广播
	void StopFindServerBroadcast();
	//停止心跳包定时
	void StopHeartbeatTimer();
	//开启抓屏线程
	void StartScreenCaptureThread( const void* msg );

	//服务器ip地址
	CString GetServerIP();

	//服务器发来的消息  
	//调整抓屏的质量
	void OnRcScreenQuality( const void* msg );
	//调整流畅度
	void OnRcScreenFluency( const void* msg );
	//停止抓屏
	void OnRcStopScreenCapture();
	//鼠标移动
	void OnRcMouseEvent( const void* msg );
	//键盘消息
	void OnRcKeybdEvent( const void* msg );
	//调整抓屏大小
	void OnRcScreenSize( const void* msg );
	//服务器通知开始接收服务器推送过来的桌面
	void OnRcPushServerDesktop();
	//停止接收服务器推送过来的桌面数据
	void StopRecvServerPushDektopThread();
	//取消推送桌面
	void OnRcCancelPushDesktop();
	//退出客户端
	void OnRcExitClient();
	//关机
	void OnRcShutdown();
	//重启
	void OnRcRestart();
	//注销
	void OnRcLoginOut();
	//字幕雨
	void OnRcLetterRain( const void* msg );
	//开启远程CMD
	BOOL OnRcStartCmd();

	//停止字母雨
	void StopLetterRain();
	
	//////////////////////////////////////////////////////////////////////////
	//获取CMD程序的路径
	//return 
	//		CMD程序的路径
	//		空串则获取失败
	CString GetCMDPath();

	//查找服务器广播的回复消息
	void OnRepeatFindServer( const CString& ip );
	//CMD命令
	void OnRcCMD( const void* msg );
	//服务器退出
	void OnRcServerExit();
	//获取制定路径里面的文件列表
	void OnRcGetFileList( const void* msg );
	void OnRcGetDriver();
	//删除文件
	void OnRcDeleteFile( const void* msg );
	void StopUploadThreads();

	//暂停发送屏幕图像
	void OnRcPauseScreen();
	//重新开始发送屏幕图像
	void OnRcResumeScreen();
};


