///////////////////////////////////////////////////////////////
//
// FileName	: publicdef.h 
// Creator	: ����
// Date		: 2013��3��2��, 22:34:29
// Comment	: ����˺Ϳͻ��˹����Ķ���
//
//////////////////////////////////////////////////////////////

#ifndef _PUBLIC_DEFINE_H_
#define _PUBLIC_DEFINE_H_

//////////////////////////////////////////////////////////////////////////
//���ֶ��������˿�

//�������Ϳͻ���С����Ϣ�˿�
#define SERVER_MSG_PORT			19509
#define CLIENT_MSG_PORT			19510

//#define CLIENT_BROADCAST_PORT	19511
#define CLIENT_SCREEN_PORT		19512

//��������������Ķ˿�
#define SERVER_PUSH_DESKTOP_PORT	19513	


//////////////////////////////////////////////////////////////////////////
//�Զ��崰����Ϣ

//�Ѿ���ȡ���ϴ��ļ����˿�
//param
//wParam	USHORT	�˿ں�
#define WM_UPLOAD_LISTEN_PORT  WM_USER + 24

//֪ͨ���ڴ����ϴ��ļ��߳�
//param
//wParam	socket	�Ѿ������˵��׽���
#define WM_CREATE_UPLOAD_THREAD		WM_USER + 25


//////////////////////////////////////////////////////////////////////////
//�ַ��������ʶ
#define STR_RP	_T("{{_STR_}}")

//һЩ�����ļ��еĶ���
//����
#define DIR_DESKTOP	_T("<<DESKTOP>>")
//�ҵ��ĵ�
#define DIR_MY_DOC	_T("<<MY_DOCUMENT>>")
//�ҵĵ���
#define DIR_COMPUTER	_T("<<COMPUTER>>")


//�����ÿ4����һ�οͻ�������״̬����������ͻ���Ӧ����2�뷢��һ��������
#define HEARTBEAT_PACKETS_TIME	(4*1000)


//���ûû����������udp�����ݰ������С(���ϰ�ͷ)64K��65536B��
//����������ʹ��65525�Ľ������ݻ��棬��֤һ�ο��Կ�һ��ȡ��һ��upd�ڲ������е����ݣ��������ݶ�ʧ
#define RECV_BUF_SIZE 65536
//�������͵������������Ϊ60K��15��ҳ�棩����������64k��
#define SEND_BUF_SIZE 61440

//Ĭ�ϵ�ץ������
#define DEFAULT_SCREEN_QUALITY 30

//////////////////////////////////////////////////////////////////////////
//������
enum FrequencyType{
	FT_LEVEL_1  = 1000,	//ÿ��1֡
	FT_LEVEL_2  = 500,	//ÿ��2֡
	FT_LEVEL_3  = 167,	//ÿ��6֡
	FT_LEVEL_4	= 83,	//ÿ��12֡
	FT_LEVEL_5	= 56,	//ÿ��18֡
	FT_LEVEL_6	= 42,	//ÿ��24֡
	FT_LEVEL_7	= 33,	//ÿ��30֡
	FT_MIN_LEVEL = FT_LEVEL_1,
	FT_MAX_LEVEL = FT_LEVEL_7,
	DEFAULT_FREQUENCY_LEVEL = FT_LEVEL_5,
};

//����������ͼ���ʶ
#define ICON_INDEX_S		1234
//�ͻ�������ͼ��
#define ICON_INDEX_C		1235



//////////////////////////////////////////////////////////////////////////
//������(1��2��...)ת����ö��FrequencyTypeֵ
//param
//		v	����
//return 
//		ö��ֵ
FrequencyType IndexToFrequency(int v);

//////////////////////////////////////////////////////////////////////////
//��ö��FrequencyTypeֵת��������(1��2��...)
//param
//		v	ö��ֵ
//return 
//		����
int FrequencyToIndex(FrequencyType v);

//////////////////////////////////////////////////////////////////////////
//��ȡö��ֵ�ľ��庬��
//param
//		v	ö��ֵ
//return 
//		ö��ֵv��Ӧ�ĺ���
const char* FrequencyToText(FrequencyType v);


//////////////////////////////////////////////////////////////////////////
//��ȡ�ƶ�ͼ�������
//param
//		format  ͼ���ʽ��Ӧ�� mine
//		pClsid	��������Ӧ��clsid
//return 
//	-1	����ʧ��
//	������ɹ�
int GetEncoderClsid(IN const WCHAR* format, OUT CLSID* pClsid);

//////////////////////////////////////////////////////////////////////////
//ץȡ����
//param
//		nfW	ץȡ���汣���ͼ��Ŀ��
//		nfH ץȡ���汣���ͼ��ĸ߶�
//		imgData ץ�������ͼ������
//		encoderClsid ץ���󱣴��ͼ��ĸ�ʽ
//		encoderParameters ����ͼ����Ҫʹ�õ�ͼ�����������
void CaptureDesktop(long nfW, long nfH,
					HDC	hDesktop//��������������Էŵ����������Լ���ȡ�ģ����ǻ�ȡ��Ч���е��
					, IN OUT AutoSizeStream* imgData,
					const CLSID& encoderClsid,
					const  EncoderParameters& encoderParameters );

//////////////////////////////////////////////////////////////////////////
//��ʼ��ͼ�����������
//param
//		encoderParameters ��Ҫ��ʼ���ı���������
//		quality			  ����ͼ����Ҫʹ�õ���������
void InitEncoderParameter( EncoderParameters &encoderParameters, ULONG& quality );

//////////////////////////////////////////////////////////////////////////
//ͨ��ip���ǻ�ȡ������
//param
//		strIP	Ҫ��ȡ��������IP��ַ
//return �մ���ʧ��
CString GetHostNameByAddr(const CString& strIP);

//////////////////////////////////////////////////////////////////////////
//�˳��߳�
//param
//		hThread	Ҫ�˳����߳̾��
//		time	�ȴ��̵߳Ľ�ֹʱ��(����)���������ʱ�仹û�˳��Ļ���ǿ�н����߳�
//return 
//		TRUE	����ǿ�н�����
//		FALSE	�߳���ǿ�н�����
BOOL WaiteExitThread( HANDLE hThread , DWORD time);

//��һ��·���滻����DIR_DESKTOP����DIR_MY_DOC��ͷ��·��
//param
//		strPath	��Ҫ��ԭ��·��
//		type	1�滻��DIR_DESKTOP��2 DIR_MY_DOC
CString ReplacePath(CString path , UINT type);

//��ԭһ����DIR_DESKTOP����DIR_MY_DOC��ͷ��Ŀ¼
//param
//		strPath	��Ҫ��ԭ��·��
//		type(���)	0����Ҫ��ԭ��1ԭʼĿ¼��DIR_DESKTOP��2 DIR_MY_DOC
CString RevertPath(CString strPath , UINT& type);

//��һ����DIR_DESKTOP����DIR_MY_DOC��ͷ��Ŀ¼���ɡ����桱���ߡ��ҵ��ĵ���
//param
//		strPath	��Ҫ��ԭ��·��
//		type(���)	0����Ҫ��ԭ��1ԭʼĿ¼��DIR_DESKTOP��2 DIR_MY_DOC
CString TranslatePath(CString strPath , UINT& type);

//��һ��·���л�����ļ���
//param
//		path	·��
//return �ļ������߿մ�
CString GetFileNameFromPath(const CString& path);

//����Сת�����ַ�����С����1024=��1K��
CString GetSizeToString(DWORD len);

//������ϴ������ļ�
DWORD WINAPI DownUploadFileThread_S(LPVOID param);

//�ϴ��ļ� 
DWORD WINAPI DownUploadFileThread_C(LPVOID param);

//�ϴ������߳�
DWORD WINAPI UploadListenerThread(LPVOID param);

//�����ļ��Ľṹ
typedef struct _DowmUploadContext{
	int		nClientID;		//�ͻ���ID
	CString clientIP;		//�ͻ���IP
	CString clientPath;		//��ҪҪ���ص��ļ�
	CString curPath;		//���غ󱣴��·��
	CWnd*	pWnd;	//���ضԻ���
	USHORT  uDownUploadListenPort;//���ؼ����˿�
	volatile LONG lThreadState;//����״̬
	DWORD	fileLen;		//�ļ�����
	DWORD	dlSize;			//�Ѿ������˴�С
	HANDLE	dlThread;		//�����߳̾��
	BOOL	isDownload;		//�Ƿ�Ϊ����

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
//�ϴ��ļ�������
typedef struct _UploadDescripter{
	volatile long lThreadState;	//�߳�״̬
	SOCKET	sSocket;			//�׽���
	HANDLE	hThread;			//�߳̾��
	CWnd*	pWnd;				//���ڽ�����Ϣ�Ĵ���
	CString	filePah;			//Ҫ���ص��ļ�·��
	
	_UploadDescripter()
	: lThreadState(0)
	, sSocket(0)
	, hThread(NULL)
	, pWnd(NULL)
	{}
}UploadDescripter , *PUploadDescripter;

//���ؼ���������
typedef struct  _DownloadListrenDescripter
{
	USHORT	uPort;	//�����˿�
	CWnd*	pWnd;	//��Ϣ������
	volatile long	lThreadState;//�߳�״̬
	HANDLE	hThread;//�߳�

	_DownloadListrenDescripter()
	: uPort(0)
	, pWnd(NULL)
	, lThreadState(0)
	, hThread(NULL)
	{}
}DownloadListrenDescripter , *PDownloadListrenDescripter;

//����һ���ļ� DownUploadFileThread_S��һ����֧
void DownloadFile_S( SOCKET fileSock, PDowmUploadContext ddes );

//�ϴ�һ���ļ� DownUploadFileThread_S��һ����֧
void UploadFile_S( SOCKET fileSock, PDowmUploadContext ddes ) ;

//DownUploadFileThread_C��һ����֧
void UploadFile_C( PUploadDescripter udes );

//DownUploadFileThread_C��һ����֧
void DownloadFile_C(PUploadDescripter udes);

//ʹ��GDI+��ͼ
BOOL DrawPictureWithGDIPP( AutoSizeStream* screenData, HDC hDC, CRect &viewRect );

//ʹ��Ole��ͼ
BOOL DrawPictureWithOle( AutoSizeStream* screenData, HDC hDC, CRect &viewRect );

//��ʾ���Ĵ�С
extern int SCREEN_SIZE_W;
extern int SCREEN_SIZE_H;

#endif