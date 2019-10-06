#include "StdAfx.h"
#include <atlbase.h>
#include <BLUESkin.h>
#include "WndSkin.h"

bool BLUESkin_Initialize(void)
{
	return SUCCEEDED(::CoInitialize(NULL));
}

void BLUESkin_Uninitialize(void)
{
	::CoUninitialize();
}

bool BLUESkin_BindNCSkin(HWND hWnd, LPCSTR lpstrDirectory)
{
	DWORD dwWStrLen = ::MultiByteToWideChar(CP_ACP, 0, lpstrDirectory, -1, NULL, 0);
	if (dwWStrLen > 0)
	{
		wchar_t* pWStr;
		pWStr = new wchar_t[dwWStrLen];
		//转换
		::MultiByteToWideChar(CP_ACP, 0, lpstrDirectory, -1, pWStr, dwWStrLen);
		bool b = BLUESkin_BindNCSkin(hWnd, pWStr);
		delete[] pWStr;
		return b;
	}
	return false;
}

bool BLUESkin_BindNCSkin(HWND hWnd, LPCWSTR lpstrDirectory)
{
	return CWndNCSkinProc::Bind(hWnd, lpstrDirectory);
}

bool BLUESkin_UnBindNCSkin(HWND hWnd)
{
	return CWndNCSkinProc::UnBind(hWnd);
}

HRGN BitmapToRegion(HBITMAP hBmp, COLORREF cTransparentColor, COLORREF cTolerance)
{
	HRGN hRgn = NULL;

	if (hBmp)
	{
		// Create a memory DC inside which we will scan the bitmap content
		HDC hMemDC = CreateCompatibleDC(NULL);
		if (hMemDC)
		{
			// Get bitmap size
			BITMAP bm;
			GetObject(hBmp, sizeof(bm), &bm);

			// Create a 32 bits depth bitmap and select it into the memory DC 
			BITMAPINFOHEADER RGB32BITSBITMAPINFO = {	
				sizeof(BITMAPINFOHEADER),	// biSize 
				bm.bmWidth,					// biWidth; 
				bm.bmHeight,				// biHeight; 
				1,							// biPlanes; 
				32,							// biBitCount 
				BI_RGB,						// biCompression; 
				0,							// biSizeImage; 
				0,							// biXPelsPerMeter; 
				0,							// biYPelsPerMeter; 
				0,							// biClrUsed; 
				0							// biClrImportant; 
			};
			VOID * pbits32; 
			HBITMAP hbm32 = CreateDIBSection(hMemDC, (BITMAPINFO *)&RGB32BITSBITMAPINFO, DIB_RGB_COLORS, &pbits32, NULL, 0);
			if (hbm32)
			{
				HBITMAP holdBmp = (HBITMAP)SelectObject(hMemDC, hbm32);

				// Create a DC just to copy the bitmap into the memory DC
				HDC hDC = CreateCompatibleDC(hMemDC);
				if (hDC)
				{
					// Get how many bytes per row we have for the bitmap bits (rounded up to 32 bits)
					BITMAP bm32;
					GetObject(hbm32, sizeof(bm32), &bm32);
					while (bm32.bmWidthBytes % 4)
						bm32.bmWidthBytes++;

					// Copy the bitmap into the memory DC
					HBITMAP holdBmp = (HBITMAP)SelectObject(hDC, hBmp);
					BitBlt(hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, hDC, 0, 0, SRCCOPY);

					// For better performances, we will use the ExtCreateRegion() function to create the
					// region. This function take a RGNDATA structure on entry. We will add rectangles by
					// amount of ALLOC_UNIT number in this structure.
#define ALLOC_UNIT	100
					DWORD maxRects = ALLOC_UNIT;
					HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects));
					RGNDATA *pData = (RGNDATA *)GlobalLock(hData);
					pData->rdh.dwSize = sizeof(RGNDATAHEADER);
					pData->rdh.iType = RDH_RECTANGLES;
					pData->rdh.nCount = pData->rdh.nRgnSize = 0;
					SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);

					// Keep on hand highest and lowest values for the "transparent" pixels
					BYTE lr = GetRValue(cTransparentColor);
					BYTE lg = GetGValue(cTransparentColor);
					BYTE lb = GetBValue(cTransparentColor);
					BYTE hr = min(0xff, lr + GetRValue(cTolerance));
					BYTE hg = min(0xff, lg + GetGValue(cTolerance));
					BYTE hb = min(0xff, lb + GetBValue(cTolerance));

					// Scan each bitmap row from bottom to top (the bitmap is inverted vertically)
					BYTE *p32 = (BYTE *)bm32.bmBits + (bm32.bmHeight - 1) * bm32.bmWidthBytes;
					for (int y = 0; y < bm.bmHeight; y++)
					{
						// Scan each bitmap pixel from left to right
						for (int x = 0; x < bm.bmWidth; x++)
						{
							// Search for a continuous range of "non transparent pixels"
							int x0 = x;
							LONG *p = (LONG *)p32 + x;
							while (x < bm.bmWidth)
							{
								BYTE b = GetRValue(*p);
								if (b >= lr && b <= hr)
								{
									b = GetGValue(*p);
									if (b >= lg && b <= hg)
									{
										b = GetBValue(*p);
										if (b >= lb && b <= hb)
											// This pixel is "transparent"
											break;
									}
								}
								p++;
								x++;
							}

							if (x > x0)
							{
								// Add the pixels (x0, y) to (x, y+1) as a new rectangle in the region
								if (pData->rdh.nCount >= maxRects)
								{
									GlobalUnlock(hData);
									maxRects += ALLOC_UNIT;
									hData = GlobalReAlloc(hData, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), GMEM_MOVEABLE);
									pData = (RGNDATA *)GlobalLock(hData);
								}
								RECT *pr = (RECT *)&pData->Buffer;
								SetRect(&pr[pData->rdh.nCount], x0, y, x, y+1);
								if (x0 < pData->rdh.rcBound.left)
									pData->rdh.rcBound.left = x0;
								if (y < pData->rdh.rcBound.top)
									pData->rdh.rcBound.top = y;
								if (x > pData->rdh.rcBound.right)
									pData->rdh.rcBound.right = x;
								if (y+1 > pData->rdh.rcBound.bottom)
									pData->rdh.rcBound.bottom = y+1;
								pData->rdh.nCount++;

								// On Windows98, ExtCreateRegion() may fail if the number of rectangles is too
								// large (ie: > 4000). Therefore, we have to create the region by multiple steps.
								if (pData->rdh.nCount == 2000)
								{
									HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
									if (hRgn)
									{
										CombineRgn(hRgn, hRgn, h, RGN_OR);
										DeleteObject(h);
									}
									else
										hRgn = h;
									pData->rdh.nCount = 0;
									SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);
								}
							}
						}

						// Go to next row (remember, the bitmap is inverted vertically)
						p32 -= bm32.bmWidthBytes;
					}

					// Create or extend the region with the remaining rectangles
					HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
					if (hRgn)
					{
						CombineRgn(hRgn, hRgn, h, RGN_OR);
						DeleteObject(h);
					}
					else
						hRgn = h;

					// Clean up
					SelectObject(hDC, holdBmp);
					DeleteDC(hDC);
				}

				DeleteObject(SelectObject(hMemDC, holdBmp));
			}

			DeleteDC(hMemDC);
		}	
	}

	return hRgn;
}

CNCBmp::CNCBmp(void)
{
	m_hBmp = NULL;
}

CNCBmp::~CNCBmp(void)
{
	Free();
}

void CNCBmp::Free()
{
	if (m_hBmp)
	{
		::DeleteObject(m_hBmp);
		m_hBmp = NULL;
	}
}

bool CNCBmp::LoadFile(LPCTSTR strFileName)
{
	BLUEASSERT(m_hBmp == NULL);
	BLUEASSERT(strFileName != NULL);

	m_hBmp = (HBITMAP)::LoadImage(NULL, strFileName, IMAGE_BITMAP, 0, 0,
		LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);

	if (m_hBmp == NULL)
		return false;

	if (::GetObject(m_hBmp, sizeof(BITMAP), static_cast<BITMAP*>(this)) == 0)
	{
		Free();
		return false;
	}

	return true;
}

bool CNCBmp::BitBltToHDC(HDC hdcDest, HDC hdcSrc, int xDest, int yDest, DWORD rop)
{
	BLUEASSERT(m_hBmp != NULL);
	HBITMAP hOldBmp;
	bool b;

	if ((hOldBmp = (HBITMAP)::SelectObject(hdcSrc, m_hBmp)) == NULL)
		return false;

	b = (::BitBlt(hdcDest, xDest, yDest, bmWidth, bmHeight, hdcSrc, 0, 0, rop) == TRUE);

	if (::SelectObject(hdcSrc, hOldBmp) == NULL)
		return false;

	return b;
}

bool CNCBmp::StretchBltToHDC(HDC hdcDest, HDC hdcSrc,int xDest, int yDest, int wDest, int hDest, DWORD rop)
{
	BLUEASSERT(m_hBmp != NULL);
	HBITMAP hOldBmp;
	bool b;

	if ((hOldBmp = (HBITMAP)::SelectObject(hdcSrc, m_hBmp)) == NULL)
		return false;

	b = (::StretchBlt(hdcDest, xDest, yDest, wDest, hDest, hdcSrc, 0, 0, bmWidth, bmHeight, rop) == TRUE);

	if (::SelectObject(hdcSrc, hOldBmp) == NULL)
		return false;

	return b;
}



CNCBmpButton::CNCBmpButton(void) :
m_lState(NCBTN_STATE_DEFAULT),
m_iPosTopToFrame(5),
m_iPosRightToFrame(5)
{
}

CNCBmpButton::~CNCBmpButton(void)
{
}

void CNCBmpButton::Free(void)
{
	PutState(NCBTN_STATE_DEFAULT);

	m_iPosTopToFrame = 5;
	m_iPosRightToFrame = 5;

	m_bmpMouseMove.Free();
	m_bmpMouseDown.Free();
}

bool CNCBmpButton::LoadFile(LPCTSTR strFileName)
{
	return LoadFile(strFileName, NULL, NULL);
}

bool CNCBmpButton::LoadFile(LPCTSTR strFileName, LPCTSTR strFileNameMouseMove, LPCTSTR strFileNameMouseDown)
{
	m_bmpMouseMove.LoadFile(strFileNameMouseMove);
	m_bmpMouseDown.LoadFile(strFileNameMouseDown);

	if (!CNCBmp::LoadFile(strFileName))
	{
		m_bmpMouseDown.Free();
		m_bmpMouseMove.Free();
		return false;
	}

	return true;
}

bool CNCBmpButton::BitBltToHDC(HDC hdcDest, HDC hdcSrc, int xDest, int yDest, DWORD rop)
{
	switch (m_lState)
	{
	case NCBTN_STATE_MOVE:
		if (m_bmpMouseMove.GetHandle() == NULL) return false;
		return m_bmpMouseMove.BitBltToHDC(hdcDest, hdcSrc, xDest, yDest);

	case NCBTN_STATE_DOWN:
		if (m_bmpMouseDown.GetHandle() == NULL) return false;
		return m_bmpMouseDown.BitBltToHDC(hdcDest, hdcSrc, xDest, yDest);

	default:
		return CNCBmp::BitBltToHDC(hdcDest, hdcSrc, xDest, yDest);
	}
}

bool CNCBmpButton::StretchBltToHDC(HDC hdcDest, HDC hdcSrc,int xDest, int yDest, int wDest, int hDest, DWORD rop)
{
	switch (m_lState)
	{
	case NCBTN_STATE_MOVE:
		if (m_bmpMouseMove.GetHandle() == NULL) return false;
		return m_bmpMouseMove.StretchBltToHDC(hdcDest, hdcSrc, xDest, yDest, wDest, hDest, rop);

	case NCBTN_STATE_DOWN:
		if (m_bmpMouseDown.GetHandle() == NULL) return false;
		return m_bmpMouseDown.StretchBltToHDC(hdcDest, hdcSrc, xDest, yDest, wDest, hDest, rop);

	default:
		return CNCBmp::StretchBltToHDC(hdcDest, hdcSrc, xDest, yDest, wDest, hDest, rop);
	}
}

void CNCBmpButton::CalcPos(const RECT& rcWndPos/*屏幕坐标*/)
{
	BLUEASSERT(GetHandle() != NULL);

	m_rcPos.top = rcWndPos.top + m_iPosTopToFrame;
	m_rcPos.left = rcWndPos.right - m_iPosRightToFrame - bmWidth;
	m_rcPos.right = rcWndPos.right - m_iPosRightToFrame;
	m_rcPos.bottom = rcWndPos.top + m_iPosTopToFrame + bmHeight;
}

void CNCBmpButton::GetPos(RECT& rc)
{
	rc.top = m_rcPos.top;
	rc.left = m_rcPos.left;
	rc.right = m_rcPos.right;
	rc.bottom = m_rcPos.bottom;
}

void CNCBmpButton::GetPos(HRGN& hRgn)
{
	RECT rc;
	GetPos(rc);
	hRgn = ::CreateRectRgnIndirect(&rc);
}

bool CNCBmpButton::IsPtInPos(const POINT& pt)
{
	return (::PtInRect(&m_rcPos, pt) == TRUE);
}

bool CNCBmpButton::IsPtInPos(const LONG x, const LONG y)
{
	POINT pt = {x, y};
	return IsPtInPos(pt);
}

long CNCBmpButton::PutState(long lState)
{
	switch (lState)
	{
	case NCBTN_STATE_MOVE:
	case NCBTN_STATE_DOWN:
		m_lState = lState;
		break;

	default:
		m_lState = NCBTN_STATE_DEFAULT;
	}
	return m_lState;
}

long CNCBmpButton::GetState(void)
{
	return m_lState;
}

////////////////////CWndNCSkinProc///////////////////////////
CHighSpeedMap CWndNCSkinProc::s_hsm;//需要在cpp文件中定义一下，不然会出错
bool CWndNCSkinProc::Bind(HWND hWnd, LPCTSTR lpstrDirectory)
{
	BLUEASSERT(hWnd);

	CWndNCSkinProc* pWndNCSkinProc;
	pWndNCSkinProc = new CWndNCSkinProc();
	pWndNCSkinProc->m_hWnd = hWnd;
	pWndNCSkinProc->BindSkin(lpstrDirectory);

	if (s_hsm.Add(hWnd, pWndNCSkinProc))
	{
		WNDPROC fnOldWndProc;
		fnOldWndProc = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (LONG)stWndProc);
		if (fnOldWndProc == NULL)
			return UnBind(hWnd);

		pWndNCSkinProc->m_fnOldWndProc = fnOldWndProc;
		return true;
	}
	else
	{
		delete pWndNCSkinProc;
		return false;
	}
}

bool CWndNCSkinProc::UnBind(HWND hWnd)
{
	BLUEASSERT(hWnd);

	CWndNCSkinProc* pWndNCSkinProc;
	pWndNCSkinProc = (CWndNCSkinProc*)s_hsm.Remove(hWnd);
	if (pWndNCSkinProc)
	{
		::SetWindowLong(hWnd, GWL_WNDPROC, (LONG)pWndNCSkinProc->m_fnOldWndProc);
		delete pWndNCSkinProc;
		return true;
	}

	return false;
}

LRESULT CWndNCSkinProc::stWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BLUEASSERT(hWnd);

	CWndNCSkinProc* pWndNCSkinProc;
	pWndNCSkinProc = (CWndNCSkinProc*)s_hsm.Search(hWnd);

	if (pWndNCSkinProc)
		return pWndNCSkinProc->WndPorc(uMsg, wParam, lParam);

	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CWndNCSkinProc::WndPorc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult;//返回结果
	BOOL bHandled;//是否不再调用m_fnOldWndProc;

	lResult = 0;
	bHandled = TRUE;

	switch (uMsg)
	{
	case WM_NCPAINT:
		lResult = OnNcPaint(wParam, lParam, bHandled);
		break;
	case WM_NCCALCSIZE:
		lResult = OnNcCalcSize(wParam, lParam, bHandled);
		break;
	case WM_NCHITTEST:
		lResult = OnNcHitTest(wParam, lParam, bHandled);
		break;
	case WM_NCACTIVATE:
		lResult = OnNcActivate(wParam, lParam, bHandled);
		break;
	case WM_NCMOUSEMOVE:
		lResult = OnNcMouseMove(wParam, lParam, bHandled);
		break;
	case WM_NCLBUTTONDOWN:
		lResult = OnNcLButtonDown(wParam, lParam, bHandled);
		break;
	case WM_NCRBUTTONDOWN:
		lResult = OnNcRButtonDown(wParam, lParam, bHandled);
		break;
	case WM_MOUSEMOVE:
		lResult = OnMouseMove(wParam, lParam, bHandled);
		break;
	case WM_LBUTTONDOWN:
		lResult = OnLButtonDown(wParam, lParam, bHandled);
		break;
	case WM_LBUTTONUP:
		lResult = OnLButtonUp(wParam, lParam, bHandled);
		break;
	case WM_RBUTTONUP:
		lResult = OnRButtonUp(wParam, lParam, bHandled);
		break;
	case WM_SETTEXT:
		lResult = OnSetText(wParam, lParam, bHandled);
		break;
	case WM_GETTEXT:
		lResult = OnGetText(wParam, lParam, bHandled);
		break;
	case WM_GETTEXTLENGTH:
		lResult = OnGetTextLength(wParam, lParam, bHandled);
		break;
	case WM_SETCURSOR:
		lResult = OnSetCursor(wParam, lParam, bHandled);
		break;
	case WM_SETICON:
		lResult = OnSetIcon(wParam, lParam, bHandled);
		break;
	case WM_GETICON:
		lResult = OnGetIcon(wParam, lParam, bHandled);
		break;
	case WM_STYLECHANGED:
		lResult = OnStyleChanged(wParam, lParam, bHandled);
		break;
	case WM_SETTINGCHANGE:
		lResult = OnSettingChange(wParam, lParam, bHandled);
		break;
	case WM_SYSCOMMAND:
		lResult = OnSysCommand(wParam, lParam, bHandled);
		break;
	case WM_NCSKIN_UPDATED:
		lResult = OnNcSkinUpdated(wParam, lParam, bHandled);
		break;
	default:
		bHandled = FALSE;
		break;
	}

	if (bHandled == FALSE && m_fnOldWndProc)//未处理并且有旧窗口函数，则调用
		return m_fnOldWndProc(m_hWnd, uMsg, wParam, lParam);

	return lResult;
}

CWndNCSkinProc::CWndNCSkinProc(void) :
m_hWnd(NULL),
m_fnOldWndProc(NULL)
{
	Init();
}


CWndNCSkinProc::~CWndNCSkinProc(void)
{
	Free();
}


void CWndNCSkinProc::Free(void)
{
	m_bmpTopBack.Free();
	m_bmpTopLeft.Free();
	m_bmpTopRight.Free();
	m_bmpLeftBack.Free();
	m_bmpLeftTop.Free();
	m_bmpLeftBottom.Free();
	m_bmpRightBack.Free();
	m_bmpRightTop.Free();
	m_bmpRightBottom.Free();
	m_bmpBottomBack.Free();
	m_bmpBottomLeft.Free();
	m_bmpBottomRight.Free();

	m_ncbbCloseBtn.Free();
	m_ncbbRecBtn.Free();
	m_ncbbMaxBtn.Free();
	m_ncbbMinBtn.Free();

	m_bNeedClrTransparent = false;
	m_clrTransparent = RGB(0, 0, 0);
	m_clrTitleText = RGB(0, 0, 0);
	m_iTitleTextPosTopToFrame = 5;
	m_iTitleTextPosLeftToFrame = 10;
	m_iTitleTextPosRightToFrame = 100;
	m_iTitleTextHeight = 15;
	m_iTitleTextWeight = FW_NORMAL;
	if (m_hTitleTextFont) {::DeleteObject(m_hTitleTextFont);m_hTitleTextFont = NULL;}

	m_bDrawTitleIcon = false;
	m_iTitleIconTopToFrame = 5;
	m_iTitleIconLeftToFrame = 5;
	m_iTitleIconWidth = 16;
	m_iTitleIconHeight = 16;
	if (m_hTitleIcon) {::DeleteObject(m_hTitleIcon);m_hTitleIcon = NULL;}

	m_lVer = -1;

	m_bNeedSetWndRgn = true;
	m_pCaptureBtn = NULL;
}


void CWndNCSkinProc::Init(void)
{
	m_hTitleTextFont = NULL;
	m_hTitleIcon = NULL;

	Free();
}

bool CWndNCSkinProc::UseSkin(LPCTSTR lpstrFoldName)
{
	Free();//先清理一下所有资源

	if (lpstrFoldName)
	{
		CString strFoldName(lpstrFoldName);
		CXMLOperation xmlOp;
		HRESULT hr;
		long l;

		//文件目录名称最后没有"\"符
		strFoldName += _T("\\");

		//读取配置文件
		if (FAILED_NOSOK(hr = xmlOp.Open(strFoldName + _T("config.xml"))))
			return false;
		if (FAILED_NOSOK(hr = xmlOp.SearchConfigValue(_T("ncNeedClrTransparent"), l)))
			return false;
		m_bNeedClrTransparent = (l == 1);
		if (FAILED_NOSOK(hr = xmlOp.SearchConfigValue(_T("ncClrTransparent"), l)))
			return false;
		m_clrTransparent = (COLORREF)l;
		if (FAILED_NOSOK(hr = xmlOp.SearchConfigValue(_T("ncTitleTextColor"), l)))
			return false;
		m_clrTitleText = (COLORREF)l;
		if (FAILED_NOSOK(hr = xmlOp.SearchConfigValue(_T("ncTitleTextTopToFrame"), l)))
			return false;
		m_iTitleTextPosTopToFrame = l;
		if (FAILED_NOSOK(hr = xmlOp.SearchConfigValue(_T("ncTitleTextLeftToFrame"), l)))
			return false;
		m_iTitleTextPosLeftToFrame = l;
		if (FAILED_NOSOK(hr = xmlOp.SearchConfigValue(_T("ncTitleTextRightToFrame"), l)))
			return false;
		m_iTitleTextPosRightToFrame = l;
		if (FAILED_NOSOK(hr = xmlOp.SearchConfigValue(_T("ncTitleTextHeight"), l)))
			return false;
		m_iTitleTextHeight = l;
		if (FAILED_NOSOK(hr = xmlOp.SearchConfigValue(_T("ncTitleTextWeight"), l)))
			return false;
		m_iTitleTextWeight = l;
		if (FAILED_NOSOK(hr = xmlOp.SearchConfigValue(_T("ncDrawTitleIcon"), l)))
			return false;
		m_bDrawTitleIcon = (l == 1);
		if (FAILED_NOSOK(hr = xmlOp.SearchConfigValue(_T("ncTitleIconTopToFrame"), l)))
			return false;
		m_iTitleIconTopToFrame = l;
		if (FAILED_NOSOK(hr = xmlOp.SearchConfigValue(_T("ncTitleIconLeftToFrame"), l)))
			return false;
		m_iTitleIconLeftToFrame = l;
		if (FAILED_NOSOK(hr = xmlOp.SearchConfigValue(_T("ncTitleIconWidth"), l)))
			return false;
		m_iTitleIconWidth = l;
		if (FAILED_NOSOK(hr = xmlOp.SearchConfigValue(_T("ncTitleIconHeight"), l)))
			return false;
		m_iTitleIconHeight = l;
		if (FAILED_NOSOK(hr = xmlOp.SearchConfigValue(_T("ncCloseBtnTopToFrame"), l)))
			return false;
		m_ncbbCloseBtn.m_iPosTopToFrame = (int)l;
		if (FAILED_NOSOK(hr = xmlOp.SearchConfigValue(_T("ncCloseBtnRightToFrame"), l)))
			return false;
		m_ncbbCloseBtn.m_iPosRightToFrame = (int)l;
		if (FAILED_NOSOK(hr = xmlOp.SearchConfigValue(_T("ncMaxBtnTopToFrame"), l)))
			return false;
		m_ncbbMaxBtn.m_iPosTopToFrame = (int)l;
		m_ncbbRecBtn.m_iPosTopToFrame = (int)l;
		if (FAILED_NOSOK(hr = xmlOp.SearchConfigValue(_T("ncMaxBtnRightToFrame"), l)))
			return false;
		m_ncbbMaxBtn.m_iPosRightToFrame = (int)l;
		m_ncbbRecBtn.m_iPosRightToFrame = (int)l;
		if (FAILED_NOSOK(hr = xmlOp.SearchConfigValue(_T("ncMinBtnTopToFrame"), l)))
			return false;
		m_ncbbMinBtn.m_iPosTopToFrame = (int)l;
		if (FAILED_NOSOK(hr = xmlOp.SearchConfigValue(_T("ncMinBtnRightToFrame"), l)))
			return false;
		m_ncbbMinBtn.m_iPosRightToFrame = (int)l;
		if (FAILED_NOSOK(hr = xmlOp.SearchConfigValue(_T("version"), l)))
			return false;
		m_lVer = l;

		//nc框架
		if (!m_bmpTopBack.LoadFile(strFoldName + _T("ncTopBack.bmp")))
			return false;
		if (!m_bmpTopLeft.LoadFile(strFoldName + _T("ncTopLeft.bmp")))
			return false;
		if (!m_bmpTopRight.LoadFile(strFoldName + _T("ncTopRight.bmp")))
			return false;
		if (!m_bmpLeftBack.LoadFile(strFoldName + _T("ncLeftBack.bmp")))
			return false;
		if (!m_bmpLeftTop.LoadFile(strFoldName + _T("ncLeftTop.bmp")))
			return false;
		if (!m_bmpLeftBottom.LoadFile(strFoldName + _T("ncLeftBottom.bmp")))
			return false;
		if (!m_bmpRightBack.LoadFile(strFoldName + _T("ncRightBack.bmp")))
			return false;
		if (!m_bmpRightTop.LoadFile(strFoldName + _T("ncRightTop.bmp")))
			return false;
		if (!m_bmpRightBottom.LoadFile(strFoldName + _T("ncRightBottom.bmp")))
			return false;
		if (!m_bmpBottomBack.LoadFile(strFoldName + _T("ncBottomBack.bmp")))
			return false;
		if (!m_bmpBottomLeft.LoadFile(strFoldName + _T("ncBottomLeft.bmp")))
			return false;
		if (!m_bmpBottomRight.LoadFile(strFoldName + _T("ncBottomRight.bmp")))
			return false;

		//nc各按钮
		//关闭按钮
		if (!m_ncbbCloseBtn.LoadFile(
			strFoldName + _T("ncCloseBtn.bmp"),
			strFoldName + _T("ncCloseBtnMove.bmp"),
			strFoldName + _T("ncCloseBtnDown.bmp")))
			return false;
		if (!m_ncbbRecBtn.LoadFile(
			strFoldName + _T("ncRecBtn.bmp"),
			strFoldName + _T("ncRecBtnMove.bmp"),
			strFoldName + _T("ncRecBtnDown.bmp")))
			return false;
		if (!m_ncbbMaxBtn.LoadFile(
			strFoldName + _T("ncMaxBtn.bmp"),
			strFoldName + _T("ncMaxBtnMove.bmp"),
			strFoldName + _T("ncMaxBtnDown.bmp")))
			return false;
		if (!m_ncbbMinBtn.LoadFile(
			strFoldName + _T("ncMinBtn.bmp"),
			strFoldName + _T("ncMinBtnMove.bmp"),
			strFoldName + _T("ncMinBtnDown.bmp")))
			return false;

		//创建字体
		{
			BLUEASSERT(m_hTitleTextFont == NULL);
			LOGFONT lf;
			lf.lfHeight = m_iTitleTextHeight;
			lf.lfWidth = 0;
			lf.lfEscapement = 0;
			lf.lfOrientation = 0;
			lf.lfWeight = m_iTitleTextWeight;
			lf.lfItalic = FALSE;;
			lf.lfUnderline = FALSE;
			lf.lfStrikeOut = FALSE;
			lf.lfCharSet = DEFAULT_CHARSET;
			lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
			lf.lfClipPrecision = DEFAULT_QUALITY;
			lf.lfQuality = DEFAULT_PITCH | FF_DONTCARE;
			lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
			::_tcscpy_s(lf.lfFaceName, _T("Tahoma"));
			m_hTitleTextFont = ::CreateFontIndirect(&lf);
		}

		//加载图标
		BLUEASSERT(m_hTitleIcon == NULL);
		m_hTitleIcon = (HICON)::LoadImage(NULL, strFoldName + _T("ncTitle.ico"), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);

		//最后处理
		if (::IsWindow(m_hWnd))
		{
			if (!m_bNeedClrTransparent)
				::SetWindowRgn(m_hWnd, NULL, TRUE);//设置成不使用SetWindowRgn
			return ReDrawFrame();
		}
	}
	else
	{
		//使用windows默认
		if (::IsWindow(m_hWnd))
		{
			::SetWindowRgn(m_hWnd, NULL, TRUE);//设置成不使用SetWindowRgn
			return ReDrawFrame();
		}
	}

	return true;
}


bool CWndNCSkinProc::UseDefaultSkin(void)
{
	return UseSkin();
}


bool CWndNCSkinProc::IsUseDefaultSkin(void)
{
	return (m_bmpTopBack.GetHandle() == NULL);
}


bool CWndNCSkinProc::BindSkin(LPCTSTR lpstrFoldName)
{
	if (!UseSkin(lpstrFoldName))
	{
		UseDefaultSkin();
		return false;
	}
	return true;
}


void CWndNCSkinProc::DrawNCBmpBtn(CNCBmpButton* pNcbb)
{
	BLUEASSERT(pNcbb);

	HRGN hRgn;
	pNcbb->GetPos(hRgn);
	if (hRgn)
	{
		HDC hdc, hdcMem;
		RECT rcWnd;

		hdc = ::GetDCEx(m_hWnd, hRgn, DCX_WINDOW | DCX_INTERSECTRGN | 0x10000);
		hdcMem = ::CreateCompatibleDC(hdc);
		::GetWindowRect(m_hWnd, &rcWnd);

		pNcbb->BitBltToHDC(hdc, hdcMem,
			rcWnd.right - rcWnd.left - pNcbb->m_iPosRightToFrame - pNcbb->bmWidth,
			pNcbb->m_iPosTopToFrame);

		::DeleteDC(hdcMem);
		::ReleaseDC(m_hWnd, hdc);
		::DeleteObject(hRgn);
	}
}


void CWndNCSkinProc::DrawNCBmpBtnByState(CNCBmpButton* pNcbb, long lState)
{
	BLUEASSERT(pNcbb);

	if (pNcbb->GetState() != lState)
	{
		pNcbb->PutState(lState);
		DrawNCBmpBtn(pNcbb);
	}
}


void CWndNCSkinProc::SetCaptureBtn(CNCBmpButton* p)
{
	BLUEASSERT(p);

	if (::GetCapture() != m_hWnd) {
		::SetCapture(m_hWnd);
	}
	m_pCaptureBtn = p;
}


void CWndNCSkinProc::ReleaseCaptureBtn()
{
	::ReleaseCapture();
	m_pCaptureBtn = NULL;
}


LRESULT CWndNCSkinProc::HitTest(POINT pt)
{
	BLUEASSERT(!IsUseDefaultSkin());

	RECT rcWnd;
	::GetWindowRect(m_hWnd, &rcWnd);//得到窗口矩形坐标

	//先计算各按钮所在位置
	m_ncbbCloseBtn.CalcPos(rcWnd);
	if (m_ncbbCloseBtn.IsPtInPos(pt))
		return HTCLOSE;
	if (IsHasMaxBtn())
	{
		if (::IsZoomed(m_hWnd)) {
			m_ncbbRecBtn.CalcPos(rcWnd);
			if (m_ncbbRecBtn.IsPtInPos(pt))
				return HTMAXBUTTON;
		} else {
			m_ncbbMaxBtn.CalcPos(rcWnd);
			if (m_ncbbMaxBtn.IsPtInPos(pt))
				return HTMAXBUTTON;
		}
	}
	if (IsHasMinBtn())
	{
		m_ncbbMinBtn.CalcPos(rcWnd);
		if (m_ncbbMinBtn.IsPtInPos(pt))
			return HTMINBUTTON;
	}

	return HTNOWHERE;
}


LRESULT CWndNCSkinProc::OnNcPaint(WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (::IsIconic(m_hWnd)){
		return 0;

	} else if (IsUseDefaultSkin()) {//如果使用默认界面
		bHandled = FALSE;

	} else {
		RECT rcWnd, rcClient;
		int iWndWidth, iWndHeight;
		HRGN hRgnWnd, hRgnClient, hRgnOffsetWndClient, hRgnTemp, hRgn;
		HDC hdc, hdcMemSrc, hdcMemTemp;//dc:目标，源，临时
		HBITMAP hBmpMemSrc;
		HBITMAP hOldBmpMemSrc;

		::GetWindowRect(m_hWnd, &rcWnd);
		::GetClientRect(m_hWnd, &rcClient);
		::ClientToScreen(m_hWnd, (LPPOINT)&rcClient);
		::ClientToScreen(m_hWnd, ((LPPOINT)&rcClient) + 1);

		iWndWidth = rcWnd.right - rcWnd.left;
		iWndHeight = rcWnd.bottom - rcWnd.top;

		hRgnWnd = ::CreateRectRgnIndirect(&rcWnd);
		hRgnClient = ::CreateRectRgnIndirect(&rcClient);
		hRgnOffsetWndClient = CreateTempRgn();
		hRgnTemp = CreateTempRgn();

		//hRgnOffsetWndClient是指整个窗口与客户区之间的域名差的区域，即nc的所有地方
		::CombineRgn(hRgnOffsetWndClient, hRgnWnd, hRgnClient, RGN_XOR);
		if (wParam == 1) {
			hRgn = hRgnOffsetWndClient;//wParam=1表示nc所有地方要更新
		} else {
			::CombineRgn(hRgnTemp, hRgnOffsetWndClient, (HRGN)wParam, RGN_AND);
			hRgn = hRgnTemp;//wParam != 1表示只需要更新部分地方，但不能有客户区域的地方
		}

		//这里必须加一个 | 0x10000，MSN文档是有误的（经查实）
		hdc = ::GetDCEx(m_hWnd, hRgn, DCX_WINDOW | DCX_INTERSECTRGN | 0x10000);
		hdcMemSrc = ::CreateCompatibleDC(hdc);
		hdcMemTemp = ::CreateCompatibleDC(hdc);
		hBmpMemSrc = ::CreateCompatibleBitmap(hdc, iWndWidth, iWndHeight);
		hOldBmpMemSrc = (HBITMAP)::SelectObject(hdcMemSrc, hBmpMemSrc);

		////绘制各边框
		////top
		m_bmpTopBack.StretchBltToHDC(hdcMemSrc, hdcMemTemp, 0, 0, iWndWidth, m_bmpTopBack.bmHeight);
		m_bmpTopLeft.BitBltToHDC(hdcMemSrc, hdcMemTemp);
		m_bmpTopRight.BitBltToHDC(hdcMemSrc, hdcMemTemp, iWndWidth - m_bmpTopRight.bmWidth, 0);

		///left
		m_bmpLeftBack.StretchBltToHDC(hdcMemSrc, hdcMemTemp, 0, m_bmpTopBack.bmHeight, m_bmpLeftBack.bmWidth, iWndHeight - m_bmpTopBack.bmHeight - m_bmpBottomBack.bmHeight);
		m_bmpLeftTop.BitBltToHDC(hdcMemSrc, hdcMemTemp, 0, m_bmpTopBack.bmHeight);
		m_bmpLeftBottom.BitBltToHDC(hdcMemSrc, hdcMemTemp, 0, iWndHeight - m_bmpBottomBack.bmHeight - m_bmpLeftBottom.bmHeight);

		//right
		m_bmpRightBack.StretchBltToHDC(hdcMemSrc, hdcMemTemp, iWndWidth - m_bmpRightBack.bmWidth, m_bmpTopBack.bmHeight, m_bmpRightBack.bmWidth, iWndHeight - m_bmpTopBack.bmHeight - m_bmpBottomBack.bmHeight);
		m_bmpRightTop.BitBltToHDC(hdcMemSrc, hdcMemTemp,  iWndWidth - m_bmpRightTop.bmWidth, m_bmpTopBack.bmHeight);
		m_bmpRightBottom.BitBltToHDC(hdcMemSrc, hdcMemTemp, iWndWidth - m_bmpRightBottom.bmWidth, iWndHeight - m_bmpBottomBack.bmHeight - m_bmpRightBottom.bmHeight);

		//bottom
		m_bmpBottomBack.StretchBltToHDC(hdcMemSrc, hdcMemTemp, 0, iWndHeight - m_bmpBottomBack.bmHeight, iWndWidth, m_bmpBottomBack.bmHeight);
		m_bmpBottomLeft.BitBltToHDC(hdcMemSrc, hdcMemTemp, 0, iWndHeight - m_bmpBottomLeft.bmHeight);
		m_bmpBottomRight.BitBltToHDC(hdcMemSrc, hdcMemTemp, iWndWidth - m_bmpBottomRight.bmWidth, iWndHeight - m_bmpBottomRight.bmHeight);

		//画各按钮
		m_ncbbCloseBtn.BitBltToHDC(hdcMemSrc, hdcMemTemp, iWndWidth - m_ncbbCloseBtn.bmWidth - m_ncbbCloseBtn.m_iPosRightToFrame, m_ncbbCloseBtn.m_iPosTopToFrame);
		if (IsHasMaxBtn())//是否有最大化按钮
		{
			//是否已经最大化窗口了
			::IsZoomed(m_hWnd) ?
				m_ncbbRecBtn.BitBltToHDC(hdcMemSrc, hdcMemTemp, iWndWidth - m_ncbbRecBtn.bmWidth - m_ncbbMaxBtn.m_iPosRightToFrame, m_ncbbMaxBtn.m_iPosTopToFrame) :
			m_ncbbMaxBtn.BitBltToHDC(hdcMemSrc, hdcMemTemp, iWndWidth - m_ncbbMaxBtn.bmWidth - m_ncbbRecBtn.m_iPosRightToFrame, m_ncbbRecBtn.m_iPosTopToFrame);
		}
		if (IsHasMinBtn())//是否有最小化按钮
			m_ncbbMinBtn.BitBltToHDC(hdcMemSrc, hdcMemTemp, iWndWidth - m_ncbbMinBtn.bmWidth - m_ncbbMinBtn.m_iPosRightToFrame, m_ncbbMinBtn.m_iPosTopToFrame);

		//栽剪SetWindowRgn窗口
		if (m_bNeedClrTransparent && m_bNeedSetWndRgn)//如果需要重新SetWindowRgn
		{
			HBITMAP hBmpToRgn;//用于得到区域的图像
			HRGN hRgnSetWnd, hRgnBToR, hRgnClient;

			//设置成false，说明已经SetWindowRgn，不能每次Paint时都SetWindowRgn，很浪费CPU资源（而且也没有必要）
			m_bNeedSetWndRgn = false;
			//必须复制一个BMP位图，不能直接使用hBmpMemSrc（还不知道为什么）
			hBmpToRgn = (HBITMAP)::CopyImage(hBmpMemSrc, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE);
			hRgnSetWnd = CreateTempRgn();
			//重要步骤，根据位图和透明色得到区域
			hRgnBToR = ::BitmapToRegion(hBmpToRgn, m_clrTransparent, m_clrTransparent);
			//得到客户区的区域
			hRgnClient = ::CreateRectRgn(
				rcClient.left - rcWnd.left,
				rcClient.top - rcWnd.top,
				rcClient.right - rcWnd.left,
				rcClient.bottom - rcWnd.top
				);
			//合并客户区域和根据位图得到的区域（因为客户区的区域必须使用，不能被去掉）
			::CombineRgn(hRgnSetWnd, hRgnBToR, hRgnClient, RGN_OR);
			//最终使用SetWindowRgn设置
			::SetWindowRgn(m_hWnd, hRgnSetWnd, TRUE);

			//释放所有用于SetWindowRgn的资源
			::DeleteObject(hRgnClient);
			::DeleteObject(hRgnBToR);
			::DeleteObject(hRgnSetWnd);
			::DeleteObject(hBmpToRgn);
		}

		//标题文字（必须在SetWindowRgn后，因为不能把文字色混入透明色）
		if (m_hTitleTextFont)
		{
			CString strTitle, strDrawTitle;
			HFONT hOldFont;
			SIZE sizeTP;
			int iTextLength;
			int iMaxWidth;

			//先获取标题内容长度
			iTextLength = ::GetWindowTextLength(m_hWnd);
			//根据长度获取标题内容
			::GetWindowText(m_hWnd, strTitle.GetBuffer(iTextLength + 1), iTextLength + 1);
			strTitle.ReleaseBuffer(iTextLength);
			//重新获取获取标题内容长度
			iTextLength = strTitle.GetLength();
			//得到标题文字允许最宽的宽度
			iMaxWidth = iWndWidth - m_iTitleTextPosLeftToFrame - m_iTitleTextPosRightToFrame;
			//设置文字色和透明
			::SetBkMode(hdcMemSrc, TRANSPARENT);//底色透明
			::SetTextColor(hdcMemSrc, m_clrTitleText);//文字色
			hOldFont = (HFONT)::SelectObject(hdcMemSrc, m_hTitleTextFont);
			//测试标题宽（必须在选入字体后进行，不然宽度不一样）
			::GetTextExtentPoint32(hdcMemSrc, strTitle, iTextLength, &sizeTP);
			if (iMaxWidth < sizeTP.cx) {
				//全部放入宽度不够
				SIZE sizeMore;
				CString strMore(_T("..."));
				::GetTextExtentPoint32(hdcMemSrc, strMore, strMore.GetLength(), &sizeMore);
				do {
					::GetTextExtentPoint32(hdcMemSrc, strTitle, iTextLength, &sizeTP);
					if (iMaxWidth >= (sizeTP.cx + sizeMore.cx)) break;//如果宽度（标题宽加上省略号宽）未超过允许最长宽度，则跳出
					if ((--iTextLength) <= 0) break;//字符已经减到0则也直接跳出
				} while (1);
				strDrawTitle = strTitle.Left(iTextLength) + strMore;

			} else {
				//足够全部放入
				strDrawTitle = strTitle;

			}
			//使用API TextOut将标题绘制出来
			::TextOut(hdcMemSrc,
				m_iTitleTextPosLeftToFrame,
				m_iTitleTextPosTopToFrame,
				strDrawTitle,
				strDrawTitle.GetLength());

			//选入旧字体
			::SelectObject(hdcMemSrc, m_hTitleTextFont);
		}


		//绘制图标
		if (m_hTitleIcon && m_bDrawTitleIcon)
		{
			::DrawIconEx(hdcMemSrc, 
				m_iTitleIconLeftToFrame, 
				m_iTitleIconTopToFrame,
				m_hTitleIcon,
				m_iTitleIconWidth,
				m_iTitleIconHeight,
				NULL, NULL, DI_NORMAL | DI_COMPAT);
		}

		//最终绘制
		::BitBlt(hdc, 0, 0, iWndWidth, iWndHeight, hdcMemSrc, 0, 0, SRCCOPY);

		//释放所有用于绘制的资源（除成员变量）
		::SelectObject(hdc, hOldBmpMemSrc);
		::DeleteObject(hRgnTemp);
		::DeleteObject(hRgnOffsetWndClient);
		::DeleteObject(hRgnClient);
		::DeleteObject(hRgnWnd);
		::DeleteObject(hBmpMemSrc);
		::DeleteDC(hdcMemSrc);
		::DeleteDC(hdcMemTemp);
		::ReleaseDC(m_hWnd, hdc);

		return 0;
	}
	return 0;
}


LRESULT CWndNCSkinProc::OnNcCalcSize(WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (!IsUseDefaultSkin()) {

		LPNCCALCSIZE_PARAMS p = (LPNCCALCSIZE_PARAMS)lParam;

		RECT rcClientHope;//需要的客户区矩形
		rcClientHope.top = p->rgrc[0].top + m_bmpTopBack.bmHeight;
		rcClientHope.left = p->rgrc[0].left + m_bmpLeftBack.bmWidth;
		rcClientHope.right = p->rgrc[0].right - m_bmpRightBack.bmWidth;
		rcClientHope.bottom = p->rgrc[0].bottom - m_bmpBottomBack.bmHeight;

		p->rgrc[0].top = rcClientHope.top;
		p->rgrc[0].left = rcClientHope.left;
		p->rgrc[0].right = rcClientHope.right;
		p->rgrc[0].bottom = rcClientHope.bottom;

		if (wParam == TRUE)
		{
			if  (p->rgrc[2].right - p->rgrc[2].left != rcClientHope.right - rcClientHope.left ||
				p->rgrc[2].bottom - p->rgrc[2].top != rcClientHope.bottom - rcClientHope.top)
				m_bNeedSetWndRgn = true;
		}
		return 0;

	}

	bHandled = FALSE;
	return 0;
}


LRESULT CWndNCSkinProc::OnNcActivate(WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (IsUseDefaultSkin()) {
		bHandled = FALSE;

	} else {
		if (wParam == FALSE)
			return TRUE;

		return 0;
	}

	return 0;
}


LRESULT CWndNCSkinProc::OnNcHitTest(WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (IsUseDefaultSkin()) {
		bHandled = FALSE;

	} else {
		LRESULT lResult;
		POINT pt;

		//前期处理，得到部分变量数值
		pt.x = GET_X_LPARAM(lParam);//得到鼠标坐标x
		pt.y = GET_Y_LPARAM(lParam);//得到鼠标坐标y

		lResult = HitTest(pt);
		if (lResult == HTNOWHERE)
		{
			//未处理，交给window默认处理
			lResult = ::DefWindowProc(m_hWnd, WM_NCHITTEST, wParam, lParam);

			//默认处理后如果是返回关闭按钮，最大化按钮，最小化按钮，则都转成标题栏（因为这些按钮没有用了）
			if (lResult == HTCLOSE ||
				lResult == HTMINBUTTON ||
				lResult == HTMAXBUTTON ||
				lResult == HTSYSMENU ||
				lResult == HTHELP)
				lResult = HTCAPTION;
		}

		//返回值
		return lResult;
	}

	return 0;
}


LRESULT CWndNCSkinProc::OnNcMouseMove(WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (IsUseDefaultSkin()) {
		bHandled = FALSE;

	} else {
		switch (wParam)
		{
		case HTCLOSE:
			DrawNCBmpBtnByState(&m_ncbbCloseBtn, NCBTN_STATE_MOVE);
			SetCaptureBtn(&m_ncbbCloseBtn);
			break;

		case HTMAXBUTTON:
			if (::IsZoomed(m_hWnd)) {
				DrawNCBmpBtnByState(&m_ncbbRecBtn, NCBTN_STATE_MOVE);
				SetCaptureBtn(&m_ncbbRecBtn);
			} else {
				DrawNCBmpBtnByState(&m_ncbbMaxBtn, NCBTN_STATE_MOVE);
				SetCaptureBtn(&m_ncbbMaxBtn);
			}
			break;

		case HTMINBUTTON:
			DrawNCBmpBtnByState(&m_ncbbMinBtn, NCBTN_STATE_MOVE);
			SetCaptureBtn(&m_ncbbMinBtn);
			break;

		default:
			bHandled = FALSE;
		}

		if (wParam != HTCLOSE) DrawNCBmpBtnByState(&m_ncbbCloseBtn, NCBTN_STATE_DEFAULT);
		if (wParam != HTMINBUTTON) DrawNCBmpBtnByState(&m_ncbbMinBtn, NCBTN_STATE_DEFAULT);
		if (wParam != HTMAXBUTTON)
		{
			if (::IsZoomed(m_hWnd))
				DrawNCBmpBtnByState(&m_ncbbRecBtn, NCBTN_STATE_DEFAULT);
			else
				DrawNCBmpBtnByState(&m_ncbbMaxBtn, NCBTN_STATE_DEFAULT);
		}
		return 0;
	}
	return 0;
}


LRESULT CWndNCSkinProc::OnNcLButtonDown(WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (IsUseDefaultSkin()) {
		bHandled = FALSE;

	} else {
		switch (wParam)
		{
		case HTCLOSE:
			DrawNCBmpBtnByState(&m_ncbbCloseBtn, NCBTN_STATE_DOWN);
			SetCaptureBtn(&m_ncbbCloseBtn);
			break;

		case HTMAXBUTTON:
			if (::IsZoomed(m_hWnd)) {
				DrawNCBmpBtnByState(&m_ncbbRecBtn, NCBTN_STATE_DOWN);
				SetCaptureBtn(&m_ncbbRecBtn);
			} else {
				DrawNCBmpBtnByState(&m_ncbbMaxBtn, NCBTN_STATE_DOWN);
				SetCaptureBtn(&m_ncbbMaxBtn);
			}
			break;

		case HTMINBUTTON:
			DrawNCBmpBtnByState(&m_ncbbMinBtn, NCBTN_STATE_DOWN);			
			SetCaptureBtn(&m_ncbbMinBtn);
			break;

		case HTTOP:
			::SendMessage(m_hWnd, WM_SYSCOMMAND, SC_SIZE | WMSZ_TOP, lParam);
			break;

		case HTLEFT:
			::SendMessage(m_hWnd, WM_SYSCOMMAND, SC_SIZE | WMSZ_LEFT, lParam);
			break;

		case HTRIGHT:
			::SendMessage(m_hWnd, WM_SYSCOMMAND, SC_SIZE | WMSZ_RIGHT, lParam);
			break;

		case HTTOPLEFT:
			::SendMessage(m_hWnd, WM_SYSCOMMAND, SC_SIZE | WMSZ_TOPLEFT, lParam);
			break;

		case HTTOPRIGHT:
			::SendMessage(m_hWnd, WM_SYSCOMMAND, SC_SIZE | WMSZ_TOPRIGHT, lParam);
			break;

		case HTBOTTOM:
			::SendMessage(m_hWnd, WM_SYSCOMMAND, SC_SIZE | WMSZ_BOTTOM, lParam);
			break;

		case HTBOTTOMLEFT:
			::SendMessage(m_hWnd, WM_SYSCOMMAND, SC_SIZE | WMSZ_BOTTOMLEFT, lParam);
			break;

		case HTBOTTOMRIGHT:
			::SendMessage(m_hWnd, WM_SYSCOMMAND, SC_SIZE | WMSZ_BOTTOMRIGHT, lParam);
			break;

		case HTCAPTION:
			if (!::IsZoomed(m_hWnd)) {
				::SendMessage(m_hWnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, lParam);
			}
			break;
		default:
			bHandled = FALSE;
		}
		return 0;
	}

	return 0;
}



LRESULT CWndNCSkinProc::OnNcRButtonDown(WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (IsUseDefaultSkin()) {
		bHandled = FALSE;

	} else {

	}

	return 0;
}


LRESULT CWndNCSkinProc::OnMouseMove(WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (!IsUseDefaultSkin()) {
		CNCBmpButton* p = GetCaptureBtn();
		if (p)
		{
			LRESULT lResHit;
			POINT pt;

			//前期处理，得到部分变量数值
			pt.x = GET_X_LPARAM(lParam);//得到鼠标坐标x
			pt.y = GET_Y_LPARAM(lParam);//得到鼠标坐标y
			::ClientToScreen(m_hWnd, &pt);//WM_MOUSEMOVE消息得到的坐标为窗口坐标，必须转化为屏幕坐标
			lResHit = HitTest(pt);

			if (wParam & MK_LBUTTON)//如果按下了鼠标左键
			{
				bool bHasDo = false;
				switch (lResHit)
				{
				case HTCLOSE:
					if (p == &m_ncbbCloseBtn)
					{
						DrawNCBmpBtnByState(p, NCBTN_STATE_DOWN);
						bHasDo = true;
					}
					break;
				case HTMAXBUTTON:
					if (p == &m_ncbbMaxBtn || p == &m_ncbbRecBtn)
					{
						DrawNCBmpBtnByState(p, NCBTN_STATE_DOWN);
						bHasDo = true;
					}
					break;
				case HTMINBUTTON:
					if (p == &m_ncbbMinBtn)
					{
						DrawNCBmpBtnByState(p, NCBTN_STATE_DOWN);
						bHasDo = true;
					}
					break;
				}

				if (!bHasDo) DrawNCBmpBtnByState(p, NCBTN_STATE_DEFAULT);
			}
			else//未按鼠标左键
			{
				bool bHasDo = false;
				switch (lResHit)
				{
				case HTCLOSE:
					if (p == &m_ncbbCloseBtn)
					{
						DrawNCBmpBtnByState(p, NCBTN_STATE_MOVE);
						bHasDo = true;
					}
					break;
				case HTMAXBUTTON:
					if (p == &m_ncbbMaxBtn || p == &m_ncbbRecBtn)
					{
						DrawNCBmpBtnByState(p, NCBTN_STATE_MOVE);
						bHasDo = true;
					}
					break;
				case HTMINBUTTON:
					if (p == &m_ncbbMinBtn)
					{
						DrawNCBmpBtnByState(p, NCBTN_STATE_MOVE);
						bHasDo = true;
					}
					break;
				}
				if (!bHasDo)
				{
					DrawNCBmpBtnByState(p, NCBTN_STATE_DEFAULT);
					ReleaseCaptureBtn();
				}
			}
		}
	}

	bHandled = FALSE;
	return 0;
}


LRESULT CWndNCSkinProc::OnLButtonDown(WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (IsUseDefaultSkin()) {
		bHandled = FALSE;

	} else {
		CNCBmpButton* p = GetCaptureBtn();
		if (p)
		{
			POINT pt;
			//前期处理，得到部分变量数值
			pt.x = GET_X_LPARAM(lParam);//得到鼠标坐标x
			pt.y = GET_Y_LPARAM(lParam);//得到鼠标坐标y
			::ClientToScreen(m_hWnd, &pt);//WM_MOUSEMOVE消息得到的坐标为窗口坐标，必须转化为屏幕坐标

			switch (HitTest(pt))
			{
			case HTCLOSE:
				DrawNCBmpBtnByState(&m_ncbbCloseBtn, NCBTN_STATE_DOWN);
				break;
			case HTMAXBUTTON:
				if (::IsZoomed(m_hWnd)) {
					DrawNCBmpBtnByState(&m_ncbbRecBtn, NCBTN_STATE_DOWN);
				} else {
					DrawNCBmpBtnByState(&m_ncbbMaxBtn, NCBTN_STATE_DOWN);
				}
				break;
			case HTMINBUTTON:
				DrawNCBmpBtnByState(&m_ncbbMinBtn, NCBTN_STATE_DOWN);
				break;
			}

			bHandled = FALSE;
			return 0;
		}
	}

	bHandled = FALSE;
	return 0;
}

LRESULT CWndNCSkinProc::OnLButtonUp(WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (!IsUseDefaultSkin()) {
		CNCBmpButton* p = GetCaptureBtn();
		if (p)
		{
			POINT pt;
			//前期处理，得到部分变量数值
			pt.x = GET_X_LPARAM(lParam);//得到鼠标坐标x
			pt.y = GET_Y_LPARAM(lParam);//得到鼠标坐标y
			::ClientToScreen(m_hWnd, &pt);//WM_MOUSEMOVE消息得到的坐标为窗口坐标，必须转化为屏幕坐标

			switch (HitTest(pt))
			{
			case HTCLOSE:
				if (p == &m_ncbbCloseBtn) {
					DrawNCBmpBtnByState(p, NCBTN_STATE_DEFAULT);
					::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
				}
				break;
			case HTMAXBUTTON:
				if (p == &m_ncbbMaxBtn || p == &m_ncbbRecBtn) {
					DrawNCBmpBtnByState(p, NCBTN_STATE_DEFAULT);
					::IsZoomed(m_hWnd) ?
						::ShowWindow(m_hWnd, SW_RESTORE) :
					::ShowWindow(m_hWnd, SW_MAXIMIZE);
				}
				break;
			case HTMINBUTTON:
				if (p == &m_ncbbMinBtn) {
					DrawNCBmpBtnByState(p, NCBTN_STATE_DEFAULT);
					::ShowWindow(m_hWnd, SW_MINIMIZE);
				}
				break;
			}

			ReleaseCaptureBtn();
			bHandled = FALSE;
			return 0;
		}
	}

	bHandled = FALSE;
	return 0;
}

LRESULT CWndNCSkinProc::OnRButtonUp(WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (IsUseDefaultSkin()) {
		bHandled = FALSE;

	} else {

	}
	return 0;
}

LRESULT CWndNCSkinProc::OnSetText(WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (IsUseDefaultSkin()) {
		bHandled = FALSE;

	} else {
		LRESULT lResult;
		DWORD dwRemoveStyle = WS_CAPTION;
		DWORD dwOldStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);

		if (dwOldStyle & dwRemoveStyle) {
			//将WS_CAPTION窗口风格去掉后，DefWindowProc在处理时就不会重新绘制Caption区了
			//这个方法是在CodeProject上找到的
			SetWindowLong(m_hWnd, GWL_STYLE, dwOldStyle & ~dwRemoveStyle);
			lResult = ::DefWindowProc(m_hWnd, WM_SETTEXT, wParam, lParam);
			SetWindowLong(m_hWnd, GWL_STYLE, dwOldStyle);

		} else {
			//没有WS_CAPTION风格的窗口不会自动重绘
			lResult = ::DefWindowProc(m_hWnd, WM_SETTEXT, wParam, lParam);
		}
		return lResult;
	}

	return 0;
}

LRESULT CWndNCSkinProc::OnGetText(WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (IsUseDefaultSkin()) {
		bHandled = FALSE;
	} else {
		bHandled = FALSE;
	}

	return 0;
}

LRESULT CWndNCSkinProc::OnGetTextLength(WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (IsUseDefaultSkin()) {
		bHandled = FALSE;
	} else {
		bHandled = FALSE;
	}
	return 0;
}

LRESULT CWndNCSkinProc::OnSysCommand(WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (IsUseDefaultSkin()) {
		bHandled = FALSE;
	} else {
		if ((wParam & 0xFFF0) == SC_KEYMENU)
			return 0;

		bHandled = FALSE;
	}
	return 0;
}

LRESULT CWndNCSkinProc::OnSetCursor(WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (IsUseDefaultSkin()) {
		bHandled = FALSE;
	} else {
		switch (LOWORD(lParam))
		{
		case HTCLOSE:
		case HTMAXBUTTON:
		case HTMINBUTTON:
			::SetCursor(::LoadCursor(NULL, IDC_HAND));
			break;
		case HTLEFT:
		case HTRIGHT:
			::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
			break;
		case HTTOPLEFT:
		case HTBOTTOMRIGHT:
			::SetCursor(::LoadCursor(NULL, IDC_SIZENWSE));
			break;
		case HTTOPRIGHT:
		case HTBOTTOMLEFT:
			::SetCursor(::LoadCursor(NULL, IDC_SIZENESW));
			break;
		case HTTOP:
		case HTBOTTOM:
			::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
			break;
		case HTCAPTION:
			::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			break;
		default:
			bHandled = FALSE;
		}
		return TRUE;
	}

	return 0;
}

LRESULT CWndNCSkinProc::OnSetIcon(WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}

LRESULT CWndNCSkinProc::OnGetIcon(WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (IsUseDefaultSkin()) {
		bHandled = FALSE;
	} else {
		if (m_hTitleIcon)
			return (LRESULT)m_hTitleIcon;

		bHandled = FALSE;
	}

	return 0;
}

LRESULT CWndNCSkinProc::OnStyleChanged(WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{//窗口Style改变时触发
	if (IsUseDefaultSkin()) {
		bHandled = FALSE;
	} else {
		ReDrawFrame();
		return 0;
	}

	return 0;
}

LRESULT CWndNCSkinProc::OnSettingChange(WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{//Windows主题改变时触发
	if (IsUseDefaultSkin()) {
		bHandled = FALSE;
	} else {
		//使用皮肤情况不能让Windows来裁剪(SetWindowRgn)
		if (!m_bNeedClrTransparent) {
			::SetWindowRgn(m_hWnd, NULL, TRUE);
		} else {
			m_bNeedSetWndRgn = true;
		}
		return 0;
	}

	return 0;
}

LRESULT CWndNCSkinProc::OnNcSkinUpdated(WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{//默认对WM_NCSKIN_UPDATED消息的处理
	bHandled = TRUE;//表示处理过了
	return 0;
}