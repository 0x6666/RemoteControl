//////////////////////////////////////////////////////////////////////////
//库中的内部定义

#ifndef _LIB_INNER_DEF_H_
#define _LIB_INNER_DEF_H_

#include "msg.h"

#include <queue>
#include <string>
#include <afxsock.h>


#ifdef USE_MSG_QUEUE
//////////////////////////////////////////////////////////////////////////
//消息队列中保存的消息结构
typedef struct _SENDMSG{
	CString ip;
	UINT port;
	void* msg;
}SENDMSG , *PSENDMSG;

//////////////////////////////////////////////////////////////////////////
//消息队列
class MsgQueue:public std::queue<void*>{};

#endif // USE_MSG_QUEUE

class MainSocket :public CAsyncSocket{
private:
	MsgCenter* m_pCenter;
public:
	MainSocket(MsgCenter* center)
		:m_pCenter(center)
	{}
	
	//////////////////////////////////////////////////////////////////////////
	//如果程序异常退出时没有释放端口的话，套接字会无法再次绑定到制定的端口
	//这种情况需要设置SO_REUSEADDR标记，而这个设置方式需要在socket()之后
	//bind()之前执行，但是CAsyncSocket::Create()是将socket()和bind一次执行完了
	//导致根本没有机会设置SO_REUSEADDR，所以这个就写了一个新的函数来处理
	//在这里如果reuseaddr=TRUE则设置SO_REUSEADDR标记，否则和CAsyncSocket::Create()
	//一样
	BOOL CreateWitheSetReuseaddr(UINT nSocketPort = 0, int nSocketType=SOCK_STREAM , BOOL reuseaddr = FALSE,
		long lEvent = FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE,
		LPCTSTR lpszSocketAddress = NULL);


	//套接字的接收消息函数
	virtual void OnReceive(int nErrorCode);
};


#endif