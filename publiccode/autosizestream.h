//////////////////////////////////////////////////////////////////////////
//
//FileName	:	autosizestream.h
//Creator	:	����
//Date		:	2012��12��16��
//Comment	:	�Զ��������ݻ����С��������
//
//////////////////////////////////////////////////////////////////////////

#ifndef _AUTO_SIZE_STREAM_H
#define _AUTO_SIZE_STREAM_H

/************************************************************************/
/*�Զ��������ݻ���Ĵ�С��������                                        */
/************************************************************************/
class AutoSizeStream : public IStream
{
public:
	//////////////////////////////////////////////////////////////////////////
	//����һ��AutoSizesStream����
	//returns
	//		ImageStream*	����NULL����ʧ�ܣ����򷵻�һ��ͼ������������
	//////////////////////////////////////////////////////////////////////////
	static AutoSizeStream* CreateAutoSizeStream();

	//////////////////////////////////////////////////////////////////////////
	//��õ�ǰ�������ݻ����ַ
	//return 
	//		NULL	����������
	//		!=NULL	�����ݻ����ַ
	//////////////////////////////////////////////////////////////////////////
	LPCVOID  GetStreamData();

	//////////////////////////////////////////////////////////////////////////
	//��õ�ǰ������Ч���ݴ�С
	//return 
	//		��ǰ������Ч���ݴ�С
	//////////////////////////////////////////////////////////////////////////
	ULARGE_INTEGER GetValidSize();

	//////////////////////////////////////////////////////////////////////////
	//������Ч���ݣ����ǲ���ɾ������ռ䣬������Ч����ʱ��Ҳ�Ὣ��ǰ��дָ����
	//�������������ǰ��
	void CleanValidData();

	//////////////////////////////////////////////////////////////////////////
	//�����Ǵ�IStream�ӿ�(����IStream�ĸ��ӿ�)�м̳й����ĺ�����ֻ�в���ʵ��
	//////////////////////////////////////////////////////////////////////////
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
	virtual ULONG   STDMETHODCALLTYPE AddRef();
	virtual ULONG   STDMETHODCALLTYPE Release();
	virtual HRESULT STDMETHODCALLTYPE Read(void *pv, ULONG cb, ULONG *pcbRead);
	virtual HRESULT STDMETHODCALLTYPE Write(const void *pv, ULONG cb, ULONG *pcbWritten);
	virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
	virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize);
	virtual HRESULT STDMETHODCALLTYPE CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
	virtual HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags);
	virtual HRESULT STDMETHODCALLTYPE Revert(void);
	virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
	virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
	virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG *pstatstg, DWORD grfStatFlag);
	virtual HRESULT STDMETHODCALLTYPE Clone(IStream **ppstm);

protected:
	//////////////////////////////////////////////////////////////////////////
	//һ��һЩ�ӿ��ǲ���Ҫ�����Ľӿڣ��ڴ����ε�
	//////////////////////////////////////////////////////////////////////////
	AutoSizeStream();
	~AutoSizeStream();
	AutoSizeStream(const AutoSizeStream&);
	AutoSizeStream& operator=(const AutoSizeStream&);

private:
	volatile ULONG	m_uRefCount;	//��������
	LPVOID			m_lpData;		//���ݻ���
	ULARGE_INTEGER	m_uSize;		//��ǰ�����С
	ULARGE_INTEGER	m_uCurrent;		//��ǰ��дָ��
	ULARGE_INTEGER	m_uValidSize;	//��Ч�����ݴ�С
};
#endif
