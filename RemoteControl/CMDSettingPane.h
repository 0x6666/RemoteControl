///////////////////////////////////////////////////////////////
//
// FileName	: CMDSettingPane.h 
// Creator	: 杨松
// Date		: 2013年4月9日，15:17:47
// Comment	: CMD窗口设置面板类声明
//
//////////////////////////////////////////////////////////////

#pragma once
#include "afxwin.h"

class CSettingDlg;
// CCMDSettingPane 对话框

class CCMDSettingPane : public CDialog
{
	DECLARE_DYNAMIC(CCMDSettingPane)

public:
	CCMDSettingPane(CSettingDlg* setDlg , PCMD_CFG_V initValue);   // 标准构造函数
	virtual ~CCMDSettingPane();

// 对话框数据
	enum { IDD = IDD_CMD_SETTING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

private:
	//字体颜色
	COLORREF m_fontColor;
	//背景颜色
	COLORREF m_bkColor;
	//背景刷子
	CBrush* m_pBkBrush;
	//字体
	CFont* m_pFont;
	CEdit m_wndPreVew;
	//是否有数据改变
	BOOL m_bChanged;

	//字体的大小
	LONG m_lFontSize;
	//字体字符集
	BYTE m_bFontCharSet;
	//字体名字
	CString m_strFontFaceName;

	//父窗口
	CSettingDlg* m_pParent;
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedFont();
	afx_msg void OnBnClickedBgRounf();
	BOOL IsValueChanged();
	CMD_CFG_V GetValue();
	void ValueApplied();
};
