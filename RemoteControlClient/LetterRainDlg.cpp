// LetterRain.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "RemoteControlClient.h"
#include "LetterRainDlg.h"

//����Ŀ�� �߶�
#define FONT_W 10
#define FONT_H 15
// CLetterRainDlg �Ի���

IMPLEMENT_DYNAMIC(CLetterRainDlg, CDialog)

CLetterRainDlg::CLetterRainDlg(CWnd* pParent /*=NULL*/)
: CDialog(CLetterRainDlg::IDD, pParent)
, m_ccChain(NULL)
, m_iColumnCount(0)
, m_pDlgBgBrush(NULL)
{

}

CLetterRainDlg::~CLetterRainDlg()
{
}

void CLetterRainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLetterRainDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()
// CLetterRainDlg ��Ϣ�������
HBRUSH CLetterRainDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	//������ˢ�ɺ�ɫ
	if(nCtlColor == CTLCOLOR_DLG)
	{
		if (NULL == m_pDlgBgBrush)
		{
			m_pDlgBgBrush = new CBrush(RGB(0,0,0));
		}
		return (HBRUSH)(m_pDlgBgBrush->m_hObject);
	}
	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}

char CLetterRainDlg::RandomChar(void)
{
	//����ַ�
	//ֻ������Сд�ַ����ַ�(62���ַ�)
	int letter = rand() % 62;
	if (letter < 10 )
	{//����
		return (char)(letter + 0x30);
	}else if (letter < 36 )
	{//��д�ַ�
		return (char)(letter + 0x41 - 10);
	} else 
	{//Сд�ַ�
		return (char)(letter + 0x61 - 10 - 26);
	}
	return (char)0x41;
}

int CLetterRainDlg::InitColumn(CharColumn* cc , int x)
{
	PCharChain pTemp = NULL;
	//��ǰ�еĳ���(���)
	cc->iStrLen = rand() % (STRMAXLEN - STRMINLEN) + STRMINLEN; 
	//��ǰ�еĿ�ʼ��ʾ��x����
	cc->x = x + 3 ;
	//��ǰ�еĿ�ʼ��ʾ��y����
	cc->y =rand()%3 ? rand() % SCREEN_SIZE_H : 0;
	cc->iMustStopTimes = rand()% 4 ;
	cc->iStopTimes = 0 ;
	//����һ�еĿռ�
	cc->head = cc->pMem = (CharChain*)calloc(cc->iStrLen , sizeof(CharChain));
	//��ʼ��˫������
	for(int j = 0 ; j < cc->iStrLen - 1 ; j++)
	{
		cc->pMem->prev = pTemp;				//ǰһ���ڵ�
		cc->pMem->ch   = 0;
		cc->pMem->next = cc->pMem + 1;		//cc->pMem+1һ����ʾ�еĺ��Ԫ��
		pTemp	       = cc->pMem++;
	}
	//���һ���ڵ�
	cc->pMem->prev = pTemp;					
	cc->pMem->ch   = 0;
	cc->pMem->next = cc->head;
	cc->head->prev = cc->pMem;				//ͷ�ڵ��ǰһ��Ϊ���������һ��Ԫ��
	cc->pMem = cc->head;					//free��������ڴ�Ҫ��pMem������
	cc->head->ch = RandomChar();			//������ͷ�� Ԫ�����
	return 0;
}

int CLetterRainDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	//������ʱ��
	::SetTimer(this->m_hWnd , LETTER_RAIN_TIMER , 10 , NULL) ;

	HDC hdc  = ::GetDC(this->m_hWnd);
	m_hDCMem   = ::CreateCompatibleDC(hdc);
	m_hBitmap  = ::CreateCompatibleBitmap(hdc, SCREEN_SIZE_W , SCREEN_SIZE_H);
	::SelectObject(m_hDCMem, m_hBitmap);
	::ReleaseDC(m_hWnd, hdc);
	//��������
	HFONT hFont = CreateFont(FONT_H, FONT_W-5, 0, 0, FW_BOLD, 0, 0, 0,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DRAFT_QUALITY, FIXED_PITCH | FF_SWISS, TEXT("Fixedsys"));
	SelectObject(m_hDCMem, hFont);
	DeleteObject (hFont) ;

	//���ñ���ģʽΪ͸��
	SetBkMode(m_hDCMem, TRANSPARENT);
	
	//��Ļ����ʾ��ĸ�������
	m_iColumnCount = SCREEN_SIZE_W / (FONT_W * 3 / 2);
	
	//���������еĹ���ڵ�ռ�
	m_ccChain = (CharColumn*)calloc(m_iColumnCount, sizeof(CharColumn));
	
	//��ʼ��ÿһ��
	for(int i = 0; i < m_iColumnCount; i++)
	{
		InitColumn(m_ccChain + i, (FONT_W * 3 / 2 ) * i);
	}

	return 0 ;
}

void CLetterRainDlg::OnTimer(UINT_PTR nIDEvent)
{
	BOOL bStop = 0;
	PCharChain pTemp = NULL;
	if(nIDEvent == LETTER_RAIN_TIMER)
	{
		HDC hdc = ::GetDC(this->m_hWnd);
		//���ڴ��豸ӳ��ˢ�ɺ�ɫ
		::PatBlt( m_hDCMem , 0 , 0 , SCREEN_SIZE_W , SCREEN_SIZE_H , BLACKNESS) ; 
		for(int i = 0; i < m_iColumnCount; i++)
		{
			bStop = PCharColumn(m_ccChain + i)->iStopTimes++ > PCharColumn(m_ccChain + i)->iMustStopTimes;
			//point���ڱ���������ʾ��
			pTemp = PCharColumn(m_ccChain + i)->head; 

			//��һ���ַ���ʾΪ ��ɫ
			SetTextColor(m_hDCMem , RGB(255, 255, 255));
			TextOutA(m_hDCMem, PCharColumn(m_ccChain+i)->x , 
				PCharColumn(m_ccChain+i)->y, &(pTemp->ch), 1);
			int j = PCharColumn(m_ccChain+i)->y; 
			pTemp = pTemp->next;
			
			//����������ʾ��,�������ʾ������ַ�����������ʾ
			
			//temp��ɫ���ȵ���ɫ֮��
			int temp = 0 ;

			while(pTemp != PCharColumn(m_ccChain+i)->head && pTemp->ch)
			{
				SetTextColor(m_hDCMem, RGB(0, 255-(255*(temp++)/PCharColumn(m_ccChain+i)->iStrLen), 0));
				TextOutA(m_hDCMem, PCharColumn(m_ccChain+i)->x, j-=FONT_H , &(pTemp->ch), 1);
				pTemp = pTemp->next;
			}

			if(bStop)//��Ҫֹͣһ����?
				PCharColumn(m_ccChain+i)->iStopTimes = 0 ;
			else 
				continue;
			//�´ο�ʼ��ʾ��y���� Ϊ��ǰ��y������� һ���ַ��ĸ߶�
			PCharColumn(m_ccChain+i)->y += FONT_H; 
			//�����ʼ��ʾ��y�����ȥ ������ʾ�еĳ��ȳ�������Ļ�ĸ߶�
			if( PCharColumn(m_ccChain+i)->y - PCharColumn(m_ccChain+i)->iStrLen*FONT_H > SCREEN_SIZE_H)
			{
				free( PCharColumn(m_ccChain+i)->pMem );
				InitColumn(m_ccChain+i , (FONT_W*3/2)*i);
			}
			//�����ͷ Ϊ�������ǰ��Ԫ��,��Ϊ�´ο�ʼ��ʾ��ʱ�� ���൱����������ʾ�еĿ�ͷ��Ӹ�Ԫ��,Ȼ���ڿ�ʼ������ʾ
			PCharColumn(m_ccChain+i)->head = PCharColumn(m_ccChain+i)->head->prev;
			PCharColumn(m_ccChain+i)->head->ch = RandomChar();
		}
		::BitBlt(hdc, 0, 0, SCREEN_SIZE_W, SCREEN_SIZE_H, m_hDCMem, 0, 0, SRCCOPY);
		::ReleaseDC(m_hWnd, hdc);
	}

	CDialog::OnTimer(nIDEvent);
}

void CLetterRainDlg::OnDestroy()
{
	//���ٶ�ʱ��
	::KillTimer(this->m_hWnd, LETTER_RAIN_TIMER);
	
	//ɾ��λͼ
	DeleteObject(m_hBitmap);
	//ɾ����ͼ�豸
	DeleteDC(m_hDCMem);
	
	//�ͷ�ÿһ���ַ�����
	for(int i = 0; i < m_iColumnCount; i++)
	{
		free( (m_ccChain + i)->pMem );
	}
	//�ͷ��б�ͷ
	free(m_ccChain);
	
	//��ʾ���
	::ShowCursor(TRUE);

	//ɾ������ˢ��
	if (NULL != m_pDlgBgBrush)
	{
		delete m_pDlgBgBrush;
	}

	CDialog::OnDestroy();
}

BOOL CLetterRainDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	//���ô���Ϊ��㴰�ڣ���ȫ��
	::SetWindowPos(this->GetSafeHwnd() , HWND_TOPMOST , 0 , 0 , SCREEN_SIZE_W, SCREEN_SIZE_W , SWP_SHOWWINDOW);

	//���ع��
	::ShowCursor(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

BOOL CLetterRainDlg::PreTranslateMessage(MSG* pMsg)
{
	//���μ�������
	if(pMsg->message == WM_KEYDOWN)
		return TRUE;

	return CDialog::PreTranslateMessage(pMsg);
}
