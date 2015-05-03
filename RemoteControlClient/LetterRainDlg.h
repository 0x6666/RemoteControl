#pragma once

#define STRMAXLEN  25 //һ����ʾ�е���󳤶�
#define STRMINLEN  8  //һ����ʾ�е���С����
// CLetterRainDlg �Ի���

class CLetterRainDlg : public CDialog
{
	DECLARE_DYNAMIC(CLetterRainDlg)

public:
	CLetterRainDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CLetterRainDlg();
	//�ಿ��
public:
	//��ĸ�еĽڵ�(˫��ѭ������)
	typedef struct _CharChain 
	{
		_CharChain *prev;   //��һ���ڵ�
		char	   ch;      //һ����ʾ���е�һ���ַ�
		_CharChain *next;   //��һ���ڵ�
	}CharChain , *PCharChain;

	//��ĸ��
	typedef struct _CharColumn 
	{
		PCharChain head;		//��ͷ��ǰҪ��ʾ�ĵ�һ���ַ�����ɫ��
		PCharChain pMem;		//��ǰ�ַ����е���ʼ��ַ
		int x;					//��ʾ�еĿ�ʼ��ʾ��x,y����
		int y;					
		int iStrLen;			//����еĳ���
		//һ�������������ڵ����ַ�����������ٶ�
		int iStopTimes;			//�Ѿ�ͣ�͵Ĵ���
		int iMustStopTimes;		//����ͣ�͵Ĵ���(���)
	}CharColumn , *PCharColumn;

	// �Ի�������
	enum { IDD = IDD_LETTER_RAIN_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	// ��������ַ�
	char RandomChar(void);
	// ��ʼ���б�
	int InitColumn(CharColumn* cc , int x);
	
	//Ҫ��ʾ����
	CharColumn* m_ccChain;
	
	//�ڴ��еĻ�ͼ�豸������
	HDC m_hDCMem;
	//����������Ļ���λͼ
	HBITMAP m_hBitmap;
	
	//������ʾ������
	int m_iColumnCount;

	//�Ի��򱳾�ˢ��
	CBrush* m_pDlgBgBrush;

};
