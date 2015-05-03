///////////////////////////////////////////////////////////////
//
// FileName	: OptionDlg.cpp 
// Creator	: 杨松
// Date		: 2013年3月27日, 12:47:37
// Comment	: 服务器的选项对话框的实现
//
//////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "RemoteControl.h"
#include "PushSettingPane.h"
#include "SettingDlg.h"


// COptionDlg 对话框

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


// COptionDlg 消息处理程序

BOOL CPushSettingPane::OnInitDialog()
{
	CDialog::OnInitDialog();

	{//抓屏质量相关控件初始化
	//抓屏的质量显示
	CSliderCtrl* pQualitySlide = ((CSliderCtrl*)(this->GetDlgItem(IDC_SCREEN_QUALITY)));
	//设置质量的范围
	pQualitySlide->SetRange(0 , 100 , FALSE);
	CString strTemp;
	strTemp.Format(_T("%d%%") , m_iScreenQuality);
	//设置一下要显示的数值
	this->GetDlgItem(IDC_ST_QUALITY_NUM)->SetWindowText(strTemp);
	//设置每一页的调整大小
	pQualitySlide->SetPageSize(10);
	}

	{//流畅度相关控件初始化
	CSpinButtonCtrl* pFluencySpin = ((CSpinButtonCtrl*)(this->GetDlgItem(IDC_FLUENCY_SPBIN)));
	CEdit* pFluencyEdit = ((CEdit*)(this->GetDlgItem(IDC_SCREEN_FLUENCY)));

	pFluencySpin->SetBuddy(pFluencyEdit);
	pFluencySpin->SetBase(0);
	pFluencySpin->SetPos(this->m_lScreenFrequency);
	pFluencySpin->SetRange(FrequencyToIndex(FT_MIN_LEVEL), FrequencyToIndex(FT_MAX_LEVEL));

	//流畅度的 具体描述显示
	CString strTemp;
	strTemp  += FrequencyToText(IndexToFrequency((FrequencyType)m_lScreenFrequency));
	this->GetDlgItem(IDC_FLUENCY_TEXT)->SetWindowText(strTemp);
	}

	{//抓屏尺寸
	CStatic* pMaxSize = ((CStatic*)(this->GetDlgItem(IDC_MAX_SIZE)));
	CString strTemp;
	strTemp.LoadString(IDS_SCREEN_SIZE);
	strTemp.AppendFormat(_T("%d×%d") , SCREEN_SIZE_W , SCREEN_SIZE_H);
	pMaxSize->SetWindowText(strTemp);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CPushSettingPane::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{	
	if (pScrollBar->GetDlgCtrlID() == IDC_SCREEN_QUALITY)
	{//调整抓屏的质量
		int pos = ((CSliderCtrl*)(this->GetDlgItem(IDC_SCREEN_QUALITY)))->GetPos();
		CString strTemp;
		strTemp.Format(_T("%d%%") , pos);
		//设置一下要显示的数值
		this->GetDlgItem(IDC_ST_QUALITY_NUM)->SetWindowText(strTemp);

		m_bChanged = TRUE;
		m_pSetDlg->ValueChanged();
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPushSettingPane::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	//调整抓屏的流畅度
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
	// TODO: 在此添加控件通知处理程序代码
	OnOK();
}


void CPushSettingPane::OnEnChangeScreenSizeW()
{
	//抓屏尺寸改变
	UpdateData(TRUE);

	if (m_lScreenSizeW > SCREEN_SIZE_W)
	{//宽度不可一大过显示器的大小
		m_lScreenSizeW = SCREEN_SIZE_W;
	}

	if (TRUE == m_bLockSizeRatio)
	{//锁定了比例
		m_lScreenSizeH = SCREEN_SIZE_H * m_lScreenSizeW / SCREEN_SIZE_W ;	
	}
	UpdateData(FALSE);

	m_bChanged = TRUE;
	m_pSetDlg->ValueChanged();
}

void CPushSettingPane::OnEnChangeScreenSizeH()
{
	//抓屏尺寸改变
	UpdateData(TRUE);

	if (m_lScreenSizeH > SCREEN_SIZE_H)
	{//宽度不可一大过显示器的大小
		m_lScreenSizeH = SCREEN_SIZE_H;
	}

	if (TRUE == m_bLockSizeRatio)
	{//锁定了比例
		m_lScreenSizeW = SCREEN_SIZE_W * m_lScreenSizeH / SCREEN_SIZE_H ;	
	}
	UpdateData(FALSE);

	m_bChanged = TRUE;
	m_pSetDlg->ValueChanged();
}

void CPushSettingPane::OnBnClickedLockSeziRatio()
{
	//比例锁定状态被改变
	UpdateData(TRUE);
	if (TRUE == m_bLockSizeRatio)
	{//锁定了比例
		m_lScreenSizeH = SCREEN_SIZE_H * m_lScreenSizeW / SCREEN_SIZE_W ;
		UpdateData(FALSE);
	}

	m_bChanged = TRUE;
	m_pSetDlg->ValueChanged();
}

void CPushSettingPane::OnEnChangeScreenFrequency()
{
	//流畅度被直接编辑了
	UpdateData(TRUE);
	if (m_lScreenFrequency > FrequencyToIndex(FT_MAX_LEVEL))
	{//向上越界了
		m_lScreenFrequency = FrequencyToIndex(FT_MAX_LEVEL);
	}

	if (m_lScreenFrequency < FrequencyToIndex(FT_MIN_LEVEL))
	{//向下越界了
		m_lScreenFrequency = FrequencyToIndex(FT_MIN_LEVEL);
	}

	//同步到spin控件上
	((CSpinButtonCtrl*)(this->GetDlgItem(IDC_FLUENCY_SPBIN)))->SetPos(this->m_lScreenFrequency);

	//设置一下要显示的频率
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
