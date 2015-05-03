///////////////////////////////////////////////////////////////
//
// FileName	: CMDDlg.h 
// Creator	: 杨松
// Date		: 2013年5月8日，13:24:47
// Comment	: CMD对话框类声明
//
//////////////////////////////////////////////////////////////

#pragma once
#include "afxwin.h"


// CCMDDlg 对话框
class CRemoteControlDoc;

class CCMDDlg : public CDialog
{
	DECLARE_DYNAMIC(CCMDDlg)

public:
	// 标准构造函数
	CCMDDlg(const CString& ip ,USHORT port ,const CString& name ,
		CRemoteControlDoc* pDoc , PCMD_CFG_V cmdv, CWnd* pParent = NULL);   
	virtual ~CCMDDlg();

// 对话框数据
	enum { IDD = IDD_CMD_VIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

	CRemoteControlDoc* m_pDoc;

	//编辑区edit控件
	CEdit m_wndCMDView;
	
	//客户端地址
	CString m_strIP;
	//客户端消息端口
	USHORT	m_uPort;
	//客户端计算机名
	CString m_strName;

	//将要执行的命令
	CString m_strCMD;

	//背景刷子
	CBrush* m_pBkBrush;
	//字体
	CFont* m_pFont;

	//编辑区在最后一次编辑前的总字符数
	//这样记录是便于处理退格键和delete键
	int m_nLastCount;

	CMD_CFG_V m_cmdCfgValue;

	//对话框的大小改变
	afx_msg void OnSize(UINT nType, int cx, int cy);
	
	//添加命令行数据
	void AddCMDLineData(CString strCMDline );
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL DestroyWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	//////////////////////////////////////////////////////////////////////////
	//在命令发出去之前需要执行的处理过程
	//return 
	//		FALSE	命令不需要在继续发出去处理了
	//		TRUE	命令需要到客户端处理
	BOOL PreExecuteCMD();
	//执行命令
	BOOL ExecuteCMD();
	//一条命执行失败
	void CMDFailed( const void* msg );

	//////////////////////////////////////////////////////////////////////////
	//一下是常规菜单的跟新函数和实现函数
	afx_msg void OnCopy();
	afx_msg void OnUpdateCopy(CCmdUI *pCmdUI);
	afx_msg void OnCut();
	afx_msg void OnUpdateCut(CCmdUI *pCmdUI);
	afx_msg void OnPast();
	afx_msg void OnUpdatePast(CCmdUI *pCmdUI);

	//////////////////////////////////////////////////////////////////////////
	//在对话框中显示的菜单无法跟新，是因为对话框基类没有实现此函数，次函数会依
	//次触发每个菜单项的更新消息
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);

	//改变cmd配置数据
	void ChangeCfgValue( PCMD_CFG_V m_cmdValue );
};
