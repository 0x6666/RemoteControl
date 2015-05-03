///////////////////////////////////////////////////////////////
//
// FileName	: OptionDlg.cpp 
// Creator	: ����
// Date		: 2013��3��27��, 12:47:37
// Comment	: ��������ѡ��Ի����ʵ��
//
//////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "RemoteControl.h"
#include "PushSettingPane.h"
#include "SettingDlg.h"


// COptionDlg �Ի���

IMPLEMENT_DYNAMIC(CPushSettingPane, CDialog)

CPushSettingPane::CPushSettingPane(CSettingDlg* pSetDlg , PPUSH_CFG_V initValue)
	: CDialog(CPushSettingPane::IDD, NULL)
	, m_lScreenSizeW(initValue->nPush_DesktopW)
	, m_lScreenSizeH(initValue->nPush_DesktopH)
	, m_iScreenQuality(initValue->nPush_DesktopQuality)
	, m_bLockSizeRatio(initValue->bPush_LockScreenSizeRatio)
	, m_lScreenFrequency(FrequencyToIndex(FrequencyType(initValue->nPush_DesktopFrequency)))
	, m_bChanged(FALSE)
	, m_pSetDlg(pSetDlg)
{
}

CPushSettingPane::~CPushSettingPane()
{
}

void CPushSettingPane::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SCREEN_SIZE_W, m_lScreenSizeW);
	DDX_Text(pDX, IDC_SCREEN_SIZE_H, m_lScreenSizeH);
	DDX_Slider(pDX, IDC_SCREEN_QUALITY, m_iScreenQuality);
	DDX_Text(pDX, IDC_SCREEN_FLUENCY, m_lScreenFrequency);
	DDX_Check(pDX, IDC_LOCK_SEZI_RATIO, m_bLockSizeRatio);
}


BEGIN_MESSAGE_MAP(CPushSettingPane, CDialog)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDOK, &CPushSettingPane::OnBnClickedOk)
	ON_EN_CHANGE(IDC_SCREEN_SIZE_W, &CPushSettingPane::OnEnChangeScreenSizeW)
	ON_EN_CHANGE(IDC_SCREEN_SIZE_H, &CPushSettingPane::OnEnChangeScreenSizeH)
	ON_BN_CLICKED(IDC_LOCK_SEZI_RATIO, &CPushSettingPane::OnBnClickedLockSeziRatio)
	ON_EN_CHANGE(IDC_SCREEN_FLUENCY, &CPushSettingPane::OnEnChangeScreenFrequency)
END_MESSAGE_MAP()


// COptionDlg ��Ϣ�������

BOOL CPushSettingPane::OnInitDialog()
{
	CDialog::OnInitDialog();

	{//ץ��������ؿؼ���ʼ��
	//ץ����������ʾ
	CSliderCtrl* pQualitySlide = ((CSliderCtrl*)(this->GetDlgItem(IDC_SCREEN_QUALITY)));
	//���������ķ�Χ
	pQualitySlide->SetRange(0 , 100 , FALSE);
	CString strTemp;
	strTemp.Format(_T("%d%%") , m_iScreenQuality);
	//����һ��Ҫ��ʾ����ֵ
	this->GetDlgItem(IDC_ST_QUALITY_NUM)->SetWindowText(strTemp);
	//����ÿһҳ�ĵ�����С
	pQualitySlide->SetPageSize(10);
	}

	{//��������ؿؼ���ʼ��
	CSpinButtonCtrl* pFluencySpin = ((CSpinButtonCtrl*)(this->GetDlgItem(IDC_FLUENCY_SPBIN)));
	CEdit* pFluencyEdit = ((CEdit*)(this->GetDlgItem(IDC_SCREEN_FLUENCY)));

	pFluencySpin->SetBuddy(pFluencyEdit);
	pFluencySpin->SetBase(0);
	pFluencySpin->SetPos(this->m_lScreenFrequency);
	pFluencySpin->SetRange(FrequencyToIndex(FT_MIN_LEVEL), FrequencyToIndex(FT_MAX_LEVEL));

	//�����ȵ� ����������ʾ
	CString strTemp;
	strTemp  += FrequencyToText(IndexToFrequency((FrequencyType)m_lScreenFrequency));
	this->GetDlgItem(IDC_FLUENCY_TEXT)->SetWindowText(strTemp);
	}

	{//ץ���ߴ�
	CStatic* pMaxSize = ((CStatic*)(this->GetDlgItem(IDC_MAX_SIZE)));
	CString strTemp;
	strTemp.LoadString(IDS_SCREEN_SIZE);
	strTemp.AppendFormat(_T("%d��%d") , SCREEN_SIZE_W , SCREEN_SIZE_H);
	pMaxSize->SetWindowText(strTemp);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

void CPushSettingPane::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{	
	if (pScrollBar->GetDlgCtrlID() == IDC_SCREEN_QUALITY)
	{//����ץ��������
		int pos = ((CSliderCtrl*)(this->GetDlgItem(IDC_SCREEN_QUALITY)))->GetPos();
		CString strTemp;
		strTemp.Format(_T("%d%%") , pos);
		//����һ��Ҫ��ʾ����ֵ
		this->GetDlgItem(IDC_ST_QUALITY_NUM)->SetWindowText(strTemp);

		m_bChanged = TRUE;
		m_pSetDlg->ValueChanged();
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPushSettingPane::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	//����ץ����������
	if (pScrollBar->GetDlgCtrlID() == IDC_FLUENCY_SPBIN)
	{
		CSpinButtonCtrl* pFluencySpin = ((CSpinButtonCtrl*)(this->GetDlgItem(IDC_FLUENCY_SPBIN)));
		CString strTemp;
		strTemp.Format( _T("%d") , (int)nPos);
		pFluencySpin->GetBuddy()->SetWindowText(strTemp);
		strTemp = _T("");
		strTemp  += FrequencyToText(IndexToFrequency((FrequencyType)nPos));
		this->GetDlgItem(IDC_FLUENCY_TEXT)->SetWindowText(strTemp);

		m_bChanged = TRUE;
		m_pSetDlg->ValueChanged();
	}
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CPushSettingPane::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	OnOK();
}


void CPushSettingPane::OnEnChangeScreenSizeW()
{
	//ץ���ߴ�ı�
	UpdateData(TRUE);

	if (m_lScreenSizeW > SCREEN_SIZE_W)
	{//��Ȳ���һ�����ʾ���Ĵ�С
		m_lScreenSizeW = SCREEN_SIZE_W;
	}

	if (TRUE == m_bLockSizeRatio)
	{//�����˱���
		m_lScreenSizeH = SCREEN_SIZE_H * m_lScreenSizeW / SCREEN_SIZE_W ;	
	}
	UpdateData(FALSE);

	m_bChanged = TRUE;
	m_pSetDlg->ValueChanged();
}

void CPushSettingPane::OnEnChangeScreenSizeH()
{
	//ץ���ߴ�ı�
	UpdateData(TRUE);

	if (m_lScreenSizeH > SCREEN_SIZE_H)
	{//��Ȳ���һ�����ʾ���Ĵ�С
		m_lScreenSizeH = SCREEN_SIZE_H;
	}

	if (TRUE == m_bLockSizeRatio)
	{//�����˱���
		m_lScreenSizeW = SCREEN_SIZE_W * m_lScreenSizeH / SCREEN_SIZE_H ;	
	}
	UpdateData(FALSE);

	m_bChanged = TRUE;
	m_pSetDlg->ValueChanged();
}

void CPushSettingPane::OnBnClickedLockSeziRatio()
{
	//��������״̬���ı�
	UpdateData(TRUE);
	if (TRUE == m_bLockSizeRatio)
	{//�����˱���
		m_lScreenSizeH = SCREEN_SIZE_H * m_lScreenSizeW / SCREEN_SIZE_W ;
		UpdateData(FALSE);
	}

	m_bChanged = TRUE;
	m_pSetDlg->ValueChanged();
}

void CPushSettingPane::OnEnChangeScreenFrequency()
{
	//�����ȱ�ֱ�ӱ༭��
	UpdateData(TRUE);
	if (m_lScreenFrequency > FrequencyToIndex(FT_MAX_LEVEL))
	{//����Խ����
		m_lScreenFrequency = FrequencyToIndex(FT_MAX_LEVEL);
	}

	if (m_lScreenFrequency < FrequencyToIndex(FT_MIN_LEVEL))
	{//����Խ����
		m_lScreenFrequency = FrequencyToIndex(FT_MIN_LEVEL);
	}

	//ͬ����spin�ؼ���
	((CSpinButtonCtrl*)(this->GetDlgItem(IDC_FLUENCY_SPBIN)))->SetPos(this->m_lScreenFrequency);

	//����һ��Ҫ��ʾ��Ƶ��
 	CString strTemp;
 	strTemp  += FrequencyToText(IndexToFrequency((FrequencyType)m_lScreenFrequency));
 	this->GetDlgItem(IDC_FLUENCY_TEXT)->SetWindowText(strTemp);

	UpdateData(FALSE);

	m_bChanged = TRUE;
	m_pSetDlg->ValueChanged();
}

long CPushSettingPane::GetScreenSizeW()
{
	return m_lScreenSizeW;
}

long CPushSettingPane::GetScreenSizeH()
{
	return m_lScreenSizeH;
}

long CPushSettingPane::GetScreenQuality()
{
	return (long)m_iScreenQuality;
}

long CPushSettingPane::GetScreenFrequency()
{
	return m_lScreenFrequency;
}

BOOL CPushSettingPane::IsLockScreenSizeRatio()
{
	return m_bLockSizeRatio;
}

BOOL CPushSettingPane::IsValueChanged()
{
	return m_bChanged;
}

PUSH_CFG_V CPushSettingPane::GetValue()
{
	UpdateData(TRUE);

	PUSH_CFG_V value = {0};
	value.bPush_LockScreenSizeRatio = m_bLockSizeRatio;
	value.nPush_DesktopFrequency = IndexToFrequency(m_lScreenFrequency);
	value.nPush_DesktopH = m_lScreenSizeH;
	value.nPush_DesktopQuality = m_iScreenQuality;
	value.nPush_DesktopW = m_lScreenSizeW;
	return value;
}

void CPushSettingPane::ValueApplied()
{
	this->m_bChanged = FALSE;
}
