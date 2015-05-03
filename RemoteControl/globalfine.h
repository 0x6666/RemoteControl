///////////////////////////////////////////////////////////////
//
// FileName	: globalfine.h 
// Creator	: 杨松
// Date		: 2013年2月28日, 10:48:36
// Comment	: 服务端的全局声明
//
//////////////////////////////////////////////////////////////

#ifndef _BASIC_DEFINE_H_
#define _BASIC_DEFINE_H_

//////////////////////////////////////////////////////////////////////////
//自定义Windows消息

//找到一个客户端
#define WM_FOUND_A_CLIENT		WM_USER + 1
//以获取一个本地的screen端口
#define WM_GET_A_SCREEN_PORT	WM_USER + 2
//创建screen套接字失败
#define WM_SCREEN_SOCKET_ERR	WM_USER + 3
// screen客户端连接已经断开
// #define WM_SCREEN_SOCK_UNCON	WM_USER + 4
//screen线程断开
#define WM_SCREEN_THREAD_EXIT	WM_USER + 5
//等待客户端抓屏线程超时
#define WM_WAIT_FOR_CAPTURE_TIMEOUT WM_USER + 6 
//客户端itemView双击事件 这个消息只能send，不可以post
#define WM_ITEM_VIEW_SBCLICK	WM_USER + 7 
//调整Screen质量
#define WM_CHANGE_SCREEN_AUALITY	WM_USER + 8
//调整Screen质量
#define WM_CHANGE_SCREEN_FLUENCY	WM_USER + 9
//调整获得屏幕的控制
#define WM_SCREEN_CTRL				WM_USER + 10
//发送消息
#define WM_SEND_MSG					WM_USER + 11
//停止监控
//#define WM_STOP_CTRL				WM_USER + 12
//停止监视一个客户端
#define WM_MONITORINT_CLIENT			WM_USER + 13
//打开CMD窗口
#define WM_CMD_CLICKED				WM_USER + 14
//托盘消息
#define WM_NOTIFYION_MSG_S			WM_USER + 15
//通知主窗口客户端数量以及在数量
//param
//		wParam	客户端数量
//		lParam	在线数量
#define WM_CLIENT_COUNT				WM_USER + 16

//全屏显示
//wparam	当CRemoteControlView接收此消息是，这是CString*
//			当ClientItemView和CMainWnd接收此消息时，这是BOOL，（全屏/取消全屏）
#define WM_FULLS_CREEN				WM_USER + 17
//文件管理
#define WM_DOC_MANAGE_CLICKED		WM_USER + 18

//下载文件时套接字错误
//param
//wParam	PDowmloadDescripter	下载描述符地址
#define WM_DL_ERROE			WM_USER + 19

//下载文件时的下载进度
//param	
//wParam	CString* ip
//lParam	DWORD	已经下载了的数据
#define WM_DL_PROGRESS				WM_USER + 20

//下载线程退出
//param
//wParam	CString* ip
#define WM_DL_THREAD_EXIT			WM_USER + 21

//要下载的文件大小
//param
//wParam	CString* ip
//lParam	DWORD	文件大小
#define WM_DL_FILE_SIZE				WM_USER + 22

//通知状态栏改变推送数量显示
//param
//	wParam	int 已推送的客户端数量
#define WM_PUSHED_COUNT				WM_USER + 23

//通知状态栏体是一个消息
//param
//	wParam (CString*) 要提示的消息
#define WM_MESSAGE					WM_USER + 24

//获取鼠标位置
#define GET_MOUSE_POS_TIMER		1001
//检查客户端状态定时器
#define CHECK_CLIENT_STATE_TIME	1002

//配置文件名
#define CONFIG_FILE_NAME	"rcconfigfile.cfg"

//{{配置文件中的键

//推送桌面时抓屏的长和宽
#define CK_PUSH_SD_W		"Push_DesktopW"
#define CK_PUSH_SD_H		"Push_DesktopH"
#define CK_PUSH_SD_QUALITY	"Push_DesktopQuality"
#define CK_PUSH_SD_FREQUNCY	"Push_DesktopFrequency"
#define CK_PUSH_SD_LOCK_RATIO	"Push_LockScreenSizeRatio"

//CMD设置
#define CK_CMD_FONT_COLOR	"CMD_FontColor"
#define CK_CMD_BG_COLOR		"CMD_BackgroundColor"
#define CK_CMD_FONT_SIZE	"CMD_FontSize"
#define CK_CMD_FONT_CHAR_SET	"CMD_FontCharset"
#define CK_CMD_FONT_FACENAME	"CMD_FontFaceName"

//}}配置文件中的键
//推送的配置文件值集合
typedef struct _PUSH_CFG_V{
	LONG nPush_DesktopW;
	LONG nPush_DesktopH;
	LONG nPush_DesktopQuality;
	LONG nPush_DesktopFrequency;
	BOOL bPush_LockScreenSizeRatio;
}PUSH_CFG_V , *PPUSH_CFG_V;

//CMD的配置文件值集合
typedef struct _CMD_CFG_V{
	LONG nCMD_FontSize;
	BYTE nCMD_FontCharset;
	DWORD nCMD_FontColor;
	DWORD nCMD_BackgroundColor;
	char czCMD_FontFaceName[32];
}CMD_CFG_V , *PCMD_CFG_V;


//接收并显示被控端图像的线程上下文
typedef struct _PrintScreenThreadContext{
	CWnd* pWnd;		//接收消息的窗口
	//线程运行状态 0->停止，1->运行
	volatile long lThreadState;
	//接收远程桌面的线程句柄
	HANDLE	hThread;
	//线程同步需要使用的事件句柄
	HANDLE	hThreadEvent;
	//CString	strIP;//被控端IP
	SOCKET	sSocket;//接收数据的socket

}PrintScreenThreadContext,*PPrintScreenThreadContext;

//接收发过来的图像，并显示出来
DWORD WINAPI PrintScreenThread(LPVOID wParam);


//推送服务器桌面线程上下文
typedef struct _PushServerDesktopThreadContext
{
	//需要推送的客户端地址链表
	typedef std::list<sockaddr_in*> PushedClientList;

	//推送服务器桌面线程
	volatile LONG lThreadState;
	//线程句柄
	HANDLE		hThread;

	//推送服务器桌面时的抓屏大小
	volatile LONG lDesktopW;
	volatile LONG lDesktopH;

	//推送服务器桌面时的抓屏质量
	volatile LONG lDesktopQuality;
	//推送服务器桌面时的流畅度
	volatile LONG lDesktopFrequency;
	
	//将要将要将要推送的客户端链表
	PushedClientList lstPushedClient;
	//将要将要将要推送的客户端链表互斥量
	HANDLE hPushedClientListMutex;
}PushServerDesktopThreadContext , *PPushServerDesktopThreadContext;

//推送服务器桌面线程
DWORD WINAPI PushSeverDesktopThread(LPVOID wParam);

//检查客户端线程上下文
typedef struct _CheckClientThreadContext{
	CDocument*	pDoc;		//文档指针
	HANDLE		hThread;	//线程句柄
	volatile LONG iThreadState;//线程运行状态
}CheckClientThreadContext , *PCheckClientThreadContext;
//////////////////////////////////////////////////////////////////////////
//用于检查局域网中的主机的
DWORD WINAPI CheckClientThread(LPVOID param);

//获得某个键的按下状态
#define KEY_DOWN(vk) (0x8000&GetKeyState(vk))

#endif