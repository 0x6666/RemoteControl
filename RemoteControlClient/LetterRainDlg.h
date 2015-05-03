#pragma once

#define STRMAXLEN  25 //一个显示列的最大长度
#define STRMINLEN  8  //一个显示列的最小长度
// CLetterRainDlg 对话框

class CLetterRainDlg : public CDialog
{
	DECLARE_DYNAMIC(CLetterRainDlg)

public:
	CLetterRainDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CLetterRainDlg();
	//类部类
public:
	//字母列的节点(双向循环链表)
	typedef struct _CharChain 
	{
		_CharChain *prev;   //上一个节点
		char	   ch;      //一个显示列中的一个字符
		_CharChain *next;   //下一个节点
	}CharChain , *PCharChain;

	//字母列
	typedef struct _CharColumn 
	{
		PCharChain head;		//列头当前要显示的第一个字符（白色）
		PCharChain pMem;		//当前字符序列的起始地址
		int x;					//显示列的开始显示的x,y坐标
		int y;					
		int iStrLen;			//这个列的长度
		//一下两个变量用于调节字符序列下落的速度
		int iStopTimes;			//已经停滞的次数
		int iMustStopTimes;		//必须停滞的次数(随机)
	}CharColumn , *PCharColumn;

	// 对话框数据
	enum { IDD = IDD_LETTER_RAIN_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	// 产生随机字符
	char RandomChar(void);
	// 初始化列表
	int InitColumn(CharColumn* cc , int x);
	
	//要显示的列
	CharColumn* m_ccChain;
	
	//内存中的绘图设备上下文
	HDC m_hDCMem;
	//用来绘制字幕雨的位图
	HBITMAP m_hBitmap;
	
	//可以显示的列数
	int m_iColumnCount;

	//对话框背景刷子
	CBrush* m_pDlgBgBrush;

};
