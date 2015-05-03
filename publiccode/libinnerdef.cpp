
#include "stdafx.h"
#include "libinnerdef.h"

void MainSocket::OnReceive(int nErrorCode)
{//�ܵ�������������ܴ���

	//��Ϣ����
	char buf[1024] = {0};
	int nRead;
	CString strIP;
	UINT nPort = 0;
	nRead = ReceiveFrom(buf, 1024 ,strIP , nPort);
#ifdef USE_MSG_QUEUE
	PSENDMSG msg = new SENDMSG();
	msg->ip = strIP;
	msg->port = nPort;
#endif // USE_MSG_QUEUE
	switch (nRead)
	{
	case 0:
		{//һ�����ӶϿ�
#ifdef USE_MSG_QUEUE
			PRCMSG pMsg = new RCMSG;
			pMsg->size = sizeof(RCMSG);
			pMsg->type = MT_CONN_CLOSED_C;
			msg->msg = pMsg;
#else
			RCMSG pMsg = {0};
			pMsg.size = sizeof(RCMSG);
			pMsg.type = MT_CONN_CLOSED_C;
			m_pCenter->GetDispatcher()->DispatchMsg(buf , strIP , nPort);
#endif USE_MSG_QUEUE

		}
		break;
	case SOCKET_ERROR:
		{//������
			if (GetLastError() != WSAEWOULDBLOCK) 
			{
#ifdef USE_MSG_QUEUE
				PRCMSG pMsg = new RCMSG;
				pMsg->size = sizeof(RCMSG);
				pMsg->type = MT_RECV_ERROR;
				msg->msg = pMsg;
#else
				RCMSG pMsg = {0};
				pMsg.size = sizeof(RCMSG);
				pMsg.type = MT_RECV_ERROR;
				m_pCenter->GetDispatcher()->DispatchMsg(&pMsg , strIP , nPort);
#endif USE_MSG_QUEUE
			}
			else
			{
				ASSERT(FALSE);
			}
		}
		break;
	default:
		{//���յ�������
			if (nRead == PRCMSG(buf)->size)
			{//��һ������Ϣ
#ifdef USE_MSG_QUEUE
				char* data = new char[nRead];
				memcpy(data , buf , nRead);
				msg->msg = data;
#else  // USE_MSG_QUEUE
				m_pCenter->GetDispatcher()->DispatchMsg(buf , strIP , nPort);
#endif// USE_MSG_QUEUE
			}
			else
			{
				ASSERT(FALSE);
			}
		}
	}
	CAsyncSocket::OnReceive(nErrorCode);

#ifdef USE_MSG_QUEUE
	MsgQueue* msgQueue = m_pCenter->GetDispatchMsgQueue();
	void* dispatchQueueMutex = m_pCenter->GetDispatchQueueMutex();
	void* pDispatchThreadEvent = m_pCenter->GetDispatchThreadEvent();

	//���������Ϣ���л�����
	WaitForSingleObject(dispatchQueueMutex , INFINITE);
	//�����Ϣ
	msgQueue->push(msg);
	//�ͷ�������Ϣ���л�����
	ReleaseMutex(dispatchQueueMutex );
	//����б�Ҫ�Ļ� ����������Ϣ�߳�
	SetEvent(pDispatchThreadEvent);
#endif // USE_MSG_QUEUE
}

BOOL MainSocket::CreateWitheSetReuseaddr(UINT nSocketPort, int nSocketType, BOOL reuseaddr ,
			long lEvent, LPCTSTR lpszSocketAddress)
{
	if (Socket(nSocketType, lEvent))
	{

		int opt = 1;
// 		if (reuseaddr)
// 		{//��Ҫ����SO_REUSEADDR���
// 			BOOL res = this->SetSockOpt(SO_REUSEADDR , &opt , sizeof(opt) , SOL_SOCKET);
// 			if (res == FALSE)
// 			{
// 				opt = GetLastError();
// 				WSASetLastError( opt );
// 				return FALSE;
// 			}
// 		}


		if (Bind(nSocketPort,lpszSocketAddress))
			return TRUE;
		opt = GetLastError();
		Close();
		WSASetLastError(opt);
	}
	return FALSE;
}