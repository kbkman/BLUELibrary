#pragma once
#include <BLUEDirectX/Def.h>

template <class T>
class _NoAddRefReleaseOnCDXComPtr : public T
{
private:
	virtual unsigned long __stdcall AddRef(void) = 0;
	virtual unsigned long __stdcall Release(void) = 0;
};

template <class T>
class CDXComPtrBase
{
protected:
	CDXComPtrBase()
	{
		m_p = NULL;
	}
	CDXComPtrBase(int nNull)
	{
		BLUEASSERT(nNull == 0);
		(void)nNull;
		m_p = NULL;
	}
	CDXComPtrBase(T* lp)
	{
		m_p = lp;
		if (m_p != NULL)
			m_p->AddRef();
	}
public:
	~CDXComPtrBase()
	{
		if (m_p)
			m_p->Release();
	}
	operator T*() const
	{
		return m_p;
	}
	T& operator*() const
	{
		BLUEASSERT(m_p!=NULL);
		return *m_p;
	}

	T** operator&()
	{
		BLUEASSERT(m_p==NULL);
		return &m_p;
	}
	_NoAddRefReleaseOnCDXComPtr<T>* operator->() const throw()
	{
		BLUEASSERT(m_p != NULL);
		return (_NoAddRefReleaseOnCDXComPtr<T>*)m_p;
	}

	bool operator!() const throw()
	{
		return (m_p == NULL);
	}

	bool operator<(T* pT) const throw()
	{
		return m_p < pT;
	}

	bool operator!=(T* pT) const
	{
		return !operator==(pT);
	}

	bool operator==(T* pT) const throw()
	{
		return m_p == pT;
	}

	void Release() throw()
	{
		T* pTemp = m_p;
		if (pTemp)
		{
			m_p = NULL;
			pTemp->Release();
		}
	}

	void Attach(T* p2) throw()
	{
		if (m_p)
			m_p->Release();
		m_p = p2;
	}

	T* Detach() throw()
	{
		T* pt = m_p;
		m_p = NULL;
		return pt;
	}

	HRESULT CopyTo(T** ppT) const throw()
	{
		BLUEASSERT(ppT != NULL);
		if (ppT == NULL)
			return E_POINTER;
		*ppT = m_p;
		if (m_p)
			m_p->AddRef();
		return S_OK;
	}

	T* m_p;
};

template <class T>
class CDXComPtr : public CDXComPtrBase<T>
{
public:
	CDXComPtr()
	{
	}

	CDXComPtr(int nNull) : CDXComPtrBase<T>(nNull)
	{
	}

	CDXComPtr(T* lp) : CDXComPtrBase<T>(lp)
	{
	}

	CDXComPtr(const CDXComPtr<T>& lp) : CDXComPtrBase<T>(lp.m_p)
	{
	}
};
