#pragma once

/*****************
*用DX的DirectDraw编写的用户UI代码
*
*****************/
#include "gui.h"
#include "engine2d.h"

/*
函数名：设置DXG2D全局参数
功能：略
说明：无
参数：无
返回值：无
*/
void DXG2DSetGlobalParam(const CDirectDraw7& dd7, const CDirectDrawSurface7& ddsBack);

/**
类名：使用DirectDraw绘制的UI窗口类型基础类
功能：无
说明：无
版本：1.0.0.1
作者：BLUE
*/
class CDXGWindow2D : public CDXGWindow
{
public:
	CDXGWindow2D(bool bAcceptMsg, bool bVisible = true) : CDXGWindow(bAcceptMsg, bVisible) {}
	CDXGWindow2D(void) {}
	virtual ~CDXGWindow2D(void);

public:
	void SetBKSrcColorKey(DWORD dwColor) {m_surfaceCanvas.SetSrcColorKey(dwColor);}
	void SetBKNoSrcColorKey(void) {m_surfaceCanvas.SetNoSrcColorKey();}
	void FillBackColor(DWORD dwFillColor) {m_surfaceCanvas.FillColor(dwFillColor);}

protected:
	virtual bool OnCreate(int nWidth, int nHeight);
	virtual void OnDestroy(void);
	virtual void OnRender(void);
	const CDirectDrawSurface7& GetCanvas(void) const {return m_surfaceCanvas;}

private:
	CDirectDrawSurface7 m_surfaceCanvas;
	bool m_bTransparent;
	DWORD m_dwBackColor;
};

/**
类名：
功能：使用DirectDraw绘制的UI窗口类型－图片控件
说明：无
版本：1.0.0.1
作者：BLUE
*/
class CDXGImage2D : public CDXGWindow2D
{
public:
	CDXGImage2D(const CDirectDrawSurface7& ddsImgSrc);
	virtual ~CDXGImage2D(void);

protected:
	virtual bool OnCreate(int nWidth, int nHeight);
	virtual void OnRender(void);

private:
	const CDirectDrawSurface7& m_ddsImgSrc;
};

/**
类名：使用DirectDraw绘制的UI窗口类型－静态文本窗口
功能：无
说明：无
版本：1.0.0.1
作者：BLUE
*/
class CDXGStatic2D : public CDXGWindow2D
{
public:
	CDXGStatic2D(void) : CDXGWindow2D(false) {}
	virtual ~CDXGStatic2D(void) {}

public:
	//文本
	void SetText(const BLUEString& strText) {m_strText = strText;OnSetText(m_strText);}
	const BLUEString& GetText(void) const {return m_strText;}

protected:
	virtual bool OnCreate(int nWidth, int nHeight);
	virtual void OnDestroy(void);
	virtual void OnRender(void);
	virtual void OnSetText(const BLUEString& strText);

private:
	BLUEString m_strText;
	CDirectDrawSurface7 m_surfaceCanvas;
};

/**
类名：使用DirectDraw绘制UI窗口类型－文字编辑窗口
功能：无
说明：无
版本：1.0.0.1
作者：BLUE
*/
class CDXGEdit2D : public CDXGWindow2D
{
public:
	CDXGEdit2D(void) :
	  m_nCurFrame(0), m_nCursorPosInChar(0), m_nCursorPosX(0),
	  m_dwTextColor(0), m_dwBackColor(0x00FFFFFF), m_dwCursorColor(0),
	  m_bPasswordStyle(false)
	  {}
	virtual ~CDXGEdit2D(void) {}

public:
	//文本
	void SetText(const BLUEString& strText) {m_strText = strText;OnSetText();}
	const BLUEString& GetText(void) const {return m_strText;}
	void SetTextColor(DWORD dwColor) {m_dwTextColor = dwColor;OnSetText();}
	void SetCursorColor(DWORD dwColor) {m_surfaceCursor.FillColor(dwColor);}
	void SetBackColor(DWORD dwColor) {m_dwBackColor = dwColor;OnSetText();}
	void SetStyle(bool bPasswordStyle) {m_bPasswordStyle = bPasswordStyle;OnSetText();}
	void SetCursorPosInChar(int nPos);
	void SetCursorPosEnd(void) {return SetCursorPosInChar((int)m_strText.length());}

protected:
	virtual bool OnCreate(int nWidth, int nHeight);
	virtual void OnSetText(void);

	//处理键盘消息
	virtual void OnKeyDown(int code);
	virtual void OnKeyUp(int code);
	virtual void OnChar(int code);

	virtual void OnRender(void);

	//自身事件
	virtual void OnCompleteInput(void) {}//完成输入（用户输入时按下回车后，触发此事件）

private:
	CDirectDrawSurface7 m_surfaceText;//存放文字表面
	CDirectDrawSurface7 m_surfaceCursor;//光标表面
	BLUEString m_strText;
	CBLUEPoint m_cursorPoint;
	int m_nCursorPosInChar;//当前光标在第几个字符后面
	int m_nCursorPosX;//光标x坐标
	int m_nCurFrame;

	DWORD m_dwTextColor;//文本颜色
	DWORD m_dwBackColor;//背景颜色
	DWORD m_dwCursorColor;//光标颜色

	bool m_bPasswordStyle;//是否为密码框形式
};

/**
类名：使用DirectDraw绘制的UI窗口类型－按钮
功能：无
说明：无
版本：1.0.0.1
作者：BLUE
*/
class CDXGButton2D : public CDXGWindow2D
{
public:
	CDXGButton2D(CDDrawSprite& ddrawSprite) : m_ddrawSprite(ddrawSprite), m_bWaitLBTUpToOnClick(false) {m_ddrawSprite.SetFrame(0);}
	virtual ~CDXGButton2D(void);

private:
	virtual bool OnCreate(int nWidth, int nHeight) {return true;}//重写，不必创建默认画布
	virtual void OnEnabled(bool bIsEnabled);

	//鼠标事件，pt均为相对坐标，相对于父窗口的坐标，如果是顶层窗口，则为绝对坐标
	virtual void OnMouseIn(const CBLUEPoint& pt);
	virtual void OnMouseMove(const CBLUEPoint& pt) {}
	virtual void OnMouseOut(const CBLUEPoint& pt);
	virtual void OnLButtonDown(const CBLUEPoint& pt);
	virtual void OnLButtonUp(const CBLUEPoint& pt);
	virtual void OnRButtonDown(const CBLUEPoint& pt) {}
	virtual void OnRButtonUp(const CBLUEPoint& pt) {}

	virtual void OnRender(void);

protected:
	virtual void OnClick(void)
	{
		if (m_pListener)
			m_pListener->OnDXGCommand(this, DXG_EVENT_BTN_CLICK, 0);
	}

private:
	CDDrawSprite& m_ddrawSprite;
	bool m_bWaitLBTUpToOnClick;//等待抬起鼠标触发OnClick事件
};