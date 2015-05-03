//////////////////////////////////////////////////////////////////////////
//线程

#ifdef USE_MSG_QUEUE

//发送消息线程
unsigned long __stdcall SendMsgThread(void* param);

//派送消息线程
unsigned long __stdcall DispatchMsgThread(void* param);

#endif // USE_MSG_QUEUE