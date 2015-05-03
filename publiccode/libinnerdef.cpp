
#include "stdafx.h"
#include "libinnerdef.h"

void MainSocket::OnReceive(int nErrorCode)
{//受到的数据由主框架处理

	//消息缓存
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
		{//一个连接断开
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
		{//出错了
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
		{//接收到了数据
			if (nRead == PRCMSG(buf)->size)
			{//是一个有消息
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

	//获得派送消息队列互斥量
	WaitForSingleObject(dispatchQueueMutex , INFINITE);
	//添加消息
	msgQueue->push(msg);
	//释放派送消息队列互斥量
	ReleaseMutex(dispatchQueueMutex );
	//如果有必要的话 唤醒派送消息线程
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
// 		{//需要设置SO_REUSEADDR标记
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