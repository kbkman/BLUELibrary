#include "gui2d.h"
#include "guiMsgType.h"

/************************************************************************/
/* 全局                                                                    */
/************************************************************************/

CDirectDraw7 g_dxg2Ddd7;//DirectDraw接口
CDirectDrawSurface7 g_dxg2DddsBack;//绘制缓存

void DXG2DSetGlobalParam(const CDirectDraw7& dd7, const CDirectDrawSurface7& ddsBack)
{
	BLUEASSERT(dd7);
	BLUEASSERT(ddsBack);

	dd7.CopyTo(&::g_dxg2Ddd7);
	ddsBack.CopyTo(&::g_dxg2DddsBack);
}

/************************************************************************/
/*                      Window2D                                                */
/************************************************************************/
CDXGWindow2D::~CDXGWindow2D(void)
{
}

bool CDXGWindow2D::OnCreate(int nWidth, int nHeight)
{
	BLUEASSERT(nWidth >= 0);
	BLUEASSERT(nHeight >= 0);
	BLUEASSERT(::g_dxg2Ddd7);

	//创建画面表面
	return SUCCEEDED(::g_dxg2Ddd7.CreateGeneralSurface(nWidth, nHeight, 0, &m_surfaceCanvas));
}

void CDXGWindow2D::OnDestroy(void)
{
}

void CDXGWindow2D::OnRender(void)
{
	m_surfaceCanvas.Blt(::g_dxg2DddsBack, (LPRECT)&GetAbsRect(), NULL);
}

/************************************************************************/
/* Image2D                                                                     */
/************************************************************************/
CDXGImage2D::CDXGImage2D(const CDirectDrawSurface7& ddsImgSrc) :
CDXGWindow2D(false),
m_ddsImgSrc(ddsImgSrc)
{
}

CDXGImage2D::~CDXGImage2D(void)
{
}

bool CDXGImage2D::OnCreate(int nWidth, int nHeight)
{
	return true;
}

void CDXGImage2D::OnRender(void)
{
	m_ddsImgSrc.Blt(::g_dxg2DddsBack, (LPRECT)&GetAbsRect(), NULL);
}

/************************************************************************/
/* Static2D                                                                     */
/************************************************************************/
void CDXGStatic2D::OnSetText(const BLUEString& strText)
{
	m_surfaceCanvas.FillColor(0x00FF00FF);
	HDC hDC;
	m_surfaceCanvas->GetDC(&hDC);
	::SetTextColor(hDC, 0x00000000);
	::SetBkColor(hDC, 0x00FF00FF);
	DWORD dwWidth, dwHeight;
	m_surfaceCanvas.GetBaseDesc(dwWidth, dwHeight);
	RECT rc = {0, 0, dwWidth, dwHeight};
	::DrawText(hDC, strText.c_str(), (int)strText.length(), &rc, 0);
	m_surfaceCanvas->ReleaseDC(hDC);
}

bool CDXGStatic2D::OnCreate(int nWidth, int nHeight)
{
	BLUEASSERT(nWidth >= 0);
	BLUEASSERT(nHeight >= 0);
	BLUEASSERT(::g_dxg2Ddd7);

	//创建画面表面
	if (FAILED(::g_dxg2Ddd7.CreateGeneralSurface(nWidth, nHeight, 0, &m_surfaceCanvas)))
		return false;

	if (FAILED(m_surfaceCanvas.SetSrcColorKey(0x00FF00FF)))
		return false;

	m_surfaceCanvas.FillColor(0x00FF00FF);
	return true;
}

void CDXGStatic2D::OnDestroy(void)
{
	m_surfaceCanvas.Release();
}

void CDXGStatic2D::OnRender(void)
{
	m_surfaceCanvas.Blt(::g_dxg2DddsBack, (LPRECT)&GetAbsRect(), NULL);
}

/************************************************************************/
/* Edit2D                                                                     */
/************************************************************************/
void CDXGEdit2D::SetCursorPosInChar(int nPos)
{
	m_nCursorPosInChar = nPos;

	HDC hDC;
	m_surfaceText->GetDC(&hDC);
	SIZE size;
	if (!m_bPasswordStyle)
		::GetTextExtentPoint32(hDC, m_strText.substr(0, m_nCursorPosInChar).c_str(), m_nCursorPosInChar, &size);
	else
	{
		BLUEString strText(m_strText.length(), _BLUET('*'));
		::GetTextExtentPoint32(hDC, strText.substr(0, m_nCursorPosInChar).c_str(), m_nCursorPosInChar, &size);
	}
	m_nCursorPosX = size.cx;
	m_surfaceText->ReleaseDC(hDC);
}

bool CDXGEdit2D::OnCreate(int nWidth, int nHeight)
{
	//创建绘制文字的表面
	if (FAILED(::g_dxg2Ddd7.CreateGeneralSurface(nWidth, nHeight, 0, &m_surfaceText)))
		return false;

	//创建光标表面
	if (FAILED(::g_dxg2Ddd7.CreateGeneralSurface(1, 1, 0, &m_surfaceCursor)))
	{
		m_surfaceText.Release();
		return false;
	}

	m_surfaceText.FillColor(m_dwBackColor);
	m_surfaceCursor.FillColor(m_dwCursorColor);
	return true;
}

void CDXGEdit2D::OnSetText(void)
{
	m_surfaceText.FillColor(m_dwBackColor);

	HDC hDC;
	m_surfaceText->GetDC(&hDC);
	::SetTextColor(hDC, m_dwTextColor);
	::SetBkMode(hDC, TRANSPARENT);
	DWORD dwWidth, dwHeight;
	m_surfaceText.GetBaseDesc(dwWidth, dwHeight);
	RECT rc = {0, 0, dwWidth, dwHeight};
	if (!m_bPasswordStyle)
		::TextOut(hDC, 0, 0, m_strText.c_str(), (int)m_strText.length());
	else
	{
		BLUEString strText(m_strText.length(), _BLUET('*'));
		::TextOut(hDC, 0, 0, strText.c_str(), (int)strText.length());
	}
	m_surfaceText->ReleaseDC(hDC);

}

void CDXGEdit2D::OnKeyDown(int code)
{
	switch (code)
	{
	case DXGUIVK_LEFT:
		m_nCursorPosInChar--;
		if (m_nCursorPosInChar < 0)
			m_nCursorPosInChar = 0;
		break;
	case DXGUIVK_RIGHT:
		m_nCursorPosInChar++;
		if (m_nCursorPosInChar > (int)m_strText.length())
			m_nCursorPosInChar = (int)m_strText.length();
		break;
	case DXGUIVK_UP:
		break;
	case DXGUIVK_DOWN:
		break;
	case DXGUIVK_HOME:
		m_nCursorPosInChar = 0;
		break;
	case DXGUIVK_END:
		m_nCursorPosInChar = (int)m_strText.length();
		break;
	}

	HDC hDC;
	m_surfaceText->GetDC(&hDC);
	SIZE size;
	if (!m_bPasswordStyle)
		::GetTextExtentPoint32(hDC, m_strText.substr(0, m_nCursorPosInChar).c_str(), m_nCursorPosInChar, &size);
	else
	{
		BLUEString strText(m_strText.length(), _BLUET('*'));
		::GetTextExtentPoint32(hDC, strText.substr(0, m_nCursorPosInChar).c_str(), m_nCursorPosInChar, &size);
	}
	m_nCursorPosX = size.cx;
	m_surfaceText->ReleaseDC(hDC);
}

void CDXGEdit2D::OnKeyUp(int code)
{

}

void CDXGEdit2D::OnChar(int code)
{
	if (code == DXGUIVK_BACK) {//删除字符
		if (m_nCursorPosInChar > 0)
			m_strText = m_strText.substr(0, m_nCursorPosInChar - 1) + m_strText.substr(m_nCursorPosInChar, m_strText.length() - 1);
		m_nCursorPosInChar--;
		if (m_nCursorPosInChar < 0)
			m_nCursorPosInChar = 0;
		if (m_nCursorPosInChar > (int)m_strText.length())
			m_nCursorPosInChar = (int)m_strText.length();
	} else if (code == DXGUIVK_RETURN) {//回车换行直接触发OnCompleteInput事件
		OnCompleteInput();
	} else if (code == DXGUIVK_TAB) {//不支持Tab键
		//////
	} else {
		m_strText.insert(m_nCursorPosInChar, 1, (BLUETCHAR)code);
		m_nCursorPosInChar++;
	}

	HDC hDC;
	m_surfaceText->GetDC(&hDC);
	SIZE size;
	if (!m_bPasswordStyle)
		::GetTextExtentPoint32(hDC, m_strText.substr(0, m_nCursorPosInChar).c_str(), m_nCursorPosInChar, &size);
	else
	{
		BLUEString strText(m_strText.length(), _BLUET('*'));
		::GetTextExtentPoint32(hDC, strText.substr(0, m_nCursorPosInChar).c_str(), m_nCursorPosInChar, &size);
	}
	m_nCursorPosX = size.cx;
	m_surfaceText->ReleaseDC(hDC);

	//触发文字本被修改事件
	OnSetText();
}

void CDXGEdit2D::OnRender(void)
{
	//绘制文字
	m_surfaceText.Blt(::g_dxg2DddsBack, (LPRECT)&GetAbsRect(), NULL);

	//绘制光标
	if (IsFocus() && (m_nCurFrame++ & 1) == 0)
	{
		m_surfaceCursor.Blt(
			::g_dxg2DddsBack,
			m_nCursorPosX + GetAbsRect().m_nLeft,
			GetAbsRect().m_nTop,
			1, 17, 0, 0, 1, 1
			);
	}
}

/************************************************************************/
/* Button2D                                                                     */
/************************************************************************/
CDXGButton2D::~CDXGButton2D(void)
{
}

void CDXGButton2D::OnRender(void)
{
	BLUEASSERT(::g_dxg2DddsBack);
	m_ddrawSprite.Paint(::g_dxg2DddsBack, (LPCRECT)&GetAbsRect());
}

void CDXGButton2D::OnEnabled(bool bIsEnabled)
{
	m_ddrawSprite.SetFrame(bIsEnabled ? 0 : 3);
}

void CDXGButton2D::OnMouseIn(const CBLUEPoint& pt)
{
	if (IsEnabled())
		m_ddrawSprite.SetFrame(1);
}

void CDXGButton2D::OnMouseOut(const CBLUEPoint& pt)
{
	CDXGWindow2D::OnMouseOut(pt);

	if (IsEnabled())
	{
		m_bWaitLBTUpToOnClick = false;
		m_ddrawSprite.SetFrame(0);
	}
}

void CDXGButton2D::OnLButtonDown(const CBLUEPoint& pt)
{
	if (IsEnabled())
	{
		m_bWaitLBTUpToOnClick = true;
		m_ddrawSprite.SetFrame(2);
	}
}

void CDXGButton2D::OnLButtonUp(const CBLUEPoint& pt)
{
	if (IsEnabled() && m_bWaitLBTUpToOnClick)
	{
		OnClick();
		m_ddrawSprite.SetFrame(1);
	}
}