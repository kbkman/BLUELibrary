#include "StdAfx.h"
#include "XMLOperation.h"

CXMLOperation::CXMLOperation(void)
{
}

CXMLOperation::~CXMLOperation(void)
{
	Close();
}

HRESULT CXMLOperation::Open(LPCTSTR lpstrXMLFilePathName)
{
	CComVariant varFileName;
	VARIANT_BOOL isSuccessful;
	HRESULT hr;

	//¶ÁÈ¡ÅäÖÃÎÄ¼þ
	varFileName = lpstrXMLFilePathName;
	if (FAILED(hr = m_spDomDocument.CoCreateInstance(CLSID_DOMDocument)))
		return hr;
	if (FAILED(hr = m_spDomDocument->put_async(VARIANT_FALSE)))
		return hr;
	if (FAILED(hr = m_spDomDocument->load(varFileName, &isSuccessful)))
		return hr;
	if (isSuccessful == VARIANT_FALSE)
		return hr;

	return S_OK;
}

void CXMLOperation::Close(void)
{
}

HRESULT CXMLOperation::SearchConfigValue(LPTSTR lpstrTag, CComBSTR& bstrModify)
{
	CComPtr<IXMLDOMNodeList> spXMLDomNodeList;
	CComPtr<IXMLDOMNode> spXMLDomNode;
	HRESULT hr;
	long l;

	if (FAILED(hr = m_spDomDocument->getElementsByTagName(CComBSTR(lpstrTag), &spXMLDomNodeList)))
		return hr;

	if (FAILED(hr = spXMLDomNodeList->get_length(&l)))
		return hr;

	if (l <= 0) return S_FALSE;
	
	if (FAILED(hr = spXMLDomNodeList->get_item(0, &spXMLDomNode)))
		return hr;

	if (FAILED(hr = spXMLDomNode->get_text(&bstrModify)))
		return hr;

	return S_OK;
}

HRESULT CXMLOperation::SearchConfigValue(LPTSTR lpstrTag, CString& strModify)
{
	CComBSTR bstr;
	HRESULT hr;

	if (SUCCEEDED(hr = SearchConfigValue(lpstrTag, bstr)))
	{
		strModify = bstr;
	}
	return hr;

}

HRESULT CXMLOperation::SearchConfigValue(LPTSTR lpstrTag, long& l)
{
	CComBSTR bstr;
	CComVariant v;
	HRESULT hr;

	if (FAILED(hr = SearchConfigValue(lpstrTag, bstr)))
		return hr;

	v = bstr;
	if (FAILED(hr = v.ChangeType(VT_I4)))
		return hr;

	l = v.lVal;
	return S_OK;
}