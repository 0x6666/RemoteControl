// LetterRain.cpp : 实现文件
//

#include "stdafx.h"
#include "RemoteControlClient.h"
#include "LetterRainDlg.h"

//字体的宽度 高度
#define FONT_W 10
#define FONT_H 15
// CLetterRainDlg 对话框

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
// CLetterRainDlg 消息处理程序
HBRUSH CLetterRainDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	//将背景刷成黑色
	if(nCtlColor == CTLCOLOR_DLG)
	{
		if (NULL == m_pDlgBgBrush)
		{
			m_pDlgBgBrush = new CBrush(RGB(0,0,0));
		}
		return (HBRUSH)(m_pDlgBgBrush->m_hObject);
	}
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

char CLetterRainDlg::RandomChar(void)
{
	//随机字符
	//只包含大小写字符和字符(62个字符)
	int letter = rand() % 62;
	if (letter < 10 )
	{//数字
		return (char)(letter + 0x30);
	}else if (letter < 36 )
	{//大写字符
		return (char)(letter + 0x41 - 10);
	} else 
	{//小写字符
		return (char)(letter + 0x61 - 10 - 26);
	}
	return (char)0x41;
}

int CLetterRainDlg::InitColumn(CharColumn* cc , int x)
{
	PCharChain pTemp = NULL;
	//当前列的长度(随机)
	cc->iStrLen = rand() % (STRMAXLEN - STRMINLEN) + STRMINLEN; 
	//当前列的开始显示的x坐标
	cc->x = x + 3 ;
	//当前列的开始显示的y坐标
	cc->y =rand()%3 ? rand() % SCREEN_SIZE_H : 0;
	cc->iMustStopTimes = rand()% 4 ;
	cc->iStopTimes = 0 ;
	//分配一列的空间
	cc->head = cc->pMem = (CharChain*)calloc(cc->iStrLen , sizeof(CharChain));
	//初始化双向链表
	for(int j = 0 ; j < cc->iStrLen - 1 ; j++)
	{
		cc->pMem->prev = pTemp;				//前一个节点
		cc->pMem->ch   = 0;
		cc->pMem->next = cc->pMem + 1;		//cc->pMem+1一个显示列的后个元素
		pTemp	       = cc->pMem++;
	}
	//最后一个节点
	cc->pMem->prev = pTemp;					
	cc->pMem->ch   = 0;
	cc->pMem->next = cc->head;
	cc->head->prev = cc->pMem;				//头节点的前一个为此链的最后一个元素
	cc->pMem = cc->head;					//free掉申请的内存要用pMem当参数
	cc->head->ch = RandomChar();			//对链表头的 元素填充
	return 0;
}

int CLetterRainDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	//开启定时器
	::SetTimer(this->m_hWnd , LETTER_RAIN_TIMER , 10 , NULL) ;

	HDC hdc  = ::GetDC(this->m_hWnd);
	m_hDCMem   = ::CreateCompatibleDC(hdc);
	m_hBitmap  = ::CreateCompatibleBitmap(hdc, SCREEN_SIZE_W , SCREEN_SIZE_H);
	::SelectObject(m_hDCMem, m_hBitmap);
	::ReleaseDC(m_hWnd, hdc);
	//创建字体
	HFONT hFont = CreateFont(FONT_H, FONT_W-5, 0, 0, FW_BOLD, 0, 0, 0,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DRAFT_QUALITY, FIXED_PITCH | FF_SWISS, TEXT("Fixedsys"));
	SelectObject(m_hDCMem, hFont);
	DeleteObject (hFont) ;

	//设置背景模式为透明
	SetBkMode(m_hDCMem, TRANSPARENT);
	
	//屏幕所显示字母雨的列数
	m_iColumnCount = SCREEN_SIZE_W / (FONT_W * 3 / 2);
	
	//分配所有列的管理节点空间
	m_ccChain = (CharColumn*)calloc(m_iColumnCount, sizeof(CharColumn));
	
	//初始化每一列
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
		//将内存设备映像刷成黑色
		::PatBlt( m_hDCMem , 0 , 0 , SCREEN_SIZE_W , SCREEN_SIZE_H , BLACKNESS) ; 
		for(int i = 0; i < m_iColumnCount; i++)
		{
			bStop = PCharColumn(m_ccChain + i)->iStopTimes++ > PCharColumn(m_ccChain + i)->iMustStopTimes;
			//point用于遍历整个显示列
			pTemp = PCharColumn(m_ccChain + i)->head; 

			//第一个字符显示为 白色
			SetTextColor(m_hDCMem , RGB(255, 255, 255));
			TextOutA(m_hDCMem, PCharColumn(m_ccChain+i)->x , 
				PCharColumn(m_ccChain+i)->y, &(pTemp->ch), 1);
			int j = PCharColumn(m_ccChain+i)->y; 
			pTemp = pTemp->next;
			
			//遍历整个显示列,将这个显示列里的字符从下往上显示
			
			//temp绿色过度到黑色之用
			int temp = 0 ;

			while(pTemp != PCharColumn(m_ccChain+i)->head && pTemp->ch)
			{
				SetTextColor(m_hDCMem, RGB(0, 255-(255*(temp++)/PCharColumn(m_ccChain+i)->iStrLen), 0));
				TextOutA(m_hDCMem, PCharColumn(m_ccChain+i)->x, j-=FONT_H , &(pTemp->ch), 1);
				pTemp = pTemp->next;
			}

			if(bStop)//需要停止一下了?
				PCharColumn(m_ccChain+i)->iStopTimes = 0 ;
			else 
				continue;
			//下次开始显示的y坐标 为当前的y坐标加上 一个字符的高度
			PCharColumn(m_ccChain+i)->y += FONT_H; 
			//如果开始显示的y坐标减去 整个显示列的长度超过了屏幕的高度
			if( PCharColumn(m_ccChain+i)->y - PCharColumn(m_ccChain+i)->iStrLen*FONT_H > SCREEN_SIZE_H)
			{
				free( PCharColumn(m_ccChain+i)->pMem );
				InitColumn(m_ccChain+i , (FONT_W*3/2)*i);
			}
			//链表的头 为此链表的前个元素,因为下次开始显示的时候 就相当与在整个显示列的开头添加个元素,然后在开始往上显示
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
	//销毁定时器
	::KillTimer(this->m_hWnd, LETTER_RAIN_TIMER);
	
	//删除位图
	DeleteObject(m_hBitmap);
	//删除绘图设备
	DeleteDC(m_hDCMem);
	
	//释放每一个字符序列
	for(int i = 0; i < m_iColumnCount; i++)
	{
		free( (m_ccChain + i)->pMem );
	}
	//释放列表头
	free(m_ccChain);
	
	//显示光标
	::ShowCursor(TRUE);

	//删除背景刷子
	if (NULL != m_pDlgBgBrush)
	{
		delete m_pDlgBgBrush;
	}

	CDialog::OnDestroy();
}

BOOL CLetterRainDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	//设置窗口为最顶层窗口，且全屏
	::SetWindowPos(this->GetSafeHwnd() , HWND_TOPMOST , 0 , 0 , SCREEN_SIZE_W, SCREEN_SIZE_W , SWP_SHOWWINDOW);

	//隐藏光标
	::ShowCursor(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

BOOL CLetterRainDlg::PreTranslateMessage(MSG* pMsg)
{
	//屏蔽键盘输入
	if(pMsg->message == WM_KEYDOWN)
		return TRUE;

	return CDialog::PreTranslateMessage(pMsg);
}
