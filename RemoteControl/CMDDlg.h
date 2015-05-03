///////////////////////////////////////////////////////////////
//
// FileName	: CMDDlg.h 
// Creator	: ����
// Date		: 2013��5��8�գ�13:24:47
// Comment	: CMD�Ի���������
//
//////////////////////////////////////////////////////////////

#pragma once
#include "afxwin.h"


// CCMDDlg �Ի���
class CRemoteControlDoc;

class CCMDDlg : public CDialog
{
	DECLARE_DYNAMIC(CCMDDlg)

public:
	// ��׼���캯��
	CCMDDlg(const CString& ip ,USHORT port ,const CString& name ,
		CRemoteControlDoc* pDoc , PCMD_CFG_V cmdv, CWnd* pParent = NULL);   
	virtual ~CCMDDlg();

// �Ի�������
	enum { IDD = IDD_CMD_VIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

	CRemoteControlDoc* m_pDoc;

	//�༭��edit�ؼ�
	CEdit m_wndCMDView;
	
	//�ͻ��˵�ַ
	CString m_strIP;
	//�ͻ�����Ϣ�˿�
	USHORT	m_uPort;
	//�ͻ��˼������
	CString m_strName;

	//��Ҫִ�е�����
	CString m_strCMD;

	//����ˢ��
	CBrush* m_pBkBrush;
	//����
	CFont* m_pFont;

	//�༭�������һ�α༭ǰ�����ַ���
	//������¼�Ǳ��ڴ����˸����delete��
	int m_nLastCount;

	CMD_CFG_V m_cmdCfgValue;

	//�Ի���Ĵ�С�ı�
	afx_msg void OnSize(UINT nType, int cx, int cy);
	
	//�������������
	void AddCMDLineData(CString strCMDline );
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL DestroyWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	//////////////////////////////////////////////////////////////////////////
	//�������ȥ֮ǰ��Ҫִ�еĴ������
	//return 
	//		FALSE	�����Ҫ�ڼ�������ȥ������
	//		TRUE	������Ҫ���ͻ��˴���
	BOOL PreExecuteCMD();
	//ִ������
	BOOL ExecuteCMD();
	//һ����ִ��ʧ��
	void CMDFailed( const void* msg );

	//////////////////////////////////////////////////////////////////////////
	//һ���ǳ���˵��ĸ��º�����ʵ�ֺ���
	afx_msg void OnCopy();
	afx_msg void OnUpdateCopy(CCmdUI *pCmdUI);
	afx_msg void OnCut();
	afx_msg void OnUpdateCut(CCmdUI *pCmdUI);
	afx_msg void OnPast();
	afx_msg void OnUpdatePast(CCmdUI *pCmdUI);

	//////////////////////////////////////////////////////////////////////////
	//�ڶԻ�������ʾ�Ĳ˵��޷����£�����Ϊ�Ի������û��ʵ�ִ˺������κ�������
	//�δ���ÿ���˵���ĸ�����Ϣ
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);

	//�ı�cmd��������
	void ChangeCfgValue( PCMD_CFG_V m_cmdValue );
};
