///////////////////////////////////////////////////////////////
//
// FileName	: globalfine.cpp 
// Creator	: 杨松
// Date		: 2013年3月11日, 22:55:50
// Comment	: 服务器全局函数的定义
//
//////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "globalfine.h"
#include "clientItemView.h"
#include "RemoteControlDoc.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//接收发过来的图像，并显示出来
DWORD WINAPI PrintScreenThread(LPVOID wParam)
{
	PPrintScreenThreadContext pContext = (PPrintScreenThreadContext)wParam;
	ASSERT(NULL != pContext);

	USHORT		localPort = 0;	//绑定的本地端口
	long		threadState = 0;
	char*		buf = NULL;
	AutoSizeStream* screenData = NULL;
	CRect		viewRect;
	HDC			hDC  = ::GetDC(pContext->pWnd->GetSafeHwnd()); 

	pContext->sSocket = ::socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP);
	if(pContext->sSocket == INVALID_SOCKET) //创建套接字失败
	{
		pContext->pWnd->PostMessage(WM_SCREEN_SOCKET_ERR	, 0 ,0);
		pContext->sSocket = 0;	
		goto exit_print_screnn; 
	}
	else
	{
		sockaddr_in local_addr = {0};
		int sin_size = sizeof(local_addr);
		memset(&local_addr, 0, sin_size );
		local_addr.sin_family = AF_INET;
		local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		local_addr.sin_port = 0;

		if ((bind(pContext->sSocket , (struct sockaddr *) (&local_addr) , sin_size )) == -1)
		{//绑定失败
			goto exit_print_screnn;
		}
		//获得本地端口
		getsockname(pContext->sSocket , (struct sockaddr*)&local_addr, &sin_size);
		localPort = ntohs(local_addr.sin_port);

		pContext->pWnd->PostMessage(WM_GET_A_SCREEN_PORT , localPort , 0);
		ResetEvent(pContext->hThreadEvent);
		if(WAIT_TIMEOUT == WaitForSingleObject(pContext->hThreadEvent , 10*1000))
		{//十秒过后还没唤醒当前线程就当是超时
			pContext->pWnd->PostMessage(WM_WAIT_FOR_CAPTURE_TIMEOUT);
			goto exit_print_screnn;
		}else
		{
			if(0 == pContext->lThreadState)
				goto exit_print_screnn;//停止了线程
		}
	}

	//屏幕数据流
	screenData = AutoSizeStream::CreateAutoSizeStream();
	buf = new char[RECV_BUF_SIZE];

	while(0 != (threadState = pContext->lThreadState))
	{
		static const int DATALEN_MSG = sizeof(RCMSG_BUFF) - 1 + sizeof(UINT);
		//文件的
		UINT nScreenDataSize = 0;
		UINT nRcvTotal = 0;
		UINT nRcvOne   = 0; 
		LARGE_INTEGER move = {0};

		//接收此次要传输的数据的大小
		if((nRcvOne = recvfrom(pContext->sSocket , buf , RECV_BUF_SIZE , 0 , NULL , NULL)) == SOCKET_ERROR)
		{//接收此次数据的大小，失败
			pContext->pWnd->PostMessage(WM_SCREEN_SOCKET_ERR);
			goto exit_print_screnn; 
		}else if (nRcvOne == 0)	{//连接已经断开,做错误处理
			pContext->pWnd->PostMessage(WM_SCREEN_SOCKET_ERR);
			goto exit_print_screnn;
		}else if(nRcvOne != DATALEN_MSG)
			continue; //接到了一次无效数据
		else{//获得了数据长度
			if(MT_SCREEN_DATA_SIZE_C == PRCMSG_BUFF(buf)->msgHead.type)
				nScreenDataSize = *((UINT*)(PRCMSG_BUFF(buf)->buf));
			else continue;
		}

		while(nRcvTotal < nScreenDataSize)
		{
			nRcvOne = recvfrom( pContext->sSocket , buf , RECV_BUF_SIZE , 0 , NULL , NULL);
			if(SOCKET_ERROR == nRcvOne) 
			{//接受数据时  套接字错误
				if (0 == pContext->lThreadState)
				{//已经需要停止线程了
					goto exit_print_screnn;
				}
				pContext->pWnd->PostMessage(WM_SCREEN_SOCKET_ERR);
				goto exit_print_screnn; 
			} 

			if (nRcvOne == DATALEN_MSG)
			{//在有数据包丢失的情况可能会进入这里,在客户端应避免发送大小为DATALEN_MSG的数据
				if(MT_SCREEN_DATA_SIZE_C == PRCMSG_BUFF(buf)->msgHead.type)
				{
					nScreenDataSize = *((UINT*)(PRCMSG_BUFF(buf)->buf));
					nRcvTotal = 0;
					move.QuadPart = 0;
					screenData->Seek(move , STREAM_SEEK_SET , NULL);
					continue;
				}
			}
			if ((nRcvTotal + nRcvOne) == 1 + nScreenDataSize)
			{//一个数据包只有DATALEN_MSG 所以多发了一个字节
				--nRcvOne;
			}
			screenData->Write(buf , nRcvOne , NULL );
			nRcvTotal += nRcvOne;
		}

		viewRect = ((ClientItemView*)(pContext->pWnd))->GetScreenViewRect();

		screenData->Seek(move , STREAM_SEEK_SET , NULL);

		//使用GDI+绘图
		DrawPictureWithGDIPP(screenData, hDC, viewRect);
		//使用OLE
		//DrawPictureWithOle(screenData, hDC, viewRect);

		screenData->CleanValidData();
	}

exit_print_screnn:
	if (NULL != screenData)
		screenData->Release();
	if (hDC)
		ReleaseDC( pContext->pWnd->GetSafeHwnd() , hDC);

	pContext->pWnd->PostMessage(WM_SCREEN_THREAD_EXIT , 0 , 0);
	closesocket(pContext->sSocket);

	if (NULL != buf)
		delete[] buf;

	return 1;
}

//向客户端链表中的每一个客户端发送消息
void SendToAllAddr( PushServerDesktopThreadContext::PushedClientList* pPushedClientList, SOCKET localSock, void* pMsg, const int msgSize )
{
	for (PushServerDesktopThreadContext::PushedClientList::iterator it = pPushedClientList->begin() ; 
		it != pPushedClientList->end() ; ++it)
	{
		int err = sendto(localSock , (char*)pMsg , msgSize , 0 , (sockaddr*)(*it) , sizeof(sockaddr_in));
		if (err == SOCKET_ERROR)
		{
			err = WSAGetLastError();
			err = err;
		}
	}
}

DWORD WINAPI PushSeverDesktopThread( LPVOID wParam )
{
	PPushServerDesktopThreadContext pContext = PPushServerDesktopThreadContext(wParam);
	ASSERT(NULL != pContext);

	//CRemoteControlDoc*	pDoc = (CRemoteControlDoc*)wParam;
	SOCKET				localSock; 		//本地套接字
	LARGE_INTEGER		move = {0};
	//HANDLE				hMutex = ((CRemoteControlDoc*)(pContext->pDoc))->GetPushedClientListMutex();
	//PushedClientList*	pPushedClientList = ((CRemoteControlDoc*)(pContext->pDoc))->GetPushedClientLIst();
	char*				buf = NULL;
	AutoSizeStream*	imgData = AutoSizeStream::CreateAutoSizeStream();
	HDC					hDesktopDC = GetDC(GetDesktopWindow());

	localSock = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP);
	if(localSock == INVALID_SOCKET) //创建套接字失败
	{//创建套接字失败
		localSock = 0;
		goto exit_print_screnn; 
	}

	long nfW			= 10;	//缩放后的大小
	long nfH			= 10;
	CLSID				encoderClsid;
	EncoderParameters	encoderParameters;
	ULONG				quality;
	

	//获取jpg类型图片的clsid
	GetEncoderClsid(L"image/jpeg" , &encoderClsid);
	//初始化图片编码器参数
	InitEncoderParameter(encoderParameters, quality);

	buf = new char[SEND_BUF_SIZE];	//数据缓存

	//开始抓屏了
	while(0 != pContext->lThreadState)
	{
		static const int DATALEN_MSG = sizeof(RCMSG_BUFF) - 1 + sizeof(UINT);

		nfW = pContext->lDesktopW;
		nfH = pContext->lDesktopH;
		quality = pContext->lDesktopQuality;
		CaptureDesktop( nfW, nfH , hDesktopDC , imgData, encoderClsid, encoderParameters);

		//将数据发出去
		UINT imgDataSize	= (UINT)imgData->GetValidSize().QuadPart;
		UINT sendTotal		= 0;	//总共发了的数据大小
		ULONG sendOne		= 0;	//一次发送的大小
	
		PRCMSG_BUFF pMsg	= PRCMSG_BUFF(buf);

		//获取对客客户端地址链表的互斥量
		WaitForSingleObject(pContext->hPushedClientListMutex , INFINITE);

		{//发送一下数据的长度
		pMsg->msgHead.size	= DATALEN_MSG;
		pMsg->msgHead.type	= MT_SCREEN_DATA_SIZE_C;
		memcpy(pMsg->buf , &imgDataSize , sizeof(UINT));
		SendToAllAddr(&(pContext->lstPushedClient), localSock, pMsg, DATALEN_MSG);
		}

		//循环发送数据
		while(sendTotal < imgDataSize)
		{
			::memset(buf , 0 , SEND_BUF_SIZE);//初始化缓存
			imgData->Read(buf , SEND_BUF_SIZE , &sendOne);
			if( sendOne == 0)
				break;
			else if (DATALEN_MSG == sendOne)//避开发送一个数据报时大小刚好是数据大小消息大小
				++sendOne;
			SendToAllAddr(&(pContext->lstPushedClient), localSock, buf, sendOne);
			if(sendOne == SOCKET_ERROR)
			{//发送时套接字错误
				goto exit_print_screnn;
			}
			sendTotal += sendOne;
		}
		//释放对客户端地址链表互斥量的占用
		ReleaseMutex(pContext->hPushedClientListMutex);

		imgData->CleanValidData();//清理有效数据，避免每次读取数据是都来分配空间
		Sleep(pContext->lDesktopFrequency);
	}

exit_print_screnn:
	closesocket(localSock);
	imgData->Release();
	ReleaseDC(GetDesktopWindow() , hDesktopDC);
	
	if (NULL != buf)
		delete[] buf;

	return 0;
}

//检查局域网主机
DWORD WINAPI  CheckClientThread(LPVOID param)
{
	PCheckClientThreadContext pContext = PCheckClientThreadContext(param);
	ASSERT(NULL != pContext);

	struct hostent *host;//主机信息  
	DWORD dwScope = RESOURCE_CONTEXT;
	NETRESOURCE *NetResource = NULL;
	HANDLE hEnum;//枚举句柄

	while( pContext->iThreadState )
	{
		//打开枚举设备
		WNetOpenEnum( dwScope , NULL , NULL , NULL , &hEnum);
		//开始枚举网络资源
		if ( hEnum )     //如果句柄有效
		{
			DWORD Count = 0xFFFFFFFF;
			DWORD BufferSize = 2048;
			char nrBuf[2048] = {0};//枚举到资源缓存
			//调用WNetEnumResource做进一步的枚举工作
			WNetEnumResource( hEnum, &Count , nrBuf , &BufferSize );
			NetResource = (NETRESOURCE*)nrBuf;
			for(UINT i = 0 ; pContext->iThreadState &&
				i <  Count; i++ , NetResource++)        
			{
				if (NetResource->dwUsage == RESOURCEUSAGE_CONTAINER &&
					NetResource->dwType == RESOURCETYPE_ANY )
				{
					if (NetResource->lpRemoteName )
					{
						CString strName;
						CString strIP;

						strName = NetResource->lpRemoteName;
						if (!strName.Left(2).Compare(_T("\\\\")))//去掉前面的两个'\'
							strName = strName.Right(strName.GetLength()-2);

						//将Unicode 转换成 Ascll
						char szStr[256] = {0};
						wcstombs(szStr , strName , strName.GetLength());

						//由主机名获得跟它对应的主机信息
						host = gethostbyname(szStr);
						if(host == NULL) continue;

						strIP += inet_ntoa(*(in_addr*)host->h_addr) ;
						//在这里将信息发出去
						((CRemoteControlDoc*)pContext->pDoc)->FoundAClient(strName.Trim() , strIP.Trim());
					}
				}
			}
			// 结束枚举工作
			WNetCloseEnum( hEnum );    
		}
		//五秒钟检查一次
		Sleep(5000);
	}
	return 1;
}
