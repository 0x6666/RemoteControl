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
	
	//获取互斥量
	WaitForSingleObject(m_hSendQueueMutex , INFINITE);
	m_pSendQueue->push(smsg);
	ReleaseMutex(m_hSendQueueMutex);

	//如果需要 启动发送线程
	SetEvent(m_hSendThreadEvent);
#else // USE_MSG_QUEUE
	int s = m_pMainSocket->SendTo(data , PRCMSG(data)->size , port , addr);

#endif // USE_MSG_QUEUE
	return TRUE;
}

BOOL MsgCenter::InitMsgCenter( Dispatcher* dispatcher ,UINT port ,BOOL reuseaddr, BOOL recv , BOOL send )
{
	//消息派送器
	if (NULL == dispatcher)
		return FALSE;
	this->m_pDispatch = dispatcher;
	
	//主套接字
	m_pMainSocket = new MainSocket(this);

	if (FALSE == m_pMainSocket->CreateWitheSetReuseaddr( port , SOCK_DGRAM , reuseaddr))
	{//创建套接字失败
		delete m_pMainSocket;
		m_pMainSocket = NULL;
		return FALSE;
	}

#ifdef USE_MSG_QUEUE

	if(TRUE == send){
		//发送消息队列互斥量&发送消息队列
		m_hSendQueueMutex = CreateMutex(NULL , TRUE , NULL/*RCSQ_MUTEX*/);
		if (NULL == m_hSendQueueMutex)
		{//创建互斥量失败
			goto INIT_ERROR;
		}
		m_pSendQueue = new MsgQueue();
		ReleaseMutex(m_hSendQueueMutex);

		//发送消息线程等待
		m_hSendThreadEvent = CreateEvent(NULL , TRUE , TRUE , NULL/*RCSQ_EVENT*/);
		if(NULL == m_hSendThreadEvent )
		{//创建事件失败
			goto INIT_ERROR;
		}

		//创建发送消息线程
		DWORD dwThreadID = 0;
		m_hSendThread = CreateThread(NULL , 0 , SendMsgThread , this , CREATE_SUSPENDED  , &dwThreadID );
		if(NULL == m_hSendThread)
		{//创建线程失败
			goto INIT_ERROR;
		}
	}

	if (TRUE == recv)
	{
		//接收消息队列互斥量&接收消息队列
		m_hDispatchQueueMutex = CreateMutex(NULL , TRUE , NULL/*RCRQ_MUTEX*/);
		if (NULL == m_hDispatchQueueMutex)
		{//创建互斥量失败
			goto INIT_ERROR;
		}
		m_pDispatchQueue = new MsgQueue();
		ReleaseMutex(m_hDispatchQueueMutex);

		//派送消息线程等待
		m_hDispatchThreadEvent = CreateEvent(NULL , TRUE , TRUE , NULL/*RCDQ_EVENT*/);
		if(NULL == m_hDispatchThreadEvent )
		{//创建派送消息线程事件失败
			goto INIT_ERROR;
		}

		//派送消息线程
		DWORD dwThreadID = 0;
		m_hDispatchThread = CreateThread(NULL , 0 , DispatchMsgThread , this , CREATE_SUSPENDED  , &dwThreadID );
		if(NULL == m_hDispatchThread)
		{//创建派送消息线程失败
			goto INIT_ERROR;
		}
	}

	if(TRUE == send)
	{
		//启动发送消息线程
		InterlockedExchange(&m_iSendThreadRunning , 1);
		ResumeThread(m_hSendThread);
	}

	if (TRUE == recv)
	{
		//启动派送消息线程
		InterlockedExchange(&m_iDispatchThreadRunning , 1);
		ResumeThread(m_hDispatchThread);
	}

#endif // // USE_MSG_QUEUE
	return TRUE;
#ifdef USE_MSG_QUEUE
INIT_ERROR:
	m_pDispatch = NULL;
	
	if(NULL != m_pMainSocket) 
	{//销毁套接字
		m_pMainSocket->Close();
		delete m_pMainSocket;
		m_pMainSocket = NULL;
	}

	if(NULL != m_pSendQueue)
	{//发送消息队列
		delete m_pSendQueue;
		m_pSendQueue = NULL;
	}

	if(NULL != m_pDispatchQueue)
	{//接收消息队列
		delete m_pDispatchQueue;
		m_pDispatchQueue = NULL;
	}

	if (NULL != m_hSendQueueMutex)
	{//互斥量
		ReleaseMutex(m_hSendQueueMutex);
		CloseHandle(m_hSendQueueMutex);
		m_hSendQueueMutex = NULL;
	}

	if (m_iSendThreadRunning == 1)
	{//发送消息线程已经运行了
		InterlockedExchange(&m_iSendThreadRunning , 0);
		SetEvent(m_hSendThreadEvent);
		CloseHandle(m_hSendThread);
		m_hSendThread = NULL;
	}

	if (NULL != m_hSendThreadEvent)
	{//发送消息线程等待事件
		SetEvent(m_hSendThreadEvent);
		CloseHandle(m_hSendThreadEvent);
		m_hSendThreadEvent = NULL;
	}

	if (NULL != m_hDispatchQueueMutex)
	{//接受消息队列互斥量
		ReleaseMutex(m_hDispatchQueueMutex);
		CloseHandle(m_hDispatchQueueMutex);
		m_hDispatchQueueMutex = NULL;
	}

	if (m_iDispatchThreadRunning == 1)
	{//发送消息线程已经运行了
		InterlockedExchange(&m_iDispatchThreadRunning , 0);
		SetEvent(m_hDispatchThreadEvent);
		CloseHandle(m_hDispatchThread);
		m_hDispatchThread = NULL;
	}

	if (NULL != m_hDispatchThreadEvent)
	{//发送消息线程等待事件
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
		//获得发送消息队列互斥量
		WaitForSingleObject(m_hSendQueueMutex , INFINITE);
		//释放发发送消息队列
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
		//释放发送消息队列互斥量
		ReleaseMutex(m_hSendQueueMutex);
		//关闭互斥量句柄
		CloseHandle(m_hSendQueueMutex);
		m_hSendQueueMutex = NULL;
		SetEvent(m_hSendThreadEvent);
		CloseHandle(m_hSendThreadEvent);
		m_hSendThreadEvent = NULL;
	}

	if (NULL != m_hDispatchQueueMutex)
	{
		InterlockedExchange(&m_iDispatchThreadRunning , 0);
		//获得派送消息队列互斥量
		WaitForSingleObject(m_hDispatchQueueMutex , INFINITE);
		//释派送消息队列
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
		//释放派送消息队列互斥量
		ReleaseMutex(m_hDispatchQueueMutex);
		//关闭互斥量句柄
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
	{//套接字貌似还没创建
		return  0;
	}

	if (m_pMainSocket->GetSockName(strIP , port))
	{//已经获取了监听端口
		return (USHORT)port;
	}
	else
	{//获取端口失败
		return 0;
	}
}