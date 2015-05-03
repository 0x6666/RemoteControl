///////////////////////////////////////////////////////////////
//
// FileName	: DownuploadDlg.h 
// Creator	: 杨松
// Date		: 2013年4月17日，12:23:32
// Comment	: 下载/上传对话框类的声明
//
//////////////////////////////////////////////////////////////


#ifndef _UP_DOWNLOAD_DLG_H_
#define _UP_DOWNLOAD_DLG_H_
#include "afxcmn.h"


// CDownloadDlg 对话框

class CDownUploadDlg : public CDialog
{
	DECLARE_DYNAMIC(CDownUploadDlg)

public:
	CDownUploadDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDownUploadDlg();

// 对话框数据
	enum { IDD = IDD_DOWNLOAD_DLG };

protected:

	typedef std::list<PDowmUploadContext> DownloadDesList;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

	//下载文件是出错
	//param
	//wParam	（CString*）ip
	afx_msg LRESULT OnDlSocketError(WPARAM wParam , LPARAM lParam);

	//下载进度
	//param
	//wParam	（CString*）ip
	//lParam	DWORD	已经下载了的数据
	afx_msg LRESULT OnDlProgress(WPARAM wParam , LPARAM lParam);

	//设置文件大小
	//param
	//wParam	CString* ip
	//lParam	DWORD	文件大小
	afx_msg LRESULT OnDlFileSize(WPARAM wParam , LPARAM lParam);

	//一个下载线程退出
	//下载线程退出
	//param
	//wParam	CString* ip
	afx_msg LRESULT OnDlThreadExit(WPARAM wParam , LPARAM lParam);

	//获得下载项描述符
	//PDowmloadContext GetDowmloadDescripter(const CString& ip);

	//获得指定下载项的位置
	int GetDowmloadListPos(PDowmUploadContext ddes);

private:
	//下载列表
	DownloadDesList m_listDownloadDescripter;

public:
	//添加一个下载对象
	void AddDownUploadItem( PDowmUploadContext ddes );
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CListCtrl m_wndDownloadList;
	virtual BOOL DestroyWindow();
};

#endif