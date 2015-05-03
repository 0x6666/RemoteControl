///////////////////////////////////////////////////////////////
//
// FileName	: publicdef.cpp 
// Creator	: 杨松
// Date		: 2013年3月22日, 23:42:22
// Comment	: 服务端和客户端公共的定义
//
//////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "publicdef.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//显示器的尺寸
int SCREEN_SIZE_W = ::GetSystemMetrics(SM_CXSCREEN);
int SCREEN_SIZE_H = ::GetSystemMetrics(SM_CYSCREEN);

FrequencyType IndexToFrequency(int v)
{
	switch(v)
	{
	case 1:
		return FT_LEVEL_1;
	case 2:
		return FT_LEVEL_2;
	case 3:
		return FT_LEVEL_3;
	case 4:
		return FT_LEVEL_4;
	case 5:
		return FT_LEVEL_5;
	case 6:
		return FT_LEVEL_6;
	case 7:
		return FT_LEVEL_7;
	default:
		ASSERT(FALSE);
	}
	return DEFAULT_FREQUENCY_LEVEL;
}

int FrequencyToIndex(FrequencyType v)
{
	switch(v)
	{
	case FT_LEVEL_1:
		return 1;
	case FT_LEVEL_2:
		return 2;
	case FT_LEVEL_3:
		return 3;
	case FT_LEVEL_4:
		return 4;
	case FT_LEVEL_5:
		return 5;
	case FT_LEVEL_6:
		return 6;
	case FT_LEVEL_7:
		return 7;
	default:
		ASSERT(FALSE);
	}
	return 6;
}

const char* FrequencyToText( FrequencyType v )
{
	switch(v)
	{
	case FT_LEVEL_1:
		return "每秒1帧";
	case FT_LEVEL_2:
		return "每秒2帧";
	case FT_LEVEL_3:
		return "每秒6帧";
	case FT_LEVEL_4:
		return "每秒12帧";
	case FT_LEVEL_5:
		return "每秒18帧";
	case FT_LEVEL_6:
		return "每秒24帧";
	case FT_LEVEL_7:
		return "每秒30帧";
	default:
		ASSERT(FALSE);
	}
	return NULL;
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT num= 0;
	UINT size= 0;

	//图像编码信息
	ImageCodecInfo* pImageCodecInfo= NULL;

	//获得图像编码器大小和数量
	GetImageEncodersSize(&num, &size);
	if(size== 0)
	{
		return -1;
	}
	pImageCodecInfo= (ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo== NULL)
	{
		return -1;
	}

	//获得图像编码器序列
	GetImageEncoders(num, size, pImageCodecInfo);

	for(UINT j = 0 ; j < num ; ++j )
	{
		if(wcscmp(pImageCodecInfo[j].MimeType, format)== 0)
		{//找到指定的图像编码器
			*pClsid= pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;
		}
	}

	//貌似没有指定的图像编码器
	free(pImageCodecInfo);
	return -1;
}

void CaptureDesktop(long nfW, long nfH ,//抓屏后保存图像的长和宽
					HDC	hDesktop		//桌面设备上下文句柄
					,IN OUT AutoSizeStream* imgData,//抓屏后的图像数据缓存
					const CLSID& encoderClsid,	//抓屏保存的图像编码器
					const EncoderParameters& encoderParameters)//编码器参数，用于调整图像质量
{
	//获得显示器的尺寸
	LARGE_INTEGER move = {0};
	//创建于桌面相容的DC
	HDC hCaptionDC = ::CreateCompatibleDC(hDesktop);
	//创建桌面相容位图
	HBITMAP hDeskBit = ::CreateCompatibleBitmap(hDesktop , nfW , nfH);
	::SelectObject(hCaptionDC , hDeskBit);
	
	//设置图像缩放模式，如果这两句就执行StretchBlt的话去掉的像素边缘不会做一个平和的处理,
	//图像基本会有些混乱，图像质量极差
	::SetStretchBltMode(hCaptionDC , HALFTONE);  
	::SetBrushOrgEx(hCaptionDC , 0 , 0 , NULL );
	
	//抓取的图片并进行放缩
	::StretchBlt( hCaptionDC , 0 , 0 , nfW , nfH , hDesktop , 0 , 0 , SCREEN_SIZE_W , SCREEN_SIZE_H , SRCCOPY );

	{//获取抓起的图片数据
	Bitmap bmp( hDeskBit , NULL);
	bmp.Save(imgData , &encoderClsid , &encoderParameters );
	imgData->Seek(move , STREAM_SEEK_SET , NULL );
	}

	//清理相关对象
	::DeleteObject(hCaptionDC);
	::DeleteObject(hDeskBit);
}

void InitEncoderParameter( EncoderParameters &encoderParameters, ULONG& quality )
{
	//初始化煮面图片调整参数
	encoderParameters.Count = 1;
	encoderParameters.Parameter[0].Guid = EncoderQuality;
	encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
	encoderParameters.Parameter[0].NumberOfValues = 1;
	encoderParameters.Parameter[0].Value = &quality;
}

CString GetHostNameByAddr( const CString& strIP )
{
	char buf[20] = {0};
	::setlocale(LC_ALL , "");
	::wcstombs(buf, strIP , 20);
	::setlocale(LC_ALL , "C");

	unsigned long addr = inet_addr(buf);
	struct hostent *host = gethostbyaddr((char*)&addr ,4 ,AF_INET);
	if (NULL == host)
		return CString();
	else
	{
		CString name;
		name += host->h_name;
		return name;
	}
}

BOOL WaiteExitThread( HANDLE hThread , DWORD time)
{
	if(WAIT_TIMEOUT == WaitForSingleObject(hThread , time))
	{//等待线程退出
		DWORD dwExitCode = 0;
		GetExitCodeThread(hThread , &dwExitCode);
		if (STILL_ACTIVE == dwExitCode)
		{
			TerminateThread(hThread , dwExitCode);
			return FALSE;
		}
	}
	return TRUE;
}

CString RevertPath( CString strPath , UINT& type)
{
	type = 0;

	CString strPreDir;
	if (strPath.IsEmpty())
		ASSERT(FALSE);
	else if (strPath.Left(_tcslen(DIR_DESKTOP)) == DIR_DESKTOP)
	{//桌面
		type = 1;
		TCHAR MyDir[MAX_PATH];
		SHGetSpecialFolderPath(NULL , MyDir , CSIDL_DESKTOP , 0 );
		strPreDir += MyDir;
		if (strPreDir.Right(1) == _T('\\'))
			strPreDir = strPreDir.Mid(0 , strPreDir.GetLength()-1); 
		strPath.Replace(DIR_DESKTOP , strPreDir);
	}else if(strPath.Left(_tcslen(DIR_MY_DOC)) == DIR_MY_DOC)
	{//我的文档
		type = 2;
		TCHAR MyDir[MAX_PATH];
		SHGetSpecialFolderPath(NULL , MyDir , CSIDL_PERSONAL , 0 );
		strPreDir += MyDir;
		if (strPreDir.Right(1) == _T('\\'))
			strPreDir = strPreDir.Mid(0 , strPreDir.GetLength()-1);
		strPath.Replace(DIR_MY_DOC , strPreDir);
	}
	return strPath;
}

CString TranslatePath(CString strPath , UINT& type)
{
	type = 0;

	CString strPreDir;
	if (strPath.IsEmpty())
		ASSERT(FALSE);
	else if (strPath.Left(_tcslen(DIR_DESKTOP)) == DIR_DESKTOP)
	{//桌面
		type = 1;
		strPath.Replace(DIR_DESKTOP , _T("桌面"));
	}else if(strPath.Left(_tcslen(DIR_MY_DOC)) == DIR_MY_DOC)
	{//我的文档
		type = 2;
		strPath.Replace(DIR_MY_DOC , _T("我的文档"));
	}
	return strPath;
}

CString ReplacePath( CString path , UINT type )
{
	TCHAR MyDir[MAX_PATH];
	CString strPreDir; 
	if (type == 1 )
	{//桌面
		SHGetSpecialFolderPath(NULL , MyDir , CSIDL_DESKTOP , 0 );
		strPreDir += MyDir;
		if (strPreDir.Right(1) == _T('\\'))
			strPreDir = strPreDir.Mid(0 , strPreDir.GetLength()-1); 
		path.Replace( strPreDir , DIR_DESKTOP);
	}else if(type == 2)
	{//我的文档
		TCHAR MyDir[MAX_PATH];
		SHGetSpecialFolderPath(NULL , MyDir , CSIDL_PERSONAL , 0 );
		strPreDir += MyDir;
		if (strPreDir.Right(1) == _T('\\'))
			strPreDir = strPreDir.Mid(0 , strPreDir.GetLength()-1);
		path.Replace( strPreDir , DIR_MY_DOC);
	}
	return path;
}

CString GetFileNameFromPath( const CString& path )
{
	//空串
	if(path.IsEmpty())
		return CString();
	
	//没有路径 只是一个文件名
	int index = path.ReverseFind(_T('\\'));
	if (index == -1)
		return path;
	
	CString name = path.Mid(index + 1);
	if (name.IsEmpty())
	{//路径是一个文件夹
		return CString();
	}
	return name;
}

DWORD WINAPI DownUploadFileThread_S( LPVOID param )
{
	PDowmUploadContext ddes = PDowmUploadContext(param);

	char cIP[20] = {0};
	::wcstombs(cIP , ddes->clientIP , ddes->clientIP.GetLength() );
		
	SOCKET	fileSock;
	sockaddr_in  addr;
	//int		len = sizeof(addr);
	
	{//创建套接字
	fileSock = ::socket(AF_INET , SOCK_STREAM , 0);
	if(fileSock == INVALID_SOCKET)
	{//创建套接字失败
		ddes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(ddes) , 0);
		goto exit_updownload; 
	}
	}//创建套接字

	{//连接客户端
	::memset(&addr,0,sizeof(addr));
	addr.sin_family     = AF_INET;//地址族
	addr.sin_addr.s_addr= ::inet_addr(cIP);//ip地址
	addr.sin_port       = ::htons(ddes->uDownUploadListenPort);//端口

	if(::connect(fileSock , (sockaddr*)&addr , sizeof(addr) )== SOCKET_ERROR)
	{
		::closesocket(fileSock);
		ddes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(ddes) , 0);
		goto exit_updownload; 
	}
	}//配置远程地址信息


	{//先发送是否是下载
	RCMSG_BUFF rcMsg = {0};
	rcMsg.msgHead.size = sizeof(RCMSG_BUFF);
	rcMsg.msgHead.type = MT_DU_DOWNLOAD_S;
	rcMsg.buf[0] = ddes->isDownload ? 1 : 0;
	if(::send(fileSock , (char*)&rcMsg , rcMsg.msgHead.size , 0 ) == SOCKET_ERROR)
	{//接受文件长度时 套接字错误
		::closesocket(fileSock);
		ddes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(ddes) , 0);
		goto exit_updownload; 
	}
	}

	if (ddes->isDownload)
	{//下载文件
		DownloadFile_S(fileSock , ddes);
	}else
	{//上传文件
		UploadFile_S(fileSock , ddes);
	}

exit_updownload:
	ddes->pWnd->SendMessage(WM_DL_THREAD_EXIT , (WPARAM)(ddes) , 0 );
	return 1;
}

void DownloadFile_S( SOCKET fileSock, PDowmUploadContext ddes ) 
{
	UINT	nFileLen  = 0;//文件的长度
	UINT	nRcvTotal = 0;//总共发送了的数据
	UINT	nRcvOne   = 0;//一次发送的数据

	//要下载的文件保存路径(临时文件名)
	CString  strFile = (ddes->curPath.Trim()+_T(".temp"));

	HANDLE hFile = ::CreateFile(strFile , GENERIC_WRITE , NULL , NULL ,
		CREATE_ALWAYS , FILE_ATTRIBUTE_NORMAL , NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{//创建文件失败
		ddes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(ddes) , 0);
		return ; 
	}

	{//发送文件名
	int size = 0;
	char* pMsgBuf = NULL;
	PRCMSG_BUFF pMsg = NULL;
	{//字符编码转换
		::setlocale(LC_ALL , "");
		size = ::wcstombs(NULL , ddes->clientPath , 0);
		pMsgBuf = new char[sizeof(RCMSG_BUFF) - 1 + size + 2];
		pMsg = PRCMSG_BUFF(pMsgBuf);
		pMsg->msgHead.size = sizeof(RCMSG_BUFF) - 1 + size + 2;
		pMsg->msgHead.type = MT_DOWNLOAD_FILE_PATH_S;
		::wcstombs(pMsg->buf , ddes->clientPath , size);
		::setlocale(LC_ALL , "C");
		pMsg->buf[size] = 0;
	}//字符编码转换

	if(::send(fileSock , (char*)pMsg , pMsg->msgHead.size , 0 )==SOCKET_ERROR)
	{//接受文件长度时 套接字错误
		::CloseHandle(hFile);	
		::closesocket(fileSock);
		ddes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(ddes) , 0);
		return ; 
	}
	delete[] pMsgBuf;
	}//发送文件名


	//数据缓存
	char*	buf = new char[RECV_BUF_SIZE];

	{//接收文件大小
	if(((nFileLen = ::recv(fileSock , (char*)buf , (sizeof(RCMSG_BUFF)-1 + sizeof(DWORD)) , 0) )== SOCKET_ERROR)
		|| (nFileLen != PRCMSG_BUFF(buf)->msgHead.size )
		|| (PRCMSG_BUFF(buf)->msgHead.type != MT_DOWNLOAD_FILE_SIZE_S))
	{//接受文件长度时 套接字错误
		::CloseHandle(hFile);	
		::closesocket(fileSock);
		ddes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(ddes) , 0);
		delete[] buf;
		return ; 
	}
	nFileLen = *(DWORD*)(PRCMSG_BUFF(buf)->buf);
	ddes->pWnd->SendMessage(WM_DL_FILE_SIZE , (WPARAM)(ddes)  , (WPARAM)nFileLen);
	}//接收文件大小


	while((nRcvTotal < nFileLen) && (ddes->lThreadState))
	{
		::memset(buf , 0 , RECV_BUF_SIZE);	
		nRcvOne = ::recv( fileSock , buf , RECV_BUF_SIZE , 0);
		if(SOCKET_ERROR == nRcvOne) 
		{//接受数据时  套接字错误
			::CloseHandle(hFile);	
			::closesocket(fileSock);
			ddes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(ddes) , 0);
			delete[] buf;
			return ;
		} 

		DWORD nWriten=0;
		if(!::WriteFile(hFile , buf , nRcvOne , &nWriten , NULL))
		{//写失败
			::CloseHandle(hFile);	
			::closesocket(fileSock);
			ddes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(ddes) , 0);
			delete[] buf;
			return ;
		}
		nRcvTotal += nRcvOne;
		//通知框架下载进度
		ddes->pWnd->SendMessage(WM_DL_PROGRESS , (WPARAM)(ddes) , (LPARAM)nRcvTotal);
	}

	::CloseHandle(hFile);	
	::closesocket(fileSock);
	if (NULL != buf)
	{
		delete[] buf;
	}
	

	//重命名文件到目标文件
	MoveFile(strFile , ddes->curPath);
	DeleteFile(strFile);

	return ;
}

void UploadFile_S( SOCKET fileSock, PDowmUploadContext ddes ) 
{
	//要上传的文件
	HANDLE hFile = ::CreateFile(ddes->curPath , GENERIC_READ , NULL , NULL ,
		OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{//打开文件失败
		ddes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(ddes) , 0);
		return ; 
	}
	//文件的长度
	DWORD	nFileLen  = GetFileSize(hFile , 0);

	{//发送文件名
	int size = 0;
	char* pMsgBuf = NULL;
	PRCMSG_BUFF pMsg = NULL;
	{//字符编码转换
	::setlocale(LC_ALL , "");
	size = ::wcstombs(NULL , ddes->clientPath , 0);
	pMsgBuf = new char[sizeof(RCMSG_BUFF) - 1 + size + 2];
	pMsg = PRCMSG_BUFF(pMsgBuf);
	pMsg->msgHead.size = sizeof(RCMSG_BUFF) - 1 + size + 2;
	pMsg->msgHead.type = MT_DOWNLOAD_FILE_PATH_S;
	::wcstombs(pMsg->buf , ddes->clientPath , size);
	::setlocale(LC_ALL , "C");
	pMsg->buf[size] = 0;
	}//字符编码转换

	if(::send(fileSock , (char*)pMsg , pMsg->msgHead.size , 0 ) == SOCKET_ERROR)
	{// 套接字错误
		::CloseHandle(hFile);	
		::closesocket(fileSock);
		ddes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(ddes) , 0);
		return ; 
	}
	delete[] pMsgBuf;
	}//发送文件名

	{//接收客服端是否就绪  这个环节是必须的,但目的不是为了真正的确认，否则在此次消息通讯中有可能会出现数据紊乱(这和流式套接字数据缓存有关)
	char ok;
	if(::recv(fileSock , (char*)&ok , 1 , 0 ) == SOCKET_ERROR)
	{// 套接字错误
		::CloseHandle(hFile);	
		::closesocket(fileSock);
		ddes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(ddes) , 0);
		return ; 
	}
	}//接收客服端是否就绪

	//数据缓存
	char*	buf = new char[SEND_BUF_SIZE];

	{//发送文件大小
	char msgBuf[sizeof(RCMSG_BUFF) - 1 + sizeof(DWORD)] = {0};
	PRCMSG_BUFF pMsg = (PRCMSG_BUFF)msgBuf;
	pMsg->msgHead.size = sizeof(RCMSG_BUFF) - 1 + sizeof(DWORD);
	pMsg->msgHead.type = MT_DOWNLOAD_FILE_SIZE_S;
	*(DWORD*)(pMsg->buf) = nFileLen;

	if(::send(fileSock , (char*)pMsg , pMsg->msgHead.size , 0 )==SOCKET_ERROR)
	{//发送文件长度失败
		::closesocket(fileSock);
		fileSock = 0;
		CloseHandle(hFile);
		hFile = NULL;
		ddes->pWnd->SendMessage(WM_DL_ERROE , 0 , 0);
		delete[] buf;
		return ;
	}
	}//发送文件大小

	DWORD	sendTotal = 0;//总共发送了的数据
	DWORD	sendOne   = 0;//一次发送的数据

	while((sendTotal < nFileLen) && (ddes->lThreadState))
	{
		//::memset(buf , 0 , SEND_BUF_SIZE);//清理缓存
		sendOne = 0;
		if(FALSE == ReadFile(hFile , buf , SEND_BUF_SIZE , &sendOne , NULL))
		{//读取文件失败
			break;
		}
		if(!sendOne)
			break;
		sendOne = ::send(fileSock , buf , sendOne , 0);//发送已经读取的数据
		if(sendOne == SOCKET_ERROR)
		{//发送时套接字错误
			int err = WSAGetLastError();
			break;
		}
		sendTotal += sendOne;
		//通知框架下载进度
		ddes->pWnd->SendMessage(WM_DL_PROGRESS , (WPARAM)(ddes) , (LPARAM)sendTotal);
	}

	::CloseHandle(hFile);	
	::closesocket(fileSock);
	if (NULL != buf)
		delete[] buf;

	return ;
}

DWORD WINAPI DownUploadFileThread_C( LPVOID param )
{
	PUploadDescripter udes = PUploadDescripter(param);
	
	BOOL isDownload = TRUE;
	int size = 0;

	{//先接受是否是上传
	RCMSG_BUFF rcMsg = {0};
	if((size = ::recv(udes->sSocket , (char*)&rcMsg , sizeof(rcMsg) , 0)) == SOCKET_ERROR
		|| (size != rcMsg.msgHead.size)//接收到的消息大小不对
		|| (rcMsg.msgHead.type != MT_DU_DOWNLOAD_S))//接收到的消息类型不对
	{//接收文件路径时出错
		::closesocket(udes->sSocket);
		udes->sSocket = 0;
		udes->pWnd->SendMessage(WM_DL_ERROE , 0 , 0);
		return 0; 
	}
	isDownload = (rcMsg.buf[0] == 1);
	}

	if (isDownload)
	{//服务端下载文件，这边是上传
		UploadFile_C(udes);
	}else 
		DownloadFile_C(udes);

	return 1;
}

void UploadFile_C( PUploadDescripter udes ) 
{
	//要上传的文件路径
	CString filePath;
	DWORD	fileLen = 0;
	HANDLE  hFile = NULL;
	char*	buf = new char[SEND_BUF_SIZE];//数据缓存

	{//接收文件路径
	if((fileLen = ::recv(udes->sSocket , (char*)buf , SEND_BUF_SIZE , 0)) == SOCKET_ERROR
		|| (fileLen != PRCMSG_BUFF(buf)->msgHead.size)//接收到的消息大小不对
		|| (PRCMSG_BUFF(buf)->msgHead.type != MT_DOWNLOAD_FILE_PATH_S))//接收到的消息类型不对
	{//接收文件路径时出错	
		int er = WSAGetLastError();
		::closesocket(udes->sSocket);
		udes->sSocket = 0;
		udes->pWnd->SendMessage(WM_DL_ERROE , 0 , 0);
		delete[] buf;
		buf = NULL;
		return ; 
	}
	filePath += PRCMSG_BUFF(buf)->buf;
	udes->filePah = filePath;
	}//接收文件路径

	{//文件路径处理
	UINT type = 0;
	filePath = RevertPath(filePath , type);
	}//文件路径处理

	{//打开文件
	hFile = CreateFile( filePath , GENERIC_READ , FILE_SHARE_READ , NULL , OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);
	if ( INVALID_HANDLE_VALUE == hFile)
	{//文件打开失败
		::closesocket(udes->sSocket);
		udes->sSocket = 0;
		hFile = NULL;
		udes->pWnd->SendMessage(WM_DL_ERROE , 0 , 0);
		delete[] buf;
		buf = NULL;
		return ;
	}
	}//打开文件

	{//发送文件大小
	fileLen = GetFileSize(hFile , 0);
	if (INVALID_FILE_SIZE == fileLen)
	{//获取文件大小失败
		::closesocket(udes->sSocket);
		udes->sSocket = 0;
		CloseHandle(hFile);
		hFile = NULL;
		udes->pWnd->SendMessage(WM_DL_ERROE , 0 , 0);
		delete[] buf;
		buf = NULL;
		return ;
	}
	char msgBuf[sizeof(RCMSG_BUFF) - 1 + sizeof(DWORD)] = {0};
	PRCMSG_BUFF pMsg = (PRCMSG_BUFF)msgBuf;
	pMsg->msgHead.size = sizeof(RCMSG_BUFF) - 1 + sizeof(DWORD);
	pMsg->msgHead.type = MT_DOWNLOAD_FILE_SIZE_S;
	*(DWORD*)(pMsg->buf) = fileLen;

	if(::send(udes->sSocket , (char*)pMsg , pMsg->msgHead.size , 0 )==SOCKET_ERROR)
	{//发送文件长度失败
		::closesocket(udes->sSocket);
		udes->sSocket = 0;
		CloseHandle(hFile);
		hFile = NULL;
		udes->pWnd->SendMessage(WM_DL_ERROE , 0 , 0);
		delete[] buf;
		buf = NULL;
		return ;
	}
	}//发送文件大小

	DWORD sendTotal = 0;//总共发了的数据大小
	DWORD sendOne   = 0;//一次发送的大小

	//循环发送文件    同时下载标志必须为 true
	while((sendTotal < fileLen) && (udes->lThreadState))
	{
		::memset(buf , 0 , SEND_BUF_SIZE);//清理缓存
		sendOne = 0;
		if(FALSE == ReadFile(hFile , buf , SEND_BUF_SIZE , &sendOne , NULL))
		{//读取文件失败
			break;
		}
		if(!sendOne)
			break;
		sendOne = ::send(udes->sSocket , buf , sendOne , 0);//发送已经读取的数据
		if(sendOne == SOCKET_ERROR)
		{//发送时套接字错误
			int err = WSAGetLastError();
			break;
		}
		sendTotal += sendOne;
	}
	CloseHandle(hFile);
	::closesocket(udes->sSocket);
	udes->sSocket = 0;
	if (NULL != buf)
		delete[] buf;

	return ;
}
void DownloadFile_C(PUploadDescripter udes)
{
	//要上传的文件路径
	CString filePath;
	DWORD	nFileLen = 0;
	HANDLE  hFile = NULL;
	char*	buf = new char[RECV_BUF_SIZE];//数据缓存

	{//接收文件路径
	if((nFileLen = ::recv(udes->sSocket , (char*)buf , RECV_BUF_SIZE , 0)) == SOCKET_ERROR
		|| (nFileLen != PRCMSG_BUFF(buf)->msgHead.size)//接收到的消息大小不对
		|| (PRCMSG_BUFF(buf)->msgHead.type != MT_DOWNLOAD_FILE_PATH_S))//接收到的消息类型不对
	{//接收文件路径时出错	
		::closesocket(udes->sSocket);
		udes->sSocket = 0;
		udes->pWnd->SendMessage(WM_DL_ERROE , 0 , 0);
		delete[] buf;
		buf = NULL;
		return ; 
	}
	filePath += PRCMSG_BUFF(buf)->buf;
	UINT type = 0;
	filePath = RevertPath(filePath , type);
	udes->filePah = filePath;
	}//接收文件路径

	{//发送客服端是否就绪  这个环节是必须的,但目的不是为了真正的确认，否则在此次消息通讯中有可能会出现数据紊乱(这和流式套接字数据缓存有关)
	char ok = 1;
	if(::send(udes->sSocket , (char*)&ok , 1 , 0 ) == SOCKET_ERROR)
	{// 套接字错误
		::closesocket(udes->sSocket);
		udes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(udes) , 0);
		delete[] buf;
		buf = NULL;
		return ; 
	}
	}//发送客服端是否就绪

	{//接收文件大小
	if(((nFileLen = ::recv(udes->sSocket , (char*)buf , (sizeof(RCMSG_BUFF)-1 + sizeof(DWORD)) , 0) )== SOCKET_ERROR)
		|| (nFileLen != PRCMSG_BUFF(buf)->msgHead.size )
		|| (PRCMSG_BUFF(buf)->msgHead.type != MT_DOWNLOAD_FILE_SIZE_S))
	{//接受文件长度时 套接字错误
		::closesocket(udes->sSocket);
		udes->sSocket = 0;
		udes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(udes) , 0);
		delete[] buf;
		buf = NULL;
		return ; 
	}
	nFileLen = *(DWORD*)(PRCMSG_BUFF(buf)->buf);
	}//接收文件大小

	{//打开文件
	hFile = ::CreateFile(filePath , GENERIC_WRITE , NULL , NULL ,
			CREATE_ALWAYS , FILE_ATTRIBUTE_NORMAL , NULL);
	if ( INVALID_HANDLE_VALUE == hFile)
	{//文件打开失败
		::closesocket(udes->sSocket);
		udes->sSocket = 0;
		hFile = NULL;
		udes->pWnd->SendMessage(WM_DL_ERROE , 0 , 0);
		delete[] buf;
		buf = NULL;
		return ;
	}
	}//打开文件

	DWORD nRcvTotal = 0;//总共发了的数据大小
	DWORD nRcvOne   = 0;//一次发送的大小

	while((nRcvTotal < nFileLen) && (udes->lThreadState))
	{	
		nRcvOne = ::recv( udes->sSocket , buf , RECV_BUF_SIZE , 0);
		if(SOCKET_ERROR == nRcvOne) 
		{//接受数据时  套接字错误
			::CloseHandle(hFile);	
			::closesocket(udes->sSocket);
			udes->sSocket = 0;
			udes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(udes) , 0);
			delete[] buf;
			buf = NULL;
			return ;
		} 

		DWORD nWriten=0;
		if(!::WriteFile(hFile , buf , nRcvOne , &nWriten , NULL))
		{//写失败
			::CloseHandle(hFile);	
			::closesocket(udes->sSocket);
			udes->sSocket = 0;
			udes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(udes) , 0);
			delete[] buf;
			buf = NULL;
			return ;
		}
		nRcvTotal += nRcvOne;
	}

	CloseHandle(hFile);
	::closesocket(udes->sSocket);
	udes->sSocket = 0;
	if (NULL != buf)
		delete[] buf;

	return ;
}

DWORD WINAPI UploadListenerThread( LPVOID param )
{
	PDownloadListrenDescripter lisDes = PDownloadListrenDescripter(param);

	SOCKET listenSock;
	SOCKET dlSocket;

	sockaddr_in addr;
	int len = sizeof(sockaddr_in);

	listenSock = ::socket(AF_INET , SOCK_STREAM , 0);
	if(listenSock == INVALID_SOCKET)
	{//创建套接字失败
		//通知创建监听线程失败
		return 0;
	}
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = 0;

	if(::bind(listenSock , (sockaddr*)&addr , sizeof(addr)) == SOCKET_ERROR)
	{//绑定失败
		//通知创建监听线程失败
		::closesocket(listenSock);
		return 0;
	}

	{//获取监听端口
	sockaddr_in lisAddr = {0};
	int len = sizeof(lisAddr);
	getsockname(listenSock , (struct sockaddr*)&lisAddr , &len );
	lisDes->uPort = ntohs(lisAddr.sin_port);
	lisDes->pWnd->PostMessage(WM_UPLOAD_LISTEN_PORT , (WPARAM)(lisDes->uPort) , 0 );
	}

	if(::listen(listenSock,5)==SOCKET_ERROR)
	{//监听失败
		//通知创建监听线程失败
		::closesocket(listenSock);
		return 0;
	}

	while (lisDes->lThreadState)
	{
		dlSocket = ::accept(listenSock , (sockaddr*)&addr , &len);
		if(dlSocket == INVALID_SOCKET)
		{//接受失败
			::closesocket(listenSock);
			return 0;
		}

		//创建上传线程
		lisDes->pWnd->PostMessage(WM_CREATE_UPLOAD_THREAD , (WPARAM)(dlSocket) , 0 );
	}

	::closesocket(listenSock);
	return 0;
}

CString GetSizeToString( DWORD len )
{
	CString strSize(_T("0"));

	if(len < 1024)
	{//小于1K
		strSize.Format(_T("%dB") , len);
	}else if (len < (LONGLONG)1024 * 1024)
	{//小于1M
		strSize.Format(_T("%.2fKB") , len / 1024.0);
	} 
	else if (len < 1024 * 1024 * 1024)
	{//小于1G
		strSize.Format(_T("%.2fMB") , len / 1024.0 / 1024.0);
	} 
	else 
	{
		strSize.Format(_T("%.2fGB") , len / 1024.0 / 1024.0/1024.0);
	} 

	return strSize;
}

BOOL DrawPictureWithGDIPP( AutoSizeStream* screenData, HDC hDC, CRect &viewRect )
{
	//做显示处理
	Bitmap bit(screenData , FALSE);
	Graphics gh(hDC);
	gh.DrawImage( &bit , viewRect.left , viewRect.top , viewRect.Width() , viewRect.Height());
	
	return TRUE;
}

BOOL DrawPictureWithOle( AutoSizeStream* screenData, HDC hDC, CRect &viewRect )
{
	IPicture *pPic;

	if(SUCCEEDED(OleLoadPicture(screenData , 
		(LONG)screenData->GetValidSize().QuadPart 
		, TRUE ,IID_IPicture , (LPVOID*)&pPic)))
	{//加载图像成功
		OLE_XSIZE_HIMETRIC hmWidth;
		OLE_YSIZE_HIMETRIC hmHeight;
		pPic->get_Width(&hmWidth);
		pPic->get_Height(&hmHeight);
		if(FAILED(pPic->Render(hDC , viewRect.left , viewRect.top , (DWORD)viewRect.Width() 
			, (DWORD)viewRect.Height(), 0 , hmHeight , hmWidth , -hmHeight , NULL)))  
		{//绘制失败
			pPic->Release();
			return FALSE;
		}else
		{
			pPic->Release();
			return TRUE;
		}
	}  
	else  
		return FALSE;
}
