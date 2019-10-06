#include "gui2dList.h"

//////////////////////////////////////////////////////////////////////////
/////////////////CDXGList2DElement实现////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
CDXGList2DElement::CDXGList2DElement(
									 const CDirectDrawSurface7& surfaceBtn,
									 int nBtnCellWidth, int nBtnCellHeight, int nTextTop, int nTextLeft,
									 const BLUEString& strCaption,
									 int nUserData) :
m_pButton(BLUENULL),
m_pStatic(BLUENULL),
m_nBtnCellWidth(nBtnCellWidth),
m_nBtnCellHeight(nBtnCellHeight),
m_nTextTop(nTextTop),
m_nTextLeft(nTextLeft),
m_strCaption(strCaption),
m_nUserData(nUserData)
{
	m_spriteBtn.Create(surfaceBtn, nBtnCellWidth, nBtnCellHeight);
}

CDXGList2DElement::~CDXGList2DElement(void)
{
	if (m_pStatic) delete m_pStatic;
	if (m_pButton) delete m_pButton;
}

bool CDXGList2DElement::OnCreate(int nWidth, int nHeight)
{
	BLUEASSERT(m_pStatic == BLUENULL);
	BLUEASSERT(m_pButton == BLUENULL);

	m_pButton = new CDXGButton2D(m_spriteBtn);
	m_pStatic = new CDXGStatic2D;
	m_pButton->SetListener(this);
	m_pStatic->SetListener(this);

	AddChildWindow(m_pButton, CBLUERect(CBLUEPoint(0, 0), CBLUEPoint(m_nBtnCellWidth, m_nBtnCellHeight)));
	AddChildWindow(m_pStatic, CBLUERect(CBLUEPoint(m_nTextLeft, m_nTextTop), CBLUEPoint(nWidth, nHeight)));
	m_pStatic->SetText(m_strCaption);

	return true;
}

void CDXGList2DElement::OnDestroy(void)
{
	if (m_pStatic) {delete m_pStatic;m_pStatic = BLUENULL; }
	if (m_pButton) {delete m_pButton;m_pButton = BLUENULL;}
}

void CDXGList2DElement::OnRender(void)
{
}

void CDXGList2DElement::OnDXGCommand(CDXGWindow* pWindow, unsigned int nEvent, int nDetail)
{
	BLUEASSERT(pWindow);
	if (pWindow == m_pButton && m_pListener && nEvent == DXG_EVENT_BTN_CLICK)
		m_pListener->OnDXGCommand(this, 0, m_nUserData);
}

//////////////////////////////////////////////////////////////////////////
/////////////////CDXGList2D实现////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
CDXGList2D::CDXGList2D(const CDirectDrawSurface7& surfaceBtn, int nCellWidth, int nCellHeight, int nElementTextTop, int nElementTextLeft) :
m_surfaceBtn(surfaceBtn),
m_nBtnSpriteCellWidth(nCellWidth),
m_nBtnSpriteCellHeight(nCellHeight),
m_nElementTextTop(nElementTextTop),
m_nElementTextLeft(nElementTextLeft),
m_nElementVSpace(20)
{

}

CDXGList2D::~CDXGList2D(void)
{
	Clear();
}

bool CDXGList2D::OnCreate(int nWidth, int nHeight)
{
	return true;
}

void CDXGList2D::OnDestroy(void)
{

}

void CDXGList2D::OnRender(void)
{

}

void CDXGList2D::OnDXGCommand(CDXGWindow* pWindow, unsigned int nEvent, int nDetail)
{
	BLUEASSERT(pWindow);
	if (m_pListener)
		m_pListener->OnDXGCommand(this, DXG_LIST_ITEMSELECTED, nDetail);
}

void CDXGList2D::Clear(void)
{
	//先清除所有子项窗口
	RemoveAllChildWinow();

	//删除自存列表
	for (ELEMENTCOLL::iterator it = m_elementColl.begin(), itEnd = m_elementColl.end(); it != itEnd; it++)
		delete *it;
	m_elementColl.clear();
}

bool CDXGList2D::AddElement(const BLUEString& strCaption, int nUserData)
{
	CDXGList2DElement* pNewElement = new CDXGList2DElement(
		m_surfaceBtn,
		m_nBtnSpriteCellWidth,
		m_nBtnSpriteCellHeight,
		m_nElementTextTop,
		m_nElementTextLeft,
		strCaption,
		nUserData
		);
	pNewElement->SetListener(this);

	if (AddChildWindow(
		pNewElement,
		CBLUERect(
			CBLUEPoint(0, (int)m_elementColl.size() * (m_nBtnSpriteCellHeight + m_nElementVSpace)),
			CBLUESize(m_nBtnSpriteCellWidth, m_nBtnSpriteCellHeight))
		))
	{
		m_elementColl.push_back(pNewElement);
		return true;
	}

	delete pNewElement;
	return false;
}