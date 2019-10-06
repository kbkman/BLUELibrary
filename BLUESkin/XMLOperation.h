#pragma once

#include <atlbase.h>
#include <atlstr.h>

#define FAILED_NOSOK(hr)			(hr != S_OK)
class CXMLOperation
{
	CComPtr<IXMLDOMDocument> m_spDomDocument;

public:
	CXMLOperation(void);
	virtual ~CXMLOperation(void);

public:
	void Close(void);
	HRESULT Open(LPCTSTR lpstrXMLFilePathName);
	HRESULT SearchConfigValue(LPTSTR lpstrTag, CComBSTR& bstrModify);
	HRESULT SearchConfigValue(LPTSTR lpstrTag, CString& lpstrModify);
	HRESULT SearchConfigValue(LPTSTR lpstrTag, long& l);
};
