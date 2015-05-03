///////////////////////////////////////////////////////////////
//
// FileName	: FileManageDlg.cpp 
// Creator	: ����
// Date		: ?2013��4��17�գ�12:23:32
// Comment	: Զ���ļ�����Ի����������
//
//////////////////////////////////////////////////////////////

#pragma once
#include "afxcmn.h"


class CRemoteControlDoc;
// CDocManageDlg �Ի���

class CFileManageDlg : public CDialog
{
	DECLARE_DYNAMIC(CFileManageDlg)

public:
	CFileManageDlg(const CString& ip , USHORT port , const CString& name , CRemoteControlDoc* pDoc , CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CFileManageDlg();

// �Ի�������
	enum { IDD = IDD_DOC_MANAGE_DLG };

private:
	//�ͻ��˵�ַ
	CString m_strIP;
	//�ͻ��˼������
	CString m_strName;
	//�ͻ�����Ϣ�˿�
	USHORT	m_nPort;
	CRemoteControlDoc* m_pDoc;

	//�ҵ��ĵ�
	HTREEITEM m_hMyDoc;
	HTREEITEM m_hComputer;
	HTREEITEM m_hDesktop;

private:
	//////////////////////////////////////////////////////////////////////////
	//����ƶ��ļ����
	//param
	//	path	·��
	//return 
	//		NULL	������ָ����·��
	HTREEITEM GetItemHandle(CString path);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CTreeCtrl m_wndFileTree;
	CImageList m_wndImageList;
	afx_msg void OnNMClickFileTree(NMHDR *pNMHDR, LRESULT *pResult);

	//��õ�ǰѡ�е��ļ�/Ŀ¼·��
	CString GetCurSel();
	void InsertFilePath( const CString& data );
	void InsertMyDoc( const CString& param1 );
	void InsertDesktop( const CString& param1 );
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnNMRClickFileTree(NMHDR *pNMHDR, LRESULT *pResult);

	//////////////////////////////////////////////////////////////////////////
	//�ڶԻ�������ʾ�Ĳ˵��޷����£�����Ϊ�Ի������û��ʵ�ִ˺������κ�������
	//�δ���ÿ���˵���ĸ�����Ϣ
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnFlush();
	afx_msg void OnDownload();
	afx_msg void OnUpdateFlush(CCmdUI *pCmdUI);
	afx_msg void OnUpdateDownload(CCmdUI *pCmdUI);
	afx_msg void OnUpload();
	afx_msg void OnUpdateUpload(CCmdUI *pCmdUI);
	afx_msg void OnDelete();
	afx_msg void OnUpdateDelete(CCmdUI *pCmdUI);


	//////////////////////////////////////////////////////////////////////////
	//��ͻ��˷���һ����Ϣ�����ߵ��ƶ�Ŀ¼�е������ļ��б�
	BOOL GetAllFiles(const CString& strPath);
	void FlushDir( const CString& strDir );
	void FlushItem( HTREEITEM item );
};
