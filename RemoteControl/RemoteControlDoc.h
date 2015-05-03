///////////////////////////////////////////////////////////////
//
// FileName	: RemoteControlDoc.h
// Creator	: 杨松
// Date		: 2013年2月27日, 20:10:26
// Comment	: 远控控制服务器文档视图结构中的文档类的声明
//
//////////////////////////////////////////////////////////////

#pragma once

//客户端列表列号
enum{
	CLIENT_LIST_ID = 0 ,
	CLIENT_LIST_STATU ,
	CLIENT_LIST_PUSH ,	
	CLIENT_LIST_NAME ,
	CLIENT_LIST_IP,
};

class CRemoteControlView;
class CCMDDlg;
class CFileManageDlg;
class CDownUploadDlg;

//////////////////////////////////////////////////////////////////////////
//客户端描述符
typedef struct ClientDescripter{
	int			mIndex;				//客户端索引
	CString		mName;				//被控端计算机名
	CString		mIP;				//被控端地址
	USHORT		mPort;				//主消息端口
	BOOL		mIsOnline;			//是否在线
	BOOL		mPreCheckOnlineState;//用于检查在线程状态的辅助标志
	BOOL		mIsPushed;			//推送状态
	sockaddr_in* mPushAddr;			//用于推送桌面的地址结构
	CCMDDlg*	mCMDDlg;			//命令行对话框
	CFileManageDlg* mDocManageDlg;	//文件管理对话框
	USHORT		mDownUploadListenPort;//下载监听端口

	//构造函数用于初始化
	ClientDescripter()
		: mIndex(LAST_INDEX++)
		, mPort(CLIENT_MSG_PORT)
		, mIsOnline(FALSE)
		, mPreCheckOnlineState(FALSE)
		, mIsPushed(FALSE)
		, mPushAddr(NULL)
		, mCMDDlg(NULL)
		, mDocManageDlg(NULL)
		, mDownUploadListenPort(0)
	{}
	static int LAST_INDEX;	//用于辅助给客户端分配索引
}*PClientDescripter;


class CRemoteControlDoc : public CDocument , public Dispatcher
{
protected: // 仅从序列化创建
	CRemoteControlDoc();
	DECLARE_DYNCREATE(CRemoteControlDoc)

private:
	
	//检查客户端线程上下文
	CheckClientThreadContext m_CheckClientThreadContext;

	//客户端链表描述链表
	typedef std::list<ClientDescripter*> ClientList;
	ClientList m_lstClient;

	//客户端链表控件
	CListCtrl*	m_pClientList;

	//消息中心
	MsgCenter* m_pMsgCenter;

	//文档视图的视图指针，在这里作为一个成员变量是为了操作方便
	CRemoteControlView* m_pView;

	//推送服务端桌面的线程上下文
	PushServerDesktopThreadContext m_PushServerDesktopThreadContext;

	//锁定推送桌面的大小的比例
	BOOL m_bLockScreenSizeRatio;

	//CMD窗口的配置数据，这里没有像推送的数
	//据一样单独写开是因为那些需要在多线程中使用
	CMD_CFG_V m_cmdValue;

	//配置文件路径
	char* m_czConfigfilePath;

	//CMD颜色配置
	COLORREF m_fontColor;
	// 背景颜色
	COLORREF m_bkColor;

	//下载文件对话框
	CDownUploadDlg* m_pDownUploadDlg;

// 操作
public:
	//////////////////////////////////////////////////////////////////////////
	//找到一个客户端,这是有主动查询客户线程调用的函数
	//param
	//		name	找到的主机的计算机名
	//		ip		找到的主机的IP地址
	void FoundAClient(const CString& name ,const CString& ip );
	
	//////////////////////////////////////////////////////////////////////////
	//停止检查客户端线程
	void StopCheckClientThread();
	
	//////////////////////////////////////////////////////////////////////////
	//开始检查客户端线程
	void StartChekClientThread();

	//////////////////////////////////////////////////////////////////////////
	//向指定客户端发送消息
	//param
	//		ip		客户端地址
	//		port	客户端接收消息的端口
	//		msg		消息数据
	void SendRcMessage(const CString& ip , USHORT port , void* msg);

	//////////////////////////////////////////////////////////////////////////
	//加载配置文件
	void LoadConfigFileData();

	//////////////////////////////////////////////////////////////////////////
	//创建CMD对话框，如果已经存在则显示/隐藏
	//param
	//		strIP	客户端ip
	//return 
	//		TRUE	操作成功
	//		FALSE	操作失败
	BOOL CreateCMDDlg(const CString& strIP);

	//检查客户端状态，由住框架定时器处理函数调用
	void CheckClientState();

	//刷新客户端列表控件中的数据
	void FlushClientCtrl();

	//停止推送服务器桌面
	void StopPushServerDesktopTHread();

	//删除一个客户端
	void StopMonitoring( const CString& strIP );

	//添加一个客户端
	void AddClient(  const CString& ip, USHORT port ,  const CString& strName );

	//推送相关配置改变
	void ChangePushCfgValue( PPUSH_CFG_V pushValue );
	
	//CMD相关配置改变
	void ChangeCMDCfgValue( PCMD_CFG_V cmdValue );
	
	//向每一个客户端发送消息
	void SendAllClient( PRCMSG rcMsg );
	
	//文件管理对话框
	BOOL CreateDocManageDlg( const CString& ip );
	
	//向制定客户端下载指定文件
	void DownloadFile( const CString& strPath, const CString& ip);
	
	//向指定客户端上传文件
	void UploadFile( const CString& strPath, const CString& m_strIP );

	//获得指定的客户端描述符
	PClientDescripter GetClientDescripter(const CString& ip);

//客户机发来的消息处理函数
private:
	//////////////////////////////////////////////////////////////////////////
	//查找服务器消息
	//param
	//		ip	客户机的地址
	//		port 客户端心跳包
	//		msg	消息数据
	void OnRcHeartbeat(const CString& ip ,USHORT port, const void* msg ) ;
	//一个客户端断开连接
	void OnRcClosed( const CString& ip ) ;
	//被控端已经抓屏了 
	void OnRcStartCapture(const CString& ip , USHORT port , const void* msg);
	//确认推送桌面
	void OnRcPushedServerDesktop( const CString& ip );
	//一个客户端已经确认了取消桌面的推送
	void OnRcCancelPushDesktop( const CString& ip );
	//客户端关机失败
	void OnRcShutdownFailed( const CString& ip );
	//重启失败
	void OnRcRestartFailed( const CString& ip );
	//注销失败
	void OnRcLoginOutFailed( const CString& ip );
	//开启CMD失败
	void OnRcStartCMDFailed( const CString& ip );
	//命令行回显数据
	void OnRcCMDLineData( const CString& ip , USHORT port , const void* msg );
	//客户端举手
	void OnRcHandsUp( const CString& ip );
	//客户端查询服务器的广播消息
	void OnRcFindServerBraodcast( const CString& ip, const void* msg );
	//一条cmd命令执行失败
	void OnRcCMDFailed( const CString& ip, const void* msg );
	//收到一个文件路径消息
	void OnRcFilePath( const CString& ip, const void* msg );
	//删除文件失败
	void OnRcDeleteFileFailed( const CString& ip, const void* msg );
	//删除文件成功
	void OnRcDeleteFileSuccess( const CString& ip, const void* msg );
	//监听下载文件端口
	void OnRcListenDownloadPort( const CString& ip, const void* msg );

// 重写
public:
	//视图文档结构的文档创建函数
	virtual BOOL OnNewDocument();

	//文档关闭函数，在此做一些数据清理工作
	virtual void OnCloseDocument();

	//////////////////////////////////////////////////////////////////////////
	//消息派送函数，派送的是客户端发送过来的消息
	//param
	//		msg		消息数据(RCMSG)
	//		ip		客户端ip地址
	//		port	客户端发消息的端口
	virtual void DispatchMsg(const void* msg , CString ip, UINT port);

// 实现
public:
	virtual ~CRemoteControlDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// 生成的消息映射函数
protected:
	//////////////////////////////////////////////////////////////////////////
	//此消息是m_pContentList的双击击事件的相应函数也需要由子类去实现
	afx_msg void OnClientListDbClicked(NMHDR* pNMHDR, LRESULT* pResult);

	//////////////////////////////////////////////////////////////////////////
	//在列表中右击了列表,需要显示快捷菜单
	afx_msg void OnClientListRClicked(NMHDR *pNMHDR, LRESULT *pResult);

	//响应推送服务器桌面的菜单消息
	afx_msg void OnPushServer();

	//响应选项菜单消息
	afx_msg void OnOption();

	//取消推送
	afx_msg void OnCancelPush();

	//显示cmd对话框
	afx_msg void OnCmd();
	
	//显示文件管理对话框
	afx_msg void OnFileManage();
	
	//下载文件
	afx_msg void OnDownload();
	
	//上传文件
	afx_msg void OnUpdateDownload(CCmdUI *pCmdUI);

	DECLARE_MESSAGE_MAP()
};
