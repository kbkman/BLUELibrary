#include "gui2d.h"
#include <list>

/**
类名：DX2D列表元素
功能：无
说明：无
版本：1.0.0.1
作者：BLUE
*/
class CDXGList2DElement : public CDXGWindow2D, private IDXGCommandListener
{
	friend class CDXGList2D;

private:
	CDXGList2DElement(
		const CDirectDrawSurface7& surfaceBtn,
		int nCellWidth,
		int nCellHeight,
		int nElementTextTop,
		int nElementTextLeft,
		const BLUEString& strCaption,
		int nUserData
		);
	virtual ~CDXGList2DElement(void);

protected:
	virtual bool OnCreate(int nWidth, int nHeight);
	virtual void OnDestroy(void);
	virtual void OnRender(void);

private:
	virtual void OnDXGCommand(CDXGWindow* pWindow, unsigned int nEvent, int nDetail);

private:
	CDDrawSprite m_spriteBtn;
	CDXGButton2D* m_pButton;
	CDXGStatic2D* m_pStatic;

	int m_nBtnCellWidth;//按钮精灵宽
	int m_nBtnCellHeight;//按钮精灵高
	int m_nTextTop;//元素中文本离元素上面的距离
	int m_nTextLeft;//元素中文本离元素左面的距离

	BLUEString m_strCaption;//文件框内的文字

	int m_nUserData;//存储的自定义数据
};

/*
函数名：DX2D控件
功能：略
说明：无
参数：无
返回值：无
*/
class CDXGList2D : public CDXGWindow2D, private IDXGCommandListener
{
	typedef std::list<CDXGList2DElement*> ELEMENTCOLL;

public:
	CDXGList2D(const CDirectDrawSurface7& surfaceBtn, int nCellWidth, int nCellHeight, int nElementTextTop = 5, int nElementTextLeft = 5);
	virtual ~CDXGList2D(void);

public:
	void SetElementVSpace(int nSpace) {m_nElementVSpace = nSpace;}
	void Clear(void);
	bool AddElement(const BLUEString& strCaption, int nUserData);
	int GetSize(void) const {return (int)m_elementColl.size();}

protected:
	virtual bool OnCreate(int nWidth, int nHeight);
	virtual void OnDestroy(void);
	virtual void OnRender(void);

private:
	virtual void OnDXGCommand(CDXGWindow* pWindow, unsigned int nEvent, int nDetail);

private:
	const CDirectDrawSurface7& m_surfaceBtn;
	const int m_nBtnSpriteCellWidth;//精灵分块宽
	const int m_nBtnSpriteCellHeight;//精灵分块高
	int m_nElementTextTop;//元素中文本离元素上面的距离
	int m_nElementTextLeft;//元素中文本离元素左面的距离
	int m_nElementVSpace;//元素垂直间隔
	ELEMENTCOLL m_elementColl;//元素集合
};