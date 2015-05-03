///////////////////////////////////////////////////////////////
//
// FileName	: MainFrm.h  
// Creator	: ����
// Date		: 2013��2��27��, 20:10:26
// Comment	: Զ�ؿ��Ʒ���������ܵ��������
//
//////////////////////////////////////////////////////////////


#pragma once

class CCtrlBar;

class CMainFrame : public CFrameWnd
{
	
protected: // �������л�����
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// ����
public:
	//�ͻ����б�
	CCtrlBar*	m_pClientPane;
	CListCtrl*	m_pClientList;


// ����
public:

private:
	//�����ͻ����б����
	BOOL CreateClientPane();
	void InitClientListCtrl();
	
// ��д
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// ʵ��
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // �ؼ���Ƕ���Ա
	CStatusBar  m_wndStatusBar;
	
	//����ͼ��
	NOTIFYICONDATA m_notifyIcon;

	//ȫ��֮ǰ�Ƿ���ʾ״̬��
	BOOL m_bFSStatusBarWasVisible;
	//ȫ��֮ǰ�Ƿ���ʾ�ͻ����б�
	BOOL m_bFSClientListWasVisible;
	//ȫ��֮ǰ���ڵ�λ��
	CRect m_rcFSPos;

// ���ɵ���Ϣӳ�亯��
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	//��ʼ��״̬�� 
	BOOL InitStatusBar();

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL DestroyWindow();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClientList();
	afx_msg void OnUpdateClientList(CCmdUI *pCmdUI);

	//����ͼ�귢��������Ϣ
	afx_msg LRESULT OnNotifyIconMsg(WPARAM wParam , LPARAM lParam);
	afx_msg void OnExit();

	//�ı�״̬���ͻ����Լ�����������ʾ
	afx_msg LRESULT OnClientCount(WPARAM wParam , LPARAM lParam);
	
	//�Ѿ������˵Ŀͻ������� 
	afx_msg LRESULT OnPushedCount(WPARAM wParam , LPARAM lParam);

	//��״̬������һ����Ϣ
	//param
	//	wParam	(CString*) Ҫ��ʾ����Ϣ
	afx_msg LRESULT OnMessage(WPARAM wParam , LPARAM lParam);

	//ȫ�� 
	afx_msg LRESULT OnFullScreen(WPARAM wParam , LPARAM lParam);
	afx_msg void OnMove(int x, int y);
};


