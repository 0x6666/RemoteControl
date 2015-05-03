///////////////////////////////////////////////////////////////
//
// FileName	: CMDSettingPane.h 
// Creator	: ����
// Date		: 2013��4��9�գ�15:17:47
// Comment	: CMD�����������������
//
//////////////////////////////////////////////////////////////

#pragma once
#include "afxwin.h"

class CSettingDlg;
// CCMDSettingPane �Ի���

class CCMDSettingPane : public CDialog
{
	DECLARE_DYNAMIC(CCMDSettingPane)

public:
	CCMDSettingPane(CSettingDlg* setDlg , PCMD_CFG_V initValue);   // ��׼���캯��
	virtual ~CCMDSettingPane();

// �Ի�������
	enum { IDD = IDD_CMD_SETTING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

private:
	//������ɫ
	COLORREF m_fontColor;
	//������ɫ
	COLORREF m_bkColor;
	//����ˢ��
	CBrush* m_pBkBrush;
	//����
	CFont* m_pFont;
	CEdit m_wndPreVew;
	//�Ƿ������ݸı�
	BOOL m_bChanged;

	//����Ĵ�С
	LONG m_lFontSize;
	//�����ַ���
	BYTE m_bFontCharSet;
	//��������
	CString m_strFontFaceName;

	//������
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
