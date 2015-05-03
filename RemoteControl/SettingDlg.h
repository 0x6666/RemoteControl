///////////////////////////////////////////////////////////////
//
// FileName	: SettingDlg.h
// Creator	: 杨松
// Date		: 2013年4月9日，10:34:00
// Comment	: 设置对话框类的声明
//
//////////////////////////////////////////////////////////////

#pragma once
#include "afxcmn.h"

class CPushSettingPane;
class CCMDSettingPane;
class CRemoteControlDoc;

// CSettingDlg 对话框

class CSettingDlg : public CDialog
{
	DECLARE_DYNAMIC(CSettingDlg)

public:
	CSettingDlg( PUSH_CFG_V pushValue , CMD_CFG_V cmdValue ,CRemoteControlDoc* doc, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSettingDlg();

// 对话框数据
	enum { IDD = IDD_SETTING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

private:
	//推送的配置值
	PUSH_CFG_V m_pushValue;
	//CMD的配置值
	CMD_CFG_V m_cmdValue;

	CTabCtrl m_wndTab;
	virtual BOOL OnInitDialog();

	//推送属性页
	CPushSettingPane* m_pPushPane;
	//cmd设置属性页
	CCMDSettingPane* m_pCmdPane;

	//文档类
	CRemoteControlDoc* m_pDoc;

public:

	afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL DestroyWindow();
	void ValueChanged();
	afx_msg void OnBnClickedApply();
	afx_msg void OnBnClickedOk();
};
