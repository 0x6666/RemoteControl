///////////////////////////////////////////////////////////////
//
// FileName	: CMDTable.h 
// Creator	: ����
// Date		: 2013��3��15��, 16:09:32
// Comment	: �ͻ��˼�����ͼ�������������
//
//////////////////////////////////////////////////////////////

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CCMDTable �Ի���
class ClientItemView;

class CCMDTable : public CDialog
{
	DECLARE_DYNAMIC(CCMDTable)

public:
	CCMDTable(ClientItemView* pParent , int id);   // ��׼���캯��
	virtual ~CCMDTable();
	int GetMaxWiWidth();
	
	void SmoothShowWindow();
	
	//��ʾΪ������ͼ��
	void ShowAsIndexLabel();

// �Ի�������
	enum { IDD = IDD_CMDTABLE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnStnClickedTerminal();
	afx_msg void OnStnClickedDesktop();
	afx_msg void OnStnClickedDocManager();
	virtual BOOL OnInitDialog();
	//void SmoothHideWindow();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//////////////////////////////////////////////////////////////////////////
	//�ڶԻ�������ʾ�Ĳ˵��޷����£�����Ϊ�Ի������û��ʵ�ִ˺������κ�������
	//�δ���ÿ���˵���ĸ�����Ϣ
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
private:
	BOOL m_bTracking;
	ClientItemView* m_pParent;
	int m_nFullHieght;
	CStatic m_wndShutdwon;
	CButton m_wndDesktopCtrl;
	CStatic m_wndFluencyLabel;
	CEdit m_wndFluencyEdit;
	CSpinButtonCtrl m_wndFluencySpin;
	CStatic m_wndQualityLabel;
	CSliderCtrl m_wndQualitySlide;
	CStatic m_wndQualityNum;
	CStatic m_wndTerminal;
	CStatic m_wndDesktop;
	CStatic m_wndDocManager;
	CStatic m_wndFullScrenn;
	CStatic m_wndIndexLebel;

	//�Ƿ��Ѿ�ȫ���ˣ�
	BOOL m_bIsFullScreen;

	//��ʾΪ����ͼ�� ���Ƿ�����ʾ�ͻ���ID��
	BOOL m_bShowAsLabel;

	//�ͻ���ID
	int m_iClientID;

	//label��ʾ�ĳߴ�
	CSize m_szLabelSize;

public:
	afx_msg void OnBnClickedDesktopCtrl();
	afx_msg void OnStnClickedShutdown();
	afx_msg void OnStnClickedFullScreen();

	afx_msg void OnMove(int x, int y);
	afx_msg void OnStnClickedIndexLabel();

	//�Ƿ���ʾΪ����ͼ��
	BOOL IsShowAsLabel();
};
