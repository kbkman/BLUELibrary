#pragma once
#include "BLUEDirectX/Def.h"
#include "BLUEDirectX/Game.h"
#include "BLUEDirectX/Engine2D.h"

//#define DXGAME2D_USE_EXCLUSIVE_MODE

#ifndef DXGAME2D_SCREEN_WIDTH
#define DXGAME2D_SCREEN_WIDTH		0//定义屏幕宽，0表示使用当前宽(不改变)
#endif

#ifndef DXGAME2D_SCREEN_HEIGHT
#define DXGAME2D_SCREEN_HEIGHT		0//定义屏幕高，0表示使用当前高(不改变)
#endif

#ifndef DXGAME2D_SCREEN_BIT
#define DXGAME2D_SCREEN_BIT			0//定义屏幕色深，0表示使用当前色深(不改变)
#endif

#ifndef DXGAME2D_EXCLUSIVE_MODE_BACKBUFFERCOUNT
#define DXGAME2D_EXCLUSIVE_MODE_BACKBUFFERCOUNT			2//定义主表面缓冲数量，只有独占模式才有用
#endif

#ifndef DXGAME2D_COMPUTECLIENTRECT_TYPE
#define DXGAME2D_COMPUTECLIENTRECT_TYPE							2//0为不使用任何计算方式，1为窗口计算方式(默认)，2为全屏幕和窗口计算方式
#endif

///////////////DXGAMERUN2D引擎错误代码列表////////////////////
#define DXGR2D_FAC  0xA88
#define MAKE_DXGR2DHRESULT(code)                MAKE_HRESULT(1, DXGR2D_FAC, code)
#define DXGR2D_OK                                            S_OK//成功
#define DXGR2D_ERR_ENDGAME                         MAKE_DXGR2DHRESULT(1)//结束游戏
#define DXGR2D_ERR_NOSUPPORTCOLORBIT     MAKE_DXGR2DHRESULT(2)//不支持的深色

////////////////////////////////

class CDXGameRun2D :
	public CDXGameRun
{
public:
	/*
	枚举名：主表面绘制方法
	功能：略
	说明：无
	枚举：imm 直接将后备表面绘制到到主表面（在独占模式时推荐使用该方法）
	　　　windowstart 将后备表面绘制到窗口起始点（在窗口模式时，推荐使用该方法）
	　　　fillwindow 将后备表面填充到窗口中，这样会有缩放作用（不推荐使用）
	*/
	enum PSDRAWTYPE {imm, windowstart, fillwindow};

public:
	CDXGameRun2D(void);
	~CDXGameRun2D(void);

public:
	HRESULT SetPaletteToPrimarySurface(LPCTSTR lpstrFileName);//从位图文件中加载调色板，并为主表面设置调色板
	HRESULT SetPaletteToPrimarySurface(LPPALETTEENTRY pPalette, DWORD dwPalCount);//主表面设置调色板

public:
#ifdef DXGAME2D_USE_EXCLUSIVE_MODE//独占模式下
	HRESULT Flip(void) {return m_ddsprimary->Flip(NULL, DDFLIP_WAIT);}//页面切换(在独占模式下)

#else//非独占模式下
#if (DXGAME2D_COMPUTECLIENTRECT_TYPE == 0)//不作任何计算
	HRESULT Flip(void) {return m_ddsprimary->Blt(NULL, m_ddsback, NULL,  DDBLT_WAIT, NULL);}//页面切换(在窗口模式下)
#else//作一些计算
	HRESULT Flip(void) {return m_ddsprimary->Blt(&m_rcMainWndClient, m_ddsback, NULL,  DDBLT_WAIT, NULL);}//页面切换(在窗口模式下)
#endif

#endif

protected:
	virtual HRESULT RunInitBeforeLoop(void);//在进入游戏循环之前初始
	virtual HRESULT RunEndAfterLoop(void);//在退出游戏循环之后释放
	virtual void ComputeClientRect(void);//计算客户区矩形(相对于屏幕)

protected:
	CDirectDraw7 m_dd7;//IDirectDraw7接口
	CDirectDrawSurface7 m_ddsprimary;//主表面接口
	CDirectDrawSurface7 m_ddsback;//主表面后备接口
	CDirectDrawPalette m_ddpprimary;//主表面使用的调色板

	CDirectInput8 m_di8;//IDirectInput8接口
	CDirectInputKeyboard m_diKeyboard;//键盘
	CDirectInputMouse m_diMouse;//鼠标

private:
	PSDRAWTYPE m_PSDrawType;

};