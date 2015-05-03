///////////////////////////////////////////////////////////////
//
// FileName	: globalfine.h 
// Creator	: ����
// Date		: 2013��2��28��, 10:48:36
// Comment	: ����˵�ȫ������
//
//////////////////////////////////////////////////////////////

#ifndef _BASIC_DEFINE_H_
#define _BASIC_DEFINE_H_

//////////////////////////////////////////////////////////////////////////
//�Զ���Windows��Ϣ

//�ҵ�һ���ͻ���
#define WM_FOUND_A_CLIENT		WM_USER + 1
//�Ի�ȡһ�����ص�screen�˿�
#define WM_GET_A_SCREEN_PORT	WM_USER + 2
//����screen�׽���ʧ��
#define WM_SCREEN_SOCKET_ERR	WM_USER + 3
// screen�ͻ��������Ѿ��Ͽ�
// #define WM_SCREEN_SOCK_UNCON	WM_USER + 4
//screen�̶߳Ͽ�
#define WM_SCREEN_THREAD_EXIT	WM_USER + 5
//�ȴ��ͻ���ץ���̳߳�ʱ
#define WM_WAIT_FOR_CAPTURE_TIMEOUT WM_USER + 6 
//�ͻ���itemView˫���¼� �����Ϣֻ��send��������post
#define WM_ITEM_VIEW_SBCLICK	WM_USER + 7 
//����Screen����
#define WM_CHANGE_SCREEN_AUALITY	WM_USER + 8
//����Screen����
#define WM_CHANGE_SCREEN_FLUENCY	WM_USER + 9
//���������Ļ�Ŀ���
#define WM_SCREEN_CTRL				WM_USER + 10
//������Ϣ
#define WM_SEND_MSG					WM_USER + 11
//ֹͣ���
//#define WM_STOP_CTRL				WM_USER + 12
//ֹͣ����һ���ͻ���
#define WM_MONITORINT_CLIENT			WM_USER + 13
//��CMD����
#define WM_CMD_CLICKED				WM_USER + 14
//������Ϣ
#define WM_NOTIFYION_MSG_S			WM_USER + 15
//֪ͨ�����ڿͻ��������Լ�������
//param
//		wParam	�ͻ�������
//		lParam	��������
#define WM_CLIENT_COUNT				WM_USER + 16

//ȫ����ʾ
//wparam	��CRemoteControlView���մ���Ϣ�ǣ�����CString*
//			��ClientItemView��CMainWnd���մ���Ϣʱ������BOOL����ȫ��/ȡ��ȫ����
#define WM_FULLS_CREEN				WM_USER + 17
//�ļ�����
#define WM_DOC_MANAGE_CLICKED		WM_USER + 18

//�����ļ�ʱ�׽��ִ���
//param
//wParam	PDowmloadDescripter	������������ַ
#define WM_DL_ERROE			WM_USER + 19

//�����ļ�ʱ�����ؽ���
//param	
//wParam	CString* ip
//lParam	DWORD	�Ѿ������˵�����
#define WM_DL_PROGRESS				WM_USER + 20

//�����߳��˳�
//param
//wParam	CString* ip
#define WM_DL_THREAD_EXIT			WM_USER + 21

//Ҫ���ص��ļ���С
//param
//wParam	CString* ip
//lParam	DWORD	�ļ���С
#define WM_DL_FILE_SIZE				WM_USER + 22

//֪ͨ״̬���ı�����������ʾ
//param
//	wParam	int �����͵Ŀͻ�������
#define WM_PUSHED_COUNT				WM_USER + 23

//֪ͨ״̬������һ����Ϣ
//param
//	wParam (CString*) Ҫ��ʾ����Ϣ
#define WM_MESSAGE					WM_USER + 24

//��ȡ���λ��
#define GET_MOUSE_POS_TIMER		1001
//���ͻ���״̬��ʱ��
#define CHECK_CLIENT_STATE_TIME	1002

//�����ļ���
#define CONFIG_FILE_NAME	"rcconfigfile.cfg"

//{{�����ļ��еļ�

//��������ʱץ���ĳ��Ϳ�
#define CK_PUSH_SD_W		"Push_DesktopW"
#define CK_PUSH_SD_H		"Push_DesktopH"
#define CK_PUSH_SD_QUALITY	"Push_DesktopQuality"
#define CK_PUSH_SD_FREQUNCY	"Push_DesktopFrequency"
#define CK_PUSH_SD_LOCK_RATIO	"Push_LockScreenSizeRatio"

//CMD����
#define CK_CMD_FONT_COLOR	"CMD_FontColor"
#define CK_CMD_BG_COLOR		"CMD_BackgroundColor"
#define CK_CMD_FONT_SIZE	"CMD_FontSize"
#define CK_CMD_FONT_CHAR_SET	"CMD_FontCharset"
#define CK_CMD_FONT_FACENAME	"CMD_FontFaceName"

//}}�����ļ��еļ�
//���͵������ļ�ֵ����
typedef struct _PUSH_CFG_V{
	LONG nPush_DesktopW;
	LONG nPush_DesktopH;
	LONG nPush_DesktopQuality;
	LONG nPush_DesktopFrequency;
	BOOL bPush_LockScreenSizeRatio;
}PUSH_CFG_V , *PPUSH_CFG_V;

//CMD�������ļ�ֵ����
typedef struct _CMD_CFG_V{
	LONG nCMD_FontSize;
	BYTE nCMD_FontCharset;
	DWORD nCMD_FontColor;
	DWORD nCMD_BackgroundColor;
	char czCMD_FontFaceName[32];
}CMD_CFG_V , *PCMD_CFG_V;


//���ղ���ʾ���ض�ͼ����߳�������
typedef struct _PrintScreenThreadContext{
	CWnd* pWnd;		//������Ϣ�Ĵ���
	//�߳�����״̬ 0->ֹͣ��1->����
	volatile long lThreadState;
	//����Զ��������߳̾��
	HANDLE	hThread;
	//�߳�ͬ����Ҫʹ�õ��¼����
	HANDLE	hThreadEvent;
	//CString	strIP;//���ض�IP
	SOCKET	sSocket;//�������ݵ�socket

}PrintScreenThreadContext,*PPrintScreenThreadContext;

//���շ�������ͼ�񣬲���ʾ����
DWORD WINAPI PrintScreenThread(LPVOID wParam);


//���ͷ����������߳�������
typedef struct _PushServerDesktopThreadContext
{
	//��Ҫ���͵Ŀͻ��˵�ַ����
	typedef std::list<sockaddr_in*> PushedClientList;

	//���ͷ����������߳�
	volatile LONG lThreadState;
	//�߳̾��
	HANDLE		hThread;

	//���ͷ���������ʱ��ץ����С
	volatile LONG lDesktopW;
	volatile LONG lDesktopH;

	//���ͷ���������ʱ��ץ������
	volatile LONG lDesktopQuality;
	//���ͷ���������ʱ��������
	volatile LONG lDesktopFrequency;
	
	//��Ҫ��Ҫ��Ҫ���͵Ŀͻ�������
	PushedClientList lstPushedClient;
	//��Ҫ��Ҫ��Ҫ���͵Ŀͻ�����������
	HANDLE hPushedClientListMutex;
}PushServerDesktopThreadContext , *PPushServerDesktopThreadContext;

//���ͷ����������߳�
DWORD WINAPI PushSeverDesktopThread(LPVOID wParam);

//���ͻ����߳�������
typedef struct _CheckClientThreadContext{
	CDocument*	pDoc;		//�ĵ�ָ��
	HANDLE		hThread;	//�߳̾��
	volatile LONG iThreadState;//�߳�����״̬
}CheckClientThreadContext , *PCheckClientThreadContext;
//////////////////////////////////////////////////////////////////////////
//���ڼ��������е�������
DWORD WINAPI CheckClientThread(LPVOID param);

//���ĳ�����İ���״̬
#define KEY_DOWN(vk) (0x8000&GetKeyState(vk))

#endif