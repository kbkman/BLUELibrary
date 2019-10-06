#pragma once
/*****************
*用DX编写的用户UI底层代码
*
*****************/

#include "Def.h"
#include "guiEvent.h"
#include <list>

//预声明
class IDXGCommandListener;
class CDXGManager;
class CDXGWindow;

//类型定义
typedef std::list<CDXGWindow*> DXGWINDOWLIST;

/**
接口名：DXG命令接口
功能：略
说明：无
版本：1.0.0.1
作者：BLUE
*/
class IDXGCommandListener
{
public:
	/*
	函数名：DXGUI命令发生
	功能：略
	说明：无
	参数：pWindow 产生命令的窗口的指针
	　　　nEvent 产生的事件
	　　　nDetail 详细信息
	返回值：无
	*/
	virtual void OnDXGCommand(CDXGWindow* pWindow, unsigned int nEvent, int nDetail) = 0;
};

/**
类名：DXGUI管理器
功能：无
说明：所有（3D和2D）的UI管理器的基础类
版本：1.0.0.1
作者：BLUE
*/
class CDXGManager
{
public:
	CDXGManager(void);
	virtual ~CDXGManager(void);

public:
	/*
	函数名：添加顶层窗口
	功能：略
	说明：无
	参数：无
	返回值：是否成功
	*/
	bool AddTopWindow(CDXGWindow* pWindow, const CBLUERect& rt, bool bIsModel = false);

	/*
	函数名：移除一个顶层窗口
	功能：略
	说明：无
	参数：无
	返回值：无
	*/
	void RemoveTopWindow(CDXGWindow* pWindow);

	/*
	函数名：移除所有顶层窗口
	功能：略
	说明：无
	参数：无
	返回值：无
	*/
	void RemoveAllTopWindow(void);

	/*
	函数名：消息处理
	功能：略
	说明：无
	参数：无
	返回值：无
	*/
	void ProcessMessage(unsigned int uMsg, WPARAM wParam, LPARAM lParam);

	/*
	函数名：渲染所有UI
	功能：略
	说明：无
	参数：无
	返回值：无
	*/
	void Render(void);

private:
	/*
	变量名：顶层窗口链表
	功能：略
	说明：无
	*/
	DXGWINDOWLIST m_topWndList;

	/*
	变量名：当前鼠标所在窗口
	功能：用于触发窗口的OnMouseIn和OnMouseOver事件
	说明：无
	*/
	CDXGWindow* m_pCurMouseInWnd;

	/*
	变量名：当前获取焦点的窗口
	功能：略
	说明：无
	*/
	CDXGWindow* m_pCurFocusWnd;

};

/**
类名：DXGUI窗口
功能：无
说明：所有UI元素类的基础类
版本：1.0.0.1
作者：BLUE
*/
class CDXGWindow
{
	friend class CDXGManager;

protected:
	//保护构造，只能派生
	CDXGWindow(void);
	CDXGWindow(bool bAcceptMsg, bool bVisible = true);
	virtual ~CDXGWindow(void);

public:
	//监听器
	const IDXGCommandListener* GetListener(void) {return m_pListener;}
	void SetListener(IDXGCommandListener* pListener) {m_pListener = pListener;}

	//是否处理消息
	bool IsProcessMsg(void) const {return m_bProcessMsg;}
	void AcceptMsg(void) {m_bProcessMsg = true;}
	void UnacceptMsg(void) {m_bProcessMsg = false;}
	void SetAcceptMsg(bool bAccept) {m_bProcessMsg = bAccept;}

	//可操作性
	bool IsEnabled(void) const {return m_bIsEnabled;}
	void SetEnabled(bool bEnabled) {m_bIsEnabled = bEnabled;OnEnabled(m_bIsEnabled);}

	//焦点设置
	void SetFocus(bool bFocus);
	bool IsFocus(void) const {return m_bIsFocus;}

	//可见操作
	bool IsVisible(void) const {return m_bVisible;}
	void SetVisible(bool bVisible) {m_bVisible = bVisible;}
	void Hide(void) {m_bVisible = false;}
	void Show(void) {m_bVisible = true;}

	//坐标尺寸
	const CBLUERect& GetAbsRect(void) const {return m_absoluteRect;}
	int GetAbsXPos(void) const {return m_absoluteRect.m_nLeft;}
	int GetAbsYPos(void) const {return m_absoluteRect.m_nTop;}
	int GetWidth(void) const {return m_absoluteRect.GetWidth();}//获取窗口宽度
	int GetHeight(void) const {return m_absoluteRect.GetHeight();}//获取窗口高度


	/*
	函数名：坐标是否位于本窗口
	功能：略
	说明：无
	参数：无
	返回值：无
	*/
	bool PointIn(const CBLUEPoint& pt) {return m_absoluteRect.PointIn(pt);}

	/*
	函数名：是否已经添加到UI系统中
	功能：略
	说明：无
	参数：无
	返回值：无
	*/
	bool IsUsing(void) const {return m_parent.pManager != BLUENULL;}

	/*
	函数名：是否为顶层窗口
	功能：略
	说明：无
	参数：无
	返回值：无
	*/
	bool IsTopWindow(void) const {return IsUsing() && !m_bWindowParent;}

	/*
	函数名：是否为模式窗口
	功能：略
	说明：无
	参数：无
	返回值：无
	*/
	bool IsModelWindow(void) const {return m_bIsModel;}

	/*
	函数名：添加子窗口
	功能：略
	说明：无
	参数：pWindow 要添加的子窗口对象
	　　　rt 子窗口的位置，相对于本窗口的坐标
	返回值：是否成功
	*/
	bool AddChildWindow(CDXGWindow* pWindow, const CBLUERect& rt);

	/*
	函数名：移除一个子窗口，包括子窗口的子窗口
	功能：略
	说明：无
	参数：pWindow 要移除的子窗口对象
	返回值：是否移除成功，不成功是因为没有该子窗口
	*/
	void RemoveChildWindow(CDXGWindow* pWindow);

	/*
	函数名：移除所有子窗口，包括子窗口的子窗口
	功能：略
	说明：无
	参数：无
	返回值：无
	*/
	void RemoveAllChildWinow(void);

	/*
	函数名：渲染UI，包括所有子窗口
	功能：略
	说明：无
	参数：无
	返回值：无
	*/
	void Render(void);

protected:
	//基本事件
	virtual bool OnCreate(int nWidth, int nHeight) {return true;}
	virtual void OnDestroy(void) {}
	virtual void OnEnabled(bool bIsEnabled) {}//状态变为可用或不可用状态
	virtual void OnFocus(bool bFocus) {}//窗口获取焦点

	//鼠标事件，pt均为相对坐标，相对于父窗口的坐标，如果是顶层窗口，则等价于绝对坐标
	virtual void OnMouseIn(const CBLUEPoint& pt) {}           //鼠标进入窗口
	virtual void OnMouseMove(const CBLUEPoint& pt) {}      //鼠标在窗口内移动
	virtual void OnMouseOut(const CBLUEPoint& pt)              //鼠标离开窗口
	{
		if (m_pCurMouseInWnd)//如果自己当前有子窗口处于鼠标In，则也要触发OnMouseOut事件
		{
			m_pCurMouseInWnd->OnMouseOut(pt);
			m_pCurMouseInWnd = BLUENULL;
		}
	}
	virtual void OnLButtonDown(const CBLUEPoint& pt) {}
	virtual void OnLButtonUp(const CBLUEPoint& pt) {}
	virtual void OnRButtonDown(const CBLUEPoint& pt) {}
	virtual void OnRButtonUp(const CBLUEPoint& pt) {}

	//键盘事件
	virtual void OnKeyDown(int code) {}
	virtual void OnKeyUp(int code) {}
	virtual void OnChar(int code) {}

	//渲染事件
	virtual void OnRender(void) {}

private:
	/*
	函数名：处理鼠标消息
	功能：略
	说明：无
	参数：uMouseMsg 鼠标消息
	　　　pt 鼠标坐标（绝对坐标）
	返回值：无
	*/
	void ProcessMouseMsg(unsigned int uMouseMsg, const CBLUEPoint& pt);

	/*
	函数名：处理键盘消息
	功能：略
	说明：无
	参数：无
	返回值：无
	*/
	void ProcessKeyMsg(unsigned int uKeyMsg, int code);

protected:
	/*
	变量名：命令监听器
	功能：略
	说明：无
	*/
	IDXGCommandListener* m_pListener;

private:
	/*
	变量名：是否处理事件
	功能：略
	说明：无
	*/
	bool m_bProcessMsg;

	/*
	变量名：是否为模式形式（只有为顶层窗口才有用）
	功能：略
	说明：无
	*/
	bool m_bIsModel;

	/*
	变量名：是否可见
	功能：略
	说明：无
	*/
	bool m_bVisible;

	/*
	变量名：是否是焦点窗口
	功能：略
	说明：无
	*/
	bool m_bIsFocus;

	/*
	变量名：是否可用
	功能：略
	说明：无
	*/
	bool m_bIsEnabled;

	/*
	变量名：在界面上的绝对坐标
	功能：略
	说明：无
	*/
	CBLUERect m_absoluteRect;

	/*
	变量名：在界面上的相对坐标
	功能：略
	说明：相对于其父窗口的坐标
	*/
	CBLUERect m_relativeRect;

	/*
	变量名：父窗口或管理器指针
	功能：略
	说明：无
	*/
	union
	{
		CDXGManager* pManager;
		CDXGWindow* pParentWindow;
	} m_parent;

	/*
	变量名：是否有父窗口，如果为false则m_pParent则指向一个CDXGManager对象，否则指向CDXGWindow对象
	功能：略
	说明：无
	*/
	bool m_bWindowParent;

	/*
	变量名：子类窗口链表
	功能：略
	说明：无
	*/
	DXGWINDOWLIST m_childWndList;

	/*
	变量名：当前鼠标所在窗口
	功能：用于触发窗口的OnMouseIn和OnMouseOver事件
	说明：无
	*/
	CDXGWindow* m_pCurMouseInWnd;

	/*
	变量名：当前获取焦点的窗口
	功能：略
	说明：无
	*/
	CDXGWindow* m_pCurFocusWnd;
};