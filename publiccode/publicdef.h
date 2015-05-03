///////////////////////////////////////////////////////////////
//
// FileName	: publicdef.h 
// Creator	: 杨松
// Date		: 2013年3月2日, 22:34:29
// Comment	: 服务端和客户端公共的定义
//
//////////////////////////////////////////////////////////////

#ifndef _PUBLIC_DEFINE_H_
#define _PUBLIC_DEFINE_H_

//////////////////////////////////////////////////////////////////////////
//各种定义的网络端口

//服务器和客户端小主消息端口
#define SERVER_MSG_PORT			19509
#define CLIENT_MSG_PORT			19510

//#define CLIENT_BROADCAST_PORT	19511
#define CLIENT_SCREEN_PORT		19512

//服务器推送桌面的端口
#define SERVER_PUSH_DESKTOP_PORT	19513	


//////////////////////////////////////////////////////////////////////////
//自定义窗口消息

//已经获取了上传的监听端口
//param
//wParam	USHORT	端口号
#define WM_UPLOAD_LISTEN_PORT  WM_USER + 24

//通知窗口创建上传文件线程
//param
//wParam	socket	已经连接了的套接字
#define WM_CREATE_UPLOAD_THREAD		WM_USER + 25


//////////////////////////////////////////////////////////////////////////
//字符串替代标识
#define STR_RP	_T("{{_STR_}}")

//一些常用文件夹的定义
//桌面
#define DIR_DESKTOP	_T("<<DESKTOP>>")
//我的文档
#define DIR_MY_DOC	_T("<<MY_DOCUMENT>>")
//我的电脑
#define DIR_COMPUTER	_T("<<COMPUTER>>")


//服务端每4秒检查一次客户端在线状态，保险起见客户端应该是2秒发送一次心跳包
#define HEARTBEAT_PACKETS_TIME	(4*1000)


//如果没没有做调整，udp的数据包缓存大小(算上包头)64K（65536B）
//所以在这里使用65525的接收数据缓存，保证一次可以可一读取完一个upd内部缓存中的数据，避免数据丢失
#define RECV_BUF_SIZE 65536
//而将发送的最大数据设置为60K（15个页面），而不能是64k。
#define SEND_BUF_SIZE 61440

//默认的抓屏质量
#define DEFAULT_SCREEN_QUALITY 30

//////////////////////////////////////////////////////////////////////////
//流畅度
enum FrequencyType{
	FT_LEVEL_1  = 1000,	//每秒1帧
	FT_LEVEL_2  = 500,	//每秒2帧
	FT_LEVEL_3  = 167,	//每秒6帧
	FT_LEVEL_4	= 83,	//每秒12帧
	FT_LEVEL_5	= 56,	//每秒18帧
	FT_LEVEL_6	= 42,	//每秒24帧
	FT_LEVEL_7	= 33,	//每秒30帧
	FT_MIN_LEVEL = FT_LEVEL_1,
	FT_MAX_LEVEL = FT_LEVEL_7,
	DEFAULT_FREQUENCY_LEVEL = FT_LEVEL_5,
};

//服务器托盘图标标识
#define ICON_INDEX_S		1234
//客户端托盘图标
#define ICON_INDEX_C		1235



//////////////////////////////////////////////////////////////////////////
//将索引(1、2、...)转换成枚举FrequencyType值
//param
//		v	索引
//return 
//		枚举值
FrequencyType IndexToFrequency(int v);

//////////////////////////////////////////////////////////////////////////
//将枚举FrequencyType值转换成索引(1、2、...)
//param
//		v	枚举值
//return 
//		索引
int FrequencyToIndex(FrequencyType v);

//////////////////////////////////////////////////////////////////////////
//获取枚举值的具体含义
//param
//		v	枚举值
//return 
//		枚举值v对应的含义
const char* FrequencyToText(FrequencyType v);


//////////////////////////////////////////////////////////////////////////
//获取制定图像编码器
//param
//		format  图像格式对应的 mine
//		pClsid	编码器对应的clsid
//return 
//	-1	操作失败
//	其他则成功
int GetEncoderClsid(IN const WCHAR* format, OUT CLSID* pClsid);

//////////////////////////////////////////////////////////////////////////
//抓取桌面
//param
//		nfW	抓取桌面保存的图像的宽度
//		nfH 抓取桌面保存的图像的高度
//		imgData 抓屏输出的图像数据
//		encoderClsid 抓屏后保存的图像的格式
//		encoderParameters 保存图像需要使用的图像编码器参数
void CaptureDesktop(long nfW, long nfH,
					HDC	hDesktop//这个参数本来可以放到函数里面自己获取的，但是获取的效率有点低
					, IN OUT AutoSizeStream* imgData,
					const CLSID& encoderClsid,
					const  EncoderParameters& encoderParameters );

//////////////////////////////////////////////////////////////////////////
//初始化图像编码器参数
//param
//		encoderParameters 需要初始化的编码器参数
//		quality			  保存图像需要使用的质量参数
void InitEncoderParameter( EncoderParameters &encoderParameters, ULONG& quality );

//////////////////////////////////////////////////////////////////////////
//通过ip的是获取主机名
//param
//		strIP	要获取主机名的IP地址
//return 空串则失败
CString GetHostNameByAddr(const CString& strIP);

//////////////////////////////////////////////////////////////////////////
//退出线程
//param
//		hThread	要退出的线程句柄
//		time	等待线程的截止时间(毫秒)，如果超过时间还没退出的话就强行结束线程
//return 
//		TRUE	不是强行结束的
//		FALSE	线程是强行结束的
BOOL WaiteExitThread( HANDLE hThread , DWORD time);

//将一个路径替换成以DIR_DESKTOP或者DIR_MY_DOC开头的路径
//param
//		strPath	是要还原的路径
//		type	1替换成DIR_DESKTOP，2 DIR_MY_DOC
CString ReplacePath(CString path , UINT type);

//还原一个以DIR_DESKTOP或者DIR_MY_DOC开头的目录
//param
//		strPath	是要还原的路径
//		type(输出)	0不需要还原，1原始目录是DIR_DESKTOP，2 DIR_MY_DOC
CString RevertPath(CString strPath , UINT& type);

//将一个以DIR_DESKTOP或者DIR_MY_DOC开头的目录换成“桌面”或者“我的文档”
//param
//		strPath	是要还原的路径
//		type(输出)	0不需要还原，1原始目录是DIR_DESKTOP，2 DIR_MY_DOC
CString TranslatePath(CString strPath , UINT& type);

//从一个路径中获得其文件名
//param
//		path	路径
//return 文件名或者空串
CString GetFileNameFromPath(const CString& path);

//将大小转换成字符串大小（如1024=》1K）
CString GetSizeToString(DWORD len);

//服务端上传下载文件
DWORD WINAPI DownUploadFileThread_S(LPVOID param);

//上传文件 
DWORD WINAPI DownUploadFileThread_C(LPVOID param);

//上传监听线程
DWORD WINAPI UploadListenerThread(LPVOID param);

//下载文件的结构
typedef struct _DowmUploadContext{
	int		nClientID;		//客户端ID
	CString clientIP;		//客户端IP
	CString clientPath;		//需要要下载的文件
	CString curPath;		//下载后保存的路径
	CWnd*	pWnd;	//下载对话框
	USHORT  uDownUploadListenPort;//下载监听端口
	volatile LONG lThreadState;//下载状态
	DWORD	fileLen;		//文件长度
	DWORD	dlSize;			//已经下载了大小
	HANDLE	dlThread;		//下载线程句柄
	BOOL	isDownload;		//是否为下载

	_DowmUploadContext()
		: pWnd(NULL)
		, nClientID(0)
		, uDownUploadListenPort(0)
		, lThreadState(0)
		, fileLen(0)
		, dlSize(0)
		, dlThread(NULL)
		, isDownload(TRUE)
	{}
}DowmUploadContext , *PDowmUploadContext;

//////////////////////////////////////////////////////////////////////////
//上传文件描述符
typedef struct _UploadDescripter{
	volatile long lThreadState;	//线程状态
	SOCKET	sSocket;			//套接字
	HANDLE	hThread;			//线程句柄
	CWnd*	pWnd;				//用于接收消息的窗口
	CString	filePah;			//要下载的文件路径
	
	_UploadDescripter()
	: lThreadState(0)
	, sSocket(0)
	, hThread(NULL)
	, pWnd(NULL)
	{}
}UploadDescripter , *PUploadDescripter;

//下载监听描述符
typedef struct  _DownloadListrenDescripter
{
	USHORT	uPort;	//监听端口
	CWnd*	pWnd;	//消息处理窗口
	volatile long	lThreadState;//线程状态
	HANDLE	hThread;//线程

	_DownloadListrenDescripter()
	: uPort(0)
	, pWnd(NULL)
	, lThreadState(0)
	, hThread(NULL)
	{}
}DownloadListrenDescripter , *PDownloadListrenDescripter;

//下载一个文件 DownUploadFileThread_S的一个分支
void DownloadFile_S( SOCKET fileSock, PDowmUploadContext ddes );

//上传一个文件 DownUploadFileThread_S的一个分支
void UploadFile_S( SOCKET fileSock, PDowmUploadContext ddes ) ;

//DownUploadFileThread_C的一个分支
void UploadFile_C( PUploadDescripter udes );

//DownUploadFileThread_C的一个分支
void DownloadFile_C(PUploadDescripter udes);

//使用GDI+绘图
BOOL DrawPictureWithGDIPP( AutoSizeStream* screenData, HDC hDC, CRect &viewRect );

//使用Ole绘图
BOOL DrawPictureWithOle( AutoSizeStream* screenData, HDC hDC, CRect &viewRect );

//显示器的大小
extern int SCREEN_SIZE_W;
extern int SCREEN_SIZE_H;

#endif