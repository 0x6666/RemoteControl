///////////////////////////////////////////////////////////////
//
// FileName	: RemoteControlDoc.cpp 
// Creator	: ����
// Date		: 2013��2��27��, 20:10:26
// Comment	: Զ�ؿ��Ʒ������ĵ���ͼ�ṹ�е��ĵ����ʵ��
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


//���ڸ����ͻ���ID����
int ClientDescripter::LAST_INDEX = 1;

// CRemoteControlDoc ����/����

CRemoteControlDoc::CRemoteControlDoc()
: m_pClientList(NULL)
, m_pMsgCenter(NULL)
, m_pView(NULL)
, m_bLockScreenSizeRatio(TRUE)
, m_czConfigfilePath(NULL)
, m_pDownUploadDlg(NULL)
{
	//�����߳������ĳ�ʼ��
	m_PushServerDesktopThreadContext.lThreadState = 0;
	m_PushServerDesktopThreadContext.hThread = NULL;
	m_PushServerDesktopThreadContext.lDesktopQuality = DEFAULT_SCREEN_QUALITY;
	m_PushServerDesktopThreadContext.lDesktopFrequency = DEFAULT_FREQUENCY_LEVEL;
	m_PushServerDesktopThreadContext.lDesktopW = SCREEN_SIZE_W;
	m_PushServerDesktopThreadContext.lDesktopH = SCREEN_SIZE_H;
	m_PushServerDesktopThreadContext.hPushedClientListMutex = NULL;

	//���ͻ��߳������ĳ�ʼ��
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

	//��ȡ�����ļ���·��
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
		if (i == 0 )//��ȡ·��ʧ��
			return FALSE;

		//ƴ�������ļ�·��
		strcat(modulePath , CONFIG_FILE_NAME);
		pathLen = strlen(modulePath);
		m_czConfigfilePath = new char[pathLen + 1];
		strcpy(m_czConfigfilePath , modulePath);
		m_czConfigfilePath[pathLen] = 0;
	}

	//���������ļ��е�����
	LoadConfigFileData();

	POSITION pos = GetFirstViewPosition();
	m_pView = dynamic_cast<CRemoteControlView*>(GetNextView(pos));
	ASSERT(NULL != m_pView);
	m_pClientList = (((CMainFrame*)m_pView->GetParentFrame())->m_pClientList);

	m_lstClient.clear();

	//����
	m_pMsgCenter = new MsgCenter();
	if(FALSE == m_pMsgCenter->InitMsgCenter(this , SERVER_MSG_PORT , TRUE))
	{//��ʼ��ʧ��
		return FALSE;
	}

	{//�������ضԻ���
	m_pDownUploadDlg = new CDownUploadDlg(AfxGetMainWnd());
	if (!m_pDownUploadDlg->Create(CDownUploadDlg::IDD , AfxGetMainWnd()))
	{
		delete m_pDownUploadDlg ;
		m_pDownUploadDlg = NULL;
		return FALSE;
	}
	m_pDownUploadDlg->UpdateWindow();
	}//�������ضԻ���

	//��ʼ���ͻ���
	StartChekClientThread();

	return TRUE;
}

// CRemoteControlDoc ���

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
	{//���ͻ����̲߳������еĻ�������
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
		{//���ض��Ѿ�����
			if ((*it)->mIsOnline == FALSE)
			{//���ض˲�����
				m_pMsgCenter->SendMsg(ip , (*it)->mPort , &rcMsg);
				(*it)->mIsOnline = FALSE;
				(*it)->mPreCheckOnlineState = FALSE;
			}
			break;
		}
	}

	if (it == m_lstClient.end())
	{//�ͻ��˲�����
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

		//�������ͻ���������ӵĻ���������Ҫ����һ����Ϣ�����ڿͻ��˻�ȡ�������ĵ�ַ
		m_pMsgCenter->SendMsg(ip , client->mPort , &rcMsg);
	}
	//ˢ�¿ͻ����б�ؼ�����ʾ������
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
	//ֹͣ���ͻ����߳�
	StopCheckClientThread();

	{//�������ضԻ���
	m_pDownUploadDlg->DestroyWindow();
	delete m_pDownUploadDlg;
	}//�������ضԻ���

	{//�����пͻ��˷��ͷ������˳�����Ϣ
	DEF_RCMSG(rcMsg , MT_SERVER_EXIT_S);
	SendAllClient(&rcMsg);
	}//�����пͻ��˷��ͷ������˳�����Ϣ

	//ֹͣ�����߳�
	StopPushServerDesktopTHread();

	//����������Ҫ���ٵ�����
	m_pView->DestroyView();

	if (NULL != m_pMsgCenter)
	{
		m_pMsgCenter->Close();
		delete m_pMsgCenter;
		m_pMsgCenter = NULL;
	}

	//����һ��
	for (ClientList::iterator it = m_lstClient.begin() ; it != m_lstClient.end(); ++it)
	{
		if (NULL != (*it)->mPushAddr)
		{
			delete (*it)->mPushAddr;
			(*it)->mPushAddr = NULL;
		}
		if (NULL != (*it)->mCMDDlg)
		{//����CMD�Ի���
			CCMDDlg* pDlg = (*it)->mCMDDlg;
			pDlg->DestroyWindow();
			delete pDlg;
			(*it)->mCMDDlg = NULL;
		}

		if (NULL != (*it)->mDocManageDlg)
		{//�����ļ��Ի���
			CFileManageDlg* pDlg = (*it)->mDocManageDlg;
			pDlg->DestroyWindow();
			delete pDlg;
			(*it)->mDocManageDlg = NULL;
		}

		delete (*it);
	}
	m_lstClient.clear();


	//ɾ������·������
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
	{//û��ѡ���κοͻ���
		TRACE0("û��ѡ���κοͻ���!\n");
		*pResult = 0;
		return ;
	}

	//���ѡ�����������
	int nItem = m_pClientList->GetNextSelectedItem(pos);
	CString temp;
	temp.LoadString(IDS_ONLINE);
	if (0 != m_pClientList->GetItemText(nItem , CLIENT_LIST_STATU).Compare(temp))
	{//������
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

	//��ݲ˵�
	menu.LoadMenu(IDR_CLIENT_LIST);
	menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN|TPM_TOPALIGN , point.x , point.y , AfxGetMainWnd() , NULL);
}

void CRemoteControlDoc::DispatchMsg(const void* msg , CString ip, UINT port)
{
	if (NULL == msg || ip.IsEmpty())
		return ;

	switch(PRCMSG(msg)->type)
	{
	case MT_FIND_SERVER_BROADCAST_C://�ͻ��˲�ѯ�������Ĺ㲥
		OnRcFindServerBraodcast(ip , msg);
		break;
	case MT_HEARTBEAT_C://�ͻ���������
		OnRcHeartbeat(ip , port , msg); 
		break;
	case MT_CONN_CLOSED_C: //һ���ͻ��˶Ͽ�����
		OnRcClosed(ip); 
		break;
	case MT_SCREEN_CAPTURE_START_C://���ض��Ѿ�ץ���� 
		OnRcStartCapture(ip , port , msg);
		break;
	case MT_PUSH_SERVER_DESKTOP_C://ȷ����������
		OnRcPushedServerDesktop(ip);
		break;
	case MT_CALCEL_PUSH_DESKTOP_C://һ���ͻ����Ѿ�ȷ����ȡ�����������
		OnRcCancelPushDesktop(ip);
		break;
	case MT_SHUTDOWN_FALIED_C://�ػ�ʧ��
		OnRcShutdownFailed(ip);
		break;
	case MT_RESTART_FALIED_C://����ʧ��
		OnRcRestartFailed(ip);
		break;
	case MT_LOGIN_OUT_FALIED_C://ע��ʧ��
		OnRcLoginOutFailed(ip);
		break;
	case MT_START_CMD_FAILED_C://����CMDʧ��
		OnRcStartCMDFailed(ip);
		break;
	case MT_CMD_LINE_DATA_C://�����л�������
		OnRcCMDLineData(ip , port , msg);
		break;
	case MT_HANDS_UP_C://�ͻ��˾���
		OnRcHandsUp(ip);
		break;
	case MT_CMD_FAILED_C://cmd����ִ��ʧ��
		OnRcCMDFailed(ip , msg);
		break;
	case MT_FILE_PATH_C://�յ�һ���ļ�·����Ϣ
		OnRcFilePath(ip , msg);
		break;
	case MT_DELETE_FILE_FAILED_C://ɾ���ļ�ʧ��
		OnRcDeleteFileFailed(ip , msg);
		break;
	case MT_DELETE_FILE_SUCCESS_C://ɾ���ļ��ɹ�
		OnRcDeleteFileSuccess(ip , msg);
		break;
	case MT_LISTEN_DOWNLOAD_PORT_C://�����ļ������˿�
		OnRcListenDownloadPort( ip , msg);
		break;
	default:
		ASSERT(FALSE);
	}
	return ;
}

void CRemoteControlDoc::OnRcHeartbeat( const CString& ip ,USHORT port,   const void* msg ) 
{//һ���ͻ��˷�����һ��������

	RCMSG rcMsg = {0};
	rcMsg.size = sizeof(RCMSG);
	rcMsg.type = MT_REPEAT_FIND_SERVER_S;

	ClientList::iterator it = m_lstClient.begin();
	for ( ; it != m_lstClient.end() ; ++it)
	{
		if ((*it)->mIP == ip)
		{//�ͻ����Ѿ�����
			if ((*it)->mIsOnline == FALSE)
			{//���֮ǰ�ͻ����ǵ����˵ģ���������Ҫ����һ����Ϣ�����ڿͻ��˻�ȡ�������ĵ�ַ
				m_pMsgCenter->SendMsg(ip , (*it)->mPort , &rcMsg);
			}
			(*it)->mIsOnline = TRUE;
			(*it)->mPreCheckOnlineState = TRUE;
			break;
		}
	}

	if (it == m_lstClient.end())
	{//�ͻ��˲�����  �����������ǲ���ִ�е�
		CString name = GetHostNameByAddr(ip);
		AddClient(ip, port , name.GetLength()?name:ip);
	}
	//ˢ�¿ͻ����б�ؼ�����ʾ������
	FlushClientCtrl();
}

void CRemoteControlDoc::OnRcClosed( const CString& ip )
{
	for (ClientList::iterator it = m_lstClient.begin() ; it != m_lstClient.end() ; ++it)
	{//�ð���ͻ��˽���еĿͻ���״̬
		if ((*it)->mIP == ip){
			(*it)->mIsOnline = FALSE;
			(*it)->mPreCheckOnlineState = FALSE;
			(*it)->mIsPushed = FALSE; 
			break;
		}
	}

	//���ͻ����б��е�����ˢ�µ��ؼ���ȥ
	FlushClientCtrl();

	//�����ͼ����ʾ�˿ͻ�����Ҳ��Ҫ֪ͨ��ı�״̬
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
		{//��ο���ȷ����������
			(*it)->mIsOnline = FALSE;
			m_pView->ClientDropped((*it)->mIP);
		}
		else
		{//������μ���״̬
			(*it)->mPreCheckOnlineState = FALSE;
		}
	}
	FlushClientCtrl();
}

void CRemoteControlDoc::FlushClientCtrl()
{//���ͻ����б��е�����ˢ���ͻ����б�ؼ���ȥ
	CString strOnline , strCloseed , strPushed , strUnPushed;
	strOnline.LoadString(IDS_ONLINE);
	strCloseed.LoadString(IDS_CLOSED);
	strPushed.LoadString(IDS_YES);
	strUnPushed.LoadString(IDS_NO);

	//���ߵĿͻ�������
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
				//��������״̬
				if ((*it)->mIsOnline)  //����
				{
					m_pClientList->SetItemText( i , CLIENT_LIST_STATU , strOnline);
					++nOnlineCnt;
					
					if ((*it)->mIsPushed)
					{//������
						++nPushCnt;
						m_pClientList->SetItemText( i , CLIENT_LIST_PUSH , strPushed);
					}else
						m_pClientList->SetItemText( i , CLIENT_LIST_PUSH , strUnPushed);
				}
				else//������
				{
					m_pClientList->SetItemText( i , CLIENT_LIST_STATU , strCloseed);
					m_pClientList->SetItemText( i , CLIENT_LIST_PUSH , strUnPushed);
				}
				break;
			}
		}

		if(i == cnt)
		{//�ؼ��в����������
			CString temp;
			temp.Format(_T("%d") , (*it)->mIndex);
			m_pClientList->InsertItem( i , temp );
			m_pClientList->SetItemText( i , CLIENT_LIST_IP , (*it)->mIP);
			m_pClientList->SetItemText( i , CLIENT_LIST_NAME , (*it)->mName);

			//��������״̬
			if ((*it)->mIsOnline)
			{//����
				++nOnlineCnt;
				m_pClientList->SetItemText( i , CLIENT_LIST_STATU , strOnline );

				if ((*it)->mIsPushed)
				{//������
					++nPushCnt;
					m_pClientList->SetItemText( i , CLIENT_LIST_PUSH , strPushed );
				}else
					m_pClientList->SetItemText( i , CLIENT_LIST_STATU , strUnPushed );
			}else
			{//������
				m_pClientList->SetItemText( i , CLIENT_LIST_STATU , strCloseed );
				m_pClientList->SetItemText( i , CLIENT_LIST_PUSH , strUnPushed );
			}

			//����IP��ַ
			m_pClientList->SetItemText( i , CLIENT_LIST_IP , (*it)->mIP );
		}
	}
	CWnd* pMainFrame = AfxGetMainWnd();
	if ((NULL != pMainFrame)&&
		(IsWindow(pMainFrame->GetSafeHwnd())) )
	{
		//֪ͨ�����ڸı�״̬��ʾ
		pMainFrame->PostMessage(WM_CLIENT_COUNT , m_lstClient.size() , nOnlineCnt);
		
		//����������
		pMainFrame->PostMessage(WM_PUSHED_COUNT , nPushCnt , 0 );
	}
}

void CRemoteControlDoc::OnPushServer()
{
	//TODO: �����û�������������̵߳Ļ����������������������߳�
	if (0 == m_PushServerDesktopThreadContext.lThreadState)
	{
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lThreadState) , 1);
		m_PushServerDesktopThreadContext.hPushedClientListMutex = CreateMutex(NULL , FALSE , NULL);
		DWORD dwThreadId = 0;
		m_PushServerDesktopThreadContext.hThread =
			CreateThread(NULL , 0 , PushSeverDesktopThread , &m_PushServerDesktopThreadContext, 0 , &dwThreadId );
	}

	//��ѡ���˵Ŀͻ�����������
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
				continue;//�Ѿ������������������ͻ���
			else
			{//��������������Ϣ
				m_pMsgCenter->SendMsg(strIP , cdes->mPort , &rcMsg);
			}
		}
	}
}

void CRemoteControlDoc::OnRcPushedServerDesktop( const CString& ip )
{//�յ���һ���ͻ��˻�����������ȷ����Ϣ
	char czIP[20] = {0};
	PClientDescripter cdes = GetClientDescripter(ip);

	if ((NULL != cdes) && (cdes->mIsPushed == FALSE))
	{
		if (cdes->mPushAddr == NULL)
		{//��ַ�ṹ��û�д���
			wcstombs( czIP , cdes->mIP , 20 );
			cdes->mPushAddr = new sockaddr_in;
			memset(cdes->mPushAddr , 0 , sizeof(sockaddr_in));
			cdes->mPushAddr->sin_family = AF_INET;
			cdes->mPushAddr->sin_port = htons(SERVER_PUSH_DESKTOP_PORT);
			cdes->mPushAddr->sin_addr.s_addr = inet_addr(czIP);
		}
		cdes->mIsPushed = TRUE;

		//���ͻ��˵ĵ�ַ�ṹָ��������Ϳͻ�������
		WaitForSingleObject(m_PushServerDesktopThreadContext.hPushedClientListMutex , INFINITE);
		m_PushServerDesktopThreadContext.lstPushedClient.push_back(cdes->mPushAddr);
		ReleaseMutex(m_PushServerDesktopThreadContext.hPushedClientListMutex);

		//���¿ͻ����б���ʾ
		FlushClientCtrl();
	}
}

void CRemoteControlDoc::StopPushServerDesktopTHread()
{
	if (m_PushServerDesktopThreadContext.lThreadState)
	{//���ͷ����������߳������е������
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lThreadState) , 0);
		WaiteExitThread(m_PushServerDesktopThreadContext.hThread, 2*1000);
		CloseHandle(m_PushServerDesktopThreadContext.hThread);
		m_PushServerDesktopThreadContext.hThread = NULL;
	}
}

void CRemoteControlDoc::OnOption()
{
	//���öԻ���ĳ�ʼֵ
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
	//ȡ������
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
			if (cdes->mIsPushed)//�Ѿ�������ȡ��
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
		//�����Ϳͻ������Ƴ����͵�ַ
		WaitForSingleObject(m_PushServerDesktopThreadContext.hPushedClientListMutex , INFINITE);
		m_PushServerDesktopThreadContext.lstPushedClient.remove(cdes->mPushAddr);
		if (m_PushServerDesktopThreadContext.lstPushedClient.empty())//�Ѿ����ˣ���Ҫֹͣ�����߳�
			emptyFlag = 1;
		ReleaseMutex(m_PushServerDesktopThreadContext.hPushedClientListMutex);

		if(emptyFlag == 1)//û������Ҫ���͵Ŀͻ��˾�ֹͣ�����߳�
			StopPushServerDesktopTHread();

		//ȡ�����ͱ��
		cdes->mIsPushed = FALSE;

		//������ʾ
		FlushClientCtrl();
	}
}

void CRemoteControlDoc::LoadConfigFileData()
{
	//���ڱ�����ֵת�����ַ������ֵ
	char vBuf[100] = {0};
	RCF rcf = RCF_OK;
	long temp = 0;

	{//������ص��������ݼ���
	//��������ʱ�Ŀ��
	rcf = ReadConfValue(CK_PUSH_SD_W , vBuf , 100 , m_czConfigfilePath);
	if (rcf == RCF_OK)
	{
		temp = atoi(vBuf);
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lDesktopW) , temp);
	}else
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lDesktopW) , SCREEN_SIZE_W);
	
	//��������ʱ�ĸ߶�
	rcf = ReadConfValue(CK_PUSH_SD_H , vBuf , 100 , m_czConfigfilePath);
	if (rcf == RCF_OK)
	{
		temp = atoi(vBuf);
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lDesktopH) , temp);
	}else
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lDesktopH) , SCREEN_SIZE_H);
	
	//��������ʱ������
	rcf = ReadConfValue(CK_PUSH_SD_QUALITY , vBuf , 100 , m_czConfigfilePath);
	if (rcf == RCF_OK)
	{
		temp = atoi(vBuf);
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lDesktopQuality) , temp );
	}else
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lDesktopQuality), DEFAULT_SCREEN_QUALITY );

	//��������ʱ��������
	rcf = ReadConfValue(CK_PUSH_SD_FREQUNCY , vBuf , 100 , m_czConfigfilePath);
	if (rcf == RCF_OK)
	{
		temp = atoi(vBuf);
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lDesktopFrequency), temp );
	}else
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lDesktopFrequency) , DEFAULT_FREQUENCY_LEVEL );

	//��������ʱ���Ƿ��������߱�
	rcf = ReadConfValue(CK_PUSH_SD_LOCK_RATIO , vBuf , 100 , m_czConfigfilePath);
	if (rcf == RCF_OK)
	{
		temp = atoi(vBuf);
		m_bLockScreenSizeRatio = temp ;
	}else
		m_bLockScreenSizeRatio = TRUE ;
	}//������ص��������ݼ���

	{//CMD���ڵ��������ݼ���
	//������ɫ
	rcf = ReadConfValue(CK_CMD_FONT_COLOR , vBuf , 100 , m_czConfigfilePath);
	if (rcf == RCF_OK)
	{
		temp = atoi(vBuf);
		m_cmdValue.nCMD_FontColor = temp;
	}else
		m_cmdValue.nCMD_FontColor = RGB(255 , 255 , 255);//Ĭ�ϰ�ɫ
	//������ɫ
	rcf = ReadConfValue(CK_CMD_BG_COLOR , vBuf , 100 , m_czConfigfilePath);
	if (rcf == RCF_OK)
	{
		temp = atoi(vBuf);
		m_cmdValue.nCMD_BackgroundColor = temp;
	}else
		m_cmdValue.nCMD_BackgroundColor = RGB(0 , 0 , 0);//Ĭ�Ϻ�ɫ;
	//�����С
	rcf = ReadConfValue(CK_CMD_FONT_SIZE , vBuf , 100 , m_czConfigfilePath);
	if (rcf == RCF_OK)
	{
		temp = atoi(vBuf);
		m_cmdValue.nCMD_FontSize = temp;
	}else
		m_cmdValue.nCMD_FontSize = -25;
	//�����ַ���
	rcf = ReadConfValue(CK_CMD_FONT_CHAR_SET , vBuf , 100 , m_czConfigfilePath);
	if (rcf == RCF_OK)
	{
		temp = atoi(vBuf);
		m_cmdValue.nCMD_FontCharset = (BYTE)temp;
	}else
		m_cmdValue.nCMD_FontCharset = 1;
	//��������
	rcf = ReadConfValue(CK_CMD_FONT_FACENAME , vBuf , 100 , m_czConfigfilePath);
	if (rcf == RCF_OK)
	{
		strcpy(m_cmdValue.czCMD_FontFaceName , vBuf);
	}else
		strcpy(m_cmdValue.czCMD_FontFaceName , "������");
	}
}

void CRemoteControlDoc::StopMonitoring( const CString& strIP )
{
// 	int emptyFlag = 0;
// 	for (ClientList::iterator it = m_lstClient.begin() ; it != m_lstClient.end() ; ++it)
// 	{
// 		if ((*it)->mIp == strIP)
// 		{//�ҵ����ƶ����Ѿ����͵Ŀͻ���
// 			if ((*it)->bPushed)
// 			{//�Ѿ������͵Ļ���ȡ��
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
		msg.Format(_T("���ض� ID=%d ����") , pcdes->mIndex);
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
		{//�ͻ����Ѿ�����
			if ((*it)->mIsOnline == FALSE)
			{//���֮ǰ�ͻ����ǵ����˵ģ���������Ҫ����һ����Ϣ�����ڿͻ��˻�ȡ�������ĵ�ַ
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
	{//�ͻ��˲�����
		CString name ;
		name += pMsg->name;
		AddClient(ip, pMsg->port , name);

		//�������ͻ���������ӵĻ���������Ҫ����һ����Ϣ�����ڿͻ��˻�ȡ�������ĵ�ַ
		m_pMsgCenter->SendMsg(ip , pMsg->port , &rcMsg);

	}
	//ˢ�¿ͻ����б�ؼ�����ʾ������
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
		{//�Ի����Ѿ�����
			pdes->mCMDDlg->ShowWindow(!pdes->mCMDDlg->IsWindowVisible());
			return TRUE;
		}else
		{//�Ի��򲻴���,����

			pdes->mCMDDlg = new CCMDDlg(pdes->mIP ,pdes->mPort , pdes->mName , this , &m_cmdValue);

			if (!pdes->mCMDDlg->Create(IDD_CMD_VIEW , pMainFrame))
			{
				TRACE0("����CMD�Ի���ʧ��\n");
				delete pdes->mCMDDlg ;
				pdes->mCMDDlg = NULL;
				return FALSE;
			}
			pdes->mCMDDlg->UpdateWindow();
			pdes->mCMDDlg->ShowWindow(SW_SHOW);

			//����һ������CMD����Ϣ
			DEF_RCMSG(rcMsg , MT_START_CMD_S);
			m_pMsgCenter->SendMsg(pdes->mIP , pdes->mPort , &rcMsg);
			return TRUE;
		}
	}
	return FALSE;
}

void CRemoteControlDoc::OnCmd()
{
	//��ʾcmd�Ի���
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
	//���ڱ�����ֵת�����ַ������ֵ
	char vBuf[10] = {0};
	//����ȡ�������ύ
	if (m_PushServerDesktopThreadContext.lDesktopW != pushValue->nPush_DesktopW)
	{//��������ʱ�Ŀ��
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lDesktopW) , pushValue->nPush_DesktopW);
		WriteConfValue(CK_PUSH_SD_W , itoa( (int)pushValue->nPush_DesktopW , vBuf , 10)
			, m_czConfigfilePath);
	}

	if (m_PushServerDesktopThreadContext.lDesktopH != pushValue->nPush_DesktopH)
	{//��������ʱ�ĸ߶�
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lDesktopH) , pushValue->nPush_DesktopH);
		WriteConfValue(CK_PUSH_SD_H , itoa( (int)pushValue->nPush_DesktopH , vBuf , 10) 
			, m_czConfigfilePath);
	}
	if (m_PushServerDesktopThreadContext.lDesktopQuality != pushValue->nPush_DesktopQuality)
	{//��������ʱ������
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lDesktopQuality) , pushValue->nPush_DesktopQuality);
		WriteConfValue(CK_PUSH_SD_QUALITY , itoa( (int)pushValue->nPush_DesktopQuality , vBuf , 10) 
			, m_czConfigfilePath);
	}
	if (m_PushServerDesktopThreadContext.lDesktopFrequency != pushValue->nPush_DesktopFrequency)
	{//��������ʱ��������
		InterlockedExchange(&(m_PushServerDesktopThreadContext.lDesktopFrequency) , pushValue->nPush_DesktopFrequency);
		WriteConfValue(CK_PUSH_SD_FREQUNCY , itoa( (int)pushValue->nPush_DesktopFrequency , vBuf , 10)
			, m_czConfigfilePath);
	}
	if (m_bLockScreenSizeRatio != pushValue->bPush_LockScreenSizeRatio)
	{//��������ʱ���Ƿ��������߱�
		m_bLockScreenSizeRatio = pushValue->bPush_LockScreenSizeRatio;
		WriteConfValue(CK_PUSH_SD_LOCK_RATIO , itoa( (int)pushValue->bPush_LockScreenSizeRatio , vBuf , 10) 
			, m_czConfigfilePath);
	}
}

void CRemoteControlDoc::ChangeCMDCfgValue( PCMD_CFG_V cmdValue )
{
	//���ڱ�����ֵת�����ַ������ֵ
	char vBuf[10] = {0};
	//����ȡ�������ύ
	if (strcmp(m_cmdValue.czCMD_FontFaceName , cmdValue->czCMD_FontFaceName))
	{//��������
		strcpy(m_cmdValue.czCMD_FontFaceName , cmdValue->czCMD_FontFaceName);
		WriteConfValue(CK_CMD_FONT_FACENAME , cmdValue->czCMD_FontFaceName, m_czConfigfilePath);
	}

	if (m_cmdValue.nCMD_BackgroundColor != cmdValue->nCMD_BackgroundColor)
	{//������ɫ
		m_cmdValue.nCMD_BackgroundColor = cmdValue->nCMD_BackgroundColor;
		WriteConfValue(CK_CMD_BG_COLOR , itoa( (int)cmdValue->nCMD_BackgroundColor , vBuf , 10) 
			, m_czConfigfilePath);
	}
	if (m_cmdValue.nCMD_FontCharset != cmdValue->nCMD_FontCharset)
	{//�ַ���
		m_cmdValue.nCMD_FontCharset = cmdValue->nCMD_FontCharset;
		WriteConfValue(CK_CMD_FONT_CHAR_SET , itoa( (int)cmdValue->nCMD_FontCharset , vBuf , 10) 
			, m_czConfigfilePath);
	}
	if (m_cmdValue.nCMD_FontColor != cmdValue->nCMD_FontColor)
	{//������ɫ
		m_cmdValue.nCMD_FontColor = cmdValue->nCMD_FontColor;
		WriteConfValue(CK_CMD_FONT_COLOR, itoa( (int)cmdValue->nCMD_FontColor , vBuf , 10)
			, m_czConfigfilePath);
	}
	if (m_cmdValue.nCMD_FontSize != cmdValue->nCMD_FontSize )
	{//�����С
		m_cmdValue.nCMD_FontSize = cmdValue->nCMD_FontSize;
		WriteConfValue(CK_CMD_FONT_SIZE , itoa( (int)cmdValue->nCMD_FontSize , vBuf , 10) 
			, m_czConfigfilePath);
	}

	//����ÿһ������֪ͨ��ı���������
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
			{//�Ի����Ѿ�����
				(*it)->mDocManageDlg->ShowWindow(!(*it)->mDocManageDlg->IsWindowVisible());
				return TRUE;
			}else
			{//�Ի��򲻴���,����

				(*it)->mDocManageDlg = new CFileManageDlg((*it)->mIP ,(*it)->mPort , (*it)->mName , this );

				if (!(*it)->mDocManageDlg->Create(IDD_DOC_MANAGE_DLG , pMainFrame))
				{
					TRACE0("�����ļ�����Ի���ʧ��\n");
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

		//��ʾһ����ʾ��Ϣ
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
			if (strPath.Right(1) == _T('\\'))//ȥ�������"\"
				strPath = strPath.Left(strPath.GetLength()-1);
			int index = strPath.ReverseFind(_T('\\'));
			strPath = strPath.Left(index);//��ȡ��Ŀ¼
			pdes->mDocManageDlg->FlushDir(strPath);
		}
		strPath = _T("");

		//��ʾһ����ʾ��Ϣ
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
	//��ʾcmd�Ի���
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
	//���ƶ��ͻ�������ָ���ļ�

	CString savePath;
	PDowmUploadContext ddes = new DowmUploadContext();
	PClientDescripter cdes = GetClientDescripter(ip);
	if (NULL == cdes)
		return ;
	
	{//��ȡ���غ���ļ�����·��
	CString fileName = GetFileNameFromPath(strPath);
	CFileDialog dlg(FALSE , NULL , fileName);
	if(IDOK != dlg.DoModal())//ȡ���ˣ�
		return;

	//��ñ���·��
	savePath = dlg.GetPathName();
	}//��ȡ���غ���ļ�����·��

	ddes->nClientID = cdes->mIndex;
	ddes->clientPath = strPath;
	ddes->curPath = savePath;
	ddes->pWnd = m_pDownUploadDlg;
	ddes->uDownUploadListenPort = cdes->mDownUploadListenPort;
	ddes->isDownload = TRUE;
	ddes->clientIP = cdes->mIP;

	//��ӵ����ض���
	m_pDownUploadDlg->AddDownUploadItem(ddes);

	{//���������߳�
	InterlockedExchange(&(ddes->lThreadState) , 1);
	DWORD dwThreadID = 0;
	HANDLE hHandle = CreateThread(NULL , 0 ,DownUploadFileThread_S , ddes , CREATE_SUSPENDED  , &dwThreadID);
	ddes->dlThread = hHandle;
	ResumeThread(hHandle);
	}//���������߳�

	//��ʾ�����б�
	m_pDownUploadDlg->ShowWindow(SW_SHOW);
}

void CRemoteControlDoc::UploadFile( const CString& strPath, const CString& ip )
{
	//���ƶ��ͻ�������ָ���ļ�

	CString curPath;
	PDowmUploadContext ddes = new DowmUploadContext();
	PClientDescripter cdes = GetClientDescripter(ip);
	if (NULL == cdes)
		return ;

	{//��ȡҪ�ϴ����ļ�
	CFileDialog dlg(TRUE , NULL , NULL);
	if(IDOK != dlg.DoModal())//ȡ���ˣ�
		return;
	
	//��Ҫ�ϴ����ļ�
	curPath = dlg.GetPathName();

	{//����ļ���С
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
	}//��ȡҪ�ϴ����ļ�

	//��ӵ����ض���
	m_pDownUploadDlg->AddDownUploadItem(ddes);

	{//���������߳�
	InterlockedExchange(&(ddes->lThreadState) , 1);
	DWORD dwThreadID = 0;
	HANDLE hHandle = CreateThread(NULL , 0 ,DownUploadFileThread_S , ddes , CREATE_SUSPENDED  , &dwThreadID);
	ddes->dlThread = hHandle;
	ResumeThread(hHandle);
	}//���������߳�

	//��ʾ�����б�
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
