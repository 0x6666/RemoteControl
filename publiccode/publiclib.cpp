//////////////////////////////////////////////////////////////////////////
//
#include "stdafx.h"
#include "publiclib.h"
#include "msg.h"
#include "thread.h"
#include "libinnerdef.h"



MsgCenter::MsgCenter()
:m_pDispatch(NULL)
,m_pMainSocket(NULL)
#ifdef USE_MSG_QUEUE
,m_pSendQueue(NULL)
,m_hSendQueueMutex(NULL)
,m_hSendThread(NULL)
,m_hSendThreadEvent(NULL)
,m_iSendThreadRunning(0)
,m_pDispatchQueue(NULL)
,m_hDispatchQueueMutex(NULL)
,m_hDispatchThreadEvent(NULL)
,m_hDispatchThread(NULL)
,m_iDispatchThreadRunning(0)
#endif //USE_MSG_QUEUE
{
}

BOOL MsgCenter::SendMsg( const CString& addr , UINT port , const  void* data )
{
	if(PRCMSG(data)->type <= MT_MIN || PRCMSG(data)->type >= MT_MAX)
	{
		ASSERT(FALSE);
	}
#ifdef USE_MSG_QUEUE

	char* buf = new char[PRCMSG(data)->size];
	memcpy(buf , data , PRCMSG(data)->size);

	PSENDMSG smsg = new SENDMSG();
	smsg->ip = addr;
	smsg->port = port;
	smsg->msg = buf;
	
	//��ȡ������
	WaitForSingleObject(m_hSendQueueMutex , INFINITE);
	m_pSendQueue->push(smsg);
	ReleaseMutex(m_hSendQueueMutex);

	//�����Ҫ ���������߳�
	SetEvent(m_hSendThreadEvent);
#else // USE_MSG_QUEUE
	int s = m_pMainSocket->SendTo(data , PRCMSG(data)->size , port , addr);

#endif // USE_MSG_QUEUE
	return TRUE;
}

BOOL MsgCenter::InitMsgCenter( Dispatcher* dispatcher ,UINT port ,BOOL reuseaddr, BOOL recv , BOOL send )
{
	//��Ϣ������
	if (NULL == dispatcher)
		return FALSE;
	this->m_pDispatch = dispatcher;
	
	//���׽���
	m_pMainSocket = new MainSocket(this);

	if (FALSE == m_pMainSocket->CreateWitheSetReuseaddr( port , SOCK_DGRAM , reuseaddr))
	{//�����׽���ʧ��
		delete m_pMainSocket;
		m_pMainSocket = NULL;
		return FALSE;
	}

#ifdef USE_MSG_QUEUE

	if(TRUE == send){
		//������Ϣ���л�����&������Ϣ����
		m_hSendQueueMutex = CreateMutex(NULL , TRUE , NULL/*RCSQ_MUTEX*/);
		if (NULL == m_hSendQueueMutex)
		{//����������ʧ��
			goto INIT_ERROR;
		}
		m_pSendQueue = new MsgQueue();
		ReleaseMutex(m_hSendQueueMutex);

		//������Ϣ�̵߳ȴ�
		m_hSendThreadEvent = CreateEvent(NULL , TRUE , TRUE , NULL/*RCSQ_EVENT*/);
		if(NULL == m_hSendThreadEvent )
		{//�����¼�ʧ��
			goto INIT_ERROR;
		}

		//����������Ϣ�߳�
		DWORD dwThreadID = 0;
		m_hSendThread = CreateThread(NULL , 0 , SendMsgThread , this , CREATE_SUSPENDED  , &dwThreadID );
		if(NULL == m_hSendThread)
		{//�����߳�ʧ��
			goto INIT_ERROR;
		}
	}

	if (TRUE == recv)
	{
		//������Ϣ���л�����&������Ϣ����
		m_hDispatchQueueMutex = CreateMutex(NULL , TRUE , NULL/*RCRQ_MUTEX*/);
		if (NULL == m_hDispatchQueueMutex)
		{//����������ʧ��
			goto INIT_ERROR;
		}
		m_pDispatchQueue = new MsgQueue();
		ReleaseMutex(m_hDispatchQueueMutex);

		//������Ϣ�̵߳ȴ�
		m_hDispatchThreadEvent = CreateEvent(NULL , TRUE , TRUE , NULL/*RCDQ_EVENT*/);
		if(NULL == m_hDispatchThreadEvent )
		{//����������Ϣ�߳��¼�ʧ��
			goto INIT_ERROR;
		}

		//������Ϣ�߳�
		DWORD dwThreadID = 0;
		m_hDispatchThread = CreateThread(NULL , 0 , DispatchMsgThread , this , CREATE_SUSPENDED  , &dwThreadID );
		if(NULL == m_hDispatchThread)
		{//����������Ϣ�߳�ʧ��
			goto INIT_ERROR;
		}
	}

	if(TRUE == send)
	{
		//����������Ϣ�߳�
		InterlockedExchange(&m_iSendThreadRunning , 1);
		ResumeThread(m_hSendThread);
	}

	if (TRUE == recv)
	{
		//����������Ϣ�߳�
		InterlockedExchange(&m_iDispatchThreadRunning , 1);
		ResumeThread(m_hDispatchThread);
	}

#endif // // USE_MSG_QUEUE
	return TRUE;
#ifdef USE_MSG_QUEUE
INIT_ERROR:
	m_pDispatch = NULL;
	
	if(NULL != m_pMainSocket) 
	{//�����׽���
		m_pMainSocket->Close();
		delete m_pMainSocket;
		m_pMainSocket = NULL;
	}

	if(NULL != m_pSendQueue)
	{//������Ϣ����
		delete m_pSendQueue;
		m_pSendQueue = NULL;
	}

	if(NULL != m_pDispatchQueue)
	{//������Ϣ����
		delete m_pDispatchQueue;
		m_pDispatchQueue = NULL;
	}

	if (NULL != m_hSendQueueMutex)
	{//������
		ReleaseMutex(m_hSendQueueMutex);
		CloseHandle(m_hSendQueueMutex);
		m_hSendQueueMutex = NULL;
	}

	if (m_iSendThreadRunning == 1)
	{//������Ϣ�߳��Ѿ�������
		InterlockedExchange(&m_iSendThreadRunning , 0);
		SetEvent(m_hSendThreadEvent);
		CloseHandle(m_hSendThread);
		m_hSendThread = NULL;
	}

	if (NULL != m_hSendThreadEvent)
	{//������Ϣ�̵߳ȴ��¼�
		SetEvent(m_hSendThreadEvent);
		CloseHandle(m_hSendThreadEvent);
		m_hSendThreadEvent = NULL;
	}

	if (NULL != m_hDispatchQueueMutex)
	{//������Ϣ���л�����
		ReleaseMutex(m_hDispatchQueueMutex);
		CloseHandle(m_hDispatchQueueMutex);
		m_hDispatchQueueMutex = NULL;
	}

	if (m_iDispatchThreadRunning == 1)
	{//������Ϣ�߳��Ѿ�������
		InterlockedExchange(&m_iDispatchThreadRunning , 0);
		SetEvent(m_hDispatchThreadEvent);
		CloseHandle(m_hDispatchThread);
		m_hDispatchThread = NULL;
	}

	if (NULL != m_hDispatchThreadEvent)
	{//������Ϣ�̵߳ȴ��¼�
		SetEvent(m_hDispatchThreadEvent);
		CloseHandle(m_hDispatchThreadEvent);
		m_hDispatchThreadEvent = NULL;
	}
#endif // USE_MSG_QUEUE
	return TRUE;
}

void MsgCenter::Close()
{
	m_pDispatch = NULL;
	if (NULL != m_pMainSocket)
	{
		m_pMainSocket->Close();
		delete m_pMainSocket;
		m_pMainSocket = NULL;
	}
#ifdef USE_MSG_QUEUE
	if (m_hSendQueueMutex != NULL)
	{
		InterlockedExchange(&m_iSendThreadRunning , 0);
		//��÷�����Ϣ���л�����
		WaitForSingleObject(m_hSendQueueMutex , INFINITE);
		//�ͷŷ�������Ϣ����
		int size = m_pSendQueue->size();
		while ( 0 < size-- )
		{
			PSENDMSG msg = (PSENDMSG)m_pSendQueue->front();
			if (NULL != msg)
			{
				delete[] msg->msg;
				delete msg;
			}
			m_pSendQueue->pop();
		}
		//�ͷŷ�����Ϣ���л�����
		ReleaseMutex(m_hSendQueueMutex);
		//�رջ��������
		CloseHandle(m_hSendQueueMutex);
		m_hSendQueueMutex = NULL;
		SetEvent(m_hSendThreadEvent);
		CloseHandle(m_hSendThreadEvent);
		m_hSendThreadEvent = NULL;
	}

	if (NULL != m_hDispatchQueueMutex)
	{
		InterlockedExchange(&m_iDispatchThreadRunning , 0);
		//���������Ϣ���л�����
		WaitForSingleObject(m_hDispatchQueueMutex , INFINITE);
		//��������Ϣ����
		int size = m_pDispatchQueue->size();
		while ( 0 < size-- )
		{
			PSENDMSG msg = (PSENDMSG)m_pDispatchQueue->front();
			if (NULL != msg)
			{
				delete[] msg->msg;
				delete msg;
			}
			m_pDispatchQueue->pop();
		}
		//�ͷ�������Ϣ���л�����
		ReleaseMutex(m_hDispatchQueueMutex);
		//�رջ��������
		CloseHandle(m_hDispatchQueueMutex);
		m_hDispatchQueueMutex = NULL;
		SetEvent(m_hDispatchThreadEvent);
		CloseHandle(m_hDispatchThreadEvent);
		m_hDispatchThreadEvent = NULL;
	}
#endif // USE_MSG_QUEUE
}

Dispatcher* MsgCenter::GetDispatcher()
{
	return m_pDispatch;
}
MainSocket* MsgCenter::GetMainSocket()
{
	return m_pMainSocket;
}
BOOL MsgCenter::SetBroadcast( BOOL roadcase)
{
	return this->m_pMainSocket->SetSockOpt( SO_BROADCAST, &roadcase, sizeof(BOOL), SOL_SOCKET);
}

BOOL MsgCenter::IsBroadcast()
{
	BOOL roadcase = FALSE;
	if(TRUE == this->m_pMainSocket->SetSockOpt( SO_BROADCAST, &roadcase, sizeof(BOOL), SOL_SOCKET))
	{
		return roadcase;
	}
	return FALSE;
}

#ifdef USE_MSG_QUEUE
MsgQueue* MsgCenter::GetSendMsgQueue()
{
	return m_pSendQueue;
}

int MsgCenter::IsSendThreadRunning()
{
	return m_iSendThreadRunning;
}

MsgQueue* MsgCenter::GetDispatchMsgQueue()
{
	return m_pDispatchQueue;
}

int MsgCenter::IsDispatchThreadRunning()
{
	return m_iDispatchThreadRunning;
}

void* MsgCenter::GetDispatchQueueMutex()
{
	return m_hDispatchQueueMutex;
}

void* MsgCenter::GetDispatchThreadEvent()
{
	return m_hDispatchThreadEvent;
}

void* MsgCenter::GetSendQueueMutex()
{
	return m_hSendQueueMutex;
}

void* MsgCenter::GetSendThreadEvent()
{
	return m_hSendThreadEvent;
}




#endif // USE_MSG_QUEUE

USHORT MsgCenter::GetMsgPort()
{
	CString strIP;
	UINT port = 0;

	if (NULL == m_pMainSocket)
	{//�׽���ò�ƻ�û����
		return  0;
	}

	if (m_pMainSocket->GetSockName(strIP , port))
	{//�Ѿ���ȡ�˼����˿�
		return (USHORT)port;
	}
	else
	{//��ȡ�˿�ʧ��
		return 0;
	}
}