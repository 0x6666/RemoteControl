///////////////////////////////////////////////////////////////
//
// FileName	: CMDSettingPane.cpp
// Creator	: ����
// Date		: 2013��4��9�գ�15:17:47
// Comment	: CMD�������������ʵ��
//
//////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "RemoteControl.h"
#include "CMDSettingPane.h"
#include "SettingDlg.h"
#include <locale.h>


// CCMDSettingPane �Ի���

IMPLEMENT_DYNAMIC(CCMDSettingPane, CDialog)

CCMDSettingPane::CCMDSettingPane(CSettingDlg* setDlg , PCMD_CFG_V initValue)
	: CDialog(CCMDSettingPane::IDD, NULL)
	, m_pBkBrush(NULL)
	, m_pFont(NULL)
	, m_bChanged(FALSE)
	, m_pParent(setDlg)
{
	this->m_bkColor = initValue->nCMD_BackgroundColor;
	this->m_fontColor = initValue->nCMD_FontColor;
	m_lFontSize = initValue->nCMD_FontSize;
	m_bFontCharSet = initValue->nCMD_FontCharset;
	m_strFontFaceName += initValue->czCMD_FontFaceName;
}

CCMDSettingPane::~CCMDSettingPane()
{
}

void CCMDSettingPane::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ET_PREVIEW, m_wndPreVew);
}


BEGIN_MESSAGE_MAP(CCMDSettingPane, CDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_FONT, &CCMDSettingPane::OnBnClickedFont)
	ON_BN_CLICKED(IDC_BG_ROUNF, &CCMDSettingPane::OnBnClickedBgRounf)
END_MESSAGE_MAP()


// CCMDSettingPane ��Ϣ�������

HBRUSH CCMDSettingPane::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// TODO:  �ڴ˸��� DC ���κ�����
	if(pWnd == &m_wndPreVew)
	{
		pDC->SetTextColor(m_fontColor);
		DWORD colo = RGB(255 , 255 , 255);
		pDC->SetBkColor(m_bkColor);
		return static_cast <HBRUSH>(m_pBkBrush->GetSafeHandle());
	}

	//û�б�Ҫ��ʱ���Ƿ���ԭ�еĻ�ˢ
	//����������ģ������Ҽ���ʾ�Ĳ˵��޷���ʾ
	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL CCMDSettingPane::OnInitDialog()
{
	CDialog::OnInitDialog();

	//����
	LOGFONT lf = {0};
	lf.lfHeight =  m_lFontSize;
	lf.lfCharSet =  m_bFontCharSet;
	wcscpy(lf.lfFaceName ,  m_strFontFaceName );
	this->m_pFont = new CFont();
	this->m_pFont->CreateFontIndirect(&lf);
	m_wndPreVew.SetFont( m_pFont );

	//������ɫ
	this->m_pBkBrush = new CBrush();
	this->m_pBkBrush->CreateSolidBrush(m_bkColor);

	m_wndPreVew.SetWindowText(_T("Ԥ��.\r\n\r\nC:\\WINDOWS>"));

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

BOOL CCMDSettingPane::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_RBUTTONUP)
	{//��Ҫ��ʾ�����˵�
		return TRUE;
	}

	if (pMsg->message == WM_LBUTTONDOWN)
	{//Ԥ���������Ի�����뽹��
		if (pMsg->hwnd == m_wndPreVew.GetSafeHwnd())
		{
			GetDlgItem(IDC_FONT)->SetFocus();
			return TRUE;
		}
	}

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
	{//����ESC��
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CCMDSettingPane::OnBnClickedFont()
{
	//��������
	LOGFONT lf , bkup;
	m_pFont->GetLogFont(&lf);
	bkup = lf;
	CFontDialog dlg(&lf , CF_NOSCRIPTSEL|CF_EFFECTS | CF_SCREENFONTS );
	//������ɫ
	dlg.m_cf.rgbColors = m_fontColor;

	if(dlg.DoModal() != IDOK)
		return ;

	if (  (lf.lfHeight != bkup.lfHeight)
		||(lf.lfCharSet != bkup.lfCharSet)
		||(memcmp(lf.lfFaceName , bkup.lfFaceName , wcslen(bkup.lfFaceName))))
	{//����ı�
		bkup.lfHeight = lf.lfHeight;
		bkup.lfCharSet = lf.lfCharSet;
		wcscpy(bkup.lfFaceName , lf.lfFaceName);

		this->m_pFont->DeleteObject();
		this->m_pFont->CreateFontIndirect(&bkup);
		m_wndPreVew.SetFont(this->m_pFont);
		m_bChanged = TRUE;
		m_pParent->ValueChanged();
	}

	if(this->m_fontColor != dlg.GetColor())
	{//������ɫ�ı�
		this->m_fontColor = dlg.GetColor();
		m_bChanged = TRUE;
		m_pParent->ValueChanged();
	}

	this->Invalidate();
}

void CCMDSettingPane::OnBnClickedBgRounf()
{
	CColorDialog dlg(m_bkColor);
	if(dlg.DoModal() != IDOK)
		return ;
	if(this->m_bkColor != dlg.GetColor())
	{//������ɫ�ı���
		this->m_bkColor = dlg.GetColor();
		this->m_pBkBrush->DeleteObject();
		this->m_pBkBrush->CreateSolidBrush(this->m_bkColor);

		this->Invalidate(FALSE);

		m_bChanged = TRUE;
		m_pParent->ValueChanged();
	}
}

BOOL CCMDSettingPane::IsValueChanged()
{
	return m_bChanged;
}

CMD_CFG_V CCMDSettingPane::GetValue()
{
	CMD_CFG_V cmdValue = {0};
	char nameBuf[40] = {0};
	cmdValue.nCMD_BackgroundColor = m_bkColor;
	LOGFONT lf = {0};
	this->m_pFont->GetLogFont(&lf);
	cmdValue.nCMD_FontCharset = lf.lfCharSet;
	cmdValue.nCMD_FontColor = m_fontColor;
	cmdValue.nCMD_FontSize = lf.lfHeight;
	::setlocale(LC_ALL , "");
	::wcstombs(nameBuf , lf.lfFaceName , 40);
	::setlocale(LC_ALL , "C");
	strcpy(cmdValue.czCMD_FontFaceName , nameBuf);
	return cmdValue;
}

void CCMDSettingPane::ValueApplied()
{
	this->m_bChanged = FALSE;
}
