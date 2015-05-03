///////////////////////////////////////////////////////////////
//
// FileName	: msg.h 
// Creator	: ����
// Date		: 2013��3��6��, 23:16:42
// Comment	: ��Ҫ������Ϣ������ݽṹ�ͺ�ȣ������ض˺ͱ��ض�ʹ��
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

//1�ֽڶ��䣬�������ٴ�������ݴ�С
#pragma pack(push , 1)

//��Ϣͷ�Ķ���
typedef struct _RCMSG{
	//��Ϣ����  RCMSG_TYPE
	WORD type;
	//��Ϣ���ݽṹ�Ĵ�С
	WORD size;
}RCMSG , *PRCMSG;

typedef struct _RCMSG_BUF {
	RCMSG msgHead;
	char	buf[1];
}RCMSG_BUFF , *PRCMSG_BUFF;

//�����Ϣ
typedef struct _MOUSE_EVENT_MSG{
	RCMSG msgHead;	//��Ϣͷ
	UINT message;	//�����Ϣ���ͣ���WM_MOVE��
	UINT param;		//Windows��Ϣ�ṹMSG::wParam 
	int x;			//��ǰ������������ϵͳ�ĺ���λ��
	int y;			//��ǰ������������ϵͳ������λ��
}MOUSE_EVENT_MSG, *PMOUSE_EVENT_MSG;

//�����¼�
typedef struct _KEYBD_EVENT_MSG{
	RCMSG msgHead;	//��Ϣ��
	UINT message;	//��Ϣ������WM_KEYUP
	UINT vKey;		//���������
	BYTE keyState;//��������״̬����Ҫ���ڴ�����ϼ�KS_CTRL��KS_SHIFT��KS_ALT�����
}KEYBD_EVENT_MSG , *PKEYBD_EVENT_MSG;
#define KS_CTRL		0x0001		//ctrl������
#define KS_SHIFT	0x0002		//ctrl������
#define KS_ALT		0x0004		//alt������

//����ץͼ�Ĵ�С
typedef struct _RCMSG_SCREEN_SIZE{
	RCMSG msgHead;
	long x;
	long y;
}RCMSG_SCREEN_SIZE , *PRCMSG_SCREEN_SIZE;

//�㲥��Ϣ����ѯ������
typedef struct _RCMSG_BROADCAST_MSG{
	RCMSG	msgHead;
	USHORT	port;		//��ǰ�ͻ��˼����Ķ˿�
	char	name[1];	//��ǰ�ͻ��˵ļ������
}RCMSG_BROADCAST_MSG , *PRCMSG_BROADCAST_MSG;

//ʹ��ԭ�еĶ��䷽ʽ
#pragma pack(pop)

//��һ����������Ϣ���ͱ���
#define DEF_RCMSG(_name , _type) 	RCMSG _name  = {0};			\
									_name.size = sizeof(RCMSG);	\
									_name.type = _type


//////////////////////////////////////////////////////////////////////////
//��Ϣ����
enum RCMSG_TYPE{
	MT_MIN = 0,							//��Ϣֵ����Сֵ���ⲻ����Ϣ����

	MT_CONN_CLOSED_C,					//no param				�����ѽ��Ͽ�
	MT_FIND_SERVER_BROADCAST_C,			//RCMSG_BROADCAST_MSG	�ͻ��˲�ѯ
	MT_RECV_ERROR,						//no param				��������ʱ����
	MT_HEARTBEAT_C,						//no param				�ͻ�����������Ϣ
	MT_SERVER_EXIT_S,					//no param				����������
	MT_REPEAT_FIND_SERVER_S,			//no param				�ظ�MT_HEARTBEAT_C(��ΪMT_HEARTBEAT_C���ù㲥���͵ģ�
										//						�ͻ����ǲ�֪����������ַ�ģ����Ի���Ҫ�ظ�һ�£�����
										//						�ͻ��˻�ȡ�������ĵ�ַ)
	MT_SCREEN_PORT_S,					//USHORT				����˵�screen�˿ڣ���ظ�MT_SCREEN_CAPTURE_START_C 
	MT_SCREEN_CAPTURE_START_C,			//no param				�ͻ����Ѿ���ʼץ����
	MT_SCREEN_DATA_SIZE_C,				//UINT					screen���ݵĴ�С���ǿ�����Ϣ		
	MT_SCREEN_QUALITY_S,				//LONG					ץ��������
	MT_SCREEN_FLUENCY_S,				//LONG					����SCREEN��������
	MT_STOP_SCREEN_CAPTURE_S,			//no param				ֹͣץ��
	MT_SCREEN_CTRL_S,					//char					screen����
	MT_MOUSE_EVENT_S,					//MOUSE_EVENT_MSG		����¼�
	MT_KETBD_EVENT_S,					//KEYBD_EVENT_MSG		�����¼�
	MT_SCREEN_SIZE_S,					//RCMSG_SCREEN_SIZE		����ץ���Ĵ�С
	MT_PUSH_SERVER_DESKTOP_S,			//no param				���ͷ���������
	MT_PUSH_SERVER_DESKTOP_C,			//no param				ȷ�����ͷ��������棨���ڻظ�MT_PUSH_SERVER_DESKTOP_S��
	MT_CALCEL_PUSH_DESKTOP_S,			//no param				ȡ�����ͷ��������� 
	MT_CALCEL_PUSH_DESKTOP_C,			//no param				ȷ��ȡ���������ͣ��ظ�MT_CALCEL_PUSH_DESKTOP_S��
	MT_EXIT_CLIENT_S,					//no param				�˳��ͻ���
	MT_SHUTDOWN_CLIENT_S,				//no param				�ͻ��˹ػ�
	MT_SHUTDOWN_FALIED_C,				//no param				�ػ�ʧ��
	MT_RESTART_CLIENT_S,				//no param				�ͻ�������
	MT_RESTART_FALIED_C,				//no param				����ʧ��
	MT_LOGIN_OUT_CLIENT_S,				//no param				�ͻ���ע��
	MT_LOGIN_OUT_FALIED_C,				//no param				ע��ʧ��
	MT_LETTER_RAIN_S,					//BYTE					��ĸ��
	MT_START_CMD_S,						//no param				����CMD
	MT_START_CMD_FAILED_C,				//no param				����CMDʧ��
	MT_CMD_LINE_DATA_C,					//char* ������			����������
	MT_HANDS_UP_C,						//no param				�ͻ��˾���
	MT_CMD_S,							//char* ������			һ��CMD����
	MT_CMD_FAILED_C,					//char* ������			CMD����ִ��ʧ��
	MT_GET_FILE_LIST_S,					//char* ������			����ļ��б�����
	MT_FILE_PATH_C,						//char* ������			һ���ļ������ļ���·��
	MT_GET_DRIVER_S,					//no param				��ȡ������������Ϣ
	MT_DELETE_FILE_S,					//char* ������			ɾ���ļ������ļ���
	MT_DELETE_FILE_FAILED_C,			//char* ������			ɾ���ļ������ļ���ʧ��
	MT_DELETE_FILE_SUCCESS_C,			//char* ������			ɾ���ļ������ļ��гɹ�
	MT_LISTEN_DOWNLOAD_PORT_C,			//SHORT					�������ض˿�
	MT_DOWNLOAD_FILE_PATH_S,			//char* ������			Ҫ���ص��ļ�·��(�ϴ������߳�)
	MT_DOWNLOAD_FILE_SIZE_S,			//DWORD					Ҫ���ص��ļ���С
	MT_DU_DOWNLOAD_S,					//BYTE					�����ļ����ϴ������̣߳�
	MT_PAUSE_SCREEN_S,					//no param				��ͣ������Ļͼ��
	MT_RESUME_SCREEN_S,					//no param				���¿�ʼ������Ļͼ��

	MT_MAX,								//��Ϣֵ�����ֵ�ⲻ����Ч����Ϣ����
};

#endif