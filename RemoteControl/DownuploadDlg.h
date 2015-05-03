///////////////////////////////////////////////////////////////
//
// FileName	: DownuploadDlg.h 
// Creator	: ����
// Date		: 2013��4��17�գ�12:23:32
// Comment	: ����/�ϴ��Ի����������
//
//////////////////////////////////////////////////////////////


#ifndef _UP_DOWNLOAD_DLG_H_
#define _UP_DOWNLOAD_DLG_H_
#include "afxcmn.h"


// CDownloadDlg �Ի���

class CDownUploadDlg : public CDialog
{
	DECLARE_DYNAMIC(CDownUploadDlg)

public:
	CDownUploadDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDownUploadDlg();

// �Ի�������
	enum { IDD = IDD_DOWNLOAD_DLG };

protected:

	typedef std::list<PDowmUploadContext> DownloadDesList;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

	//�����ļ��ǳ���
	//param
	//wParam	��CString*��ip
	afx_msg LRESULT OnDlSocketError(WPARAM wParam , LPARAM lParam);

	//���ؽ���
	//param
	//wParam	��CString*��ip
	//lParam	DWORD	�Ѿ������˵�����
	afx_msg LRESULT OnDlProgress(WPARAM wParam , LPARAM lParam);

	//�����ļ���С
	//param
	//wParam	CString* ip
	//lParam	DWORD	�ļ���С
	afx_msg LRESULT OnDlFileSize(WPARAM wParam , LPARAM lParam);

	//һ�������߳��˳�
	//�����߳��˳�
	//param
	//wParam	CString* ip
	afx_msg LRESULT OnDlThreadExit(WPARAM wParam , LPARAM lParam);

	//���������������
	//PDowmloadContext GetDowmloadDescripter(const CString& ip);

	//���ָ���������λ��
	int GetDowmloadListPos(PDowmUploadContext ddes);

private:
	//�����б�
	DownloadDesList m_listDownloadDescripter;

public:
	//���һ�����ض���
	void AddDownUploadItem( PDowmUploadContext ddes );
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CListCtrl m_wndDownloadList;
	virtual BOOL DestroyWindow();
};

#endif