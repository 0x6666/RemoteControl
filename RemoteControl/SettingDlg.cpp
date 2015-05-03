///////////////////////////////////////////////////////////////
//
// FileName	: SettingDlg.cpp 
// Creator	: ����
// Date		: 2013��4��9�գ�10:34:00
// Comment	: ���öԻ������ʵ��
//
//////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RemoteControl.h"
#include "SettingDlg.h"
#include "PushSettingPane.h"
#include "CMDSettingPane.h"
#include "RemoteControlDoc.h"


// CSettingDlg �Ի���

IMPLEMENT_DYNAMIC(CSettingDlg, CDialog)

CSettingDlg::CSettingDlg(PUSH_CFG_V pushValue , CMD_CFG_V cmdValue ,CRemoteControlDoc* doc, CWnd* pParent /*=NULL*/)
	: CDialog(CSettingDlg::IDD, pParent)
	, m_pPushPane(NULL)
	, m_pCmdPane(NULL)
	, m_pDoc(doc)
{
	m_pushValue = pushValue;
	m_cmdValue = cmdValue;
}

CSettingDlg::~CSettingDlg()
{
}

void CSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, m_wndTab);
}


BEGIN_MESSAGE_MAP(CSettingDlg, CDialog)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CSettingDlg::OnTcnSelchangeTab)
//	ON_WM_SETFOCUS()
ON_BN_CLICKED(IDC_APPLY, &CSettingDlg::OnBnClickedApply)
ON_BN_CLICKED(IDOK, &CSettingDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CSettingDlg ��Ϣ�������

BOOL CSettingDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	{//��ʼ��m_tab�ؼ�
	CString strTemp;
	strTemp.LoadString(IDS_PUSH);
	m_wndTab.InsertItem(0 , strTemp); 
	strTemp.LoadString(IDS_CMD);
	m_wndTab.InsertItem(1 , strTemp); 
	}//��ʼ��m_tab�ؼ�

	{//��������ҳ��ҳ 
	m_pPushPane = new CPushSettingPane(this ,&m_pushValue);
	m_pPushPane->Create(IDD_PUSH_SETTING , &m_wndTab); 
	m_pCmdPane = new CCMDSettingPane(this , &m_cmdValue);
	m_pCmdPane->Create(IDD_CMD_SETTING , &m_wndTab); 
	}//��������ҳ��ҳ 

	{//����ҳ���λ����m_wndTab�ؼ���Χ�� 
	CRect rect; 
	m_wndTab.GetClientRect(&rect); 
	rect.top += 22; 
	rect.bottom -= 4; 
	rect.left += 2; 
	rect.right -= 4; 
	m_pPushPane->MoveWindow(&rect); 
	m_pPushPane->ShowWindow(TRUE); 
	m_pCmdPane->MoveWindow(&rect);
 	m_wndTab.SetCurSel(0); 
 	}//����ҳ���λ����m_wndTab�ؼ���Χ�� 

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

void CSettingDlg::OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	int CurSel = m_wndTab.GetCurSel(); 
	switch(CurSel) 
	{ 
	case 0: //����ҳ
		m_pPushPane->ShowWindow(TRUE); 
		m_pCmdPane->ShowWindow(FALSE); 
		break; 
	case 1: //cmd����ҳ
		m_pPushPane->ShowWindow(FALSE); 
		m_pCmdPane->ShowWindow(TRUE); 
		break; 
	default: 
		ASSERT(FALSE); 
	} 

	*pResult = 0; 
}

BOOL CSettingDlg::DestroyWindow()
{
	//ɾ��ѡ�����Ķ���
	m_wndTab.DeleteAllItems();
	
	if (NULL != m_pPushPane)
	{//ɾ���������
		m_pPushPane->DestroyWindow();
		delete m_pPushPane;
		m_pPushPane = NULL;
	}

	if (NULL != m_pCmdPane)
	{//ɾ��cmd�������
		m_pCmdPane->DestroyWindow();
		delete m_pCmdPane;
		m_pCmdPane = NULL;
	}

	return CDialog::DestroyWindow();
}

void CSettingDlg::ValueChanged()
{
	//�����ݸı�
	this->GetDlgItem(IDC_APPLY)->EnableWindow(TRUE);
}

void CSettingDlg::OnBnClickedApply()
{
	if(this->m_pPushPane->IsValueChanged())
	{//���͵����������޸���
		PUSH_CFG_V pushValue = this->m_pPushPane->GetValue();
		m_pDoc->ChangePushCfgValue(&pushValue);
		m_pPushPane->ValueApplied();
	}

	if(this->m_pCmdPane->IsValueChanged())
	{//CMD�����������޸���
		CMD_CFG_V cmdValue = this->m_pCmdPane->GetValue();
		m_pDoc->ChangeCMDCfgValue(&cmdValue);
		m_pCmdPane->ValueApplied();
	}
	this->GetDlgItem(IDC_APPLY)->EnableWindow(FALSE);
}

void CSettingDlg::OnBnClickedOk()
{
	if(this->GetDlgItem(IDC_APPLY)->IsWindowEnabled())
		OnBnClickedApply();
	OnOK();
}
