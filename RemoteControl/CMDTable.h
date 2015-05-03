///////////////////////////////////////////////////////////////
//
// FileName	: CMDTable.h 
// Creator	: 杨松
// Date		: 2013年3月15日, 16:09:32
// Comment	: 客户端监视视图控制面类的申明
//
//////////////////////////////////////////////////////////////

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CCMDTable 对话框
class ClientItemView;

class CCMDTable : public CDialog
{
	DECLARE_DYNAMIC(CCMDTable)

public:
	CCMDTable(ClientItemView* pParent , int id);   // 标准构造函数
	virtual ~CCMDTable();
	int GetMaxWiWidth();
	
	void SmoothShowWindow();
	
	//显示为索引号图标
	void ShowAsIndexLabel();

// 对话框数据
	enum { IDD = IDD_CMDTABLE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

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
	//在对话框中显示的菜单无法跟新，是因为对话框基类没有实现此函数，次函数会依
	//次触发每个菜单项的更新消息
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

	//是否已经全屏了？
	BOOL m_bIsFullScreen;

	//显示为索引图标 （是否是显示客户端ID）
	BOOL m_bShowAsLabel;

	//客户端ID
	int m_iClientID;

	//label显示的尺寸
	CSize m_szLabelSize;

public:
	afx_msg void OnBnClickedDesktopCtrl();
	afx_msg void OnStnClickedShutdown();
	afx_msg void OnStnClickedFullScreen();

	afx_msg void OnMove(int x, int y);
	afx_msg void OnStnClickedIndexLabel();

	//是否显示为索引图标
	BOOL IsShowAsLabel();
};
