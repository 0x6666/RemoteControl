///////////////////////////////////////////////////////////////
//
// FileName	: autosizestream.cpp 
// Creator	: 杨松
// Date		: 2012年12月16日
// Comment	: 自动调整数据缓存大小的数据流
//
//////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "autosizestream.h"

AutoSizeStream::AutoSizeStream()
	: m_uRefCount(1)
{
	m_uCurrent.QuadPart = 0;
	m_lpData = malloc(1);
	m_uSize.QuadPart = 1;
	this->m_uValidSize.QuadPart = 0;
}

AutoSizeStream::~AutoSizeStream()
{
	if ( m_lpData )
		free(m_lpData);
}

AutoSizeStream::AutoSizeStream(const AutoSizeStream&)
{
}

AutoSizeStream& AutoSizeStream::operator=(const AutoSizeStream&)
{
	return (*this);
}

AutoSizeStream* AutoSizeStream::CreateAutoSizeStream()
{
	return new AutoSizeStream();
}

LPCVOID AutoSizeStream::GetStreamData()
{
	if (0 == this->m_uValidSize.QuadPart)
		return NULL;
	else
		return m_lpData;
}

ULARGE_INTEGER AutoSizeStream::GetValidSize()
{
	return m_uValidSize;
}

HRESULT STDMETHODCALLTYPE AutoSizeStream::QueryInterface(REFIID riid, void **ppvObject)
{
	return E_NOINTERFACE;
}
ULONG STDMETHODCALLTYPE AutoSizeStream::AddRef()
{
	return InterlockedIncrement((LONG*)&m_uRefCount);
}
ULONG STDMETHODCALLTYPE AutoSizeStream::Release()
{
	ULONG ret = InterlockedDecrement((volatile long*)&m_uRefCount);
	if (ret == 0)
		delete this;
	return ret;
}
HRESULT STDMETHODCALLTYPE AutoSizeStream::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
	//如果将要读取的数据大小(cb)大于实际剩余的有效数据
	//则调整数据可以读取的数据大小
	ULONG cbr = (ULONG)(m_uValidSize.QuadPart - m_uCurrent.QuadPart);
	if (cbr > cb) 
		cbr = cb;

	//复制需要读取的数据
	memcpy(pv, ((UCHAR*)m_lpData) + m_uCurrent.LowPart, cbr);

	//移动当前读写指针
	m_uCurrent.QuadPart += cbr;

	if (pcbRead)
		*pcbRead = cbr;

	return S_OK;
}
HRESULT STDMETHODCALLTYPE AutoSizeStream::Write(const void *pv, ULONG cb, ULONG *pcbWritten)
{
	/************************************************************************
		在当前读写指针处写入指定的数据(pv)
		写入数据时，如果数据缓存不足会自动增加缓存的大小。
	************************************************************************/

	//写入后的有效数据大小
	ULARGE_INTEGER validSize = m_uCurrent;
	validSize.QuadPart += cb;

	//空间不足？
	if(validSize.QuadPart > this->m_uSize.QuadPart)
	{
		LPVOID pData = realloc(m_lpData , (size_t)validSize.QuadPart);
		if (NULL == pData)//分配内存失败？
			return STG_E_CANTSAVE;
		//分配成功
		m_lpData = pData;
		this->m_uSize = validSize;
	}

	//拷贝数据到流中去
	memcpy(((BYTE*)m_lpData) + this->m_uCurrent.QuadPart , pv , cb);

	//当前读写指针
	this->m_uCurrent.QuadPart += cb;
	this->m_uValidSize = m_uCurrent;
	if (pcbWritten) *pcbWritten = cb;

	return S_OK;
}
HRESULT STDMETHODCALLTYPE AutoSizeStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
	HRESULT ret = S_OK;
	LONGLONG lOffset = 0;
	switch (dwOrigin)
	{
	case STREAM_SEEK_SET:
		lOffset = dlibMove.QuadPart;
		break;
	case STREAM_SEEK_CUR:
		lOffset = dlibMove.QuadPart + m_uCurrent.QuadPart;
		break;
	case STREAM_SEEK_END:
		lOffset = dlibMove.QuadPart + m_uSize.QuadPart;
		break;
	default:
		ret = STG_E_INVALIDFUNCTION;
		break;
	}
	if (ret == S_OK)
	{
		if (lOffset < 0LL)
			lOffset = 0LL;
		if ((ULONGLONG)lOffset > m_uSize.QuadPart)
			lOffset = m_uSize.QuadPart;
		m_uCurrent.QuadPart = lOffset;
		if (plibNewPosition)
			plibNewPosition->QuadPart = lOffset;
	}
	return ret;
}
HRESULT STDMETHODCALLTYPE AutoSizeStream::SetSize(ULARGE_INTEGER libNewSize)
{
	return E_NOTIMPL;
}
HRESULT STDMETHODCALLTYPE AutoSizeStream::CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
	return E_NOTIMPL;
}
HRESULT STDMETHODCALLTYPE AutoSizeStream::Commit(DWORD grfCommitFlags)
{
	return E_NOTIMPL;
}
HRESULT STDMETHODCALLTYPE AutoSizeStream::Revert(void)
{
	return E_NOTIMPL;
}
HRESULT STDMETHODCALLTYPE AutoSizeStream::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
	return E_NOTIMPL;
}
HRESULT STDMETHODCALLTYPE AutoSizeStream::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
	return E_NOTIMPL;
}
HRESULT STDMETHODCALLTYPE AutoSizeStream::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
	HRESULT ret = S_OK;
	if ((grfStatFlag != STATFLAG_NONAME) && (grfStatFlag != STATFLAG_DEFAULT))
	{
		ret = STG_E_INVALIDFLAG;
	}
	else if (pstatstg == NULL)
	{
		ret = STG_E_INVALIDPOINTER;
	}
	else
	{
		memset(pstatstg, 0, sizeof(STATSTG));
		pstatstg->type = STGTY_STREAM;
		pstatstg->cbSize = m_uSize;
	}
	return ret;
}
HRESULT STDMETHODCALLTYPE AutoSizeStream::Clone(IStream **ppstm)
{
	return E_NOTIMPL;
}

void AutoSizeStream::CleanValidData()
{
	m_uValidSize.QuadPart = 0;
	m_uCurrent.QuadPart = 0;
}
