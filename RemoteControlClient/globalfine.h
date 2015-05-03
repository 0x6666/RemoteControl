//////////////////////////////////////////////////////////////////////////
//��������

#ifndef _BASIC_DEFINE_H_
#define _BASIC_DEFINE_H_

//�㲥��ѯ����˶�ʱ��
#define BROADCAST_TIMER		1001
//Ҳ�Ƿ����������Ķ�ʱ��
#define HEARTBEAT_TIMER		1002
//��Ļ�궨ʱ��
#define LETTER_RAIN_TIMER	1003

//�׽��ֲ�������
#define WM_SCREEN_SOCKET_ERR		WM_USER + 1
//#define WM_CLOSE_BROADCAST_TIMER	WM_USER + 2 
//������Ϣ
#define WM_NOTIFYION_MSG_C			WM_USER + 3



//�����ļ�ʱ����
//param
//wParam	CString*	ip
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

//////////////////////////////////////////////////////////////////////////
//����һ���׽���
SOCKET GetSocket(CString ip , USHORT port);

//ץ���ֳ�������
typedef struct _CaptureScreenThreadContext
{
	CString strServerIP;		//�����IP��ַ
	USHORT	uScreenPort;		//����Screenͼ��������
	HANDLE	hEvent;				//ͬ���¼�
	HANDLE	hThread;			//�߳̾��
	//ץ���߳�����״̬0->ֹͣ 1->���� 2->��ͣ ����ͨ�����߳��ⲿ������ֵ�������̵߳�����״̬
	volatile long	lThreadState;
	SOCKET	sSocket;			//��������ͼ�����ݵ�socket
	CWnd*	pWnd;				//����������Ϣ�Ĵ���
	volatile long	lScreenW;	//ץ���Ŀ��
	volatile long	lScreenH;	//ץ���ĸ߶�
	volatile long	lFluency;	//ץ����������
	volatile long	lScreenQuality;//ץȡ����Ļ����  [0,100]

}CaptureScreenThreadContext ,*PCaptureScreenThreadContext;


//CMD�߳�������
typedef struct _CMDContext
{
	//cmd�����ܵ�
	//��
	HANDLE hRead;
	//д
	HANDLE hWrite;
	//cmd�ӽ��̾��
	HANDLE hCmd;
	//��ȡcmd�������ݵ��߳� 
	HANDLE hReadCMDThread;
	//�߳�����״̬
	volatile LONG lReadCMDThreadState;
	//���ض�IP
	CString strIP;
}CMDContext ,*PCMDContext;

//ץ����������Ļ����
DWORD WINAPI CaptureScreenThread(LPVOID wParam);

//�������������߳������� 
typedef struct _RecvPushedDesktopThreadContext
{
	HANDLE hThread;
	volatile LONG lThreadState;
	CWnd* pFrame;	//���ƴ���
	CWnd* pScreenView;//��Ҫ����ͼ��Ĵ���
}RecvPushedDesktopThreadContext , *PRecvPushedDesktopThreadContext;
//���շ��������͹����ķ����������߳�
DWORD WINAPI RecvServerPushedDesktopThread(LPVOID wParam);

//��ȡCMD�����
DWORD WINAPI ReadCMDThread(LPVOID wParam);
//ö���ļ���������ļ�
DWORD WINAPI EnumFileThread(LPVOID param);

//ɾ��һ���ļ�����Ŀ¼
BOOL DeleteFileOrDir(CString path);


//�������¼��е�ctrl��shift����״̬
#define MOUSE_TEST_CTRL_SHIFT_D(v)	if((v)& MK_CONTROL) \
										::keybd_event(VK_CONTROL, 0 , 0 , 0);\
									if((v)& MK_SHIFT)\
										::keybd_event(VK_SHIFT , 0 , 0 , 0)
#define MOUSE_TEST_CTRL_SHIFT_U(v)	if((v)& MK_SHIFT)\
										::keybd_event(VK_SHIFT , 0 , KEYEVENTF_KEYUP , 0);\
									if((v)& MK_CONTROL)\
										::keybd_event(VK_CONTROL , 0 , KEYEVENTF_KEYUP , 0)

enum SHUTDOWN{
	SD_SHUTDOWN,	//�ػ�
	SD_RESTART,		//����
	SD_LOGIN_OUT,	//ע��
};
//////////////////////////////////////////////////////////////////////////
//�ػ�
//param
//		sd	��������
BOOL Shutdown(SHUTDOWN sd);


#endif