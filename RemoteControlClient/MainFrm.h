// MainFrm.h : CMainFrame ��Ľӿ�
//


#pragma once

#include "ChildView.h"
class CLetterRainDlg;
typedef std::list<PUploadDescripter> UploadDesList;

class CMainFrame : public CFrameWnd ,public Dispatcher
{
	
public:
	CMainFrame();
protected: 
	DECLARE_DYNAMIC(CMainFrame)

private:
	//������ip��ַ
	CString m_strServerIP;
	//��Ϣ����
	MsgCenter* m_pMsgCenter;
	//�ò��ҷ������Ĺ㲥�׽���
	CAsyncSocket* m_pBroadcastSocket;
	//�������㲥�Ƿ�������
	BOOL m_bHeartbeatTimerRun;
	//��ǰ�������������
	char* m_czHostName;
	//��ǰ�ͻ��˼����Ķ˿� 
	USHORT m_uPort;

	//ץ���߳�������
	CaptureScreenThreadContext m_CCScreenThreadContext;
	
	//���շ��������͹���������ͼ�������߳�������
	RecvPushedDesktopThreadContext m_RecvPushedDesktopThreadContext;

	//��Ļ��Ի���
	CLetterRainDlg* m_pLetterRainDlg;

	//CMD�߳�������
	CMDContext m_cmdContext;

	//����ͼ��
	NOTIFYICONDATA m_notifyIcon;

	//��Ҫö�ٵ��ļ���·��
	CString m_strEnumPath;

	//�����߳�������
	PDownloadListrenDescripter m_pDlListenDes;

	//�ϴ��߳�������
	UploadDesList m_listUploadDescripter;

// ����
public:

	//���ö��·��
	CString GetEnumPath();

	//ֹͣ��ȡcmd�����߳�
	void StopReadCMDThread();


// ��д
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

	//////////////////////////////////////////////////////////////////////////
	//��Ϣ���ͺ��������͵��Ƿ��������͹�������Ϣ
	//param
	//		msg		��Ϣ����(RCMSG)
	//		ip		������ip��ַ
	//		port	����������Ϣ�Ķ˿�
	virtual void DispatchMsg(const void* msg , CString ip, UINT port);

// ʵ��
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	CChildView    m_wndView;

// ���ɵ���Ϣӳ�亯��
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	void StartFindServerBroadcastTimer();

	afx_msg void OnSetFocus(CWnd *pOldWnd);
	DECLARE_MESSAGE_MAP()
public:
	//��������
	virtual BOOL DestroyWindow();
	//��ʱ���ص�����
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	//����ͼ����Ϣ
	afx_msg LRESULT OnNotifyIconMsg(WPARAM wParam , LPARAM lParam);
	//����ͼ����Ϣ �˳� 
	afx_msg void OnExit();
	//�����������һ�����ֵ���Ϣ
	afx_msg void OnHandsUp();

	//�����ϴ��ļ��߳�
	//wparam	SOCKET*	�Ѿ������˵��׽���
	afx_msg LRESULT OnCreateUploadThread(WPARAM wParam , LPARAM lParam);

	//������ϴ������˿�
	//param
	//wParam	USHORT	�˿�
	afx_msg LRESULT OnUploadListenPort(WPARAM wParam , LPARAM lParam);

	//�����ϴ������˿�
	void SendUploadPort();

	//ֹͣ��ѯ�������Ĺ㲥
	void StopFindServerBroadcast();
	//ֹͣ��������ʱ
	void StopHeartbeatTimer();
	//����ץ���߳�
	void StartScreenCaptureThread( const void* msg );

	//������ip��ַ
	CString GetServerIP();

	//��������������Ϣ  
	//����ץ��������
	void OnRcScreenQuality( const void* msg );
	//����������
	void OnRcScreenFluency( const void* msg );
	//ֹͣץ��
	void OnRcStopScreenCapture();
	//����ƶ�
	void OnRcMouseEvent( const void* msg );
	//������Ϣ
	void OnRcKeybdEvent( const void* msg );
	//����ץ����С
	void OnRcScreenSize( const void* msg );
	//������֪ͨ��ʼ���շ��������͹���������
	void OnRcPushServerDesktop();
	//ֹͣ���շ��������͹�������������
	void StopRecvServerPushDektopThread();
	//ȡ����������
	void OnRcCancelPushDesktop();
	//�˳��ͻ���
	void OnRcExitClient();
	//�ػ�
	void OnRcShutdown();
	//����
	void OnRcRestart();
	//ע��
	void OnRcLoginOut();
	//��Ļ��
	void OnRcLetterRain( const void* msg );
	//����Զ��CMD
	BOOL OnRcStartCmd();

	//ֹͣ��ĸ��
	void StopLetterRain();
	
	//////////////////////////////////////////////////////////////////////////
	//��ȡCMD�����·��
	//return 
	//		CMD�����·��
	//		�մ����ȡʧ��
	CString GetCMDPath();

	//���ҷ������㲥�Ļظ���Ϣ
	void OnRepeatFindServer( const CString& ip );
	//CMD����
	void OnRcCMD( const void* msg );
	//�������˳�
	void OnRcServerExit();
	//��ȡ�ƶ�·��������ļ��б�
	void OnRcGetFileList( const void* msg );
	void OnRcGetDriver();
	//ɾ���ļ�
	void OnRcDeleteFile( const void* msg );
	void StopUploadThreads();

	//��ͣ������Ļͼ��
	void OnRcPauseScreen();
	//���¿�ʼ������Ļͼ��
	void OnRcResumeScreen();
};


