///////////////////////////////////////////////////////////////
//
// FileName	: DownUploadDlg.cpp
// Creator	: ����
// Date		: 2013��4��17�գ�12:23:32
// Comment	: ����/�ϴ��Ի������ʵ��
//
//////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RemoteControl.h"
#include "downuploaddlg.h"


// CDownloadDlg �Ի���

IMPLEMENT_DYNAMIC(CDownUploadDlg, CDialog)

CDownUploadDlg::CDownUploadDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDownUploadDlg::IDD, pParent)
{

}

CDownUploadDlg::~CDownUploadDlg()
{
}

void CDownUploadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DOWNLOAD_LIST, m_wndDownloadList);
}


BEGIN_MESSAGE_MAP(CDownUploadDlg, CDialog)
	ON_WM_SIZE()
	ON_MESSAGE(WM_DL_ERROE , OnDlSocketError)
	ON_MESSAGE(WM_DL_PROGRESS , OnDlProgress)
	ON_MESSAGE(WM_DL_FILE_SIZE , OnDlFileSize)
	ON_MESSAGE(WM_DL_THREAD_EXIT , OnDlThreadExit)
END_MESSAGE_MAP()

// CDownloadDlg ��Ϣ�������

//�б��е�����
#define DU_LIST_ID			0
#define DU_LIST_OPT			1
#define DU_LIST_SIZE		2
#define DU_LIST_PROGRESS	3
#define DU_LIST_RPATH		4
#define DU_LIST_CPATH		5

BOOL CDownUploadDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//���öԻ�����Ե�����С
	ModifyStyle( NULL , WS_THICKFRAME);


	//��ʼ���б�ؼ�
	m_wndDownloadList.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_wndDownloadList.InsertColumn(DU_LIST_ID , _T("ID") , 0 , 35);
	m_wndDownloadList.InsertColumn(DU_LIST_OPT , _T("����") , 0 , 40);
	m_wndDownloadList.InsertColumn(DU_LIST_SIZE , _T("��С") , 0 , 55);
	m_wndDownloadList.InsertColumn(DU_LIST_PROGRESS , _T("����") , 0 , 60);
	m_wndDownloadList.InsertColumn(DU_LIST_RPATH , _T("Զ��·��") , 0 , 110);
	m_wndDownloadList.InsertColumn(DU_LIST_CPATH , _T("����·��") , 0 , 110);


	{//���Ի���ŵ��������½� 
	CRect rt;  
	SystemParametersInfo(SPI_GETWORKAREA , 0 , (PVOID)&rt , 0 ); 
	
	CRect wRect;
	GetWindowRect(&wRect);

	SetWindowPos(NULL , rt.Width() - wRect.Width() , rt.Height() - wRect.Height() , 0 , 0 , SWP_NOSIZE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}
void CDownUploadDlg::AddDownUploadItem( PDowmUploadContext ddes )
{
	CString temp;

	m_listDownloadDescripter.push_back(ddes);
	int index = m_wndDownloadList.GetItemCount();
	//IP
	temp.Format(_T("%d") , ddes->nClientID);
	m_wndDownloadList.InsertItem(index , temp);
	//Զ��·��
	UINT type = 0;
	m_wndDownloadList.SetItemText(index , DU_LIST_RPATH , TranslatePath(ddes->clientPath , type));
	//����·��
	m_wndDownloadList.SetItemText(index , DU_LIST_CPATH , ddes->curPath);

	if (!ddes->isDownload)
	{//�ϴ��ļ�
		//�ļ���С
		m_wndDownloadList.SetItemText(index , DU_LIST_SIZE , GetSizeToString(ddes->fileLen));
		m_wndDownloadList.SetItemText(index , DU_LIST_OPT , _T("�ϴ�"));
	}else
	{
		m_wndDownloadList.SetItemText(index , DU_LIST_OPT , _T("����"));
	}
}

void CDownUploadDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	//�������ؼ��Ĵ�С��λ��
	if (IsWindow(m_wndDownloadList.GetSafeHwnd()))
	{
		CRect cRc;
		GetClientRect(&cRc);
		m_wndDownloadList.MoveWindow( 0 , 0 ,cRc.Width() , cRc.Height() , TRUE);
	}
}

LRESULT CDownUploadDlg::OnDlSocketError( WPARAM wParam , LPARAM lParam )
{
	PDowmUploadContext ddes = PDowmUploadContext(wParam);

	InterlockedExchange(&(ddes->lThreadState), 0);

	int nItem = GetDowmloadListPos(ddes);
	if (-1 == nItem)//û��ָ������Ŀ
		return 0;

	CString temp;
	temp.LoadString(IDS_DOWNLOAD_ERROR);
	m_wndDownloadList.SetItemText(nItem , DU_LIST_PROGRESS , temp);

	return 0;
}

LRESULT CDownUploadDlg::OnDlProgress( WPARAM wParam , LPARAM lParam )
{
	PDowmUploadContext ddes = PDowmUploadContext(wParam);
	if (NULL == ddes)
		return 0;

	int nItem = GetDowmloadListPos(ddes);
	if (-1 == nItem)//û��ָ������Ŀ
		return 0;

	ddes->dlSize = (DWORD)lParam;

	//����ٷֱ�
	double f = (DWORD)lParam * 100.0 / (ddes->fileLen * 1.0);
	CString temp;
	temp.Format(_T("%.2f%%") , f);
	m_wndDownloadList.SetItemText(nItem , DU_LIST_PROGRESS , temp);

	return 0;
}

LRESULT CDownUploadDlg::OnDlFileSize( WPARAM wParam , LPARAM lParam )
{
	PDowmUploadContext ddes = PDowmUploadContext(wParam);
	if (NULL == ddes)
		return 0;

	//�ļ���С
	ddes->fileLen = (DWORD)lParam;

	int nItem = GetDowmloadListPos(ddes);
	if (-1 == nItem)//û��ָ������Ŀ
		return 0;

	CString temp = GetSizeToString(lParam);
	m_wndDownloadList.SetItemText(nItem , DU_LIST_SIZE , temp);

	return 0;
}

LRESULT CDownUploadDlg::OnDlThreadExit( WPARAM wParam , LPARAM lParam )
{
	PDowmUploadContext ddes = PDowmUploadContext(wParam);
	if (NULL == ddes)
		return 0;

	InterlockedExchange(&(ddes->lThreadState) , 0);
	CloseHandle(ddes->dlThread);
	ddes->dlThread = NULL;

	BOOL dlOver = ddes->fileLen == ddes->dlSize;

	int nItem = GetDowmloadListPos(ddes);
	if (-1 == nItem)//û��ָ������Ŀ
		return 0;

	CString temp;
	if (dlOver)
	{//�Ѿ�����\�ϴ����
		temp.LoadString(IDS_UD_OVER);
		m_wndDownloadList.SetItemText(nItem , DU_LIST_PROGRESS , temp);
	}else
	{//ʧ��
		temp.LoadString(IDS_UD_ERROR);
		m_wndDownloadList.SetItemText(nItem , DU_LIST_PROGRESS , temp);
	}

	return 0;
}

int CDownUploadDlg::GetDowmloadListPos( PDowmUploadContext ddes )
{
	int cnt = m_wndDownloadList.GetItemCount();
	int nItem = 0;
	CString strID , strPath , temp;
	UINT type = 0;
	temp.Format(_T("%d") , ddes->nClientID);
	while (nItem <= cnt)
	{
		//��ȡip
		strID = m_wndDownloadList.GetItemText(nItem , DU_LIST_ID);
		//��ȡ·��
		strPath =  m_wndDownloadList.GetItemText(nItem , DU_LIST_RPATH);
		if (temp == strID && strPath == TranslatePath(ddes->clientPath , type))
			return nItem;
		++nItem;
	}

	//û��ָ������Ŀ
	return -1;
}

BOOL CDownUploadDlg::DestroyWindow()
{
	while (!m_listDownloadDescripter.empty())
	{
		PDowmUploadContext ddes =  m_listDownloadDescripter.front();
		m_listDownloadDescripter.pop_front();
		InterlockedExchange(&(ddes->lThreadState ), 0);
		WaiteExitThread(ddes->dlThread , 500);
		CloseHandle(ddes->dlThread);
		ddes->dlThread = NULL;
		delete ddes;
	}

	return CDialog::DestroyWindow();
}
