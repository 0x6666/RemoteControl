///////////////////////////////////////////////////////////////
//
// FileName	: globalfine.cpp 
// Creator	: ����
// Date		: 2013��3��11��, 22:55:50
// Comment	: ������ȫ�ֺ����Ķ���
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

//���շ�������ͼ�񣬲���ʾ����
DWORD WINAPI PrintScreenThread(LPVOID wParam)
{
	PPrintScreenThreadContext pContext = (PPrintScreenThreadContext)wParam;
	ASSERT(NULL != pContext);

	USHORT		localPort = 0;	//�󶨵ı��ض˿�
	long		threadState = 0;
	char*		buf = NULL;
	AutoSizeStream* screenData = NULL;
	CRect		viewRect;
	HDC			hDC  = ::GetDC(pContext->pWnd->GetSafeHwnd()); 

	pContext->sSocket = ::socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP);
	if(pContext->sSocket == INVALID_SOCKET) //�����׽���ʧ��
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
		{//��ʧ��
			goto exit_print_screnn;
		}
		//��ñ��ض˿�
		getsockname(pContext->sSocket , (struct sockaddr*)&local_addr, &sin_size);
		localPort = ntohs(local_addr.sin_port);

		pContext->pWnd->PostMessage(WM_GET_A_SCREEN_PORT , localPort , 0);
		ResetEvent(pContext->hThreadEvent);
		if(WAIT_TIMEOUT == WaitForSingleObject(pContext->hThreadEvent , 10*1000))
		{//ʮ�����û���ѵ�ǰ�߳̾͵��ǳ�ʱ
			pContext->pWnd->PostMessage(WM_WAIT_FOR_CAPTURE_TIMEOUT);
			goto exit_print_screnn;
		}else
		{
			if(0 == pContext->lThreadState)
				goto exit_print_screnn;//ֹͣ���߳�
		}
	}

	//��Ļ������
	screenData = AutoSizeStream::CreateAutoSizeStream();
	buf = new char[RECV_BUF_SIZE];

	while(0 != (threadState = pContext->lThreadState))
	{
		static const int DATALEN_MSG = sizeof(RCMSG_BUFF) - 1 + sizeof(UINT);
		//�ļ���
		UINT nScreenDataSize = 0;
		UINT nRcvTotal = 0;
		UINT nRcvOne   = 0; 
		LARGE_INTEGER move = {0};

		//���մ˴�Ҫ��������ݵĴ�С
		if((nRcvOne = recvfrom(pContext->sSocket , buf , RECV_BUF_SIZE , 0 , NULL , NULL)) == SOCKET_ERROR)
		{//���մ˴����ݵĴ�С��ʧ��
			pContext->pWnd->PostMessage(WM_SCREEN_SOCKET_ERR);
			goto exit_print_screnn; 
		}else if (nRcvOne == 0)	{//�����Ѿ��Ͽ�,��������
			pContext->pWnd->PostMessage(WM_SCREEN_SOCKET_ERR);
			goto exit_print_screnn;
		}else if(nRcvOne != DATALEN_MSG)
			continue; //�ӵ���һ����Ч����
		else{//��������ݳ���
			if(MT_SCREEN_DATA_SIZE_C == PRCMSG_BUFF(buf)->msgHead.type)
				nScreenDataSize = *((UINT*)(PRCMSG_BUFF(buf)->buf));
			else continue;
		}

		while(nRcvTotal < nScreenDataSize)
		{
			nRcvOne = recvfrom( pContext->sSocket , buf , RECV_BUF_SIZE , 0 , NULL , NULL);
			if(SOCKET_ERROR == nRcvOne) 
			{//��������ʱ  �׽��ִ���
				if (0 == pContext->lThreadState)
				{//�Ѿ���Ҫֹͣ�߳���
					goto exit_print_screnn;
				}
				pContext->pWnd->PostMessage(WM_SCREEN_SOCKET_ERR);
				goto exit_print_screnn; 
			} 

			if (nRcvOne == DATALEN_MSG)
			{//�������ݰ���ʧ��������ܻ��������,�ڿͻ���Ӧ���ⷢ�ʹ�СΪDATALEN_MSG������
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
			{//һ�����ݰ�ֻ��DATALEN_MSG ���Զ෢��һ���ֽ�
				--nRcvOne;
			}
			screenData->Write(buf , nRcvOne , NULL );
			nRcvTotal += nRcvOne;
		}

		viewRect = ((ClientItemView*)(pContext->pWnd))->GetScreenViewRect();

		screenData->Seek(move , STREAM_SEEK_SET , NULL);

		//ʹ��GDI+��ͼ
		DrawPictureWithGDIPP(screenData, hDC, viewRect);
		//ʹ��OLE
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

//��ͻ��������е�ÿһ���ͻ��˷�����Ϣ
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
	SOCKET				localSock; 		//�����׽���
	LARGE_INTEGER		move = {0};
	//HANDLE				hMutex = ((CRemoteControlDoc*)(pContext->pDoc))->GetPushedClientListMutex();
	//PushedClientList*	pPushedClientList = ((CRemoteControlDoc*)(pContext->pDoc))->GetPushedClientLIst();
	char*				buf = NULL;
	AutoSizeStream*	imgData = AutoSizeStream::CreateAutoSizeStream();
	HDC					hDesktopDC = GetDC(GetDesktopWindow());

	localSock = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP);
	if(localSock == INVALID_SOCKET) //�����׽���ʧ��
	{//�����׽���ʧ��
		localSock = 0;
		goto exit_print_screnn; 
	}

	long nfW			= 10;	//���ź�Ĵ�С
	long nfH			= 10;
	CLSID				encoderClsid;
	EncoderParameters	encoderParameters;
	ULONG				quality;
	

	//��ȡjpg����ͼƬ��clsid
	GetEncoderClsid(L"image/jpeg" , &encoderClsid);
	//��ʼ��ͼƬ����������
	InitEncoderParameter(encoderParameters, quality);

	buf = new char[SEND_BUF_SIZE];	//���ݻ���

	//��ʼץ����
	while(0 != pContext->lThreadState)
	{
		static const int DATALEN_MSG = sizeof(RCMSG_BUFF) - 1 + sizeof(UINT);

		nfW = pContext->lDesktopW;
		nfH = pContext->lDesktopH;
		quality = pContext->lDesktopQuality;
		CaptureDesktop( nfW, nfH , hDesktopDC , imgData, encoderClsid, encoderParameters);

		//�����ݷ���ȥ
		UINT imgDataSize	= (UINT)imgData->GetValidSize().QuadPart;
		UINT sendTotal		= 0;	//�ܹ����˵����ݴ�С
		ULONG sendOne		= 0;	//һ�η��͵Ĵ�С
	
		PRCMSG_BUFF pMsg	= PRCMSG_BUFF(buf);

		//��ȡ�ԿͿͻ��˵�ַ����Ļ�����
		WaitForSingleObject(pContext->hPushedClientListMutex , INFINITE);

		{//����һ�����ݵĳ���
		pMsg->msgHead.size	= DATALEN_MSG;
		pMsg->msgHead.type	= MT_SCREEN_DATA_SIZE_C;
		memcpy(pMsg->buf , &imgDataSize , sizeof(UINT));
		SendToAllAddr(&(pContext->lstPushedClient), localSock, pMsg, DATALEN_MSG);
		}

		//ѭ����������
		while(sendTotal < imgDataSize)
		{
			::memset(buf , 0 , SEND_BUF_SIZE);//��ʼ������
			imgData->Read(buf , SEND_BUF_SIZE , &sendOne);
			if( sendOne == 0)
				break;
			else if (DATALEN_MSG == sendOne)//�ܿ�����һ�����ݱ�ʱ��С�պ������ݴ�С��Ϣ��С
				++sendOne;
			SendToAllAddr(&(pContext->lstPushedClient), localSock, buf, sendOne);
			if(sendOne == SOCKET_ERROR)
			{//����ʱ�׽��ִ���
				goto exit_print_screnn;
			}
			sendTotal += sendOne;
		}
		//�ͷŶԿͻ��˵�ַ����������ռ��
		ReleaseMutex(pContext->hPushedClientListMutex);

		imgData->CleanValidData();//������Ч���ݣ�����ÿ�ζ�ȡ�����Ƕ�������ռ�
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

//������������
DWORD WINAPI  CheckClientThread(LPVOID param)
{
	PCheckClientThreadContext pContext = PCheckClientThreadContext(param);
	ASSERT(NULL != pContext);

	struct hostent *host;//������Ϣ  
	DWORD dwScope = RESOURCE_CONTEXT;
	NETRESOURCE *NetResource = NULL;
	HANDLE hEnum;//ö�پ��

	while( pContext->iThreadState )
	{
		//��ö���豸
		WNetOpenEnum( dwScope , NULL , NULL , NULL , &hEnum);
		//��ʼö��������Դ
		if ( hEnum )     //��������Ч
		{
			DWORD Count = 0xFFFFFFFF;
			DWORD BufferSize = 2048;
			char nrBuf[2048] = {0};//ö�ٵ���Դ����
			//����WNetEnumResource����һ����ö�ٹ���
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
						if (!strName.Left(2).Compare(_T("\\\\")))//ȥ��ǰ�������'\'
							strName = strName.Right(strName.GetLength()-2);

						//��Unicode ת���� Ascll
						char szStr[256] = {0};
						wcstombs(szStr , strName , strName.GetLength());

						//����������ø�����Ӧ��������Ϣ
						host = gethostbyname(szStr);
						if(host == NULL) continue;

						strIP += inet_ntoa(*(in_addr*)host->h_addr) ;
						//�����ｫ��Ϣ����ȥ
						((CRemoteControlDoc*)pContext->pDoc)->FoundAClient(strName.Trim() , strIP.Trim());
					}
				}
			}
			// ����ö�ٹ���
			WNetCloseEnum( hEnum );    
		}
		//�����Ӽ��һ��
		Sleep(5000);
	}
	return 1;
}
