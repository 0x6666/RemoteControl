///////////////////////////////////////////////////////////////
//
// FileName	: publiclib.h 
// Creator	: ����
// Date		: 2013��3��6��, 23:16:42
// Comment	: �ͻ��˺ͷ����ͬʱ��Ҫʹ�õ�һ����
//
//////////////////////////////////////////////////////////////


#ifndef  _PUBLIB_LIB_H_
#define  _PUBLIB_LIB_H_

#include "msg.h"

//#define USE_MSG_QUEUE

//////////////////////////////////////////////////////////////////////////
//��Ϣ��

//��Ϣ�������ӿ�
interface  Dispatcher{
	//////////////////////////////////////////////////////////////////////////
	//��Ϣ����
	//param
	//		msg		�������Ϣ����
	//		ip		���͹���������IP
	//		port	���͹����Ķ˿�
	virtual void DispatchMsg(const void* msg , CString ip, UINT port) =0;
};

class MainSocket;
class MsgQueue; 

class /*PUBLIB_API*/ MsgCenter{

public:
	MsgCenter();

	//////////////////////////////////////////////////////////////////////////
	//��ʼ����Ϣ����
	//param
	//		dispatcher	��Ϣ������
	//		port		�����˿�
	//		reuseaddr	�Ƿ�����SO_REUSEADDR��־
	//		recv		�Ƿ���Ҫ������Ϣ
	//		send		�Ƿ���Ҫ������Ϣ
	BOOL InitMsgCenter(Dispatcher* dispatcher ,UINT port ,BOOL reuseaddr, BOOL recv = TRUE , BOOL send = TRUE);

	//////////////////////////////////////////////////////////////////////////
	//���ù㲥����
	//param
	//		roadcase	TRUE->���ù㲥״̬��FALSE->ȡ���㲥״̬
	//return 
	//		TRUE	���óɹ�
	//		FALSE	����ʧ��
	BOOL SetBroadcast(BOOL roadcase = TRUE);

	//////////////////////////////////////////////////////////////////////////
	//�Ƿ��ǹ㲥������Ϣ
	BOOL IsBroadcast();

	//////////////////////////////////////////////////////////////////////////
	//��Ҫ���͵�������ӵ����ݳ�
	//param
	//		addr	������Ϣ��Ŀ���ַ
	//		port	������Ϣ��Ŀ��˿�
	//		data	Ҫ���͵�����
	//return 
	//		FALSE	����ʧ��
	//		TRUE	�����Ϣ�ɹ�
	BOOL SendMsg(const CString& addr , UINT port , const void* data );

	//////////////////////////////////////////////////////////////////////////
	//�ر���Ϣ����
	void Close();

	//////////////////////////////////////////////////////////////////////////
	//��ȡ��Ϣ������
	Dispatcher* GetDispatcher();

	//////////////////////////////////////////////////////////////////////////
	//������׽���
	MainSocket* GetMainSocket();
	
	//////////////////////////////////////////////////////////////////////////
	//��ȡ��ǰ��Ϣ���ĵ��������Ķ˿�
	//return 
	//		0		��ȡ�˿�ʧ��
	//		����	��ǰ�����Ķ˿�
	USHORT GetMsgPort();

#ifdef USE_MSG_QUEUE

	//////////////////////////////////////////////////////////////////////////
	//��÷�����Ϣ����
	MsgQueue* GetSendMsgQueue();
	
	//////////////////////////////////////////////////////////////////////////
	//�жϷ�����Ϣ���߳��Ƿ�������
	int IsSendThreadRunning();

	//////////////////////////////////////////////////////////////////////////
	//���������Ϣ����
	MsgQueue* GetDispatchMsgQueue();

	//////////////////////////////////////////////////////////////////////////
	//�ж�������Ϣ���߳��Ƿ�������
	int IsDispatchThreadRunning();

	//////////////////////////////////////////////////////////////////////////
	//���������Ϣ���л�����
	void* GetDispatchQueueMutex();

	//////////////////////////////////////////////////////////////////////////
	//���������Ϣ�߳��¼�
	void* GetDispatchThreadEvent();
	void* GetSendQueueMutex();
	void* GetSendThreadEvent();
#endif // USE_MSG_QUEUE

private:

	//�յ�����Ϣ��ǲ�к���
	Dispatcher* m_pDispatch;

	//�׽���
	MainSocket* m_pMainSocket;

#ifdef USE_MSG_QUEUE
	//������Ϣ���У��߳�...
	MsgQueue*	m_pSendQueue;
	void*	m_hSendQueueMutex;
	void*	m_hSendThread;
	void*	m_hSendThreadEvent;
	volatile long m_iSendThreadRunning;


	//������Ϣ���У��߳�...
	MsgQueue*	m_pDispatchQueue;
	void*	m_hDispatchQueueMutex;
	void*	m_hDispatchThreadEvent;
	void*	m_hDispatchThread;
	volatile long m_iDispatchThreadRunning;
#endif
};

#endif
