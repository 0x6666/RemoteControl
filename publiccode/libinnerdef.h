//////////////////////////////////////////////////////////////////////////
//���е��ڲ�����

#ifndef _LIB_INNER_DEF_H_
#define _LIB_INNER_DEF_H_

#include "msg.h"

#include <queue>
#include <string>
#include <afxsock.h>


#ifdef USE_MSG_QUEUE
//////////////////////////////////////////////////////////////////////////
//��Ϣ�����б������Ϣ�ṹ
typedef struct _SENDMSG{
	CString ip;
	UINT port;
	void* msg;
}SENDMSG , *PSENDMSG;

//////////////////////////////////////////////////////////////////////////
//��Ϣ����
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
	//��������쳣�˳�ʱû���ͷŶ˿ڵĻ����׽��ֻ��޷��ٴΰ󶨵��ƶ��Ķ˿�
	//���������Ҫ����SO_REUSEADDR��ǣ���������÷�ʽ��Ҫ��socket()֮��
	//bind()֮ǰִ�У�����CAsyncSocket::Create()�ǽ�socket()��bindһ��ִ������
	//���¸���û�л�������SO_REUSEADDR�����������д��һ���µĺ���������
	//���������reuseaddr=TRUE������SO_REUSEADDR��ǣ������CAsyncSocket::Create()
	//һ��
	BOOL CreateWitheSetReuseaddr(UINT nSocketPort = 0, int nSocketType=SOCK_STREAM , BOOL reuseaddr = FALSE,
		long lEvent = FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE,
		LPCTSTR lpszSocketAddress = NULL);


	//�׽��ֵĽ�����Ϣ����
	virtual void OnReceive(int nErrorCode);
};


#endif