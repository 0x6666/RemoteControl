///////////////////////////////////////////////////////////////
//
// FileName	: CMDDlg.cpp 
// Creator	: ����
// Date		: 2013��5��8�գ�13:24:47
// Comment	: CMD�Ի�����ʵ��
//
//////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "RemoteControl.h"
#include "CMDDlg.h"
#include "RemoteControlDoc.h"
#include <locale.h>


// CCMDDlg �Ի���

IMPLEMENT_DYNAMIC(CCMDDlg, CDialog)

CCMDDlg::CCMDDlg(const CString& ip , USHORT port ,const CString& name 
				 ,CRemoteControlDoc* pDoc, PCMD_CFG_V cmdv,  CWnd* pParent /*=NULL*/)
	: CDialog(CCMDDlg::IDD, pParent)
	, m_strIP(ip)
	, m_strName(name)
	, m_pBkBrush(NULL)
	, m_pFont(NULL)
	, m_nLastCount(0)
	, m_pDoc(pDoc)
	, m_uPort(port)
{
	m_cmdCfgValue = *cmdv;
}

CCMDDlg::~CCMDDlg()
{
}

void CCMDDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ET_CMD_VIEW, m_wndCMDView);
}


BEGIN_MESSAGE_MAP(CCMDDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_COMMAND(ID_COPY, &CCMDDlg::OnCopy)
	ON_UPDATE_COMMAND_UI(ID_COPY, &CCMDDlg::OnUpdateCopy)
	ON_COMMAND(ID_CUT, &CCMDDlg::OnCut)
	ON_UPDATE_COMMAND_UI(ID_CUT, &CCMDDlg::OnUpdateCut)
	ON_COMMAND(ID_PAST, &CCMDDlg::OnPast)
	ON_UPDATE_COMMAND_UI(ID_PAST, &CCMDDlg::OnUpdatePast)
	ON_WM_INITMENUPOPUP()
END_MESSAGE_MAP()


// CCMDDlg ��Ϣ�������

BOOL CCMDDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//���öԻ�����Ե�����С
	ModifyStyle( NULL , WS_THICKFRAME);

	//���öԻ������
	SetWindowText(m_strName + _T("(") + m_strIP + _T(")"));

	CEdit* pEdit = dynamic_cast<CEdit*>(GetDlgItem(IDC_ET_CMD_VIEW));
	ASSERT(pEdit != NULL);

	m_pFont = new CFont();
	//����
	LOGFONT lf = {0};
	CString name;
	lf.lfCharSet = m_cmdCfgValue.nCMD_FontCharset;
	lf.lfHeight = m_cmdCfgValue.nCMD_FontSize;
	name += m_cmdCfgValue.czCMD_FontFaceName;
	wcscpy(lf.lfFaceName , name);
	m_pFont->CreateFontIndirect(&lf);
	pEdit->SetFont( m_pFont );

	m_pBkBrush = new CBrush();
	m_pBkBrush->CreateSolidBrush(m_cmdCfgValue.nCMD_BackgroundColor);

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

void CCMDDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	
	//�����༭����Ĵ�С��λ��
	if (IsWindow(m_wndCMDView.GetSafeHwnd()))
	{
		CRect cRc;
		GetClientRect(&cRc);
		m_wndCMDView.MoveWindow( 0 , 0 ,cRc.Width() , cRc.Height() , FALSE);
	}
}

void CCMDDlg::AddCMDLineData( CString strCMDline )
{
	//���ظ���ʾ����
	if((FALSE == m_strCMD.IsEmpty()) && 
		(strCMDline.Left(m_strCMD.GetLength()) == m_strCMD))
		strCMDline = (_T("\r") + strCMDline.Mid(m_strCMD.GetLength()));

	//��������ʾ����
	m_wndCMDView.SetSel( m_wndCMDView.GetWindowTextLength() , -1 , 0 );
	m_wndCMDView.ReplaceSel(strCMDline);
	m_wndCMDView.SetSel( m_wndCMDView.GetWindowTextLength() , -1 , 0 );


	CString str;
	m_wndCMDView.GetWindowText(str);



	//һ������ִ�����?
	if(strCMDline.Right(1) == _T(">"))
	{
		if(m_strCMD.IsEmpty())
		{//��û��ִ�й��κ�����
			//�޸İ�Ȩ��Ϣ
			CString strInfo = _T("");
			m_wndCMDView.GetWindowText(strInfo);
			int nIndex = strInfo.Find(_T(":\\"));
			strInfo = _T("������ҵ�Ƽ���ѧ2009��������̱�ҵ���\r\nPC��Զ�̼��ϵͳ�Ŀ�����������.\r\n\r\n") + strInfo.Mid(nIndex-1);
			m_wndCMDView.SetWindowText(strInfo);
			m_wndCMDView.SetSel(strInfo.GetLength() , -1 , 0 );
		}
	}
	m_nLastCount = m_wndCMDView.GetWindowTextLength();
}

HBRUSH CCMDDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// TODO:  �ڴ˸��� DC ���κ�����
	if(nCtlColor == CTLCOLOR_EDIT)
	{
		pDC->SetTextColor(m_cmdCfgValue.nCMD_FontColor);
		pDC->SetBkColor(m_cmdCfgValue.nCMD_BackgroundColor);
		return static_cast <HBRUSH>(m_pBkBrush->GetSafeHandle());
	}

	//û�б�Ҫ��ʱ���Ƿ���ԭ�еĻ�ˢ
	//����������ģ������Ҽ���ʾ�Ĳ˵��޷���ʾ
	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL CCMDDlg::DestroyWindow()
{
	//ɾ������
	m_pFont->DeleteObject();
	delete m_pFont;
	
	//ɾ������ˢ��
	m_pBkBrush->DeleteObject();
	delete m_pBkBrush;

	return CDialog::DestroyWindow();
}

BOOL CCMDDlg::PreTranslateMessage(MSG* pMsg)
{
	if (GetFocus() != &m_wndCMDView)
	{//ֻ���ڱ༭������Ϣ�Ŵ���
		return CDialog::PreTranslateMessage(pMsg);
	}

	if((pMsg->message == WM_KEYDOWN))
	{//������Ϣ
		if(pMsg->wParam == VK_RETURN)
		{//���µ��ǻس���
			if(FALSE == ExecuteCMD())
			{//ִ������ʧ�� ���������
				return TRUE;
			}

			//�������Ļ�
			if(!m_strCMD.CompareNoCase(_T("cls")))
			{
				//�ȴ��̴߳������
				Sleep(50);

				CString strInfo;
				m_wndCMDView.GetWindowText(strInfo);
				int nIndex = strInfo.Find(_T(":\\"));
				strInfo = _T("\r\n") + strInfo.Mid(nIndex - 1);
				m_wndCMDView.SetWindowText(strInfo);
				//���ŵ����
				m_wndCMDView.SetSel(strInfo.GetLength() , -1 , 0 );
				m_nLastCount = strInfo.GetLength();
			}
			return TRUE;
		}

		//���� �˸� ��delete��
		int nCount = m_nLastCount;
		int n = 0;
		int m = 0;
		m_wndCMDView.GetSel( n , m );

		if(pMsg->wParam == VK_BACK)
		{//�˸�
			if (n == m)
			{//û��ѡ���κ�����
				if(nCount >= n)
					return TRUE;
			}else
			{//ѡ��������
				if(nCount > n)
					return TRUE;
			}
		}

		if(pMsg->wParam == VK_DELETE)
		{//delete��
			if(nCount > n)
				return TRUE;
		}

// 		if(pMsg->wParam == VK_UP)
// 		{//�ϼ���
// 			//���ϵİ����Ĵ���
// 			CString cmd = GetDocument()->GetCmdByUpKey();
// 
// 			////////�������ϴ�
// 			GetEditCtrl().SetSel(GetDocument()->m_strInfo.GetLength(),-1,0);
// 			GetEditCtrl().ReplaceSel(cmd);
// 			//GetEditCtrl().GetWindowTextW(GetDocument()->m_strInfo);
// 			//GetDocument()->m_nLastCount=GetDocument()->m_strInfo.GetLength();
// 			return TRUE;
// 		}
// 		if(pMsg->wParam==VK_DOWN)
// 		{////���µİ����Ĵ���
// 			////���ϵİ����Ĵ���
// 			CString cmd = GetDocument()->GetCmdByDownKey();
// 
// 			////////�������ϴ�
// 			GetEditCtrl().SetSel(GetDocument()->m_strInfo.GetLength(),-1,0);
// 			GetEditCtrl().ReplaceSel(cmd);
// 			//GetEditCtrl().GetWindowTextW(GetDocument()->m_strInfo);
// 			//GetDocument()->m_nLastCount=GetDocument()->m_strInfo.GetLength();
// 			return TRUE;
// 		}
	}
	//���� �ַ���
	if(pMsg->message == WM_CHAR)
	{
		int nCount = m_nLastCount;
		int n = 0;
		int m = 0;
		m_wndCMDView.GetSel( n , m );

		if(nCount > n)
			return TRUE;

	}

	if (pMsg->message == WM_RBUTTONUP)
	{//����Ҽ�  ���������˵� 
		CPoint	point;
		CMenu	menu;
		::GetCursorPos(&point);

		//��ݲ˵�
		menu.LoadMenu(IDR_CMD_MENU);
		menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN|TPM_TOPALIGN , 
			point.x , point.y , this , NULL);
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CCMDDlg::ExecuteCMD()
{
	if (FALSE == PreExecuteCMD())
	{//�����Ѿ�ִ�����ˣ�����Ҫ�ڴ�����
		return FALSE;
	}

	char* cmdBuf = NULL;
	PRCMSG_BUFF pMsg = NULL;
	int size = 0;
	{//�ַ�����ת��
	::setlocale(LC_ALL , "");
	size = ::wcstombs(NULL , m_strCMD , 0);
	cmdBuf = new char[sizeof(RCMSG_BUFF) - 1 + size + 2];
	pMsg = PRCMSG_BUFF(cmdBuf);
	::wcstombs(pMsg->buf , m_strCMD , size);
	::setlocale(LC_ALL , "C");
	}//�ַ�����ת��

	pMsg->buf[size] = '\n';
	pMsg->buf[size+1] = '\0';
	pMsg->msgHead.type = MT_CMD_S;
	pMsg->msgHead.size = sizeof(RCMSG_BUFF) - 1 + size + 2;
	//�������ȥ
	m_pDoc->SendRcMessage(m_strIP , m_uPort , pMsg);
	delete[] cmdBuf;

	return TRUE;
}

void CCMDDlg::CMDFailed( const void* msg )
{
	
}

void CCMDDlg::OnCopy()
{
	m_wndCMDView.Copy();
}

void CCMDDlg::OnUpdateCopy(CCmdUI *pCmdUI)
{
	int n;
	int m;
	m_wndCMDView.GetSel(n , m);
	if (m == n)
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->Enable(TRUE);
}

void CCMDDlg::OnCut()
{
	m_wndCMDView.Cut();
}

void CCMDDlg::OnUpdateCut(CCmdUI *pCmdUI)
{
	int n;
	int m;
	m_wndCMDView.GetSel(n , m);

	if (n == m)//û��ѡ���κζ��������Լ���
	{
		pCmdUI->Enable(FALSE);
		return ;
	}
	if (m_nLastCount > n)
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->Enable(TRUE);
}

void CCMDDlg::OnPast()
{
	m_wndCMDView.Paste();
}

void CCMDDlg::OnUpdatePast(CCmdUI *pCmdUI)
{
	int n;
	int m;
	m_wndCMDView.GetSel(n , m);

// 	if (m_nLastCount > n)
// 		pCmdUI->Enable(FALSE);
// 	else
	if (m_nLastCount <= n && OpenClipboard())
	{
		if (IsClipboardFormatAvailable(CF_TEXT))
		{//ճ������������ı�����
			pCmdUI->Enable(TRUE);
			CloseClipboard();
			return ;
		}
		CloseClipboard();	
	}
	pCmdUI->Enable(FALSE);
}

void CCMDDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	CDialog::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
	
	/*/////////////////////////////////////////////////////////////////////////
	* �������˵���ʾ��ʱ��, WM_INITMENUPOPUP��Ϣ���ȷ�������ʾ�˵��
	* MFC CFrameWnd::OnInitMenuPopup ���������˵��Ϊÿ���˵�����ø������
	* ����������еĻ���.�˵�����۱������Է�ӳ����״̬������/����,ѡ��/ȡ��ѡ
	* �񣩸����û���������ڻ��ڶԻ����Ӧ�ó����в��ܹ�������ΪCDialogû��
	* OnInitMenuPopup ����������ʹ��CWnd's Ĭ�ϴ�����,�ú���û��Ϊ�˵������
	* �������������
	* ���´���Դ��MSDN http://support.microsoft.com/kb/242577
	/////////////////////////////////////////////////////////////////////////*/

	ASSERT(pPopupMenu != NULL);
	// Check the enabled state of various menu items.

	CCmdUI state;
	state.m_pMenu = pPopupMenu;
	ASSERT(state.m_pOther == NULL);
	ASSERT(state.m_pParentMenu == NULL);

	// Determine if menu is popup in top-level menu and set m_pOther to
	// it if so (m_pParentMenu == NULL indicates that it is secondary popup).
	HMENU hParentMenu;
	if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)
		state.m_pParentMenu = pPopupMenu;    // Parent == child for tracking popup.
	else if ((hParentMenu = ::GetMenu(m_hWnd)) != NULL)
	{
		CWnd* pParent = this;
		// Child windows don't have menus--need to go to the top!
		if (pParent != NULL &&
			(hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)
		{
			int nIndexMax = ::GetMenuItemCount(hParentMenu);
			for (int nIndex = 0; nIndex < nIndexMax; nIndex++)
			{
				if (::GetSubMenu(hParentMenu, nIndex) == pPopupMenu->m_hMenu)
				{
					// When popup is found, m_pParentMenu is containing menu.
					state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
					break;
				}
			}
		}
	}

	state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
		state.m_nIndex++)
	{
		state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
		if (state.m_nID == 0)
			continue; // Menu separator or invalid cmd - ignore it.

		ASSERT(state.m_pOther == NULL);
		ASSERT(state.m_pMenu != NULL);
		if (state.m_nID == (UINT)-1)
		{
			// Possibly a popup menu, route to first item of that popup.
			state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
			if (state.m_pSubMenu == NULL ||
				(state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
				state.m_nID == (UINT)-1)
			{
				continue;       // First item of popup can't be routed to.
			}
			state.DoUpdate(this, TRUE);   // Popups are never auto disabled.
		}
		else
		{
			// Normal menu item.
			// Auto enable/disable if frame window has m_bAutoMenuEnable
			// set and command is _not_ a system command.
			state.m_pSubMenu = NULL;
			state.DoUpdate(this, FALSE);
		}

		// Adjust for menu deletions and additions.
		UINT nCount = pPopupMenu->GetMenuItemCount();
		if (nCount < state.m_nIndexMax)
		{
			state.m_nIndex -= (state.m_nIndexMax - nCount);
			while (state.m_nIndex < nCount &&
				pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
			{
				state.m_nIndex++;
			}
		}
		state.m_nIndexMax = nCount;
	}
}

void CCMDDlg::ChangeCfgValue( PCMD_CFG_V cmdValue )
{
	m_cmdCfgValue = *cmdValue;
	if (NULL != m_pFont)
	{
		//����
		LOGFONT lf = {0};
		lf.lfCharSet = cmdValue->nCMD_FontCharset;
		lf.lfHeight = cmdValue->nCMD_FontSize;
		//��������
		CString name;
		name += cmdValue->czCMD_FontFaceName;
		wcscpy(lf.lfFaceName , name);
		m_pFont->DeleteObject();
		m_pFont->CreateFontIndirect(&lf);
		m_wndCMDView.SetFont(m_pFont);
	}

	//������ɫ
	if (NULL != m_pBkBrush)
	{
		m_pBkBrush->DeleteObject();
		m_pBkBrush->CreateSolidBrush(m_cmdCfgValue.nCMD_BackgroundColor);
	}

	Invalidate();
}

BOOL CCMDDlg::PreExecuteCMD()
{
	CString strInfo;
	m_wndCMDView.GetWindowText( strInfo );
	m_strCMD = strInfo.Mid( m_nLastCount );
	m_strCMD.Trim();

	CString temp;
	if((strInfo.Mid(m_nLastCount-1 , 1) == _T(">")) && !m_strCMD.CompareNoCase(_T("exit")))
	{//�˳�
		//TODO ��exit��Ԥ����
		temp.LoadString(IDS_NOT_EXE_CMD);
		temp.Replace(STR_RP , m_strCMD);

		CString strDir;
		int nIndex = strInfo.ReverseFind(_T('\n'));
		strDir = strInfo.Mid(nIndex-1 , m_wndCMDView.GetWindowTextLength() - nIndex - m_strCMD.GetLength() + 1);
		temp = _T("\r\n") + temp + _T("\r\n");
		temp += strDir;
		m_wndCMDView.SetSel(strInfo.GetLength() , -1 , 0 );
		m_wndCMDView.ReplaceSel(temp);
		m_wndCMDView.SetSel(m_wndCMDView.GetWindowTextLength(), -1 , 0 );
		m_nLastCount = m_wndCMDView.GetWindowTextLength();
		return FALSE;
	}
	
	if(!m_strCMD.CompareNoCase(_T("cls")))
	{//����
		CString strDir;
		int nIndex = strInfo.ReverseFind(_T('\n'));
		strDir = strInfo.Mid(nIndex-1 , m_wndCMDView.GetWindowTextLength() - nIndex - m_strCMD.GetLength() + 1);
		m_wndCMDView.SetWindowText(strDir);
		m_wndCMDView.SetSel(m_wndCMDView.GetWindowTextLength(), -1 , 0 );
		m_nLastCount = m_wndCMDView.GetWindowTextLength();
		return FALSE;
	}

	temp = m_strCMD.Left(6);
	temp.Trim();
	if(!temp.CompareNoCase(_T("title")))
	{//�޸ı���
		SetWindowText(m_strCMD.Mid(5));
		return FALSE;
	}

	return TRUE;
}
