///////////////////////////////////////////////////////////////
//
// FileName	: FileManageDlg.cpp 
// Creator	: 杨松
// Date		: ?2013年4月17日，12:23:32
// Comment	: 远程文件管理对话框类的声明
//
//////////////////////////////////////////////////////////////

#pragma once
#include "afxcmn.h"


class CRemoteControlDoc;
// CDocManageDlg 对话框

class CFileManageDlg : public CDialog
{
	DECLARE_DYNAMIC(CFileManageDlg)

public:
	CFileManageDlg(const CString& ip , USHORT port , const CString& name , CRemoteControlDoc* pDoc , CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CFileManageDlg();

// 对话框数据
	enum { IDD = IDD_DOC_MANAGE_DLG };

private:
	//客户端地址
	CString m_strIP;
	//客户端计算机名
	CString m_strName;
	//客户端消息端口
	USHORT	m_nPort;
	CRemoteControlDoc* m_pDoc;

	//我的文档
	HTREEITEM m_hMyDoc;
	HTREEITEM m_hComputer;
	HTREEITEM m_hDesktop;

private:
	//////////////////////////////////////////////////////////////////////////
	//获得制定文件句柄
	//param
	//	path	路径
	//return 
	//		NULL	不存在指定的路径
	HTREEITEM GetItemHandle(CString path);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CTreeCtrl m_wndFileTree;
	CImageList m_wndImageList;
	afx_msg void OnNMClickFileTree(NMHDR *pNMHDR, LRESULT *pResult);

	//获得当前选中的文件/目录路径
	CString GetCurSel();
	void InsertFilePath( const CString& data );
	void InsertMyDoc( const CString& param1 );
	void InsertDesktop( const CString& param1 );
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnNMRClickFileTree(NMHDR *pNMHDR, LRESULT *pResult);

	//////////////////////////////////////////////////////////////////////////
	//在对话框中显示的菜单无法跟新，是因为对话框基类没有实现此函数，次函数会依
	//次触发每个菜单项的更新消息
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
	//项客户端发送一个消息，或者的制定目录中的所有文件列表
	BOOL GetAllFiles(const CString& strPath);
	void FlushDir( const CString& strDir );
	void FlushItem( HTREEITEM item );
};
