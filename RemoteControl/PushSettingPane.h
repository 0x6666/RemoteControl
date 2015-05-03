///////////////////////////////////////////////////////////////
//
// FileName	: PushSettingPane.h 
// Creator	: ����
// Date		: 2013��3��27��, 12:47:37
// Comment	: ��������ѡ��Ի��������
//
//////////////////////////////////////////////////////////////

#pragma once
class CSettingDlg;


// COptionDlg �Ի���

class CPushSettingPane : public CDialog
{
	DECLARE_DYNAMIC(CPushSettingPane)

public:
	CPushSettingPane(CSettingDlg* pset , PPUSH_CFG_V initValue);   // ��׼���캯��
	virtual ~CPushSettingPane();

// �Ի�������
	enum { IDD = IDD_PUSH_SETTING };
	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	//�Ի����ʼ��
	virtual BOOL OnInitDialog();

	//////////////////////////////////////////////////////////////////////////
	//������Ӧ�϶�����ץ��������slide
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	//////////////////////////////////////////////////////////////////////////
	//������Ӧ����ץ��Ƶ�ʵ�spin����¼�
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	//////////////////////////////////////////////////////////////////////////
	//���ȷ����ť����Ӧ����
	afx_msg void OnBnClickedOk();

	//////////////////////////////////////////////////////////////////////////
	//ץ���ߴ��ȱ仯
	afx_msg void OnEnChangeScreenSizeW();

	//////////////////////////////////////////////////////////////////////////
	//ץ���ߴ�߶ȱ仯
	afx_msg void OnEnChangeScreenSizeH();

	//////////////////////////////////////////////////////////////////////////
	//�Ƿ�����ץ������ȸı�
	afx_msg void OnBnClickedLockSeziRatio();

	//////////////////////////////////////////////////////////////////////////
	//ץ��Ƶ�ʱ��༭
	afx_msg void OnEnChangeScreenFrequency();
	long GetScreenSizeW();
	long GetScreenSizeH();
	long GetScreenQuality();
	long GetScreenFrequency();
	BOOL IsLockScreenSizeRatio();

	//�Ƿ������ݸı���
	BOOL IsValueChanged();
	//��ȡ���õ�ֵ
	PUSH_CFG_V GetValue();
	void ValueApplied();
private:
	//ץ���Ĵ�С
	long m_lScreenSizeW;
	long m_lScreenSizeH;
	//ץ��������
	int m_iScreenQuality;
	//������
	long m_lScreenFrequency;
	//�ڵ���ץ���Ĵ�Сʱ�Ƿ���������
	BOOL m_bLockSizeRatio;

	//�����޸���
	BOOL m_bChanged;
	CSettingDlg* m_pSetDlg;
};
