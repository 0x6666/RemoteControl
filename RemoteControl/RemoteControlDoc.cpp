///////////////////////////////////////////////////////////////
//
// FileName	: RemoteControlDoc.cpp 
// Creator	: 杨松
// Date		: 2013年2月27日, 20:10:26
// Comment	: 远控控制服务器文档视图结构中的文档类的实现
//
//////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RemoteControl.h"

#include "RemoteControlDoc.h"
#include "MainFrm.h"
#include "RemoteControlView.h"
#include "CMDDlg.h"
#include "SettingDlg.h"
#include "FileManageDlg.h"
#include "DownuploadDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CRemoteControlDoc

IMPLEMENT_DYNCREATE(CRemoteControlDoc, CDocument)

BEGIN_MESSAGE_MAP(CRemoteControlDoc, CDocument)
	ON_NOTIFY(NM_DBLCLK, IDC_CLIENT_LIST, OnClientListDbClicked)
	ON_NOTIFY(NM_RCLICK, IDC_CLIENT_LIST, OnClientListRClicked)
	ON_COMMAND(ID_PUSH_SERVER, &CRemoteControlDoc::OnPushServer)
	ON_COMMAND(ID_OPTION, &CRemoteControlDoc::OnOption)
	ON_COMMAND(ID_CANCEL_PUSH, &CRemoteControlDoc::OnCancelPush)
	ON_COMMAND(ID_CMD, &CRemoteControlDoc::OnCmd)
	ON_COMMAND(ID_FILE_MANAGE, &CRemoteControlDoc::OnFileManage)
	ON_COMMAND(ID_DOWNLOAD, &CRemoteControlDoc::OnDownload)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOAD, &CRemoteControlDoc::OnUpdateDownload)
END_MESSAGE_MAP()


//用于辅助客户端ID分配
int ClientDescripter::LAST_INDEX = 1;

// CRemoteControlDoc 构造/析构

CRemoteControlDoc::CRemoteControlDoc()
: m_pClientList(NULL)
, m_pMsgCenter(NULL)
, m_pView(NULL)
, m_bLockScreenSizeRatio(TRUE)
, m_czConfigfilePath(NULL)
, m_pDownUploadDlg(NULL)
{
	//推送线程上下文初始化
	m_PushServerDesktopThreadContext.lThreadState = 0;
	m_PushServerDesktopThreadContext.hThread = NULL;
	m_PushServerDesktopThreadContext.lDesktopQuality = DEFAULT_SCREEN_QUALITY;
	m_PushServerDesktopThreadContext.lDesktopFrequency = DEFAULT_FREQUENCY_LEVEL;
	m_PushServerDesktopThreadContext.lDesktopW = SCREEN_SIZE_W;
	m_PushServerDesktopThreadContext.lDesktopH = SCREEN_SIZE_H;
	m_PushServerDesktopThreadContext.hPushedClientListMutex = NULL;

	//检查客户线程上下文初始化
	m_CheckClientThreadContext.pDoc = this;
	m_CheckClientThreadContext.hThread = NULL;
	m_CheckClientThreadContext.iThreadState = 0;
}

CRemoteControlDoc::~CRemoteControlDoc()
{
}

BOOL CRemoteControlDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	//获取配置文件的路径
	char modulePath[ MAX_PATH + 1 ] = {0};
	int pathLen = 0;
	if(pathLen = GetModuleFileNameA(NULL , modulePath , MAX_PATH))
	{
		int i = pathLen - 1; 
		for ( ; i > 0; --i)
		{
			if (modulePath[i] == '\\' || modulePath[i] == '/'  )
			{
				modulePath[i+1] = 0;
				break;
			}
		}
		if (i == 0 )//获取路径失败
			return FALSE;

		//拼接配置文件路径
		strcat(modulePath , CONFIG_FILE_NAME);
		pathLen = strlen(modulePath);
		m_czConfigfilePath = new char[pathLen + 1];
		strcpy(m_czConfigfilePath , modulePath);
		m_czConfigfilePath[pathLen] = 0;
	}

	//加载配置文件中的数据
	LoadConfigFileData();

	POSITION pos = GetFirstViewPosition();
	m_pView = dynamic_cast<CRemoteControlView*>(GetNextView(pos));
	ASSERT(NULL != m_pView);
	m_pClientList = (((CMainFrame*)m_pView->GetParentFrame())->m_pClientList);

	m_lstClient.clear();

	//开启
	m_pMsgCenter = new MsgCenter();
	if(FALSE == m_pMsgCenter->InitMsgCenter(this , SERVER_MSG_PORT , TRUE))
	{//初始化失败
		return FALSE;
	}

	{//创建下载对话框
	m_pDownUploadDlg = new CDownUploadDlg(AfxGetMainWnd());
	if (!m_pDownUploadDlg->Create(CDownUploadDlg::IDD , AfxGetMainWnd()))
	{
		delete m_pDownUploadDlg ;
		m_pDownUploadDlg = NULL;
		return FALSE;
	}
	m_pDownUploadDlg->UpdateWindow();
	}//创建下载对话框

	//开始检测客户端
	StartChekClientThread();

	return TRUE;
}

// CRemoteControlDoc 诊断

#ifdef _DEBUG
void CRemoteControlDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CRemoteControlDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

void CRemoteControlDoc::StartChekClientThread()
{
	if (0 == m_CheckClientThreadContext.iThreadState)
	{//检查客户端线程不在运行的话就启动
		InterlockedExchange(&(m_CheckClientThreadContext.iThreadState), 1);
		DWORD dwThreadID = 0;
		m_CheckClientThreadContext.hThread = CreateThread(NULL , 0 , CheckClientThread , &m_CheckClientThreadContext , 0 , &dwThreadID);
	}
}

void CRemoteControlDoc::FoundAClient(const CString& name ,const CString& ip )
{
	RCMSG rcMsg = {0};
	rcMsg.size = sizeof(RCMSG);
	rcMsg.type = MT_REPEAT_FIND_SERVER_S;

	ClientList::iterator it = m_lstClient.begin();
	for ( ; it != m_lstClient.end() ; ++it)
	{
		if ((*it)->mIP == ip)
		{//被控端已经存在
			if ((*it)->mIsOnline == FALSE)
			{//被控端不在线
				m_pMsgCenter->SendMsg(ip , (*it)->mPort , &rcMsg);
				(*it)->mIsOnline = FALSE;
				(*it)->mPreCheckOnlineState = FALSE;
			}
			break;
		}
	}

	if (it == m_lstClient.end())
	{//客户端不存在
		PClientDescripter client = new ClientDescripter;
		client->mIsOnline = FALSE;
		client->mPreCheckOnlineState = FALSE;
		client->mIP = ip;
		client->mPort = CLIENT_MSG_PORT;
		client->mName = name;
		client->mIsPushed = FALSE;
		client->mPushAddr = NULL;
		client->mCMDDlg = NULL;

		m_lstClient.push_back(client);

		//如果这个客户端是新添加的话，则还是需要发送一个消息，便于客户端获取服务器的地址
		m_pMsgCenter->SendMsg(ip , client->mPort , &rcMsg);
	}
	//刷新客户端列表控件中显示的数据
	FlushClientCtrl();
}

void CRemoteControlDoc::StopCheckClientThread()
{
	InterlockedExchange(&(m_CheckClientThreadContext.iThreadState) , 0);
	WaiteExitThread(m_CheckClientThreadContext.hThread , 2*1000);
	CloseHandle(m_CheckClientThreadContext.hThread);
	m_CheckClientThreadContext.hThread = NULL;
}

void CRemoteControlDoc::OnCloseDocument()
{
	//停止检查客户端线程
	StopCheckClientThread();

	{//销毁下载对话框
	m_pDownUploadDlg->DestroyWindow();
	delete m_pDownUploadDlg;
	}//销毁下载对话框

	{//向所有客户端发送服务器退出的消息
	DEF_RCMSG(rcMsg , MT_SERVER_EXIT_S);
	SendAllClient(&rcMsg);
	}//向所有客户端发送服务器退出的消息

	//停止推送线程
	StopPushServerDesktopTHread();

	//销毁所有需要销毁的数据
	m_pView->DestroyView();

	if (NULL != m_pMsgCenter)
	{
		m_pMsgCenter->Close();
		delete m_pMsgCenter;
		m_pMsgCenter = NULL;
	}

	//清理一下
	for (ClientList::iterator it = m_lstClient.begin() ; it != m_lstClient.end(); ++it)
	{
		if (NULL != (*it)->mPushAddr)
		{
			delete (*it)->mPushAddr;
			(*it)->mPushAddr = NULL;
		}
		if (NULL != (*it)->mCMDDlg)
		{//销毁CMD对话框
			CCMDDlg* pDlg = (*it)->mCMDDlg;
			pDlg->DestroyWindow();
			delete pDlg;
			(*it)->mCMDDlg = NULL;
		}

		if (NULL != (*it)->mDocManageDlg)
		{//销毁文件对话框
			CFileManageDlg* pDlg = (*it)->mDocManageDlg;
			pDlg->DestroyWindow();
			delete pDlg;
			(*it)->mDocManageDlg = NULL;
		}

		delete (*it);
	}
	m_lstClient.clear();


	//删除配置路径缓存
	if(NULL != m_czConfigfilePath)
	{
		delete[] m_czConfigfilePath;
	}

	CDocument::OnCloseDocument();
}

void CRemoteControlDoc::OnClientListDbClicked( NMHDR* pNMHDR, LRESULT* pResult )
{
	POSITION pos = m_pClientList->GetFirstSelectedItemPosition();
	if (pos == NULL)
	{//没有选择任何客户端
		TRACE0("没有选择任何客户端!\n");
		*pResult = 0;
		return ;
	}

	//获得选中中项的索引
	int nItem = m_pClientList->GetNextSelectedItem(pos);
	CString temp;
	temp.LoadString(IDS_ONLINE);
	if (0 != m_pClientList->GetItemText(nItem , CLIENT_LIST_STATU).Compare(temp))
	{//不在线
		CString strNotify;
		strNotify.LoadString(IDS_NOTIFY);
		CString strMsg;
		strMsg.LoadString(IDS_CUR_CLIENT_NOT_ONLINE);
		MessageBox(AfxGetMainWnd()->GetSafeHwnd() , strMsg , strNotify , MB_OK | MB_ICONWARNING);
	}
	else
	{
		CString ip = m_pClientList->GetItemText(nItem , CLIENT_LIST_IP);
		for (ClientList::iterator it = m_lstClient.begin() ; it != m_lstClient.end(); ++it )
		{
			if (ip == (*it)->mIP)
			{
				m_pView->AddNewClient(*it/*(*it)->mName , (*it)->mIP , (*it)->mPort */);
			}
		}		
	}
}

void CRemoteControlDoc::OnClientListRClicked( NMHDR *pNMHDR, LRESULT *pResult )
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	CPoint	point;
	CMenu	menu;

	::GetCursorPos(&point);

	//快捷菜单
	menu.LoadMenu(IDR_CLIENT_LIST);
	menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN|TPM_TOPALIGN , point.x , point.y , AfxGetMainWnd() , NULL);
}

void CRemoteControlDoc::DispatchMsg(const void* msg , CString ip, UINT port)
{
	if (NULL == msg || ip.IsEmpty())
		return ;

	switch(PRCMSG(msg)->type)
	{
	case MT_FIND_SERVER_BROADCAST_C://客户端查询服务器的广播
		OnRcFindServerBraodcast(ip , msg);
		break;
	case MT_HEARTBEAT_C://客户端心跳包
		OnRcHeartbeat(ip , port , msg); 
		break;
	case MT_CONN_CLOSED_C: //一个客户端断开连接
		OnRcClosed(ip); 
		break;
	case MT_SCREEN_CAPTURE_START_C://被控端已经抓屏了 
		OnRcStartCapture(ip , port , msg);
		break;
	case MT_PUSH_SERVER_DESKTOP_C://确认推送桌面
		OnRcPushedServerDesktop(ip);
		break;
	case MT_CALCEL_PUSH_DESKTOP_C://一个客户端已经确认了取消桌面的推送
		OnRcCancelPushDesktop(ip);
		break;
	case MT_SHUTDOWN_FALIED_C://关机失败
		OnRcShutdownFailed(ip);
		break;
	case MT_RESTART_FALIED_C://重启失败
		OnRcRestartFailed(ip);
		break;
	case MT_LOGIN_OUT_FALIED_C://注销失败
		OnRcLoginOutFailed(ip);
		break;
	case MT_START_CMD_FAILED_C://创建CMD失败
		OnRcStartCMDFailed(ip);
		break;
	case MT_CMD_LINE_DATA_C://命令行回显数据
		OnRcCMDLineData(ip , port , msg);
		break;
	case MT_HANDS_UP_C://客户端举手
		OnRcHandsUp(ip);
		break;
	case MT_CMD_FAILED_C://cmd命令执行失败
		OnRcCMDFailed(ip , msg);
		break;
	case MT_FILE_PATH_C://收到一个文件路径消息
		OnRcFilePath(ip , msg);
		break;
	case MT_DELETE_FILE_FAILED_C://删除文件失败
		OnRcDeleteFileFailed(ip , msg);
		break;
	case MT_DELETE_FILE_SUCCESS_C://删除文件成功
		OnRcDeleteFileSuccess(ip , msg);
		break;
	case MT_LISTEN_DOWNLOAD_PORT_C://下载文件监听端口
		OnRcListenDownloadPort( ip , msg);
		break;
	default:
		ASSERT(FALSE);
	}
	return ;
}

void CRemoteControlDoc::OnRcHeartbeat( const CString& ip ,USHORT port,   const void* msg ) 
{//一个客户端发来了一个心跳包

	RCMSG rcMsg = {0};
	rcMsg.size = sizeof(RCMSG);
	rcMsg.type = MT_REPEAT_FIND_SERVER_S;

	ClientList::iterator it = m_lstClient.begin();
	for ( ; it != m_lstClient.end() ; ++it)
	{
		if ((*it)->mIP == ip)
		{//客户端已经存在
			if ((*it)->mIsOnline == FALSE)
			{//如果之前客户端是掉线了的，则现在需要发送一个消息，便于客户端获取服务器的地址
				m_pMsgCenter->SendMsg(ip , (*it)->mPort , &rcMsg);
			}
			(*it)->mIsOnline = TRUE;
			(*it)->mPreCheckOnlineState = TRUE;
			break;
		}
	}

	if (it == m_lstClient.end())
	{//客户端不存在  理论上这里是不能执行的
		CString name = GetHostNameByAddr(ip);
		AddClient(ip, port , name.GetLength()?name:ip);
	}
	//刷新客户端列表控件中显示的数据
	FlushClientCtrl();
}

void CRemoteControlDoc::OnRcClosed( const CString& ip )
{
	for (ClientList::iterator it = m_lstClient.begin() ; it != m_lstClient.end() ; ++it)
	{//该百年客户端结合中的客户端状态
		if ((*it)->mIP == ip){
			(*it)->mIsOnline = FALSE;
			(*it)->mPreCheckOnlineState = FALSE;
			(*it)->mIsPushed = FALSE; 
			break;
		}
	}

	//将客户端列表中的数据刷新到控件中去
	FlushClientCtrl();

	//如果视图中显示了客户端则也需要通知其改变状态
	m_pView->ClientDropped(ip);
}

void CRemoteControlDoc::SendRcMessage(const CString& ip , USHORT port , void* msg)
{
	m_pMsgCenter->SendMsg(ip , port , msg);
}

void CRemoteControlDoc::OnRcStartCapture( const CString& ip , USHORT port , const void* msg )
{
	m_pView->DispatchMsg(ip , port , msg);
}

void CRemoteControlDoc::CheckClientState()
{
	if (m_lstClient.empty())
		return ;

	CString strTemp;
	for(ClientList::iterator it = m_lstClient.begin() ; it !=  m_lstClient.end() ; ++it)
	{
		if((FALSE == (*it)->mPreCheckOnlineState) && (TRUE == (*it)->mIsOnline))
		{//这次可以确定不在线了
			(*it)->mIsOnline = FALSE;
			m_pView->ClientDropped((*it)->mIP);
		}
		else
		{//这是这次检查的状态
			(*it)->mPreCheckOnlineState = FALSE;
		}
	}
	FlushClientCtrl();
}

void CRemoteControlDoc::FlushClientCtrl()
{//将客户端列表中的数据刷到客户端列表控件中去
	CString strOnline , strCloseed , strPushed , strUnPushed;
	strOnline.LoadString(IDS_ONLINE);
	strCloseed.LoadString(IDS_CLOSED);
	strPushed.LoadString(IDS_YES);
	strUnPushed.LoadString(IDS_NO);

	//在线的客户端数量
	int nOnlineCnt = 0;
	int nPushCnt = 0;

	for (ClientList::iterator it = m_lstClient.begin() ; it != m_lstClient.end() ; ++it)
	{
		int cnt = m_pClientList->GetItemCount();
		int i = 0;
		for ( ; i < cnt ; ++i ) 
		{
			if ( m_pClientList->GetItemText( i , CLIENT_LIST_IP) == (*it)->mIP)
			{
				//跟新在线状态
				if ((*it)->mIsOnline)  //在线
				{
					m_pClientList->SetItemText( i , CLIENT_LIST_STATU , strOnline);
					++nOnlineCnt;
					
					if ((*it)->mIsPushed)
					{//推送了
						++nPushCnt;
						m_pClientList->SetItemText( i , CLIENT_LIST_PUSH , strPushed);
					}else
						m_pClientList->SetItemText( i , CLIENT_LIST_PUSH , strUnPushed);
				}
				else//不在线
				{
					m_pClientList->SetItemText( i , CLIENT_LIST_STATU , strCloseed);
					m_pClientList->SetItemText( i , CLIENT_LIST_PUSH , strUnPushed);
				}
				break;
			}
		}

		if(i == cnt)
		{//控件中不存在则添加
			CString temp;
			temp.Format(_T("%d") , (*it)->mIndex);
			m_pClientList->InsertItem( i , temp );
			m_pClientList->SetItemText( i , CLIENT_LIST_IP , (*it)->mIP);
			m_pClientList->SetItemText( i , CLIENT_LIST_NAME , (*it)->mName);

			//设置在线状态
			if ((*it)->mIsOnline)
			{//在线
				++nOnlineCnt;
				m_pClientList->SetItemText( i , CLIENT_LIST_STATU , strOnline );

				if ((*it)->mIsPushed)
				{//推送了
					++nPushCnt;
					m_pClientList->SetItemText( i , CLIENT_LIST_PUSH , strPushed );
				}else
					m_pClientList->SetItemText( i , CLIENT_LIST_STATU , strUnPushed );
			}else
			{//不在线
				m_pClientList->SetItemText( i , CLIENT_LIST_STATU , strCloseed );
				m_pClientList->SetItemText( i , CLIENT_LIST_PUSH , strUnPushed );
			}

			//设置IP地址
			m_pClientList->SetItemText( i , CLIENT_LIST_IP , (*it)->mIP );
		}
	}
	CWnd* pMainFrame = AfxGetMainWnd();
	if ((NULL != pMainFrame)&&
		(IsWindow(pMainFrame->GetSafeHwnd())) )
	{
		//通知主窗口改变状态显示
		pMainFrame->PostMessage(WM_CLIENT_COUNT , m_lstClient.size() , nOnlineCnt);
		
		//已推送数量
		pMainFrame->PostMessage(WM_PUSHED_COUNT , nPushCnt , 0 );
	}
}

void CRemoteControlDoc::OnPushServer()
{
	//TODO: 如果还没启动推送桌面线程的话，在这里启动推送桌面线程
	if (0 == m_PushServerDesktopThreadContext.lThreadState)
	{
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lThreadState) , 1);
		m_PushServerDesktopThreadContext.hPushedClientListMutex = CreateMutex(NULL , FALSE , NULL);
		DWORD dwThreadId = 0;
		m_PushServerDesktopThreadContext.hThread =
			CreateThread(NULL , 0 , PushSeverDesktopThread , &m_PushServerDesktopThreadContext, 0 , &dwThreadId );
	}

	//给选择了的客户端推送桌面
	POSITION pos = m_pClientList->GetFirstSelectedItemPosition();
	int nItem;
	CString strIP;
	DEF_RCMSG(rcMsg, MT_PUSH_SERVER_DESKTOP_S);
	while (pos != NULL)
	{
		nItem = m_pClientList->GetNextSelectedItem(pos);
		strIP = m_pClientList->GetItemText(nItem , CLIENT_LIST_IP);

		PClientDescripter cdes = GetClientDescripter(strIP);
		if (NULL != cdes)
		{
			if (cdes->mIsPushed)
				continue;//已经推送了桌面给了这个客户端
			else
			{//发送推送桌面消息
				m_pMsgCenter->SendMsg(strIP , cdes->mPort , &rcMsg);
			}
		}
	}
}

void CRemoteControlDoc::OnRcPushedServerDesktop( const CString& ip )
{//收到了一个客户端获得推送桌面的确认消息
	char czIP[20] = {0};
	PClientDescripter cdes = GetClientDescripter(ip);

	if ((NULL != cdes) && (cdes->mIsPushed == FALSE))
	{
		if (cdes->mPushAddr == NULL)
		{//地址结构还没有创建
			wcstombs( czIP , cdes->mIP , 20 );
			cdes->mPushAddr = new sockaddr_in;
			memset(cdes->mPushAddr , 0 , sizeof(sockaddr_in));
			cdes->mPushAddr->sin_family = AF_INET;
			cdes->mPushAddr->sin_port = htons(SERVER_PUSH_DESKTOP_PORT);
			cdes->mPushAddr->sin_addr.s_addr = inet_addr(czIP);
		}
		cdes->mIsPushed = TRUE;

		//将客户端的地址结构指针放入推送客户端链表
		WaitForSingleObject(m_PushServerDesktopThreadContext.hPushedClientListMutex , INFINITE);
		m_PushServerDesktopThreadContext.lstPushedClient.push_back(cdes->mPushAddr);
		ReleaseMutex(m_PushServerDesktopThreadContext.hPushedClientListMutex);

		//跟新客户端列表显示
		FlushClientCtrl();
	}
}

void CRemoteControlDoc::StopPushServerDesktopTHread()
{
	if (m_PushServerDesktopThreadContext.lThreadState)
	{//推送服务器桌面线程在运行的则结束
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lThreadState) , 0);
		WaiteExitThread(m_PushServerDesktopThreadContext.hThread, 2*1000);
		CloseHandle(m_PushServerDesktopThreadContext.hThread);
		m_PushServerDesktopThreadContext.hThread = NULL;
	}
}

void CRemoteControlDoc::OnOption()
{
	//设置对话框的初始值
	PUSH_CFG_V pushValue = {0};
	pushValue.bPush_LockScreenSizeRatio = m_bLockScreenSizeRatio;
	pushValue.nPush_DesktopFrequency = m_PushServerDesktopThreadContext.lDesktopFrequency;
	pushValue.nPush_DesktopH = m_PushServerDesktopThreadContext.lDesktopH;
	pushValue.nPush_DesktopQuality = m_PushServerDesktopThreadContext.lDesktopQuality;
	pushValue.nPush_DesktopW = m_PushServerDesktopThreadContext.lDesktopW;

	CSettingDlg dlg(pushValue , m_cmdValue , this);
	dlg.DoModal();
}

void CRemoteControlDoc::OnCancelPush()
{
	//取消推送
	POSITION pos = m_pClientList->GetFirstSelectedItemPosition();
	int nItem;
	CString strIP;
	DEF_RCMSG(rcMsg, MT_CALCEL_PUSH_DESKTOP_S);
	while (pos != NULL)
	{
		nItem = m_pClientList->GetNextSelectedItem(pos);
		strIP = m_pClientList->GetItemText(nItem , CLIENT_LIST_IP);

		PClientDescripter cdes = GetClientDescripter(strIP);
		if (NULL != cdes)
		{
			if (cdes->mIsPushed)//已经推送则取消
				m_pMsgCenter->SendMsg(strIP , cdes->mPort , &rcMsg);
		}
	}
}

void CRemoteControlDoc::OnRcCancelPushDesktop( const CString& ip )
{
	int emptyFlag = 0;
	PClientDescripter cdes = GetClientDescripter(ip);
	if ((NULL != cdes) && (cdes->mIsPushed == TRUE))
	{
		//从推送客户端中移除推送地址
		WaitForSingleObject(m_PushServerDesktopThreadContext.hPushedClientListMutex , INFINITE);
		m_PushServerDesktopThreadContext.lstPushedClient.remove(cdes->mPushAddr);
		if (m_PushServerDesktopThreadContext.lstPushedClient.empty())//已经空了，需要停止推送线程
			emptyFlag = 1;
		ReleaseMutex(m_PushServerDesktopThreadContext.hPushedClientListMutex);

		if(emptyFlag == 1)//没有了需要推送的客户端就停止推送线程
			StopPushServerDesktopTHread();

		//取消推送标记
		cdes->mIsPushed = FALSE;

		//跟新显示
		FlushClientCtrl();
	}
}

void CRemoteControlDoc::LoadConfigFileData()
{
	//用于保存数值转换成字符串后的值
	char vBuf[100] = {0};
	RCF rcf = RCF_OK;
	long temp = 0;

	{//推送相关的配置数据加载
	//推送桌面时的宽度
	rcf = ReadConfValue(CK_PUSH_SD_W , vBuf , 100 , m_czConfigfilePath);
	if (rcf == RCF_OK)
	{
		temp = atoi(vBuf);
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lDesktopW) , temp);
	}else
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lDesktopW) , SCREEN_SIZE_W);
	
	//推送桌面时的高度
	rcf = ReadConfValue(CK_PUSH_SD_H , vBuf , 100 , m_czConfigfilePath);
	if (rcf == RCF_OK)
	{
		temp = atoi(vBuf);
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lDesktopH) , temp);
	}else
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lDesktopH) , SCREEN_SIZE_H);
	
	//推送桌面时的质量
	rcf = ReadConfValue(CK_PUSH_SD_QUALITY , vBuf , 100 , m_czConfigfilePath);
	if (rcf == RCF_OK)
	{
		temp = atoi(vBuf);
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lDesktopQuality) , temp );
	}else
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lDesktopQuality), DEFAULT_SCREEN_QUALITY );

	//推送桌面时的流畅度
	rcf = ReadConfValue(CK_PUSH_SD_FREQUNCY , vBuf , 100 , m_czConfigfilePath);
	if (rcf == RCF_OK)
	{
		temp = atoi(vBuf);
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lDesktopFrequency), temp );
	}else
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lDesktopFrequency) , DEFAULT_FREQUENCY_LEVEL );

	//推送桌面时的是否锁定长高比
	rcf = ReadConfValue(CK_PUSH_SD_LOCK_RATIO , vBuf , 100 , m_czConfigfilePath);
	if (rcf == RCF_OK)
	{
		temp = atoi(vBuf);
		m_bLockScreenSizeRatio = temp ;
	}else
		m_bLockScreenSizeRatio = TRUE ;
	}//推送相关的配置数据加载

	{//CMD窗口的配置数据加载
	//字体颜色
	rcf = ReadConfValue(CK_CMD_FONT_COLOR , vBuf , 100 , m_czConfigfilePath);
	if (rcf == RCF_OK)
	{
		temp = atoi(vBuf);
		m_cmdValue.nCMD_FontColor = temp;
	}else
		m_cmdValue.nCMD_FontColor = RGB(255 , 255 , 255);//默认白色
	//背景颜色
	rcf = ReadConfValue(CK_CMD_BG_COLOR , vBuf , 100 , m_czConfigfilePath);
	if (rcf == RCF_OK)
	{
		temp = atoi(vBuf);
		m_cmdValue.nCMD_BackgroundColor = temp;
	}else
		m_cmdValue.nCMD_BackgroundColor = RGB(0 , 0 , 0);//默认黑色;
	//字体大小
	rcf = ReadConfValue(CK_CMD_FONT_SIZE , vBuf , 100 , m_czConfigfilePath);
	if (rcf == RCF_OK)
	{
		temp = atoi(vBuf);
		m_cmdValue.nCMD_FontSize = temp;
	}else
		m_cmdValue.nCMD_FontSize = -25;
	//字体字符集
	rcf = ReadConfValue(CK_CMD_FONT_CHAR_SET , vBuf , 100 , m_czConfigfilePath);
	if (rcf == RCF_OK)
	{
		temp = atoi(vBuf);
		m_cmdValue.nCMD_FontCharset = (BYTE)temp;
	}else
		m_cmdValue.nCMD_FontCharset = 1;
	//字体名字
	rcf = ReadConfValue(CK_CMD_FONT_FACENAME , vBuf , 100 , m_czConfigfilePath);
	if (rcf == RCF_OK)
	{
		strcpy(m_cmdValue.czCMD_FontFaceName , vBuf);
	}else
		strcpy(m_cmdValue.czCMD_FontFaceName , "新宋体");
	}
}

void CRemoteControlDoc::StopMonitoring( const CString& strIP )
{
// 	int emptyFlag = 0;
// 	for (ClientList::iterator it = m_lstClient.begin() ; it != m_lstClient.end() ; ++it)
// 	{
// 		if ((*it)->mIp == strIP)
// 		{//找到了制定的已经推送的客户端
// 			if ((*it)->bPushed)
// 			{//已经在推送的话则取消
// 				OnRcCancelPushDesktop(strIP);
// 				if (NULL != (*it)->pushAddr)
// 				{
// 					delete (*it)->pushAddr;
// 					(*it)->pushAddr = NULL;
// 				}
// 			}
// 			break;
// 		}
// 	}
}

void CRemoteControlDoc::OnRcShutdownFailed( const CString& ip )
{
	for (ClientList::iterator it = m_lstClient.begin() ; it != m_lstClient.end() ; ++it)
	{
		if ((*it)->mIP == ip)
		{
			CString strMsg;
			CString strNotify;
			CString strTemp;
			strTemp = ip;
			strTemp += _T("  ");
			strTemp += (*it)->mName;
			strNotify.LoadString(IDS_NOTIFY);
			strMsg.LoadString(IDS_SHUT_DOWN_FALIED);
			strMsg.Replace(STR_RP , strTemp);

			MessageBox(NULL , strMsg , strNotify , MB_OK|MB_ICONWARNING);
			break;
		}
	}
}

void CRemoteControlDoc::OnRcRestartFailed( const CString& ip )
{
	for (ClientList::iterator it = m_lstClient.begin() ; it != m_lstClient.end() ; ++it)
	{
		if ((*it)->mIP == ip)
		{
			CString strMsg;
			CString strNotify;
			CString strTemp;
			strTemp = ip;
			strTemp += _T("  ");
			strTemp += (*it)->mName;
			strNotify.LoadString(IDS_NOTIFY);
			strMsg.LoadString(IDS_RESTART_FALIED);
			strMsg.Replace(STR_RP , strTemp);

			MessageBox(NULL , strMsg , strNotify , MB_OK|MB_ICONWARNING);
			break;
		}
	}
}

void CRemoteControlDoc::OnRcLoginOutFailed( const CString& ip )
{
	for (ClientList::iterator it = m_lstClient.begin() ; it != m_lstClient.end() ; ++it)
	{
		if ((*it)->mIP == ip)
		{
			CString strMsg;
			CString strNotify;
			CString strTemp;
			strTemp = ip;
			strTemp += _T("  ");
			strTemp += (*it)->mName;
			strNotify.LoadString(IDS_NOTIFY);
			strMsg.LoadString(IDS_LOGINOUT_FALIED);
			strMsg.Replace(STR_RP , strTemp);

			MessageBox(NULL , strMsg , strNotify , MB_OK|MB_ICONWARNING);
			break;
		}
	}
}

void CRemoteControlDoc::OnRcStartCMDFailed( const CString& ip )
{
	for (ClientList::iterator it = m_lstClient.begin() ; it != m_lstClient.end() ; ++it)
	{
		if ((*it)->mIP == ip)
		{
			CString strMsg;
			CString strNotify;
			CString strTemp;
			strTemp = ip;
			strTemp += _T("  ");
			strTemp += (*it)->mName;
			strNotify.LoadString(IDS_NOTIFY);
			strMsg.LoadString(IDS_START_CMD_FAILED);
			strMsg.Replace(STR_RP , strTemp);

			MessageBox(NULL , strMsg , strNotify , MB_OK|MB_ICONWARNING);
			break;
		}
	}
}

void CRemoteControlDoc::OnRcCMDLineData( const CString& ip , USHORT port , const void* msg )
{
	PClientDescripter pdes = GetClientDescripter(ip);
	if ( pdes != NULL )
	{
		if (pdes->mCMDDlg != NULL)
		{
			PRCMSG_BUFF pMsg = PRCMSG_BUFF(msg);
			CString data;
			data += pMsg->buf;
			pdes->mCMDDlg->AddCMDLineData(data);
		}
	}
}

void CRemoteControlDoc::OnRcHandsUp( const CString& ip )
{
	PClientDescripter pcdes = GetClientDescripter(ip);
	if (NULL != pcdes)
	{
		CString msg;
		msg.Format(_T("被控端 ID=%d 举手") , pcdes->mIndex);
		AfxGetMainWnd()->SendMessage(WM_MESSAGE , (WPARAM)&msg , 0);
	}
}

void CRemoteControlDoc::OnRcFindServerBraodcast( const CString& ip, const void* msg )
{
	PRCMSG_BROADCAST_MSG pMsg = PRCMSG_BROADCAST_MSG(msg);
	RCMSG rcMsg = {0};
	rcMsg.size = sizeof(RCMSG);
	rcMsg.type = MT_REPEAT_FIND_SERVER_S;

	ClientList::iterator it = m_lstClient.begin();
	for ( ; it != m_lstClient.end() ; ++it)
	{
		if ((*it)->mIP == ip)
		{//客户端已经存在
			if ((*it)->mIsOnline == FALSE)
			{//如果之前客户端是掉线了的，则现在需要发送一个消息，便于客户端获取服务器的地址
				m_pMsgCenter->SendMsg(ip , pMsg->port , &rcMsg);
				(*it)->mPort = pMsg->port;
				(*it)->mName = _T("");
				(*it)->mName  += pMsg->name;
			}
			(*it)->mIsOnline = FALSE;
			(*it)->mPreCheckOnlineState = FALSE;
			break;
		}
	}

	if (it == m_lstClient.end())
	{//客户端不存在
		CString name ;
		name += pMsg->name;
		AddClient(ip, pMsg->port , name);

		//如果这个客户端是新添加的话，则还是需要发送一个消息，便于客户端获取服务器的地址
		m_pMsgCenter->SendMsg(ip , pMsg->port , &rcMsg);

	}
	//刷新客户端列表控件中显示的数据
	FlushClientCtrl();
}

BOOL CRemoteControlDoc::CreateCMDDlg( const CString& strIP )
{
	CMainFrame* pMainFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
	ASSERT(NULL != pMainFrame);

	PClientDescripter pdes = GetClientDescripter(strIP);
	if (pdes != NULL)
	{
		if (pdes->mCMDDlg != NULL)
		{//对话框已经存在
			pdes->mCMDDlg->ShowWindow(!pdes->mCMDDlg->IsWindowVisible());
			return TRUE;
		}else
		{//对话框不存在,创建

			pdes->mCMDDlg = new CCMDDlg(pdes->mIP ,pdes->mPort , pdes->mName , this , &m_cmdValue);

			if (!pdes->mCMDDlg->Create(IDD_CMD_VIEW , pMainFrame))
			{
				TRACE0("创建CMD对话框失败\n");
				delete pdes->mCMDDlg ;
				pdes->mCMDDlg = NULL;
				return FALSE;
			}
			pdes->mCMDDlg->UpdateWindow();
			pdes->mCMDDlg->ShowWindow(SW_SHOW);

			//发送一个启动CMD的消息
			DEF_RCMSG(rcMsg , MT_START_CMD_S);
			m_pMsgCenter->SendMsg(pdes->mIP , pdes->mPort , &rcMsg);
			return TRUE;
		}
	}
	return FALSE;
}

void CRemoteControlDoc::OnCmd()
{
	//显示cmd对话框
	POSITION pos = m_pClientList->GetFirstSelectedItemPosition();
	int nItem;
	CString strIP;
	while (pos != NULL)
	{
		nItem = m_pClientList->GetNextSelectedItem(pos);
		strIP = m_pClientList->GetItemText(nItem , CLIENT_LIST_IP);

		for (ClientList::iterator it = m_lstClient.begin() ; it != m_lstClient.end() ; ++it)
		{
			if ((*it)->mIP == strIP)
			{
				CreateCMDDlg(strIP);
			}
		}
	}
}

void CRemoteControlDoc::OnRcCMDFailed( const CString& ip, const void* msg )
{
	PClientDescripter pdes = GetClientDescripter(ip);
	if ((NULL != pdes) && (pdes->mCMDDlg != NULL))
	{
		pdes->mCMDDlg->CMDFailed(msg);
	}
}

void CRemoteControlDoc::ChangePushCfgValue( PPUSH_CFG_V pushValue )
{
	//用于保存数值转换成字符串后的值
	char vBuf[10] = {0};
	//将获取的数据提交
	if (m_PushServerDesktopThreadContext.lDesktopW != pushValue->nPush_DesktopW)
	{//推送桌面时的宽度
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lDesktopW) , pushValue->nPush_DesktopW);
		WriteConfValue(CK_PUSH_SD_W , itoa( (int)pushValue->nPush_DesktopW , vBuf , 10)
			, m_czConfigfilePath);
	}

	if (m_PushServerDesktopThreadContext.lDesktopH != pushValue->nPush_DesktopH)
	{//推送桌面时的高度
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lDesktopH) , pushValue->nPush_DesktopH);
		WriteConfValue(CK_PUSH_SD_H , itoa( (int)pushValue->nPush_DesktopH , vBuf , 10) 
			, m_czConfigfilePath);
	}
	if (m_PushServerDesktopThreadContext.lDesktopQuality != pushValue->nPush_DesktopQuality)
	{//推送桌面时的质量
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lDesktopQuality) , pushValue->nPush_DesktopQuality);
		WriteConfValue(CK_PUSH_SD_QUALITY , itoa( (int)pushValue->nPush_DesktopQuality , vBuf , 10) 
			, m_czConfigfilePath);
	}
	if (m_PushServerDesktopThreadContext.lDesktopFrequency != pushValue->nPush_DesktopFrequency)
	{//推送桌面时的流畅度
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lDesktopFrequency) , pushValue->nPush_DesktopFrequency);
		WriteConfValue(CK_PUSH_SD_FREQUNCY , itoa( (int)pushValue->nPush_DesktopFrequency , vBuf , 10)
			, m_czConfigfilePath);
	}
	if (m_bLockScreenSizeRatio != pushValue->bPush_LockScreenSizeRatio)
	{//推送桌面时的是否锁定长高比
		m_bLockScreenSizeRatio = pushValue->bPush_LockScreenSizeRatio;
		WriteConfValue(CK_PUSH_SD_LOCK_RATIO , itoa( (int)pushValue->bPush_LockScreenSizeRatio , vBuf , 10) 
			, m_czConfigfilePath);
	}
}

void CRemoteControlDoc::ChangeCMDCfgValue( PCMD_CFG_V cmdValue )
{
	//用于保存数值转换成字符串后的值
	char vBuf[10] = {0};
	//将获取的数据提交
	if (strcmp(m_cmdValue.czCMD_FontFaceName , cmdValue->czCMD_FontFaceName))
	{//字体名字
		strcpy(m_cmdValue.czCMD_FontFaceName , cmdValue->czCMD_FontFaceName);
		WriteConfValue(CK_CMD_FONT_FACENAME , cmdValue->czCMD_FontFaceName, m_czConfigfilePath);
	}

	if (m_cmdValue.nCMD_BackgroundColor != cmdValue->nCMD_BackgroundColor)
	{//背景颜色
		m_cmdValue.nCMD_BackgroundColor = cmdValue->nCMD_BackgroundColor;
		WriteConfValue(CK_CMD_BG_COLOR , itoa( (int)cmdValue->nCMD_BackgroundColor , vBuf , 10) 
			, m_czConfigfilePath);
	}
	if (m_cmdValue.nCMD_FontCharset != cmdValue->nCMD_FontCharset)
	{//字符集
		m_cmdValue.nCMD_FontCharset = cmdValue->nCMD_FontCharset;
		WriteConfValue(CK_CMD_FONT_CHAR_SET , itoa( (int)cmdValue->nCMD_FontCharset , vBuf , 10) 
			, m_czConfigfilePath);
	}
	if (m_cmdValue.nCMD_FontColor != cmdValue->nCMD_FontColor)
	{//字体颜色
		m_cmdValue.nCMD_FontColor = cmdValue->nCMD_FontColor;
		WriteConfValue(CK_CMD_FONT_COLOR, itoa( (int)cmdValue->nCMD_FontColor , vBuf , 10)
			, m_czConfigfilePath);
	}
	if (m_cmdValue.nCMD_FontSize != cmdValue->nCMD_FontSize )
	{//字体大小
		m_cmdValue.nCMD_FontSize = cmdValue->nCMD_FontSize;
		WriteConfValue(CK_CMD_FONT_SIZE , itoa( (int)cmdValue->nCMD_FontSize , vBuf , 10) 
			, m_czConfigfilePath);
	}

	//便利每一个窗口通知其改变配置数据
	for (ClientList::iterator it = m_lstClient.begin() ;it != m_lstClient.end(); ++it)
	{
		if (NULL != (*it)->mCMDDlg)
			(*it)->mCMDDlg->ChangeCfgValue(&m_cmdValue);
	}
}

void CRemoteControlDoc::AddClient( const CString& ip, USHORT port ,  const CString& strName)
{
	PClientDescripter client = new ClientDescripter;
	client->mIsOnline = FALSE;
	client->mPreCheckOnlineState = FALSE;
	client->mIP = ip;
	client->mPort = port;
	client->mName = strName;
	client->mIsPushed = FALSE;
	client->mPushAddr = NULL;
	client->mCMDDlg = NULL;
	client->mDocManageDlg = NULL;
	m_lstClient.push_back(client);

	FlushClientCtrl();
}

void CRemoteControlDoc::SendAllClient( PRCMSG rcMsg )
{
	for (ClientList::iterator it = m_lstClient.begin() ; it != m_lstClient.end() ; ++it)
	{
		m_pMsgCenter->SendMsg((*it)->mIP , (*it)->mPort , rcMsg);
	}
}

BOOL CRemoteControlDoc::CreateDocManageDlg( const CString& ip )
{
	CMainFrame* pMainFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
	ASSERT(NULL != pMainFrame);

	for (ClientList::iterator it = m_lstClient.begin() ; it != m_lstClient.end() ; ++it)
	{
		if ((*it)->mIP == ip)
		{
			if ((*it)->mDocManageDlg != NULL)
			{//对话框已经存在
				(*it)->mDocManageDlg->ShowWindow(!(*it)->mDocManageDlg->IsWindowVisible());
				return TRUE;
			}else
			{//对话框不存在,创建

				(*it)->mDocManageDlg = new CFileManageDlg((*it)->mIP ,(*it)->mPort , (*it)->mName , this );

				if (!(*it)->mDocManageDlg->Create(IDD_DOC_MANAGE_DLG , pMainFrame))
				{
					TRACE0("创建文件管理对话框失败\n");
					delete (*it)->mDocManageDlg ;
					(*it)->mDocManageDlg = NULL;
					return FALSE;
				}
				(*it)->mDocManageDlg->UpdateWindow();
				(*it)->mDocManageDlg->ShowWindow(SW_SHOW);
			}
		}
	}
	return FALSE;
}

void CRemoteControlDoc::OnRcFilePath( const CString& ip, const void* msg )
{
	PClientDescripter pdes = GetClientDescripter(ip);
	if ((NULL != pdes) && (pdes->mDocManageDlg != NULL))
	{
		PRCMSG_BUFF pMsg = PRCMSG_BUFF(msg);
		CString data;
		data += pMsg->buf;
		pdes->mDocManageDlg->InsertFilePath(data);
	}
}

void CRemoteControlDoc::OnRcDeleteFileFailed( const CString& ip, const void* msg )
{
	PClientDescripter pdes = GetClientDescripter(ip);
	if (NULL != pdes)
	{
		CString strPath;

		//显示一个提示消息
		CString temp;
		CString strMsg = _T("\"") + pdes->mName + _T("(") + pdes->mIP + _T("):");
		strPath += PRCMSG_BUFF(msg)->buf;
		if (strPath.Left(_tcslen(DIR_DESKTOP)) == DIR_DESKTOP)
		{
			temp.LoadString(IDS_DESKTOP);
			strPath.Replace(DIR_DESKTOP , temp);
		}else if(strPath.Left(_tcslen(DIR_MY_DOC)) == DIR_MY_DOC)
		{
			temp.LoadString(IDS_MY_DOC);
			strPath.Replace(DIR_MY_DOC , temp);
		}
		temp.LoadString(IDS_DELETE_FAILED);
		strMsg += (strPath + _T("\" ") + temp);
		temp.LoadString(IDS_NOTIFY);
		MessageBox(NULL , strMsg, temp , MB_OK|MB_ICONWARNING);
	}
}

void CRemoteControlDoc::OnRcDeleteFileSuccess( const CString& ip, const void* msg )
{
	PClientDescripter pdes = GetClientDescripter(ip);
	if (NULL != pdes)
	{
		CString strPath;
		if (pdes->mDocManageDlg)
		{
			strPath += PRCMSG_BUFF(msg)->buf;
			if (strPath.Right(1) == _T('\\'))//去掉后面的"\"
				strPath = strPath.Left(strPath.GetLength()-1);
			int index = strPath.ReverseFind(_T('\\'));
			strPath = strPath.Left(index);//获取父目录
			pdes->mDocManageDlg->FlushDir(strPath);
		}
		strPath = _T("");

		//显示一个提示消息
		CString temp;
		CString strMsg = _T("\"") + pdes->mName + _T("(") + pdes->mIP + _T("):");
		strPath += PRCMSG_BUFF(msg)->buf;
		if (strPath.Left(_tcslen(DIR_DESKTOP)) == DIR_DESKTOP)
		{
			temp.LoadString(IDS_DESKTOP);
			strPath.Replace(DIR_DESKTOP , temp);
		}else if(strPath.Left(_tcslen(DIR_MY_DOC)) == DIR_MY_DOC)
		{
			temp.LoadString(IDS_MY_DOC);
			strPath.Replace(DIR_MY_DOC , temp);
		}
		temp.LoadString(IDS_DELETE_SUCCESS);
		strMsg += (strPath + _T("\" ") + temp);
		temp.LoadString(IDS_NOTIFY);
		MessageBox(NULL , strMsg, temp , MB_OK);
	}
}

void CRemoteControlDoc::OnFileManage()
{
	//显示cmd对话框
	POSITION pos = m_pClientList->GetFirstSelectedItemPosition();
	int nItem;
	CString strIP;
	while (pos != NULL)
	{
		nItem = m_pClientList->GetNextSelectedItem(pos);
		strIP = m_pClientList->GetItemText(nItem , CLIENT_LIST_IP);

		for (ClientList::iterator it = m_lstClient.begin() ; it != m_lstClient.end() ; ++it)
		{
			if ((*it)->mIP == strIP)
			{
				CreateDocManageDlg(strIP);
			}
		}
	}
}

void CRemoteControlDoc::DownloadFile( const CString& strPath, const CString& ip)
{
	//向制定客户端下载指定文件

	CString savePath;
	PDowmUploadContext ddes = new DowmUploadContext();
	PClientDescripter cdes = GetClientDescripter(ip);
	if (NULL == cdes)
		return ;
	
	{//获取下载后的文件保存路径
	CString fileName = GetFileNameFromPath(strPath);
	CFileDialog dlg(FALSE , NULL , fileName);
	if(IDOK != dlg.DoModal())//取消了？
		return;

	//获得保存路径
	savePath = dlg.GetPathName();
	}//获取下载后的文件保存路径

	ddes->nClientID = cdes->mIndex;
	ddes->clientPath = strPath;
	ddes->curPath = savePath;
	ddes->pWnd = m_pDownUploadDlg;
	ddes->uDownUploadListenPort = cdes->mDownUploadListenPort;
	ddes->isDownload = TRUE;
	ddes->clientIP = cdes->mIP;

	//添加到下载队列
	m_pDownUploadDlg->AddDownUploadItem(ddes);

	{//创建下载线程
	InterlockedExchange(&(ddes->lThreadState) , 1);
	DWORD dwThreadID = 0;
	HANDLE hHandle = CreateThread(NULL , 0 ,DownUploadFileThread_S , ddes , CREATE_SUSPENDED  , &dwThreadID);
	ddes->dlThread = hHandle;
	ResumeThread(hHandle);
	}//创建下载线程

	//显示下载列表
	m_pDownUploadDlg->ShowWindow(SW_SHOW);
}

void CRemoteControlDoc::UploadFile( const CString& strPath, const CString& ip )
{
	//向制定客户端下载指定文件

	CString curPath;
	PDowmUploadContext ddes = new DowmUploadContext();
	PClientDescripter cdes = GetClientDescripter(ip);
	if (NULL == cdes)
		return ;

	{//获取要上传的文件
	CFileDialog dlg(TRUE , NULL , NULL);
	if(IDOK != dlg.DoModal())//取消了？
		return;
	
	//需要上传的文件
	curPath = dlg.GetPathName();

	{//获得文件大小
	CFile file(curPath , CFile::modeRead|CFile::typeBinary|CFile::shareDenyRead);
	ddes->fileLen = (DWORD)file.GetLength();
	file.Close();
	}

	ddes->nClientID = cdes->mIndex;
	ddes->clientPath = strPath;
	ddes->clientPath += GetFileNameFromPath(curPath);
	ddes->curPath = curPath;
	ddes->pWnd = m_pDownUploadDlg;
	ddes->uDownUploadListenPort = cdes->mDownUploadListenPort;
	ddes->isDownload = FALSE;
	ddes->clientIP = cdes->mIP;
	}//获取要上传的文件

	//添加到下载队列
	m_pDownUploadDlg->AddDownUploadItem(ddes);

	{//创建下载线程
	InterlockedExchange(&(ddes->lThreadState) , 1);
	DWORD dwThreadID = 0;
	HANDLE hHandle = CreateThread(NULL , 0 ,DownUploadFileThread_S , ddes , CREATE_SUSPENDED  , &dwThreadID);
	ddes->dlThread = hHandle;
	ResumeThread(hHandle);
	}//创建下载线程

	//显示下载列表
	m_pDownUploadDlg->ShowWindow(SW_SHOW);
}

void CRemoteControlDoc::OnRcListenDownloadPort( const CString& ip, const void* msg )
{
	PClientDescripter cdes = GetClientDescripter(ip);
	if (NULL != cdes)
	{
		cdes->mDownUploadListenPort = *((USHORT*)(PRCMSG_BUFF(msg)->buf));
	}
}

PClientDescripter CRemoteControlDoc::GetClientDescripter( const CString& ip )
{
	for (ClientList::iterator it = m_lstClient.begin() ; it != m_lstClient.end() ; ++it)
	{
		if ((*it)->mIP == ip)
		{
			return *it;
		}
	}
	return NULL;
}

void CRemoteControlDoc::OnDownload()
{
	m_pDownUploadDlg->ShowWindow(m_pDownUploadDlg->IsWindowVisible()?SW_HIDE:SW_SHOW);
}

void CRemoteControlDoc::OnUpdateDownload(CCmdUI *pCmdUI)
{
	if(m_pDownUploadDlg->IsWindowVisible())
		pCmdUI->SetCheck(TRUE);
	else 
		pCmdUI->SetCheck(FALSE);
}
