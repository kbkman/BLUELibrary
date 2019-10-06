#pragma once
#include "BLUEDirectX/Def.h"
#include "BLUEDirectX/com.h"
#include <ddraw.h>
#include <dinput.h>
#include <list>
#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "dxguid.lib")
//#pragma comment(lib, "dinput.lib")
#pragma comment(lib, "dinput8.lib")

///////////////E2D引擎错误代码列表////////////////////
#define E2D_FAC  0xA36
#define MAKE_E2DHRESULT(code)              MAKE_HRESULT(1, E2D_FAC, code)
#define E2D_OK                                          S_OK//成功
#define E2D_ERR_OPENFILEFAIL	                MAKE_E2DHRESULT(1)//无法打开文件
#define E2D_ERR_READFILEFAIL                 MAKE_E2DHRESULT(2)	//读取文件失败
#define E2D_ERR_FILEFORMATERROR         MAKE_E2DHRESULT(3)//文件格式不正确
#define E2D_ERR_NOFINDFUNCTION          MAKE_E2DHRESULT(4)//无法找到能够使用的函数
#define E2D_ERR_NOPALETTE	                    MAKE_E2DHRESULT(5)//没有调色板
#define E2D_ERR_NOSUPPORTCOLORBIT   MAKE_E2DHRESULT(6)//不支持的色彩模式

//////////////E2D引擎通用宏/////////////
#define E2D_RGB32(a, clrSrcRed, clrSrcGreen, clrSrcBlue)                     (clrSrcBlue | (clrSrcGreen << 8) | (clrSrcRed << 16) | (a << 24))
#define E2D_RGB15(clrSrcRed, clrSrcGreen, clrSrcBlue)	                        ((clrSrcBlue & 31) | ((clrSrcGreen & 31) << 5) | ((clrSrcRed & 31) << 10))
#define E2D_RGB16(clrSrcRed, clrSrcGreen, clrSrcBlue)	                        ((clrSrcBlue & 31) | ((clrSrcGreen & 63) << 5) | ((clrSrcRed & 31) << 11))
#define E2D_KEYDOWN(kv)	                        (kv & 0x80)//键盘是否被按下
#define E2D_INIT_STRUCT(dxstruct)           ::memset(&dxstruct, 0, sizeof(dxstruct));dxstruct.dwSize = sizeof(dxstruct)

/////////////E2D其他宏////////////////////
#define E2D_PALETTE_MAX_ENTRIES          256//调色板中可容纳项数的最大值

/////////////typedef定义表
typedef void (* PIXELCOLORCOVERFUNCTION)(BYTE*, BYTE*, PALETTEENTRY*, DWORD);
typedef struct _DIKEYBOARDSTATE {UCHAR ks[256];} DIKEYBOARDSTATE;

///////////////声明部分//////////
//class
class CDirectDraw7;
class CDirectDrawSurface7;
class CDirectDrawSurface7Back;
class CDirectDrawSurface7Sprimary;
class CDirectDrawSurface7General;
class CDirectDrawClipper;
class CDirectDrawPalette;
class CDirectInput8;
class CDirectInputDevice8;
class CDirectInputKeyboard;
class CDirectInputMouse;
class CImageAbstract;//位图抽象类
class CBMPBitmap;//BMP文件位图类
class CAnimationFrame;//一个动画帧类
class CAnimationAbstract;//动画抽象类
class CDDrawSprite;//DDraw精灵
class CDDSpriteActorManager;//DDraw精灵管理器


//function
HRESULT E2DCreateDDRAW7(LPDIRECTDRAW7*);//创建DirectDraw7
WORD E2DGetPixelNumberOfBytesFromBit(DWORD);//根据色深得出每个像素的字节数
HRESULT E2DCreateImageFromFile(LPCTSTR, CImageAbstract**);//创建一个图像类并从文件中读取
HRESULT E2DDestroyImage(CImageAbstract*);//卸载一个图像，通常是由E2DCreateImageFromFile函数创建的图像类
HRESULT E2DFindPixelCCFunction(LPDDPIXELFORMAT, DWORD, PIXELCOLORCOVERFUNCTION*);//查找一个像素色彩转换函数
void E2DPixelColorCover32To32(BYTE*, BYTE*, PALETTEENTRY*, DWORD);//一个像素的色彩转换(32转32)
void E2DPixelColorCover24To32(BYTE*, BYTE*, PALETTEENTRY*, DWORD);//一个像素的色彩转换(24转32)
void E2DPixelColorCover8To32(BYTE*, BYTE*, PALETTEENTRY*, DWORD);//一个像素的色彩转换(8转32)
void E2DPixelColorCover24To16(BYTE*, BYTE*, PALETTEENTRY*, DWORD);//一个像素的色彩转换(24转16)
void E2DPixelColorCover8To16(BYTE*, BYTE*, PALETTEENTRY*, DWORD);//一个像素的色彩转换(8转16)
void E2DPixelColorCover24To15(BYTE*, BYTE*, PALETTEENTRY*, DWORD);//一个像素的色彩转换(8转16)
void E2DPixelColorCover8To15(BYTE*, BYTE*, PALETTEENTRY*, DWORD);//一个像素的色彩转换(8转16)
void E2DPixelColorCover8To8(BYTE*, BYTE*, PALETTEENTRY*, DWORD);//一个像素的色彩转换(8转8)
void E2DPixelColorCover24To8(BYTE*, BYTE*, PALETTEENTRY*, DWORD);//一个像素的色彩转换(24转8)
HRESULT E2DCreateDInput8(HINSTANCE, LPDIRECTINPUT8*);//创建DirectInput8

inline DWORD E2DGetSurfacePixel(
	const DWORD x,
	const DWORD y,
	const long lPitch,
	char* const pSurface,
	const DWORD nBytesOfOnePixel
	)
{
	BLUEASSERT(lPitch > 0);
	BLUEASSERT(pSurface);
	BLUEASSERT(nBytesOfOnePixel > 0);

	char* pPixel = pSurface + y * lPitch + x * nBytesOfOnePixel;
	switch (nBytesOfOnePixel)
	{
	case 4:
		return *(DWORD*)pPixel;

	case 2:
		return (DWORD)*(short*)pPixel;

	case 1:
		return (DWORD)*(char*)pPixel;

	default:
		BLUEASSERT(0);
	}
	return 0;
}

inline void E2DSetSurfacePixel(
	const DWORD x,
	const DWORD y,
	const long lPitch,
	char* const pSurface,
	const DWORD nBytesOfOnePixel,
	const DWORD color
	)
{
	BLUEASSERT(lPitch > 0);
	BLUEASSERT(pSurface);
	BLUEASSERT(nBytesOfOnePixel > 0);

	char* pPixel = pSurface + y * lPitch + x * nBytesOfOnePixel;
	switch (nBytesOfOnePixel)
	{
	case 4:
		*(DWORD*)pPixel = color;
		break;

	case 2:
		*(short*)pPixel = (short)color;
		break;

	case 1:
		*(char*)pPixel = (char)color;
		break;

	default:
		BLUEASSERT(0);
	}
}

//////////////////class定义//////////////////
//派生于IDirectDraw7的一个简单扩展
class CDirectDraw7 : public CDXComPtr<IDirectDraw7>
{
public:
	CDirectDraw7(LPDIRECTDRAW7);
	CDirectDraw7(void);
	~CDirectDraw7(void);

public:
	HRESULT GetDisplayModeBase(DWORD&, DWORD&, DWORD&) const;

public:
	HRESULT InitForExclusiveMode(HWND, DWORD, DWORD, DWORD) const;//初始，设置分辨率和协作模式，独占模式下
	HRESULT InitForNormalMode(HWND, DWORD, DWORD, DWORD) const;//初始，窗口模式下

	HRESULT CreateExclusiveModePrimarySurface(DWORD, LPDIRECTDRAWSURFACE7*, LPDIRECTDRAWSURFACE7*) const;//创建独占模式下主表面
	HRESULT CreateNormalModePrimarySurface(LPDIRECTDRAWSURFACE7*) const;//创建窗口模式下的主表面

	HRESULT CreateGeneralSurface(DWORD dwWidth, DWORD dwHeight, DWORD dwFlags, LPDIRECTDRAWSURFACE7* pps) const;//创建一个普通的离屏表面
	HRESULT CreateVideoMemoryGeneralSurface(DWORD dwWidth, DWORD dwHeight, LPDIRECTDRAWSURFACE7* pps) const {return CreateGeneralSurface(dwWidth, dwHeight, DDSCAPS_VIDEOMEMORY, pps);}//创建一个普通的在显存中的离屏表面
	HRESULT CreateSystemMemoryGeneralSurface(DWORD dwWidth, DWORD dwHeight, LPDIRECTDRAWSURFACE7* pps) const {return CreateGeneralSurface(dwWidth, dwHeight, DDSCAPS_SYSTEMMEMORY, pps);}//创建一个普通的在系统内存中的离屏表面

	HRESULT CreateImageSurfaceFromFile(LPCTSTR, DWORD, LPDIRECTDRAWSURFACE7*) const;//创建一个普通离屏表面，该表面直接从文件中加载一个位图
	HRESULT CreateVideoMemoryImageSurfaceFromFile(LPCTSTR lpstrFileName, LPDIRECTDRAWSURFACE7* pps) const {return CreateImageSurfaceFromFile(lpstrFileName, DDSCAPS_VIDEOMEMORY, pps);}//同上，但是是从显存中创建
	HRESULT CreateSystemMemoryImageSurfaceFromFile(LPCTSTR lpstrFileName, LPDIRECTDRAWSURFACE7* pps) const {return CreateImageSurfaceFromFile(lpstrFileName, DDSCAPS_SYSTEMMEMORY, pps);}//同上，但是是从内存中创建

	HRESULT CreateGeneralClipper(LPDIRECTDRAWCLIPPER*) const;

	HRESULT Create8BitPalette(LPDIRECTDRAWPALETTE*) const;
};

//显示表面基类
class CDirectDrawSurface7 : public CDXComPtr<IDirectDrawSurface7>
{
public:
	CDirectDrawSurface7(IDirectDrawSurface7* lp);
	CDirectDrawSurface7(void);
	~CDirectDrawSurface7(void);

public:
	HRESULT ClipWnd(CDirectDraw7& dd7, HWND hWnd) const;//将一个表面栽剪进一个窗口客户区(通常只用于主表面)
	HRESULT ClipOwn(CDirectDraw7& dd7) const;//裁剪自己，这样Blt时可以把传递负数坐标
	HRESULT GetBaseDesc(DWORD& dwWidth, DWORD& dwHeight, DWORD& dwBit) const;//获取Surface的基本信息
	HRESULT GetBaseDesc(DWORD& dwWidth, DWORD& dwHeight) const{DWORD dwBit;return GetBaseDesc(dwWidth, dwHeight, dwBit);}
	HRESULT GetBitCount(DWORD& dwBit) const {DWORD dwWidth, dwHeight;return GetBaseDesc(dwWidth, dwHeight, dwBit);}//获取表面颜色位数
	HRESULT FillColor(DWORD dwFillColor) const;//填充整块色彩
	HRESULT SetSrcColorKey(DWORD dwColor);//设置关键色，用于透明
	HRESULT SetSrcColorKey(DWORD dwLowColor, DWORD dwHighColor);//设置关键色区域，用于透明
	void SetNoSrcColorKey(void) {m_bIsSrcTransparent = false;}
	HRESULT ChangeToGrayMode(void);//将本表面变成灰度图



	/**********所有绘制函数*****************/
	//普通的Blt绘制
	HRESULT Blt(const CDirectDrawSurface7& destSurface, LPCRECT lpDestRect, LPCRECT lpSrcRect, DWORD dwFlags = DDBLT_WAIT, LPDDBLTFX lpDDBltFX = NULL) const
	{
		BLUEASSERT(destSurface);
		BLUEASSERT(*this);
		if (m_bIsSrcTransparent)
			dwFlags |= DDBLT_KEYSRC;
		return destSurface->Blt(const_cast<LPRECT>(lpDestRect), *this, const_cast<LPRECT>(lpSrcRect), dwFlags, lpDDBltFX);
	}
	HRESULT Blt(const CDirectDrawSurface7& destSurface, int nDestLeft, int nDestTop, int nDestWidth, int nDestHeight,int nSrcLeft, int nSrcTop, int nSrcWidth, int nSrcHeight, DWORD dwFlags = DDBLT_WAIT, LPDDBLTFX lpDDBltFX = NULL) const
	{
		RECT rcDest = {nDestLeft, nDestTop, nDestLeft + nDestWidth, nDestTop + nDestHeight};
		RECT rcSrc = {nSrcLeft, nSrcTop, nSrcLeft + nSrcWidth, nSrcTop + nSrcHeight};
		return Blt(destSurface, &rcDest, &rcSrc, dwFlags, lpDDBltFX);
	}
	HRESULT Blt(const CDirectDrawSurface7& destSurface, int nDestLeft = 0, int nDestTop = 0, int nSrcLeft = 0, int nSrcTop = 0, DWORD dwFlags = DDBLT_WAIT, LPDDBLTFX lpDDBltFX = NULL) const;


	/*
	函数名：Alpha混合
	功能：以Alpha混合方式绘制到另一个表面
	说明：暂时不支持缩放
	参数：nAlpha 混合值，0为完全透明，100为不透明
	返回值：是否成功
	*/
	HRESULT AlphaBlendBlt(const CDirectDrawSurface7& destSurface, int nDestLeft, int nDestTop, LPCRECT lpSrcRect, int nAlpha) const;


private:
	bool m_bIsSrcTransparent;//该表面是否为源透明，即绘制到别的表面上时使用关键色实现透明
};

//栽剪区封装
class CDirectDrawClipper : public CDXComPtr<IDirectDrawClipper>
{
public:
	HRESULT ClipRects(DWORD dwRectCount, LPRECT lpClipList) const;
};

//调色板封装
class CDirectDrawPalette : public CDXComPtr<IDirectDrawPalette>
{
};

/**
类名：DDraw精灵类
功能：无
说明：无
版本：1.0.0.1
作者：BLUE
*/
class CDDrawSprite
{
public:
	CDDrawSprite(void);
	virtual ~CDDrawSprite(void) {Destroy();}

public:
	//创建与销毁
	bool Create(const CDirectDrawSurface7& srcSurface, int nCellWidth, int nCellHeight);
	bool Create(const CDirectDrawSurface7& srcSurface)
	{
		DWORD dwWidth, dwHeight;
		if (FAILED(srcSurface.GetBaseDesc(dwWidth, dwHeight)))
			return false;;
		return Create(srcSurface, dwWidth, dwHeight);
	}
	void Destroy(void);

public:
	//分块
	bool Split(int nCellWidth, int nCellHeight);
	int GetCellWidth(void) const {return m_nCellWidth;}
	int GetCellHeight(void) const {return m_nCellHeight;}

	//坐标设置与获取
	int GetPosX(void) const {return m_nPosX;}
	int GetPosY(void) const {return m_nPosY;}
	void SetPosX(int nPosX) {m_nPosX = nPosX;}
	void SetPosY(int nPosY) {m_nPosY = nPosY;}
	void SetPos(int nPosX, int nPosY) {SetPosX(nPosX);SetPosY(nPosY);}
	void MoveX(int nX) {m_nPosX += nX;}
	void MoveY(int nY) {m_nPosY += nY;}
	void Move(int nX, int nY) {MoveX(nX);MoveY(nY);}


	//帧
	int GetTotalFramesNum(void) const {return m_nFrameNumber;}//一共有几帧
	int GetCurFrameIndex(void) const {return m_nCurrentFrameIndex;}//获取当前帧序号
	void SetFrame(int nFrameIndex)
	{
		m_nCurrentFrameIndex = nFrameIndex;
		if (m_nCurrentFrameIndex >= m_nFrameNumber) m_nCurrentFrameIndex = 0;
		if (m_nCurrentFrameIndex < 0) m_nCurrentFrameIndex = m_nFrameNumber - 1;
	}
	void NextFrame(void) {SetFrame(++m_nCurrentFrameIndex);}
	void PrevFrame(void) {SetFrame(--m_nCurrentFrameIndex);}

	//绘制
	void Paint(const CDirectDrawSurface7& destSurface, LPCRECT lpRectDest);
	void Paint(const CDirectDrawSurface7& destSurface, int nPosX, int nPosY);
	void Paint(const CDirectDrawSurface7& destSurface) {Paint(destSurface, m_nPosX, m_nPosY);}

	//活动
	/*
	函数名：作一次活动
	功能：略
	说明：无
	参数：无
	返回值：如果活动完毕，则返回true，否则返回false
	*/
	virtual bool Actor(const CDirectDrawSurface7& surfaceCanvas) {return true;}

protected:
	int GetDrawXFromCurFrameIndex(void) const {return m_nCurrentFrameIndex % m_nCols * m_nCellWidth;}
	int GetDrawYFromCurFrameIndex(void) const {return m_nCurrentFrameIndex / m_nCols * m_nCellHeight;}

private:
	const CDirectDrawSurface7* m_pSrcSurface;  //源表面

	int m_nPosX;     //精灵x坐标
	int m_nPosY;     //精灵y坐标

	int m_nRows;            //源表面分成几行
	int m_nCols;              //源表面分成几列
	int m_nCellWidth;      //分割块宽度
	int m_nCellHeight;     //分割块高度

	int m_nFrameNumber;              //一共有多少帧画面
	int m_nCurrentFrameIndex;     //当前是第几帧画面
};

/**
接口名：精灵管理器监听者
功能：略
说明：无
版本：1.0.0.1
作者：BLUE
*/
class IDDSMListener
{
public:
	//有精灵被移除
	virtual void OnDDSMRemoveSprite(CDDrawSprite* pSprite) = 0;
};

/**
类名：精灵活动管理器
功能：无
说明：无
版本：1.0.0.1
作者：BLUE
*/
class CDDSpriteActorManager
{
	typedef std::list<CDDrawSprite*> DSPRITECOLL;

public:
	CDDSpriteActorManager(void);
	virtual ~CDDSpriteActorManager(void);

public:
	//设置监听器
	void SetListener(IDDSMListener* pListener) {m_pListener = pListener;}

	//是否有精灵要运行
	bool HasSprite(void) const {return !m_spriteColl.empty();}

	//加入一个精灵
	void Add(CDDrawSprite& sprite) {m_spriteColl.push_back(&sprite);}

	//清除所有精灵
	void Clear(void);

	//运行所有精灵
	void Actor(const CDirectDrawSurface7& surfaceCanvas);

private:
	IDDSMListener* m_pListener;
	DSPRITECOLL m_spriteColl;
};

/////////////图像类///////////////
//抽象图像类
class CImageAbstract
{
public:
	CImageAbstract(void);
	virtual ~CImageAbstract(void);

public:
	bool IsValidate(void) {return (m_dwWidth > 0 && m_dwHeight > 0 && m_dwBit > 0 && m_pBitData != NULL);}//验证位图是否有效
	bool IsHasPal(void) {return (m_pPalette != NULL && m_dwPALCount > 0);}//是否拥有调色板

	DWORD GetWidth(void) {return m_dwWidth;}
	DWORD GetHeight(void) {return m_dwHeight;}
	DWORD GetBit(void) {return m_dwBit;}
	DWORD GetPALCount(void) {return m_dwPALCount;}

	BYTE* GetBitData(void) {return m_pBitData;}
	PALETTEENTRY* GetPalette(void) {return m_pPalette;}

public:
	virtual HRESULT LoadFile(LPCTSTR lpstrFileName) = 0;//从文件加载位图，派生类必须重写
	virtual HRESULT LoadFileOnlyPalette(LPCTSTR lpstrFileName) = 0;//从文件加载位图调色板，派生类必须重写
	virtual HRESULT Unload(void) = 0;//卸载，派生类必须重写

protected:
	DWORD m_dwWidth;//位图宽
	DWORD m_dwHeight;//位图高
	DWORD m_dwBit;//位图的比特位数
	DWORD m_dwPALCount;//调色板项数量

	BYTE* m_pBitData;//指向位图数据
	PALETTEENTRY* m_pPalette;//指向位图调色板
};

//读取BMP文件的位图类
class CBMPBitmap : public CImageAbstract
{
public:
	CBMPBitmap(void);
	virtual ~CBMPBitmap(void);

public:
	virtual HRESULT LoadFile(LPCTSTR lpstrFileName);
	virtual HRESULT LoadFileOnlyPalette(LPCTSTR lpstrFileName);
	virtual HRESULT Unload(void);

private:
	HRESULT _loadFile(LPCTSTR lpstrFileName, bool bOnlyLoadPalette);
};

class CDirectInput8 : public CDXComPtr<IDirectInput8>
{
public:
	HRESULT CreateKeyboard(LPDIRECTINPUTDEVICE8* pp);
	HRESULT CreateMouse(LPDIRECTINPUTDEVICE8* pp);
};

class CDirectInputDevice8 : public CDXComPtr<IDirectInputDevice8>
{
public:
	virtual HRESULT Init(HWND hWnd, DWORD dwCLFlags, LPCDIDATAFORMAT pcdiFormat);
	HRESULT Start(void) {BLUEASSERT(m_p);return m_p->Acquire();}
	HRESULT Stop(void) {BLUEASSERT(m_p);return m_p->Unacquire();}
};


class CDirectInputKeyboard : public CDirectInputDevice8
{
public:
	DIKEYBOARDSTATE m_state;

public:
	virtual HRESULT Init(HWND hWnd, LPCDIDATAFORMAT pcdiFormat = NULL);

public:
	UCHAR IsKeyDown(DWORD index)
	{
		return E2D_KEYDOWN(m_state.ks[index]);
	}
	HRESULT GS(void)
	{
		BLUEASSERT(m_p);
		return m_p->GetDeviceState(sizeof(m_state), &m_state);
	}	
	HRESULT GSSafe(void)
	{
		HRESULT hr;

		if (FAILED(hr = GS()))
		{
			if (hr == DIERR_INPUTLOST ||
				hr == DIERR_NOTACQUIRED)
			{
				if (FAILED(hr = Start()))
					return hr;

				return GS();
			}
		}
		return hr;
	}
};

class CDirectInputMouse : public CDirectInputDevice8
{
public:
	DIMOUSESTATE m_state;

public:
	virtual HRESULT Init(HWND hWnd, LPCDIDATAFORMAT pcdiFormatpcdiFormat = NULL);

public:
	HRESULT GS(void)
	{
		BLUEASSERT(m_p);
		return m_p->GetDeviceState(sizeof(m_state), &m_state);
	}
	HRESULT GSSafe(void)
	{
		HRESULT hr;

		if (FAILED(hr = GS()))
		{
			if (hr == DIERR_INPUTLOST ||
				hr == DIERR_NOTACQUIRED)
			{
				if (FAILED(hr = Start()))
					return hr;

				return GS();
			}
		}
		return hr;
	}
};