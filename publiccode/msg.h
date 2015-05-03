///////////////////////////////////////////////////////////////
//
// FileName	: msg.h 
// Creator	: 杨松
// Date		: 2013年3月6日, 23:16:42
// Comment	: 主要定义消息相关数据结构和宏等，共主控端和被控端使用
//
//////////////////////////////////////////////////////////////

#ifndef _RC_MSG_H_
#define _RC_MSG_H_

#ifndef BOOL
typedef int	BOOL;
typedef short	SHORT;
typedef unsigned short	WORD;
typedef int	INT;
typedef unsigned int	UINT;
#define NULL	0
#define TRUE	1
#define FALSE	0
#endif

//1字节对其，尽量减少传输的数据大小
#pragma pack(push , 1)

//消息头的定义
typedef struct _RCMSG{
	//消息类型  RCMSG_TYPE
	WORD type;
	//消息数据结构的大小
	WORD size;
}RCMSG , *PRCMSG;

typedef struct _RCMSG_BUF {
	RCMSG msgHead;
	char	buf[1];
}RCMSG_BUFF , *PRCMSG_BUFF;

//鼠标消息
typedef struct _MOUSE_EVENT_MSG{
	RCMSG msgHead;	//消息头
	UINT message;	//鼠标消息类型，如WM_MOVE等
	UINT param;		//Windows消息结构MSG::wParam 
	int x;			//当前鼠标在鼠标坐标系统的横轴位置
	int y;			//当前鼠标在鼠标坐标系统的纵轴位置
}MOUSE_EVENT_MSG, *PMOUSE_EVENT_MSG;

//键盘事件
typedef struct _KEYBD_EVENT_MSG{
	RCMSG msgHead;	//消息有
	UINT message;	//消息类型如WM_KEYUP
	UINT vKey;		//虚拟键代码
	BYTE keyState;//其他件的状态，主要用于处理组合键KS_CTRL、KS_SHIFT和KS_ALT的组合
}KEYBD_EVENT_MSG , *PKEYBD_EVENT_MSG;
#define KS_CTRL		0x0001		//ctrl键按下
#define KS_SHIFT	0x0002		//ctrl键按下
#define KS_ALT		0x0004		//alt键按下

//调整抓图的大小
typedef struct _RCMSG_SCREEN_SIZE{
	RCMSG msgHead;
	long x;
	long y;
}RCMSG_SCREEN_SIZE , *PRCMSG_SCREEN_SIZE;

//广播消息，查询服务器
typedef struct _RCMSG_BROADCAST_MSG{
	RCMSG	msgHead;
	USHORT	port;		//当前客户端监听的端口
	char	name[1];	//当前客户端的计算机名
}RCMSG_BROADCAST_MSG , *PRCMSG_BROADCAST_MSG;

//使用原有的对其方式
#pragma pack(pop)

//定一个基本的消息类型变量
#define DEF_RCMSG(_name , _type) 	RCMSG _name  = {0};			\
									_name.size = sizeof(RCMSG);	\
									_name.type = _type


//////////////////////////////////////////////////////////////////////////
//消息类型
enum RCMSG_TYPE{
	MT_MIN = 0,							//消息值的最小值，这不是消息类型

	MT_CONN_CLOSED_C,					//no param				连接已将断开
	MT_FIND_SERVER_BROADCAST_C,			//RCMSG_BROADCAST_MSG	客户端查询
	MT_RECV_ERROR,						//no param				接收数据时出错
	MT_HEARTBEAT_C,						//no param				客户端心跳包消息
	MT_SERVER_EXIT_S,					//no param				服务器下线
	MT_REPEAT_FIND_SERVER_S,			//no param				回复MT_HEARTBEAT_C(因为MT_HEARTBEAT_C是用广播发送的，
										//						客户端是不知道服务器地址的，所以还是要回复一下，便于
										//						客户端获取服务器的地址)
	MT_SCREEN_PORT_S,					//USHORT				服务端的screen端口，需回复MT_SCREEN_CAPTURE_START_C 
	MT_SCREEN_CAPTURE_START_C,			//no param				客户端已经开始抓屏了
	MT_SCREEN_DATA_SIZE_C,				//UINT					screen数据的大小，非控制消息		
	MT_SCREEN_QUALITY_S,				//LONG					抓屏的质量
	MT_SCREEN_FLUENCY_S,				//LONG					设置SCREEN的流畅度
	MT_STOP_SCREEN_CAPTURE_S,			//no param				停止抓屏
	MT_SCREEN_CTRL_S,					//char					screen控制
	MT_MOUSE_EVENT_S,					//MOUSE_EVENT_MSG		鼠标事件
	MT_KETBD_EVENT_S,					//KEYBD_EVENT_MSG		键盘事件
	MT_SCREEN_SIZE_S,					//RCMSG_SCREEN_SIZE		调整抓屏的大小
	MT_PUSH_SERVER_DESKTOP_S,			//no param				推送服务器桌面
	MT_PUSH_SERVER_DESKTOP_C,			//no param				确认推送服务器桌面（用于回复MT_PUSH_SERVER_DESKTOP_S）
	MT_CALCEL_PUSH_DESKTOP_S,			//no param				取消推送服务器桌面 
	MT_CALCEL_PUSH_DESKTOP_C,			//no param				确认取消桌面推送（回复MT_CALCEL_PUSH_DESKTOP_S）
	MT_EXIT_CLIENT_S,					//no param				退出客户端
	MT_SHUTDOWN_CLIENT_S,				//no param				客户端关机
	MT_SHUTDOWN_FALIED_C,				//no param				关机失败
	MT_RESTART_CLIENT_S,				//no param				客户端重启
	MT_RESTART_FALIED_C,				//no param				重启失败
	MT_LOGIN_OUT_CLIENT_S,				//no param				客户端注销
	MT_LOGIN_OUT_FALIED_C,				//no param				注销失败
	MT_LETTER_RAIN_S,					//BYTE					字母雨
	MT_START_CMD_S,						//no param				启动CMD
	MT_START_CMD_FAILED_C,				//no param				启动CMD失败
	MT_CMD_LINE_DATA_C,					//char* 不定长			命令行数据
	MT_HANDS_UP_C,						//no param				客户端举手
	MT_CMD_S,							//char* 不定长			一条CMD命令
	MT_CMD_FAILED_C,					//char* 不定长			CMD命令执行失败
	MT_GET_FILE_LIST_S,					//char* 不定长			获得文件列表数据
	MT_FILE_PATH_C,						//char* 不定长			一个文件或者文件的路径
	MT_GET_DRIVER_S,					//no param				获取磁盘驱动器信息
	MT_DELETE_FILE_S,					//char* 不定长			删除文件或者文件夹
	MT_DELETE_FILE_FAILED_C,			//char* 不定长			删除文件或者文件夹失败
	MT_DELETE_FILE_SUCCESS_C,			//char* 不定长			删除文件或者文件夹成功
	MT_LISTEN_DOWNLOAD_PORT_C,			//SHORT					监听下载端口
	MT_DOWNLOAD_FILE_PATH_S,			//char* 不定长			要下载的文件路径(上传下载线程)
	MT_DOWNLOAD_FILE_SIZE_S,			//DWORD					要下载的文件大小
	MT_DU_DOWNLOAD_S,					//BYTE					下载文件（上传下载线程）
	MT_PAUSE_SCREEN_S,					//no param				暂停发送屏幕图像
	MT_RESUME_SCREEN_S,					//no param				重新开始发送屏幕图像

	MT_MAX,								//消息值的最大值这不是有效的消息类型
};

#endif