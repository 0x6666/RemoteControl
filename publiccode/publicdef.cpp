///////////////////////////////////////////////////////////////
//
// FileName	: publicdef.cpp 
// Creator	: ����
// Date		: 2013��3��22��, 23:42:22
// Comment	: ����˺Ϳͻ��˹����Ķ���
//
//////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "publicdef.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//��ʾ���ĳߴ�
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
		return "ÿ��1֡";
	case FT_LEVEL_2:
		return "ÿ��2֡";
	case FT_LEVEL_3:
		return "ÿ��6֡";
	case FT_LEVEL_4:
		return "ÿ��12֡";
	case FT_LEVEL_5:
		return "ÿ��18֡";
	case FT_LEVEL_6:
		return "ÿ��24֡";
	case FT_LEVEL_7:
		return "ÿ��30֡";
	default:
		ASSERT(FALSE);
	}
	return NULL;
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT num= 0;
	UINT size= 0;

	//ͼ�������Ϣ
	ImageCodecInfo* pImageCodecInfo= NULL;

	//���ͼ���������С������
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

	//���ͼ�����������
	GetImageEncoders(num, size, pImageCodecInfo);

	for(UINT j = 0 ; j < num ; ++j )
	{
		if(wcscmp(pImageCodecInfo[j].MimeType, format)== 0)
		{//�ҵ�ָ����ͼ�������
			*pClsid= pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;
		}
	}

	//ò��û��ָ����ͼ�������
	free(pImageCodecInfo);
	return -1;
}

void CaptureDesktop(long nfW, long nfH ,//ץ���󱣴�ͼ��ĳ��Ϳ�
					HDC	hDesktop		//�����豸�����ľ��
					,IN OUT AutoSizeStream* imgData,//ץ�����ͼ�����ݻ���
					const CLSID& encoderClsid,	//ץ�������ͼ�������
					const EncoderParameters& encoderParameters)//���������������ڵ���ͼ������
{
	//�����ʾ���ĳߴ�
	LARGE_INTEGER move = {0};
	//�������������ݵ�DC
	HDC hCaptionDC = ::CreateCompatibleDC(hDesktop);
	//������������λͼ
	HBITMAP hDeskBit = ::CreateCompatibleBitmap(hDesktop , nfW , nfH);
	::SelectObject(hCaptionDC , hDeskBit);
	
	//����ͼ������ģʽ������������ִ��StretchBlt�Ļ�ȥ�������ر�Ե������һ��ƽ�͵Ĵ���,
	//ͼ���������Щ���ң�ͼ����������
	::SetStretchBltMode(hCaptionDC , HALFTONE);  
	::SetBrushOrgEx(hCaptionDC , 0 , 0 , NULL );
	
	//ץȡ��ͼƬ�����з���
	::StretchBlt( hCaptionDC , 0 , 0 , nfW , nfH , hDesktop , 0 , 0 , SCREEN_SIZE_W , SCREEN_SIZE_H , SRCCOPY );

	{//��ȡץ���ͼƬ����
	Bitmap bmp( hDeskBit , NULL);
	bmp.Save(imgData , &encoderClsid , &encoderParameters );
	imgData->Seek(move , STREAM_SEEK_SET , NULL );
	}

	//������ض���
	::DeleteObject(hCaptionDC);
	::DeleteObject(hDeskBit);
}

void InitEncoderParameter( EncoderParameters &encoderParameters, ULONG& quality )
{
	//��ʼ������ͼƬ��������
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
	{//�ȴ��߳��˳�
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
	{//����
		type = 1;
		TCHAR MyDir[MAX_PATH];
		SHGetSpecialFolderPath(NULL , MyDir , CSIDL_DESKTOP , 0 );
		strPreDir += MyDir;
		if (strPreDir.Right(1) == _T('\\'))
			strPreDir = strPreDir.Mid(0 , strPreDir.GetLength()-1); 
		strPath.Replace(DIR_DESKTOP , strPreDir);
	}else if(strPath.Left(_tcslen(DIR_MY_DOC)) == DIR_MY_DOC)
	{//�ҵ��ĵ�
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
	{//����
		type = 1;
		strPath.Replace(DIR_DESKTOP , _T("����"));
	}else if(strPath.Left(_tcslen(DIR_MY_DOC)) == DIR_MY_DOC)
	{//�ҵ��ĵ�
		type = 2;
		strPath.Replace(DIR_MY_DOC , _T("�ҵ��ĵ�"));
	}
	return strPath;
}

CString ReplacePath( CString path , UINT type )
{
	TCHAR MyDir[MAX_PATH];
	CString strPreDir; 
	if (type == 1 )
	{//����
		SHGetSpecialFolderPath(NULL , MyDir , CSIDL_DESKTOP , 0 );
		strPreDir += MyDir;
		if (strPreDir.Right(1) == _T('\\'))
			strPreDir = strPreDir.Mid(0 , strPreDir.GetLength()-1); 
		path.Replace( strPreDir , DIR_DESKTOP);
	}else if(type == 2)
	{//�ҵ��ĵ�
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
	//�մ�
	if(path.IsEmpty())
		return CString();
	
	//û��·�� ֻ��һ���ļ���
	int index = path.ReverseFind(_T('\\'));
	if (index == -1)
		return path;
	
	CString name = path.Mid(index + 1);
	if (name.IsEmpty())
	{//·����һ���ļ���
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
	
	{//�����׽���
	fileSock = ::socket(AF_INET , SOCK_STREAM , 0);
	if(fileSock == INVALID_SOCKET)
	{//�����׽���ʧ��
		ddes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(ddes) , 0);
		goto exit_updownload; 
	}
	}//�����׽���

	{//���ӿͻ���
	::memset(&addr,0,sizeof(addr));
	addr.sin_family     = AF_INET;//��ַ��
	addr.sin_addr.s_addr= ::inet_addr(cIP);//ip��ַ
	addr.sin_port       = ::htons(ddes->uDownUploadListenPort);//�˿�

	if(::connect(fileSock , (sockaddr*)&addr , sizeof(addr) )== SOCKET_ERROR)
	{
		::closesocket(fileSock);
		ddes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(ddes) , 0);
		goto exit_updownload; 
	}
	}//����Զ�̵�ַ��Ϣ


	{//�ȷ����Ƿ�������
	RCMSG_BUFF rcMsg = {0};
	rcMsg.msgHead.size = sizeof(RCMSG_BUFF);
	rcMsg.msgHead.type = MT_DU_DOWNLOAD_S;
	rcMsg.buf[0] = ddes->isDownload ? 1 : 0;
	if(::send(fileSock , (char*)&rcMsg , rcMsg.msgHead.size , 0 ) == SOCKET_ERROR)
	{//�����ļ�����ʱ �׽��ִ���
		::closesocket(fileSock);
		ddes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(ddes) , 0);
		goto exit_updownload; 
	}
	}

	if (ddes->isDownload)
	{//�����ļ�
		DownloadFile_S(fileSock , ddes);
	}else
	{//�ϴ��ļ�
		UploadFile_S(fileSock , ddes);
	}

exit_updownload:
	ddes->pWnd->SendMessage(WM_DL_THREAD_EXIT , (WPARAM)(ddes) , 0 );
	return 1;
}

void DownloadFile_S( SOCKET fileSock, PDowmUploadContext ddes ) 
{
	UINT	nFileLen  = 0;//�ļ��ĳ���
	UINT	nRcvTotal = 0;//�ܹ������˵�����
	UINT	nRcvOne   = 0;//һ�η��͵�����

	//Ҫ���ص��ļ�����·��(��ʱ�ļ���)
	CString  strFile = (ddes->curPath.Trim()+_T(".temp"));

	HANDLE hFile = ::CreateFile(strFile , GENERIC_WRITE , NULL , NULL ,
		CREATE_ALWAYS , FILE_ATTRIBUTE_NORMAL , NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{//�����ļ�ʧ��
		ddes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(ddes) , 0);
		return ; 
	}

	{//�����ļ���
	int size = 0;
	char* pMsgBuf = NULL;
	PRCMSG_BUFF pMsg = NULL;
	{//�ַ�����ת��
		::setlocale(LC_ALL , "");
		size = ::wcstombs(NULL , ddes->clientPath , 0);
		pMsgBuf = new char[sizeof(RCMSG_BUFF) - 1 + size + 2];
		pMsg = PRCMSG_BUFF(pMsgBuf);
		pMsg->msgHead.size = sizeof(RCMSG_BUFF) - 1 + size + 2;
		pMsg->msgHead.type = MT_DOWNLOAD_FILE_PATH_S;
		::wcstombs(pMsg->buf , ddes->clientPath , size);
		::setlocale(LC_ALL , "C");
		pMsg->buf[size] = 0;
	}//�ַ�����ת��

	if(::send(fileSock , (char*)pMsg , pMsg->msgHead.size , 0 )==SOCKET_ERROR)
	{//�����ļ�����ʱ �׽��ִ���
		::CloseHandle(hFile);	
		::closesocket(fileSock);
		ddes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(ddes) , 0);
		return ; 
	}
	delete[] pMsgBuf;
	}//�����ļ���


	//���ݻ���
	char*	buf = new char[RECV_BUF_SIZE];

	{//�����ļ���С
	if(((nFileLen = ::recv(fileSock , (char*)buf , (sizeof(RCMSG_BUFF)-1 + sizeof(DWORD)) , 0) )== SOCKET_ERROR)
		|| (nFileLen != PRCMSG_BUFF(buf)->msgHead.size )
		|| (PRCMSG_BUFF(buf)->msgHead.type != MT_DOWNLOAD_FILE_SIZE_S))
	{//�����ļ�����ʱ �׽��ִ���
		::CloseHandle(hFile);	
		::closesocket(fileSock);
		ddes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(ddes) , 0);
		delete[] buf;
		return ; 
	}
	nFileLen = *(DWORD*)(PRCMSG_BUFF(buf)->buf);
	ddes->pWnd->SendMessage(WM_DL_FILE_SIZE , (WPARAM)(ddes)  , (WPARAM)nFileLen);
	}//�����ļ���С


	while((nRcvTotal < nFileLen) && (ddes->lThreadState))
	{
		::memset(buf , 0 , RECV_BUF_SIZE);	
		nRcvOne = ::recv( fileSock , buf , RECV_BUF_SIZE , 0);
		if(SOCKET_ERROR == nRcvOne) 
		{//��������ʱ  �׽��ִ���
			::CloseHandle(hFile);	
			::closesocket(fileSock);
			ddes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(ddes) , 0);
			delete[] buf;
			return ;
		} 

		DWORD nWriten=0;
		if(!::WriteFile(hFile , buf , nRcvOne , &nWriten , NULL))
		{//дʧ��
			::CloseHandle(hFile);	
			::closesocket(fileSock);
			ddes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(ddes) , 0);
			delete[] buf;
			return ;
		}
		nRcvTotal += nRcvOne;
		//֪ͨ������ؽ���
		ddes->pWnd->SendMessage(WM_DL_PROGRESS , (WPARAM)(ddes) , (LPARAM)nRcvTotal);
	}

	::CloseHandle(hFile);	
	::closesocket(fileSock);
	if (NULL != buf)
	{
		delete[] buf;
	}
	

	//�������ļ���Ŀ���ļ�
	MoveFile(strFile , ddes->curPath);
	DeleteFile(strFile);

	return ;
}

void UploadFile_S( SOCKET fileSock, PDowmUploadContext ddes ) 
{
	//Ҫ�ϴ����ļ�
	HANDLE hFile = ::CreateFile(ddes->curPath , GENERIC_READ , NULL , NULL ,
		OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{//���ļ�ʧ��
		ddes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(ddes) , 0);
		return ; 
	}
	//�ļ��ĳ���
	DWORD	nFileLen  = GetFileSize(hFile , 0);

	{//�����ļ���
	int size = 0;
	char* pMsgBuf = NULL;
	PRCMSG_BUFF pMsg = NULL;
	{//�ַ�����ת��
	::setlocale(LC_ALL , "");
	size = ::wcstombs(NULL , ddes->clientPath , 0);
	pMsgBuf = new char[sizeof(RCMSG_BUFF) - 1 + size + 2];
	pMsg = PRCMSG_BUFF(pMsgBuf);
	pMsg->msgHead.size = sizeof(RCMSG_BUFF) - 1 + size + 2;
	pMsg->msgHead.type = MT_DOWNLOAD_FILE_PATH_S;
	::wcstombs(pMsg->buf , ddes->clientPath , size);
	::setlocale(LC_ALL , "C");
	pMsg->buf[size] = 0;
	}//�ַ�����ת��

	if(::send(fileSock , (char*)pMsg , pMsg->msgHead.size , 0 ) == SOCKET_ERROR)
	{// �׽��ִ���
		::CloseHandle(hFile);	
		::closesocket(fileSock);
		ddes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(ddes) , 0);
		return ; 
	}
	delete[] pMsgBuf;
	}//�����ļ���

	{//���տͷ����Ƿ����  ��������Ǳ����,��Ŀ�Ĳ���Ϊ��������ȷ�ϣ������ڴ˴���ϢͨѶ���п��ܻ������������(�����ʽ�׽������ݻ����й�)
	char ok;
	if(::recv(fileSock , (char*)&ok , 1 , 0 ) == SOCKET_ERROR)
	{// �׽��ִ���
		::CloseHandle(hFile);	
		::closesocket(fileSock);
		ddes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(ddes) , 0);
		return ; 
	}
	}//���տͷ����Ƿ����

	//���ݻ���
	char*	buf = new char[SEND_BUF_SIZE];

	{//�����ļ���С
	char msgBuf[sizeof(RCMSG_BUFF) - 1 + sizeof(DWORD)] = {0};
	PRCMSG_BUFF pMsg = (PRCMSG_BUFF)msgBuf;
	pMsg->msgHead.size = sizeof(RCMSG_BUFF) - 1 + sizeof(DWORD);
	pMsg->msgHead.type = MT_DOWNLOAD_FILE_SIZE_S;
	*(DWORD*)(pMsg->buf) = nFileLen;

	if(::send(fileSock , (char*)pMsg , pMsg->msgHead.size , 0 )==SOCKET_ERROR)
	{//�����ļ�����ʧ��
		::closesocket(fileSock);
		fileSock = 0;
		CloseHandle(hFile);
		hFile = NULL;
		ddes->pWnd->SendMessage(WM_DL_ERROE , 0 , 0);
		delete[] buf;
		return ;
	}
	}//�����ļ���С

	DWORD	sendTotal = 0;//�ܹ������˵�����
	DWORD	sendOne   = 0;//һ�η��͵�����

	while((sendTotal < nFileLen) && (ddes->lThreadState))
	{
		//::memset(buf , 0 , SEND_BUF_SIZE);//������
		sendOne = 0;
		if(FALSE == ReadFile(hFile , buf , SEND_BUF_SIZE , &sendOne , NULL))
		{//��ȡ�ļ�ʧ��
			break;
		}
		if(!sendOne)
			break;
		sendOne = ::send(fileSock , buf , sendOne , 0);//�����Ѿ���ȡ������
		if(sendOne == SOCKET_ERROR)
		{//����ʱ�׽��ִ���
			int err = WSAGetLastError();
			break;
		}
		sendTotal += sendOne;
		//֪ͨ������ؽ���
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

	{//�Ƚ����Ƿ����ϴ�
	RCMSG_BUFF rcMsg = {0};
	if((size = ::recv(udes->sSocket , (char*)&rcMsg , sizeof(rcMsg) , 0)) == SOCKET_ERROR
		|| (size != rcMsg.msgHead.size)//���յ�����Ϣ��С����
		|| (rcMsg.msgHead.type != MT_DU_DOWNLOAD_S))//���յ�����Ϣ���Ͳ���
	{//�����ļ�·��ʱ����
		::closesocket(udes->sSocket);
		udes->sSocket = 0;
		udes->pWnd->SendMessage(WM_DL_ERROE , 0 , 0);
		return 0; 
	}
	isDownload = (rcMsg.buf[0] == 1);
	}

	if (isDownload)
	{//����������ļ���������ϴ�
		UploadFile_C(udes);
	}else 
		DownloadFile_C(udes);

	return 1;
}

void UploadFile_C( PUploadDescripter udes ) 
{
	//Ҫ�ϴ����ļ�·��
	CString filePath;
	DWORD	fileLen = 0;
	HANDLE  hFile = NULL;
	char*	buf = new char[SEND_BUF_SIZE];//���ݻ���

	{//�����ļ�·��
	if((fileLen = ::recv(udes->sSocket , (char*)buf , SEND_BUF_SIZE , 0)) == SOCKET_ERROR
		|| (fileLen != PRCMSG_BUFF(buf)->msgHead.size)//���յ�����Ϣ��С����
		|| (PRCMSG_BUFF(buf)->msgHead.type != MT_DOWNLOAD_FILE_PATH_S))//���յ�����Ϣ���Ͳ���
	{//�����ļ�·��ʱ����	
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
	}//�����ļ�·��

	{//�ļ�·������
	UINT type = 0;
	filePath = RevertPath(filePath , type);
	}//�ļ�·������

	{//���ļ�
	hFile = CreateFile( filePath , GENERIC_READ , FILE_SHARE_READ , NULL , OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);
	if ( INVALID_HANDLE_VALUE == hFile)
	{//�ļ���ʧ��
		::closesocket(udes->sSocket);
		udes->sSocket = 0;
		hFile = NULL;
		udes->pWnd->SendMessage(WM_DL_ERROE , 0 , 0);
		delete[] buf;
		buf = NULL;
		return ;
	}
	}//���ļ�

	{//�����ļ���С
	fileLen = GetFileSize(hFile , 0);
	if (INVALID_FILE_SIZE == fileLen)
	{//��ȡ�ļ���Сʧ��
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
	{//�����ļ�����ʧ��
		::closesocket(udes->sSocket);
		udes->sSocket = 0;
		CloseHandle(hFile);
		hFile = NULL;
		udes->pWnd->SendMessage(WM_DL_ERROE , 0 , 0);
		delete[] buf;
		buf = NULL;
		return ;
	}
	}//�����ļ���С

	DWORD sendTotal = 0;//�ܹ����˵����ݴ�С
	DWORD sendOne   = 0;//һ�η��͵Ĵ�С

	//ѭ�������ļ�    ͬʱ���ر�־����Ϊ true
	while((sendTotal < fileLen) && (udes->lThreadState))
	{
		::memset(buf , 0 , SEND_BUF_SIZE);//������
		sendOne = 0;
		if(FALSE == ReadFile(hFile , buf , SEND_BUF_SIZE , &sendOne , NULL))
		{//��ȡ�ļ�ʧ��
			break;
		}
		if(!sendOne)
			break;
		sendOne = ::send(udes->sSocket , buf , sendOne , 0);//�����Ѿ���ȡ������
		if(sendOne == SOCKET_ERROR)
		{//����ʱ�׽��ִ���
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
	//Ҫ�ϴ����ļ�·��
	CString filePath;
	DWORD	nFileLen = 0;
	HANDLE  hFile = NULL;
	char*	buf = new char[RECV_BUF_SIZE];//���ݻ���

	{//�����ļ�·��
	if((nFileLen = ::recv(udes->sSocket , (char*)buf , RECV_BUF_SIZE , 0)) == SOCKET_ERROR
		|| (nFileLen != PRCMSG_BUFF(buf)->msgHead.size)//���յ�����Ϣ��С����
		|| (PRCMSG_BUFF(buf)->msgHead.type != MT_DOWNLOAD_FILE_PATH_S))//���յ�����Ϣ���Ͳ���
	{//�����ļ�·��ʱ����	
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
	}//�����ļ�·��

	{//���Ϳͷ����Ƿ����  ��������Ǳ����,��Ŀ�Ĳ���Ϊ��������ȷ�ϣ������ڴ˴���ϢͨѶ���п��ܻ������������(�����ʽ�׽������ݻ����й�)
	char ok = 1;
	if(::send(udes->sSocket , (char*)&ok , 1 , 0 ) == SOCKET_ERROR)
	{// �׽��ִ���
		::closesocket(udes->sSocket);
		udes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(udes) , 0);
		delete[] buf;
		buf = NULL;
		return ; 
	}
	}//���Ϳͷ����Ƿ����

	{//�����ļ���С
	if(((nFileLen = ::recv(udes->sSocket , (char*)buf , (sizeof(RCMSG_BUFF)-1 + sizeof(DWORD)) , 0) )== SOCKET_ERROR)
		|| (nFileLen != PRCMSG_BUFF(buf)->msgHead.size )
		|| (PRCMSG_BUFF(buf)->msgHead.type != MT_DOWNLOAD_FILE_SIZE_S))
	{//�����ļ�����ʱ �׽��ִ���
		::closesocket(udes->sSocket);
		udes->sSocket = 0;
		udes->pWnd->SendMessage(WM_DL_ERROE , (WPARAM)(udes) , 0);
		delete[] buf;
		buf = NULL;
		return ; 
	}
	nFileLen = *(DWORD*)(PRCMSG_BUFF(buf)->buf);
	}//�����ļ���С

	{//���ļ�
	hFile = ::CreateFile(filePath , GENERIC_WRITE , NULL , NULL ,
			CREATE_ALWAYS , FILE_ATTRIBUTE_NORMAL , NULL);
	if ( INVALID_HANDLE_VALUE == hFile)
	{//�ļ���ʧ��
		::closesocket(udes->sSocket);
		udes->sSocket = 0;
		hFile = NULL;
		udes->pWnd->SendMessage(WM_DL_ERROE , 0 , 0);
		delete[] buf;
		buf = NULL;
		return ;
	}
	}//���ļ�

	DWORD nRcvTotal = 0;//�ܹ����˵����ݴ�С
	DWORD nRcvOne   = 0;//һ�η��͵Ĵ�С

	while((nRcvTotal < nFileLen) && (udes->lThreadState))
	{	
		nRcvOne = ::recv( udes->sSocket , buf , RECV_BUF_SIZE , 0);
		if(SOCKET_ERROR == nRcvOne) 
		{//��������ʱ  �׽��ִ���
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
		{//дʧ��
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
	{//�����׽���ʧ��
		//֪ͨ���������߳�ʧ��
		return 0;
	}
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = 0;

	if(::bind(listenSock , (sockaddr*)&addr , sizeof(addr)) == SOCKET_ERROR)
	{//��ʧ��
		//֪ͨ���������߳�ʧ��
		::closesocket(listenSock);
		return 0;
	}

	{//��ȡ�����˿�
	sockaddr_in lisAddr = {0};
	int len = sizeof(lisAddr);
	getsockname(listenSock , (struct sockaddr*)&lisAddr , &len );
	lisDes->uPort = ntohs(lisAddr.sin_port);
	lisDes->pWnd->PostMessage(WM_UPLOAD_LISTEN_PORT , (WPARAM)(lisDes->uPort) , 0 );
	}

	if(::listen(listenSock,5)==SOCKET_ERROR)
	{//����ʧ��
		//֪ͨ���������߳�ʧ��
		::closesocket(listenSock);
		return 0;
	}

	while (lisDes->lThreadState)
	{
		dlSocket = ::accept(listenSock , (sockaddr*)&addr , &len);
		if(dlSocket == INVALID_SOCKET)
		{//����ʧ��
			::closesocket(listenSock);
			return 0;
		}

		//�����ϴ��߳�
		lisDes->pWnd->PostMessage(WM_CREATE_UPLOAD_THREAD , (WPARAM)(dlSocket) , 0 );
	}

	::closesocket(listenSock);
	return 0;
}

CString GetSizeToString( DWORD len )
{
	CString strSize(_T("0"));

	if(len < 1024)
	{//С��1K
		strSize.Format(_T("%dB") , len);
	}else if (len < (LONGLONG)1024 * 1024)
	{//С��1M
		strSize.Format(_T("%.2fKB") , len / 1024.0);
	} 
	else if (len < 1024 * 1024 * 1024)
	{//С��1G
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
	//����ʾ����
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
	{//����ͼ��ɹ�
		OLE_XSIZE_HIMETRIC hmWidth;
		OLE_YSIZE_HIMETRIC hmHeight;
		pPic->get_Width(&hmWidth);
		pPic->get_Height(&hmHeight);
		if(FAILED(pPic->Render(hDC , viewRect.left , viewRect.top , (DWORD)viewRect.Width() 
			, (DWORD)viewRect.Height(), 0 , hmHeight , hmWidth , -hmHeight , NULL)))  
		{//����ʧ��
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
