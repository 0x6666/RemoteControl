//////////////////////////////////////////////////////////////////////////
//基础定义

#ifndef _BASIC_DEFINE_H_
#define _BASIC_DEFINE_H_

//广播查询服务端定时器
#define BROADCAST_TIMER		1001
//也是发送心跳包的定时器
#define HEARTBEAT_TIMER		1002
//字幕雨定时器
#define LETTER_RAIN_TIMER	1003

//套接字操作出错
#define WM_SCREEN_SOCKET_ERR		WM_USER + 1
//#define WM_CLOSE_BROADCAST_TIMER	WM_USER + 2 
//托盘消息
#define WM_NOTIFYION_MSG_C			WM_USER + 3



//下载文件时出错
//param
//wParam	CString*	ip
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

//////////////////////////////////////////////////////////////////////////
//创建一个套接字
SOCKET GetSocket(CString ip , USHORT port);

//抓屏现成上下文
typedef struct _CaptureScreenThreadContext
{
	CString strServerIP;		//服务端IP地址
	USHORT	uScreenPort;		//接收Screen图像的网络端
	HANDLE	hEvent;				//同步事件
	HANDLE	hThread;			//线程句柄
	//抓屏线程运行状态0->停止 1->运行 2->暂停 可以通过在线程外部调整此值来调整线程的运行状态
	volatile long	lThreadState;
	SOCKET	sSocket;			//用来发送图像数据的socket
	CWnd*	pWnd;				//用来接收消息的窗口
	volatile long	lScreenW;	//抓屏的宽度
	volatile long	lScreenH;	//抓屏的高度
	volatile long	lFluency;	//抓屏的流畅度
	volatile long	lScreenQuality;//抓取的屏幕质量  [0,100]

}CaptureScreenThreadContext ,*PCaptureScreenThreadContext;


//CMD线程上下文
typedef struct _CMDContext
{
	//cmd匿名管道
	//读
	HANDLE hRead;
	//写
	HANDLE hWrite;
	//cmd子进程句柄
	HANDLE hCmd;
	//读取cmd回显数据的线程 
	HANDLE hReadCMDThread;
	//线程运行状态
	volatile LONG lReadCMDThreadState;
	//主控端IP
	CString strIP;
}CMDContext ,*PCMDContext;

//抓屏并发送屏幕数据
DWORD WINAPI CaptureScreenThread(LPVOID wParam);

//接收桌面推送线程上下文 
typedef struct _RecvPushedDesktopThreadContext
{
	HANDLE hThread;
	volatile LONG lThreadState;
	CWnd* pFrame;	//控制窗口
	CWnd* pScreenView;//需要绘制图像的窗口
}RecvPushedDesktopThreadContext , *PRecvPushedDesktopThreadContext;
//接收服务器推送过来的服务器桌面线程
DWORD WINAPI RecvServerPushedDesktopThread(LPVOID wParam);

//读取CMD的输出
DWORD WINAPI ReadCMDThread(LPVOID wParam);
//枚举文件夹里面的文件
DWORD WINAPI EnumFileThread(LPVOID param);

//删除一个文件或者目录
BOOL DeleteFileOrDir(CString path);


//检查鼠标事件中的ctrl和shift按键状态
#define MOUSE_TEST_CTRL_SHIFT_D(v)	if((v)& MK_CONTROL) \
										::keybd_event(VK_CONTROL, 0 , 0 , 0);\
									if((v)& MK_SHIFT)\
										::keybd_event(VK_SHIFT , 0 , 0 , 0)
#define MOUSE_TEST_CTRL_SHIFT_U(v)	if((v)& MK_SHIFT)\
										::keybd_event(VK_SHIFT , 0 , KEYEVENTF_KEYUP , 0);\
									if((v)& MK_CONTROL)\
										::keybd_event(VK_CONTROL , 0 , KEYEVENTF_KEYUP , 0)

enum SHUTDOWN{
	SD_SHUTDOWN,	//关机
	SD_RESTART,		//重启
	SD_LOGIN_OUT,	//注销
};
//////////////////////////////////////////////////////////////////////////
//关机
//param
//		sd	操作类型
BOOL Shutdown(SHUTDOWN sd);


#endif