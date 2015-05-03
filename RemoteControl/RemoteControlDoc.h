///////////////////////////////////////////////////////////////
//
// FileName	: RemoteControlDoc.h
// Creator	: ����
// Date		: 2013��2��27��, 20:10:26
// Comment	: Զ�ؿ��Ʒ������ĵ���ͼ�ṹ�е��ĵ��������
//
//////////////////////////////////////////////////////////////

#pragma once

//�ͻ����б��к�
enum{
	CLIENT_LIST_ID = 0 ,
	CLIENT_LIST_STATU ,
	CLIENT_LIST_PUSH ,	
	CLIENT_LIST_NAME ,
	CLIENT_LIST_IP,
};

class CRemoteControlView;
class CCMDDlg;
class CFileManageDlg;
class CDownUploadDlg;

//////////////////////////////////////////////////////////////////////////
//�ͻ���������
typedef struct ClientDescripter{
	int			mIndex;				//�ͻ�������
	CString		mName;				//���ض˼������
	CString		mIP;				//���ض˵�ַ
	USHORT		mPort;				//����Ϣ�˿�
	BOOL		mIsOnline;			//�Ƿ�����
	BOOL		mPreCheckOnlineState;//���ڼ�����߳�״̬�ĸ�����־
	BOOL		mIsPushed;			//����״̬
	sockaddr_in* mPushAddr;			//������������ĵ�ַ�ṹ
	CCMDDlg*	mCMDDlg;			//�����жԻ���
	CFileManageDlg* mDocManageDlg;	//�ļ�����Ի���
	USHORT		mDownUploadListenPort;//���ؼ����˿�

	//���캯�����ڳ�ʼ��
	ClientDescripter()
		: mIndex(LAST_INDEX++)
		, mPort(CLIENT_MSG_PORT)
		, mIsOnline(FALSE)
		, mPreCheckOnlineState(FALSE)
		, mIsPushed(FALSE)
		, mPushAddr(NULL)
		, mCMDDlg(NULL)
		, mDocManageDlg(NULL)
		, mDownUploadListenPort(0)
	{}
	static int LAST_INDEX;	//���ڸ������ͻ��˷�������
}*PClientDescripter;


class CRemoteControlDoc : public CDocument , public Dispatcher
{
protected: // �������л�����
	CRemoteControlDoc();
	DECLARE_DYNCREATE(CRemoteControlDoc)

private:
	
	//���ͻ����߳�������
	CheckClientThreadContext m_CheckClientThreadContext;

	//�ͻ���������������
	typedef std::list<ClientDescripter*> ClientList;
	ClientList m_lstClient;

	//�ͻ�������ؼ�
	CListCtrl*	m_pClientList;

	//��Ϣ����
	MsgCenter* m_pMsgCenter;

	//�ĵ���ͼ����ͼָ�룬��������Ϊһ����Ա������Ϊ�˲�������
	CRemoteControlView* m_pView;

	//���ͷ����������߳�������
	PushServerDesktopThreadContext m_PushServerDesktopThreadContext;

	//������������Ĵ�С�ı���
	BOOL m_bLockScreenSizeRatio;

	//CMD���ڵ��������ݣ�����û�������͵���
	//��һ������д������Ϊ��Щ��Ҫ�ڶ��߳���ʹ��
	CMD_CFG_V m_cmdValue;

	//�����ļ�·��
	char* m_czConfigfilePath;

	//CMD��ɫ����
	COLORREF m_fontColor;
	// ������ɫ
	COLORREF m_bkColor;

	//�����ļ��Ի���
	CDownUploadDlg* m_pDownUploadDlg;

// ����
public:
	//////////////////////////////////////////////////////////////////////////
	//�ҵ�һ���ͻ���,������������ѯ�ͻ��̵߳��õĺ���
	//param
	//		name	�ҵ��������ļ������
	//		ip		�ҵ���������IP��ַ
	void FoundAClient(const CString& name ,const CString& ip );
	
	//////////////////////////////////////////////////////////////////////////
	//ֹͣ���ͻ����߳�
	void StopCheckClientThread();
	
	//////////////////////////////////////////////////////////////////////////
	//��ʼ���ͻ����߳�
	void StartChekClientThread();

	//////////////////////////////////////////////////////////////////////////
	//��ָ���ͻ��˷�����Ϣ
	//param
	//		ip		�ͻ��˵�ַ
	//		port	�ͻ��˽�����Ϣ�Ķ˿�
	//		msg		��Ϣ����
	void SendRcMessage(const CString& ip , USHORT port , void* msg);

	//////////////////////////////////////////////////////////////////////////
	//���������ļ�
	void LoadConfigFileData();

	//////////////////////////////////////////////////////////////////////////
	//����CMD�Ի�������Ѿ���������ʾ/����
	//param
	//		strIP	�ͻ���ip
	//return 
	//		TRUE	�����ɹ�
	//		FALSE	����ʧ��
	BOOL CreateCMDDlg(const CString& strIP);

	//���ͻ���״̬����ס��ܶ�ʱ������������
	void CheckClientState();

	//ˢ�¿ͻ����б�ؼ��е�����
	void FlushClientCtrl();

	//ֹͣ���ͷ���������
	void StopPushServerDesktopTHread();

	//ɾ��һ���ͻ���
	void StopMonitoring( const CString& strIP );

	//���һ���ͻ���
	void AddClient(  const CString& ip, USHORT port ,  const CString& strName );

	//����������øı�
	void ChangePushCfgValue( PPUSH_CFG_V pushValue );
	
	//CMD������øı�
	void ChangeCMDCfgValue( PCMD_CFG_V cmdValue );
	
	//��ÿһ���ͻ��˷�����Ϣ
	void SendAllClient( PRCMSG rcMsg );
	
	//�ļ�����Ի���
	BOOL CreateDocManageDlg( const CString& ip );
	
	//���ƶ��ͻ�������ָ���ļ�
	void DownloadFile( const CString& strPath, const CString& ip);
	
	//��ָ���ͻ����ϴ��ļ�
	void UploadFile( const CString& strPath, const CString& m_strIP );

	//���ָ���Ŀͻ���������
	PClientDescripter GetClientDescripter(const CString& ip);

//�ͻ�����������Ϣ������
private:
	//////////////////////////////////////////////////////////////////////////
	//���ҷ�������Ϣ
	//param
	//		ip	�ͻ����ĵ�ַ
	//		port �ͻ���������
	//		msg	��Ϣ����
	void OnRcHeartbeat(const CString& ip ,USHORT port, const void* msg ) ;
	//һ���ͻ��˶Ͽ�����
	void OnRcClosed( const CString& ip ) ;
	//���ض��Ѿ�ץ���� 
	void OnRcStartCapture(const CString& ip , USHORT port , const void* msg);
	//ȷ����������
	void OnRcPushedServerDesktop( const CString& ip );
	//һ���ͻ����Ѿ�ȷ����ȡ�����������
	void OnRcCancelPushDesktop( const CString& ip );
	//�ͻ��˹ػ�ʧ��
	void OnRcShutdownFailed( const CString& ip );
	//����ʧ��
	void OnRcRestartFailed( const CString& ip );
	//ע��ʧ��
	void OnRcLoginOutFailed( const CString& ip );
	//����CMDʧ��
	void OnRcStartCMDFailed( const CString& ip );
	//�����л�������
	void OnRcCMDLineData( const CString& ip , USHORT port , const void* msg );
	//�ͻ��˾���
	void OnRcHandsUp( const CString& ip );
	//�ͻ��˲�ѯ�������Ĺ㲥��Ϣ
	void OnRcFindServerBraodcast( const CString& ip, const void* msg );
	//һ��cmd����ִ��ʧ��
	void OnRcCMDFailed( const CString& ip, const void* msg );
	//�յ�һ���ļ�·����Ϣ
	void OnRcFilePath( const CString& ip, const void* msg );
	//ɾ���ļ�ʧ��
	void OnRcDeleteFileFailed( const CString& ip, const void* msg );
	//ɾ���ļ��ɹ�
	void OnRcDeleteFileSuccess( const CString& ip, const void* msg );
	//���������ļ��˿�
	void OnRcListenDownloadPort( const CString& ip, const void* msg );

// ��д
public:
	//��ͼ�ĵ��ṹ���ĵ���������
	virtual BOOL OnNewDocument();

	//�ĵ��رպ������ڴ���һЩ����������
	virtual void OnCloseDocument();

	//////////////////////////////////////////////////////////////////////////
	//��Ϣ���ͺ��������͵��ǿͻ��˷��͹�������Ϣ
	//param
	//		msg		��Ϣ����(RCMSG)
	//		ip		�ͻ���ip��ַ
	//		port	�ͻ��˷���Ϣ�Ķ˿�
	virtual void DispatchMsg(const void* msg , CString ip, UINT port);

// ʵ��
public:
	virtual ~CRemoteControlDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// ���ɵ���Ϣӳ�亯��
protected:
	//////////////////////////////////////////////////////////////////////////
	//����Ϣ��m_pContentList��˫�����¼�����Ӧ����Ҳ��Ҫ������ȥʵ��
	afx_msg void OnClientListDbClicked(NMHDR* pNMHDR, LRESULT* pResult);

	//////////////////////////////////////////////////////////////////////////
	//���б����һ����б�,��Ҫ��ʾ��ݲ˵�
	afx_msg void OnClientListRClicked(NMHDR *pNMHDR, LRESULT *pResult);

	//��Ӧ���ͷ���������Ĳ˵���Ϣ
	afx_msg void OnPushServer();

	//��Ӧѡ��˵���Ϣ
	afx_msg void OnOption();

	//ȡ������
	afx_msg void OnCancelPush();

	//��ʾcmd�Ի���
	afx_msg void OnCmd();
	
	//��ʾ�ļ�����Ի���
	afx_msg void OnFileManage();
	
	//�����ļ�
	afx_msg void OnDownload();
	
	//�ϴ��ļ�
	afx_msg void OnUpdateDownload(CCmdUI *pCmdUI);

	DECLARE_MESSAGE_MAP()
};
