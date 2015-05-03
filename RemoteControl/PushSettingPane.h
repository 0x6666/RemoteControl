///////////////////////////////////////////////////////////////
//
// FileName	: PushSettingPane.h 
// Creator	: 杨松
// Date		: 2013年3月27日, 12:47:37
// Comment	: 服务器的选项对话框的声明
//
//////////////////////////////////////////////////////////////

#pragma once
class CSettingDlg;


// COptionDlg 对话框

class CPushSettingPane : public CDialog
{
	DECLARE_DYNAMIC(CPushSettingPane)

public:
	CPushSettingPane(CSettingDlg* pset , PPUSH_CFG_V initValue);   // 标准构造函数
	virtual ~CPushSettingPane();

// 对话框数据
	enum { IDD = IDD_PUSH_SETTING };
	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	//对话框初始化
	virtual BOOL OnInitDialog();

	//////////////////////////////////////////////////////////////////////////
	//用于响应拖动调整抓屏质量的slide
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	//////////////////////////////////////////////////////////////////////////
	//用于响应调整抓屏频率的spin点击事件
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	//////////////////////////////////////////////////////////////////////////
	//点击确定按钮的响应函数
	afx_msg void OnBnClickedOk();

	//////////////////////////////////////////////////////////////////////////
	//抓屏尺寸宽度变化
	afx_msg void OnEnChangeScreenSizeW();

	//////////////////////////////////////////////////////////////////////////
	//抓屏尺寸高度变化
	afx_msg void OnEnChangeScreenSizeH();

	//////////////////////////////////////////////////////////////////////////
	//是否锁定抓屏长宽比改变
	afx_msg void OnBnClickedLockSeziRatio();

	//////////////////////////////////////////////////////////////////////////
	//抓屏频率被编辑
	afx_msg void OnEnChangeScreenFrequency();
	long GetScreenSizeW();
	long GetScreenSizeH();
	long GetScreenQuality();
	long GetScreenFrequency();
	BOOL IsLockScreenSizeRatio();

	//是否有数据改变了
	BOOL IsValueChanged();
	//获取设置的值
	PUSH_CFG_V GetValue();
	void ValueApplied();
private:
	//抓屏的大小
	long m_lScreenSizeW;
	long m_lScreenSizeH;
	//抓屏的质量
	int m_iScreenQuality;
	//流畅度
	long m_lScreenFrequency;
	//在调整抓屏的大小时是否锁定比例
	BOOL m_bLockSizeRatio;

	//数据修改了
	BOOL m_bChanged;
	CSettingDlg* m_pSetDlg;
};
