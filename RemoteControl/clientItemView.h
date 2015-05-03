///////////////////////////////////////////////////////////////
//
// FileName	: clientItemView.h 
// Creator	: ����
// Date		: 2013��2��27��, 23:39:40
// Comment	: �ͻ��˼�����ͼ������
//
//////////////////////////////////////////////////////////////

#pragma once

// ClientItemView
class CRemoteControlDoc;
class CCMDTable;
class CCMDDlg;
struct ClientDescripter;

class ClientItemView : public CWnd
{
	DECLARE_DYNAMIC(ClientItemView)

public:
	ClientItemView( ClientDescripter* clientDes ,CRemoteControlDoc* pDoc );
	virtual ~ClientItemView();

	//�ͻ�����ͼ����
	BOOL Create( CWnd *pParent , UINT id);

	//��û���Զ��screen�ľ���
	const CRect& GetScreenViewRect();
	
	//��ȡ�ͻ��˵�ip��ַ
	const CString& GetClientIP(void);

	//�ͻ��˷�������Ϣ����������
	void OnRcMessage( USHORT port, const void* msg );

	//ֹͣ���ӿͻ���
	void StopScreenCaptureThread();

	//�ͻ�������
	void Dropped();

	//�����Ƿ���Ҫ���Ʊ߿�
	BOOL SetDrawEdge(BOOL drawEdge);

	//Windows��ϢԤ����
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	//��������
	virtual BOOL DestroyWindow();

	//////////////////////////////////////////////////////////////////////////
	//��ʼ��ItemView
	void InitItemView();
protected:
	DECLARE_MESSAGE_MAP()

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	//��ǰview�Ļ�����Ϣ��Ӧ����
	afx_msg void OnPaint();

	//��ǰview��С�ı䣬��Ҫ֪ͨ�ͻ��˵������͹�����ͼ���С
	afx_msg void OnSize(UINT nType, int cx, int cy);
	
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	//������˫��
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

	//��ǰview����ʾ��Ϣ������
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	
	//////////////////////////////////////////////////////////////////////////
	//�Ѿ���ñ��ص�screen�˿�
	//param
	//		wParam	�˿�
	afx_msg LRESULT OnGetAScreenPort(WPARAM wParam,LPARAM lParam);
	
	//�׽��ֳ���
	afx_msg LRESULT OnCreatScreenSockErr(WPARAM wParam,LPARAM lParam);
	
	//�߳��˳�
	afx_msg LRESULT OnScreenThreadExit(WPARAM wParam,LPARAM lParam);
	
	//�ȴ��ͻ��˷���ScreenPort��Ӧ��Ϣʧ��
	afx_msg LRESULT OnWaitForCaptureTimeOut(WPARAM wParam,LPARAM lParam);
	
	//////////////////////////////////////////////////////////////////////////
	//����ץ������
	//param
	//		wparam	����
	afx_msg LRESULT OnChangeScreenQuality(WPARAM wParam,LPARAM lParam);
	
	//////////////////////////////////////////////////////////////////////////
	//����������
	//param
	//		wparam	������
	afx_msg LRESULT OnChangeScreenFluency(WPARAM wParam,LPARAM lParam);

	//////////////////////////////////////////////////////////////////////////
	//�����ԶԿͻ��˵Ŀ���״̬
	//param
	//		wParam	�Ƿ���Ҫ����
	afx_msg LRESULT OnScreenCtrl(WPARAM wParam,LPARAM lParam);
	
	//�����CMDͼ��
	afx_msg LRESULT OnCMDClicked(WPARAM wParam,LPARAM lParam);
	
	//ȫ�� 
	afx_msg LRESULT OnFullScreen(WPARAM wParam,LPARAM lParam);
	
	//�ļ�����
	afx_msg LRESULT OnDocManage(WPARAM wParam,LPARAM lParam);

	//ֹͣ����
	afx_msg void OnStopCtrl();
	
	//�˳��ͻ���
	afx_msg void OnExitClient();
	
	//�ͻ��˹ػ�
	afx_msg void OnShutdwonClient();
	
	//�������ض�
	afx_msg void OnResarteClient();
	
	//ע�����ض�
	afx_msg void OnLoginOut();
	
	//��ĸ�����α��ض�����
	afx_msg void OnLetterRain();
	
	//��ĸ��˵�����
	afx_msg void OnUpdateLetterRain(CCmdUI *pCmdUI);
	
	//���ڱ��ƶ���
	afx_msg void OnMove(int x, int y);

private:

	//�ͻ���������
	ClientDescripter* m_pClientDescripter;

	//���������ĵ���ͼ�ṹ���ĵ�ָ��
	CRemoteControlDoc* m_pDoc;

	//���պ���ʾ���ض�����ͼ�����ݵ��߳�������
	PrintScreenThreadContext m_PrintScreenThreadContext;

	//�������
	CCMDTable* m_pCmdTable;
	
	//�Ƿ��������
	BOOL	m_bScreenCtrled;
	
	//��Ļ��
	BOOL m_bLetterRain;

	//�Ƿ���Ҫ���Ʊ߿�
	BOOL m_bDrawEdge;

	//����ͼ������ľ���
	CRect m_rcScreenViewRect;
};
