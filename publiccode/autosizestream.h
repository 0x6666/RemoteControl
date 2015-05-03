//////////////////////////////////////////////////////////////////////////
//
//FileName	:	autosizestream.h
//Creator	:	杨松
//Date		:	2012年12月16日
//Comment	:	自动调整数据缓存大小的数据流
//
//////////////////////////////////////////////////////////////////////////

#ifndef _AUTO_SIZE_STREAM_H
#define _AUTO_SIZE_STREAM_H

/************************************************************************/
/*自动调整数据缓存的大小的数据流                                        */
/************************************************************************/
class AutoSizeStream : public IStream
{
public:
	//////////////////////////////////////////////////////////////////////////
	//创建一个AutoSizesStream对象
	//returns
	//		ImageStream*	返回NULL创建失败，否则返回一个图像数据流对象
	//////////////////////////////////////////////////////////////////////////
	static AutoSizeStream* CreateAutoSizeStream();

	//////////////////////////////////////////////////////////////////////////
	//获得当前流的数据缓存地址
	//return 
	//		NULL	流中无数据
	//		!=NULL	流数据缓存地址
	//////////////////////////////////////////////////////////////////////////
	LPCVOID  GetStreamData();

	//////////////////////////////////////////////////////////////////////////
	//获得当前流的有效数据大小
	//return 
	//		当前流中有效数据大小
	//////////////////////////////////////////////////////////////////////////
	ULARGE_INTEGER GetValidSize();

	//////////////////////////////////////////////////////////////////////////
	//清理有效数据，但是不是删除缓存空间，清理有效数据时，也会将当前读写指针移
	//到数据流缓存的前面
	void CleanValidData();

	//////////////////////////////////////////////////////////////////////////
	//以下是从IStream接口(或者IStream的父接口)中继承过来的函数，只有部分实现
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
	//一下一些接口是不需要公开的接口，在此屏蔽掉
	//////////////////////////////////////////////////////////////////////////
	AutoSizeStream();
	~AutoSizeStream();
	AutoSizeStream(const AutoSizeStream&);
	AutoSizeStream& operator=(const AutoSizeStream&);

private:
	volatile ULONG	m_uRefCount;	//引用数量
	LPVOID			m_lpData;		//数据缓存
	ULARGE_INTEGER	m_uSize;		//当前缓存大小
	ULARGE_INTEGER	m_uCurrent;		//当前读写指针
	ULARGE_INTEGER	m_uValidSize;	//有效的数据大小
};
#endif
