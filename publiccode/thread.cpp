//////////////////////////////////////////////////////////////////////////
//�߳�
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
			//��ȡ������Ϣ���л�����	
			WaitForSingleObject(hSendQueueMutex , INFINITE);
			if (pSendQueue->empty())
			{//û��Ҫ���͵���Ϣ
				ReleaseMutex(hSendQueueMutex);
				ResetEvent(hSendMsgThreadEvent);
				//�ȴ��ⲿ�����߳�
				WaitForSingleObject(hSendMsgThreadEvent , INFINITE);
				WaitForSingleObject(hSendQueueMutex , INFINITE);
			}
			if(!pCenter->IsSendThreadRunning() || pSendQueue->empty())
			{//��Ҫ�˳��߳�
				goto SEND_EXIT;
			}
			msg = (PSENDMSG)pSendQueue->front();
			pSendQueue->pop();
			ReleaseMutex(hSendQueueMutex);
			break;
		}while(pCenter->IsSendThreadRunning());
		
		if (FALSE == pCenter->IsSendThreadRunning())
		{//�̲߳���Ҫ��������
			goto SEND_EXIT;
		}

		//������Ϣ
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

//������Ϣ�߳�
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
			//��ȡ������Ϣ���л�����	
			WaitForSingleObject(hDispatchQueueMutex , INFINITE);
			if (pDispatchQueue->empty())
			{//û��Ҫ���͵���Ϣ
				ReleaseMutex(hDispatchQueueMutex);
				ResetEvent(hDispatchMsgThreadEvent);
				//�ȴ��ⲿ�����߳�
				WaitForSingleObject(hDispatchMsgThreadEvent , INFINITE);
				WaitForSingleObject(hDispatchQueueMutex , INFINITE);
			}
			if(!pCenter->IsSendThreadRunning() || pDispatchQueue->empty())
			{//��Ҫ�˳��߳�
				AfxMessageBox(_T("������Ϣ�߳��˳�"));
				goto SEND_EXIT;
			}
			msg = (PSENDMSG)pDispatchQueue->front();
			pDispatchQueue->pop();
			ReleaseMutex(hDispatchQueueMutex);
			break;
		}while(pCenter->IsDispatchThreadRunning());

		if (FALSE == pCenter->IsDispatchThreadRunning())
		{//�̲߳���Ҫ��������
			goto SEND_EXIT;
		}

		//������Ϣ
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