//////////////////////////////////////////////////////////////////////////
//线程
#include "stdafx.h"
#include "publiclib.h"
#include "thread.h"
#include "msg.h"
#include "libinnerdef.h"

#ifdef USE_MSG_QUEUE

DWORD WINAPI SendMsgThread(LPVOID param)
{
	MsgCenter* pCenter = (MsgCenter*)param;
	MsgQueue* pSendQueue = pCenter->GetSendMsgQueue();
	void*	hSendQueueMutex = pCenter->GetSendQueueMutex();
	void*	hSendMsgThreadEvent = pCenter->GetSendThreadEvent();
	MainSocket* pSocket = pCenter->GetMainSocket();

	while (pCenter->IsSendThreadRunning())
	{
		PSENDMSG msg = NULL;
		do{
			//获取发送消息队列互斥量	
			WaitForSingleObject(hSendQueueMutex , INFINITE);
			if (pSendQueue->empty())
			{//没有要发送的消息
				ReleaseMutex(hSendQueueMutex);
				ResetEvent(hSendMsgThreadEvent);
				//等待外部唤醒线程
				WaitForSingleObject(hSendMsgThreadEvent , INFINITE);
				WaitForSingleObject(hSendQueueMutex , INFINITE);
			}
			if(!pCenter->IsSendThreadRunning() || pSendQueue->empty())
			{//需要退出线程
				goto SEND_EXIT;
			}
			msg = (PSENDMSG)pSendQueue->front();
			pSendQueue->pop();
			ReleaseMutex(hSendQueueMutex);
			break;
		}while(pCenter->IsSendThreadRunning());
		
		if (FALSE == pCenter->IsSendThreadRunning())
		{//线程不需要在运行了
			goto SEND_EXIT;
		}

		//发送消息
		int res = pSocket->SendTo(msg->msg , PRCMSG(msg->msg)->size , msg->port , msg->ip );
		if (SOCKET_ERROR == res)
		{
			int i = 0;
			i++;
		}
		delete[] (char*)msg->msg;
		delete msg;
		msg = NULL;
	}


SEND_EXIT:

	ReleaseMutex(hSendQueueMutex);
	SetEvent(hSendMsgThreadEvent);

	return 0;
}

//派送消息线程
DWORD WINAPI DispatchMsgThread(LPVOID param)
{
	MsgCenter* pCenter = (MsgCenter*)param;
	MsgQueue* pDispatchQueue = pCenter->GetDispatchMsgQueue();
	void*	hDispatchQueueMutex = pCenter->GetDispatchQueueMutex();
	void*	hDispatchMsgThreadEvent = pCenter->GetDispatchThreadEvent();
	Dispatcher* pDispatcher = pCenter->GetDispatcher();

	while (pCenter->IsDispatchThreadRunning())
	{
		PSENDMSG msg = NULL;
		do{
			//获取派送消息队列互斥量	
			WaitForSingleObject(hDispatchQueueMutex , INFINITE);
			if (pDispatchQueue->empty())
			{//没有要派送的消息
				ReleaseMutex(hDispatchQueueMutex);
				ResetEvent(hDispatchMsgThreadEvent);
				//等待外部唤醒线程
				WaitForSingleObject(hDispatchMsgThreadEvent , INFINITE);
				WaitForSingleObject(hDispatchQueueMutex , INFINITE);
			}
			if(!pCenter->IsSendThreadRunning() || pDispatchQueue->empty())
			{//需要退出线程
				AfxMessageBox(_T("派送消息线程退出"));
				goto SEND_EXIT;
			}
			msg = (PSENDMSG)pDispatchQueue->front();
			pDispatchQueue->pop();
			ReleaseMutex(hDispatchQueueMutex);
			break;
		}while(pCenter->IsDispatchThreadRunning());

		if (FALSE == pCenter->IsDispatchThreadRunning())
		{//线程不需要在运行了
			goto SEND_EXIT;
		}

		//派送消息
		pDispatcher->DispatchMsg( msg->msg , msg->ip , msg->port );
		delete[] msg->msg;
		delete msg;
		msg = NULL;
	}


SEND_EXIT:

	ReleaseMutex(hDispatchQueueMutex);
	SetEvent(hDispatchMsgThreadEvent);

	return 0;
}
#endif // USE_MSG_QUEUE