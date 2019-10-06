#include <BLUEDirectX\Engine2D.h>

HRESULT E2DCreateDDRAW7(LPDIRECTDRAW7* pp)
{
	BLUEASSERT(pp);
	return ::DirectDrawCreateEx(NULL,
		(void**)pp,
		IID_IDirectDraw7,
		NULL);
}

WORD E2DGetPixelNumberOfBytesFromBit(DWORD dwBit)
{
	if (dwBit == 32)//32位图是4个字节一像素
		return 4;

	if (dwBit == 24)//24位图是3个字节一像素
		return 3;

	if (dwBit == 8)//小于等于8位图，一个像素一个字节
		return 1;

	if (dwBit == 16 || dwBit == 15)//16位图是2个字节一像素
		return 2;

	BLUEASSERT(0);//应该不可能存在其他可能性
	return 0;
}

HRESULT E2DCreateImageFromFile(LPCTSTR lpstrFileName, CImageAbstract** pp)
{
	HRESULT hr;
	CBMPBitmap* p;

	p = new CBMPBitmap();
	if (FAILED(hr = p->LoadFile(lpstrFileName)))
		return hr;

	*pp = static_cast<CImageAbstract*>(p);
	return S_OK;
}

HRESULT E2DDestroyImage(CImageAbstract* p)
{
	BLUEASSERT(p);
	delete p;
	return S_OK;
}

HRESULT E2DFindPixelCCFunction(LPDDPIXELFORMAT lpDestDPF, DWORD dwSrcBit, PIXELCOLORCOVERFUNCTION* ppfn)
{
	BLUEASSERT(lpDestDPF);

	//必须是RGB模式或者是带调色板模式
	if (!(lpDestDPF->dwFlags & DDPF_RGB) &&
		!(lpDestDPF->dwFlags & DDPF_PALETTEINDEXED8))
		return E2D_ERR_NOFINDFUNCTION;

	//验证目标像素占位
	switch (lpDestDPF->dwRGBBitCount)
	{
	case 32://转换目标为32位的
		{
			switch (dwSrcBit)
			{
			case 32:
				*ppfn = ::E2DPixelColorCover32To32;//32->32
				return S_OK;

			case 24:
				*ppfn = ::E2DPixelColorCover24To32;//24->32
				return S_OK;


			case 8:
				*ppfn = ::E2DPixelColorCover8To32;//8->32
				return S_OK;

			default:
				break;
			}
		}

	case 16://转换目标为16位的
		{
			switch (dwSrcBit)
			{
			case 24:
				*ppfn = ::E2DPixelColorCover24To16;
				return S_OK;
			case 8:
				*ppfn = ::E2DPixelColorCover8To16;
				return S_OK;
			default:
				break;
			}
		}
		break;

	case 15://转换目标为15位的(其实是16位模式，但只使用15个位)
		{
			switch (dwSrcBit)
			{
			case 24:
				*ppfn = ::E2DPixelColorCover24To15;
				return S_OK;
			case 8:
				*ppfn = ::E2DPixelColorCover8To15;
				return S_OK;
			default:
				break;
			}
		}
		break;

	case 8://转换目标为8位的
		{
			switch (dwSrcBit)
			{
			case 8:
				*ppfn = ::E2DPixelColorCover8To8;//8->8
				return S_OK;

			case 24:
				*ppfn = ::E2DPixelColorCover24To8;//24->8
				return S_OK;

			default:
				break;
			}
		}
		break;

	default:
		break;
	}

	return E2D_ERR_NOFINDFUNCTION;
}

void E2DPixelColorCover32To32(
							  BYTE* pdest,
							  BYTE* psrc,
							  PALETTEENTRY* psrcPal,
							  DWORD dwSrcPalCount)
{
	BLUEASSERT(pdest);
	BLUEASSERT(psrc);

	::memcpy(pdest, psrc, 4);
}

inline void E2DPixelColorCover24To32(
							  BYTE* pdest,
							  BYTE* psrc,
							  PALETTEENTRY* psrcPal,
							  DWORD dwSrcPalCount)
{
	BLUEASSERT(pdest);
	BLUEASSERT(psrc);

	pdest[0] = psrc[0];
	pdest[1] = psrc[1];
	pdest[2] = psrc[2];
	pdest[3] = 0;
}

inline void E2DPixelColorCover8To32(
							  BYTE* pdest,
							  BYTE* psrc,
							  PALETTEENTRY* psrcPal,
							  DWORD dwSrcPalCount)
{
	BLUEASSERT(pdest);
	BLUEASSERT(psrc);
	BLUEASSERT(psrcPal);
	BLUEASSERT(dwSrcPalCount > 0);

	DWORD index = psrc[0];
	if (index >=  dwSrcPalCount)
		index = 0;

	pdest[0] = psrcPal[index].peBlue;
	pdest[1] = psrcPal[index].peGreen;
	pdest[2] = psrcPal[index].peRed;
	pdest[3] = 0;
}

void E2DPixelColorCover24To16(
							  BYTE* pdest,
							  BYTE* psrc,
							  PALETTEENTRY* psrcPal,
							  DWORD dwSrcPalCount)
{
	BLUEASSERT(pdest);
	BLUEASSERT(psrc);

	WORD* pw = (WORD*)pdest;
	BYTE r = psrc[2];
	BYTE g = psrc[1];
	BYTE b = psrc[0];
	*pw = E2D_RGB16(r, g, b);
}

void E2DPixelColorCover8To16(
							  BYTE* pdest,
							  BYTE* psrc,
							  PALETTEENTRY* psrcPal,
							  DWORD dwSrcPalCount)
{
	BLUEASSERT(pdest);
	BLUEASSERT(psrc);
	BLUEASSERT(psrcPal);
	BLUEASSERT(dwSrcPalCount > 0);

	DWORD index = psrc[0];
	WORD* pw = (WORD*)pdest;

	if (index >=  dwSrcPalCount)
		index = 0;

	*pw = E2D_RGB16(
		psrcPal[index].peRed,
		psrcPal[index].peGreen,
		psrcPal[index].peBlue);
}

void E2DPixelColorCover24To15(
							  BYTE* pdest,
							  BYTE* psrc,
							  PALETTEENTRY* psrcPal,
							  DWORD dwSrcPalCount)
{
	BLUEASSERT(pdest);
	BLUEASSERT(psrc);

	WORD* pw = (WORD*)pdest;
	BYTE r = psrc[2] > 31 ? 31 : psrc[2];
	BYTE g = psrc[1] > 31 ? 31 : psrc[1];
	BYTE b = psrc[0] > 31 ? 31 : psrc[0];
	*pw = E2D_RGB15(r, g, b);
}

void E2DPixelColorCover8To15(
							  BYTE* pdest,
							  BYTE* psrc,
							  PALETTEENTRY* psrcPal,
							  DWORD dwSrcPalCount)
{
	BLUEASSERT(pdest);
	BLUEASSERT(psrc);
	BLUEASSERT(psrcPal);
	BLUEASSERT(dwSrcPalCount > 0);

	DWORD index = psrc[0];
	WORD* pw = (WORD*)pdest;

	if (index >=  dwSrcPalCount)
		index = 0;

	*pw = E2D_RGB15(
		psrcPal[index].peRed,
		psrcPal[index].peGreen,
		psrcPal[index].peBlue);
}

void E2DPixelColorCover8To8(
							  BYTE* pdest,
							  BYTE* psrc,
							  PALETTEENTRY* psrcPal,
							  DWORD dwSrcPalCount)
{
	BLUEASSERT(pdest);
	BLUEASSERT(psrc);

	pdest[0] = psrc[0];
}

void E2DPixelColorCover24To8(
							  BYTE* pdest,
							  BYTE* psrc,
							  PALETTEENTRY* psrcPal,
							  DWORD dwSrcPalCount)
{
	BLUEASSERT(pdest);
	BLUEASSERT(psrc);

	DWORD dwColor = E2D_RGB32(0, psrc[0], psrc[1], psrc[2]);
	pdest[0] = (BYTE)(dwColor % 256);
}

HRESULT E2DCreateDInput8(HINSTANCE hInst, LPDIRECTINPUT8* pp)
{
	BLUEASSERT(pp);
	return ::DirectInput8Create(
		hInst,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)pp,
		NULL);
}

////////////////////////////CDirectDraw7/////////////////////////////
CDirectDraw7::CDirectDraw7(LPDIRECTDRAW7 lpdd7) : CDXComPtr(lpdd7)
{
}

CDirectDraw7::CDirectDraw7(void)
{
}

CDirectDraw7::~CDirectDraw7(void)
{
}

HRESULT CDirectDraw7::GetDisplayModeBase(
	DWORD& dwScreenWidth,
	DWORD& dwScreenHeight,
	DWORD& dwBit) const
{
	BLUEASSERT(m_p);

	HRESULT hr;
	DDSURFACEDESC2 ddsd;

	E2D_INIT_STRUCT(ddsd);
	if (FAILED(hr = m_p->GetDisplayMode(&ddsd)))
		return hr;

	dwScreenWidth = ddsd.dwWidth;
	dwScreenHeight = ddsd.dwHeight;
	dwBit = ddsd.ddpfPixelFormat.dwBumpBitCount;

	return hr;
}

HRESULT CDirectDraw7::InitForExclusiveMode(HWND hMainWnd,
										   DWORD dwWidth,
										   DWORD dwHeight,
										   DWORD dwBit) const
{
	BLUEASSERT(m_p);
	BLUEASSERT(hMainWnd);

	HRESULT hr;

	if (FAILED(hr = m_p->SetCooperativeLevel(hMainWnd,
		DDSCL_ALLOWMODEX |
		DDSCL_FULLSCREEN |
		DDSCL_EXCLUSIVE |
		DDSCL_ALLOWREBOOT)))
		return hr;

	return m_p->SetDisplayMode(dwWidth, dwHeight, dwBit, 0, 0);
}

HRESULT CDirectDraw7::InitForNormalMode(HWND hMainWnd,
										DWORD dwWidth,
										DWORD dwHeight,
										DWORD dwBit) const
{
	//窗口模式
	HRESULT hr;

	if (FAILED(hr = m_p->SetCooperativeLevel(hMainWnd, DDSCL_NORMAL)))
		return hr;

	return hr;//m_p->SetDisplayMode(dwWidth, dwHeight, dwBit, 0, 0);
}

HRESULT CDirectDraw7::CreateExclusiveModePrimarySurface(
	DWORD dwBackBufferCount,
	LPDIRECTDRAWSURFACE7* pps,
	LPDIRECTDRAWSURFACE7* ppsback) const
{
	BLUEASSERT(m_p);
	BLUEASSERT(pps);
	BLUEASSERT(ppsback);

	HRESULT hr;
	DDSURFACEDESC2 ddsd;
	DDSCAPS2 ddscaps = {0};

	E2D_INIT_STRUCT(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
	ddsd.dwBackBufferCount = dwBackBufferCount;

	if (FAILED(hr = m_p->CreateSurface(&ddsd, pps, NULL)))
		return hr;

	ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
	if (FAILED(hr = (*pps)->GetAttachedSurface(&ddscaps, ppsback)))
	{
		(*pps)->Release();
		*pps = NULL;
	}

	return hr;
}

HRESULT CDirectDraw7::CreateNormalModePrimarySurface(LPDIRECTDRAWSURFACE7* pps) const
{
	BLUEASSERT(m_p);
	BLUEASSERT(pps);

	HRESULT hr;
	DDSURFACEDESC2 ddsd;
	E2D_INIT_STRUCT(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	return hr = m_p->CreateSurface(&ddsd, pps, NULL);
}

HRESULT CDirectDraw7::CreateGeneralSurface(DWORD dwWidth, DWORD dwHeight, DWORD dwFlags, LPDIRECTDRAWSURFACE7* pps) const
{
	BLUEASSERT(m_p);
	BLUEASSERT(pps);

	DDSURFACEDESC2 ddsd;
	E2D_INIT_STRUCT(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.dwWidth = dwWidth;
	ddsd.dwHeight = dwHeight;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | dwFlags;
	
	return m_p->CreateSurface(&ddsd, pps, NULL);
}

HRESULT CDirectDraw7::CreateImageSurfaceFromFile(LPCTSTR lpstrFileName, DWORD dwFlags, LPDIRECTDRAWSURFACE7* pps) const
{
	BLUEASSERT(m_p);
	BLUEASSERT(pps);

	HRESULT hr;
	CImageAbstract* pBMP;
	DWORD dwBmpWidth;
	DWORD dwBmpHeight;
	DWORD dwBmpBit;

	//先从文件中加载
	if (FAILED(hr = ::E2DCreateImageFromFile(lpstrFileName, &pBMP)))
		return hr;

	dwBmpWidth = pBMP->GetWidth();
	dwBmpHeight = pBMP->GetHeight();
	dwBmpBit = pBMP->GetBit();

	//创建一个和位图一样大小的表面
	if (SUCCEEDED(hr = CreateGeneralSurface(
		dwBmpWidth,
		dwBmpHeight,
		dwFlags, pps)))
	{
		DDSURFACEDESC2 ddsd;//表面信息
		BYTE* pBitmapData;//指向位图实际的数据
		BYTE* pSurfaceData;//指向表面实际数据
		WORD wNumOfBytesOnePixForBitmapData;//位图每个像素需要多少字节
		WORD wNumOfBytesOnePixForSurfaceData;//表面每个像素需要多少字节
		PALETTEENTRY* pBitmapPal;//指向位图的调色板
		DWORD dwBitmapPalCount;//位图调色板数量
		CDirectDrawSurface7 surface(*pps);//用CDirectDrawSurface7来代表*pps
		DDPIXELFORMAT dpfSurface;//表面的像素格式
		PIXELCOLORCOVERFUNCTION pfnPixelCC;//单个像素色彩转换函数
		DWORD i, j;//计数器

		//锁住表面
		E2D_INIT_STRUCT(ddsd);//初始ddsd
		if (FAILED(hr = surface->Lock(NULL, &ddsd,
			DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,
			NULL)))
			goto ENDFUNCTION;

		//获取表面像素信息
		E2D_INIT_STRUCT(dpfSurface);//初始dpfSurface
		if (FAILED(hr = surface->GetPixelFormat(&dpfSurface)))
		{
			//表面解锁
			hr = surface->Unlock(NULL);
			goto ENDFUNCTION;
		}

		//得到单像素转换函数
		if (FAILED(hr = ::E2DFindPixelCCFunction(&dpfSurface, dwBmpBit, &pfnPixelCC)))
		{
			//表面解锁
			hr = surface->Unlock(NULL);
			goto ENDFUNCTION;
		}

		wNumOfBytesOnePixForBitmapData = ::E2DGetPixelNumberOfBytesFromBit(dwBmpBit);
		wNumOfBytesOnePixForSurfaceData = ::E2DGetPixelNumberOfBytesFromBit(dpfSurface.dwRGBBitCount);
		pBitmapPal = pBMP->GetPalette();
		dwBitmapPalCount = pBMP->GetPALCount();
		pBitmapData = (BYTE*)pBMP->GetBitData();
		pSurfaceData = (BYTE*)ddsd.lpSurface;

		for (j = 0; j < dwBmpHeight; j++)
		{
			for (i = 0; i < dwBmpWidth; i++)
			{
				BYTE* pCurBitmapPixel;//当前位图像素的指针
				BYTE* pCurSurfacePixel;//当前表面像素的指针

				//得出当前要复制的像素分别在位图和表面中的地址
				pCurBitmapPixel = pBitmapData + (j * dwBmpWidth + i) * wNumOfBytesOnePixForBitmapData;
				pCurSurfacePixel = pSurfaceData + ddsd.lPitch * j + i * wNumOfBytesOnePixForSurfaceData;

				//复制像素
				pfnPixelCC(pCurSurfacePixel, pCurBitmapPixel, pBitmapPal, dwBitmapPalCount);
			}
		}

		//表面解锁
		hr = surface->Unlock(NULL);
	}

ENDFUNCTION:
	::E2DDestroyImage(pBMP);
	return hr;

}

HRESULT CDirectDraw7::CreateGeneralClipper(LPDIRECTDRAWCLIPPER* ppc) const
{
	BLUEASSERT(m_p);
	BLUEASSERT(ppc);

	return m_p->CreateClipper(0, ppc, NULL);
}

HRESULT CDirectDraw7::Create8BitPalette(LPDIRECTDRAWPALETTE* ppPal) const
{
	BLUEASSERT(m_p);
	BLUEASSERT(ppPal);

	PALETTEENTRY palEntries[256];
	DWORD i;

	//随机生成调色板项
	for (i = 1; i < 255; i++)
	{
		palEntries[i].peRed = ::rand() % 256;
		palEntries[i].peGreen = ::rand() % 256;
		palEntries[i].peBlue = ::rand() % 256;
		palEntries[i].peFlags = PC_NOCOLLAPSE;
	}

	//第一项为黑色
	palEntries[0].peRed = 0;
	palEntries[0].peGreen = 0;
	palEntries[0].peBlue = 0;
	palEntries[0].peFlags = PC_NOCOLLAPSE;

	//最后一项为白色
	palEntries[255].peRed = 255;
	palEntries[255].peGreen = 255;
	palEntries[255].peBlue = 255;
	palEntries[255].peFlags = PC_NOCOLLAPSE;

	return m_p->CreatePalette(
		DDPCAPS_8BIT | DDPCAPS_ALLOW256 | DDPCAPS_INITIALIZE,
		palEntries,
		ppPal,
		NULL);
}

/////////////////////////CDirectDrawSurface7//////////////////////////
CDirectDrawSurface7::CDirectDrawSurface7(IDirectDrawSurface7* lp) :
CDXComPtr(lp)
{
}

CDirectDrawSurface7::CDirectDrawSurface7(void) :
m_bIsSrcTransparent(false)
{
}

CDirectDrawSurface7::~CDirectDrawSurface7(void)
{
}

HRESULT CDirectDrawSurface7::ClipWnd(CDirectDraw7& dd7, HWND hWnd) const
{
	BLUEASSERT(m_p);
	BLUEASSERT(hWnd);

	HRESULT hr;
	CDirectDrawClipper ddClpWnd;

	//创建一个用于剪裁窗口客户区的剪裁区
	if (FAILED(hr = dd7.CreateGeneralClipper(&ddClpWnd)))
		return hr;

	//裁剪区和窗口挂钩
	if (FAILED(hr = ddClpWnd->SetHWnd(0, hWnd)))
		return hr;

	//将裁剪区选入显示表面
	return m_p->SetClipper(ddClpWnd);
}

HRESULT CDirectDrawSurface7::ClipOwn(CDirectDraw7& dd7) const
{
	BLUEASSERT(m_p);

	HRESULT hr;
	CDirectDrawClipper ddClp;
	DWORD dwWidth;
	DWORD dwHeight;
	DWORD dwBit;
	RECT rcClip;

	//创建一个用于剪裁窗口客户区的剪裁区
	if (FAILED(hr = dd7.CreateGeneralClipper(&ddClp)))
		return hr;
	
	if (FAILED(hr = GetBaseDesc(dwWidth, dwHeight, dwBit)))
		return hr;

	rcClip.left = 0;
	rcClip.top = 0;
	rcClip.right = dwWidth;
	rcClip.bottom = dwHeight;

	if (FAILED(hr = ddClp.ClipRects(1, &rcClip)))
		return hr;

	return m_p->SetClipper(ddClp);
}

HRESULT CDirectDrawSurface7::GetBaseDesc(DWORD& dwWidth, DWORD& dwHeight, DWORD& dwBit) const
{
	BLUEASSERT(m_p);

	HRESULT hr;
	DDSURFACEDESC2 ddsd;

	E2D_INIT_STRUCT(ddsd);
	if (FAILED(hr = m_p->GetSurfaceDesc(&ddsd)))
		return hr;

	dwWidth = ddsd.dwWidth;
	dwHeight = ddsd.dwHeight;
	dwBit = ddsd.ddpfPixelFormat.dwBumpBitCount;

	return hr;
}

HRESULT CDirectDrawSurface7::FillColor(DWORD dwFillColor) const
{
	BLUEASSERT(m_p);

	DDBLTFX ddbltfx;
	E2D_INIT_STRUCT(ddbltfx);
	ddbltfx.dwFillColor = dwFillColor;
	return m_p->Blt(NULL, NULL, NULL,
		DDBLT_COLORFILL | DDBLT_WAIT,
		&ddbltfx);
}

HRESULT CDirectDrawSurface7::SetSrcColorKey(DWORD dwColor)
{
	BLUEASSERT(m_p);

	DDCOLORKEY ck;
	ck.dwColorSpaceLowValue = dwColor;
	ck.dwColorSpaceHighValue = dwColor;

	HRESULT hr = m_p->SetColorKey(DDCKEY_SRCBLT, &ck);
	if (SUCCEEDED(hr))
		m_bIsSrcTransparent = true;

	return hr;
}

HRESULT CDirectDrawSurface7::SetSrcColorKey(DWORD dwLowColor, DWORD dwHighColor)
{
	BLUEASSERT(m_p);

	DDCOLORKEY ck;
	ck.dwColorSpaceLowValue = dwLowColor;
	ck.dwColorSpaceHighValue = dwHighColor;

	HRESULT hr = m_p->SetColorKey(DDCKEY_SRCBLT | DDCKEY_COLORSPACE, &ck);
	if (SUCCEEDED(hr))
		m_bIsSrcTransparent = true;

	return hr;
}

HRESULT CDirectDrawSurface7::Blt(const CDirectDrawSurface7& destSurface,
	int nDestLeft, int nDestTop,
	int nSrcLeft, int nSrcTop,
	DWORD dwFlags, LPDDBLTFX lpDDBltFX) const
{
	DWORD dwWidth, dwHeight;
	HRESULT hr;

	//先获取表面长宽
	if (FAILED(hr = GetBaseDesc(dwWidth, dwHeight)))
		return hr;

	//根据表面长宽绘制
	return Blt(destSurface,
		nDestLeft, nDestTop, dwWidth, dwHeight,
		nSrcLeft, nSrcTop, dwWidth, dwHeight,
		dwFlags, lpDDBltFX);
}

HRESULT CDirectDrawSurface7::ChangeToGrayMode(void)
{
	DDSURFACEDESC2 ddsd;
	HRESULT hr;

	E2D_INIT_STRUCT(ddsd);
	DWORD dwSrcWidth, dwSrcHeight, dwSrcBitCount, dwBitCount;
	DWORD dwSurfaceBytesOfOnePixel, dwDestSurfaceBytesOfOnePixel;

	if (FAILED(hr = GetBitCount(dwBitCount)))
		return hr;

	if (FAILED(hr = GetBaseDesc(dwSrcWidth, dwSrcHeight, dwSrcBitCount)))
		return hr;

	DDCOLORKEY ck;
	bool bIsHasColorKey;
	if (FAILED(hr = m_p->GetColorKey(DDCKEY_SRCBLT, &ck)))
	{
		if (hr != DDERR_NOCOLORKEY)
			return hr;
		bIsHasColorKey = false;
	}
	else
	{
		bIsHasColorKey = true;
	}

	dwSurfaceBytesOfOnePixel = ::E2DGetPixelNumberOfBytesFromBit(dwSrcBitCount);
	dwDestSurfaceBytesOfOnePixel = ::E2DGetPixelNumberOfBytesFromBit(dwBitCount);

	DWORD clr = 0;
	DWORD r = 0, g = 0, b = 0;
	DWORD ave = 0;//平均
	if (SUCCEEDED(hr = m_p->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL)))
	{
		for (DWORD x = 0; x < ddsd.dwWidth; x++)
		{
			for (DWORD y = 0; y < ddsd.dwHeight; y++)
			{
				clr = ::E2DGetSurfacePixel(x, y, ddsd.lPitch, (char*)ddsd.lpSurface, dwSurfaceBytesOfOnePixel);
				if (bIsHasColorKey && clr >= ck.dwColorSpaceLowValue && clr <= ck.dwColorSpaceHighValue)
					continue;
				r = (clr & 0x00FF0000) >> 16;
				g = (clr & 0x0000FF00) >> 8;
				b = clr & 0x000000FF;
				ave = (r + g + b) / 3;
				clr = E2D_RGB32(0, ave, ave, ave);
				::E2DSetSurfacePixel(x, y, ddsd.lPitch, (char*)ddsd.lpSurface, dwDestSurfaceBytesOfOnePixel, clr);
			}
		}
		m_p->Unlock(NULL);
	}

	return hr;

}

HRESULT CDirectDrawSurface7::AlphaBlendBlt(const CDirectDrawSurface7& destSurface, int nDestLeft, int nDestTop, LPCRECT lpSrcRect, int nAlpha) const
{
	DDSURFACEDESC2 ddsdSrc, ddsdDest;
	HRESULT hr;

	E2D_INIT_STRUCT(ddsdSrc);
	E2D_INIT_STRUCT(ddsdDest);

	DWORD dwSrcWidth, dwSrcHeight, dwSrcBitCount, dwDestBitCount;
	DWORD dwSrcSurfaceBytesOfOnePixel, dwDestSurfaceBytesOfOnePixel;

	if (FAILED(hr = destSurface.GetBitCount(dwDestBitCount)))
		return hr;

	if (FAILED(hr = GetBaseDesc(dwSrcWidth, dwSrcHeight, dwSrcBitCount)))
		return hr;

	DDCOLORKEY ck;
	bool bIsHasColorKey;
	if (FAILED(hr = m_p->GetColorKey(DDCKEY_SRCBLT, &ck)))
	{
		if (hr != DDERR_NOCOLORKEY)
			return hr;
		bIsHasColorKey = false;
	}
	else
	{
		bIsHasColorKey = true;
	}

	dwSrcSurfaceBytesOfOnePixel = ::E2DGetPixelNumberOfBytesFromBit(dwSrcBitCount);
	dwDestSurfaceBytesOfOnePixel = ::E2DGetPixelNumberOfBytesFromBit(dwDestBitCount);

	DWORD clrSrc = 0;
	DWORD clrDest = 0;
	DWORD clrSrcRed = 0, clrSrcGreen = 0, clrSrcBlue = 0;
	DWORD clrDestRed = 0, clrDestGreen = 0, clrDestBlue = 0;
	if (nAlpha < 0) nAlpha = 0;
	if (nAlpha > 100) nAlpha = 100;
	float fAlpha = nAlpha / 100.0f;

	if (SUCCEEDED(hr = m_p->Lock((LPRECT)lpSrcRect, &ddsdSrc, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL)))
	{
		RECT rcDest;
		if (lpSrcRect)
		{
			rcDest.left = nDestLeft;
			rcDest.top = nDestTop;
			rcDest.right = nDestLeft + lpSrcRect->right - lpSrcRect->left;
			rcDest.bottom = nDestTop + lpSrcRect->bottom - lpSrcRect->top;
		}
		else
		{
			rcDest.left = nDestLeft;
			rcDest.top = nDestTop;
			rcDest.right = nDestLeft + dwSrcWidth;
			rcDest.bottom = nDestTop + dwSrcHeight;
		}

		if (SUCCEEDED(hr = destSurface->Lock(&rcDest, &ddsdDest, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL)))
		{
			for (DWORD x = 0; x < ddsdSrc.dwWidth; x++)
			{
				for (DWORD y = 0; y < ddsdSrc.dwHeight; y++)
				{
					clrSrc = ::E2DGetSurfacePixel(x, y, ddsdSrc.lPitch, (char*)ddsdSrc.lpSurface, dwSrcSurfaceBytesOfOnePixel);
					if (bIsHasColorKey && clrSrc >= ck.dwColorSpaceLowValue && clrSrc <= ck.dwColorSpaceHighValue)
						continue;
					clrSrcRed = (clrSrc & 0x00FF0000) >> 16;
					clrSrcGreen = (clrSrc & 0x0000FF00) >> 8;
					clrSrcBlue = clrSrc & 0x000000FF;

					clrDest = ::E2DGetSurfacePixel(x, y, ddsdDest.lPitch, (char*)ddsdDest.lpSurface, dwDestSurfaceBytesOfOnePixel);
					clrDestRed = (clrDest & 0x00FF0000) >> 16;
					clrDestGreen = (clrDest & 0x0000FF00) >> 8;
					clrDestBlue = clrDest & 0x000000FF;

					clrDestRed = (DWORD)(fAlpha * clrSrcRed + (1 - fAlpha) * clrDestRed);
					clrDestGreen = (DWORD)(fAlpha * clrSrcGreen + (1 - fAlpha) * clrDestGreen);
					clrDestBlue = (DWORD)(fAlpha * clrSrcBlue + (1 - fAlpha) * clrDestBlue);
					clrDest = ((clrDestRed << 16) | (clrDestGreen << 8) | (clrDestBlue));

					::E2DSetSurfacePixel(x, y, ddsdDest.lPitch, (char*)ddsdDest.lpSurface, dwDestSurfaceBytesOfOnePixel, clrDest);
				}
			}
			destSurface->Unlock(&rcDest);
		}
		m_p->Unlock((LPRECT)lpSrcRect);
	}

	return hr;
}

/////////////////////CDirectDrawClipper///////////////////////////
HRESULT CDirectDrawClipper::ClipRects(DWORD dwRectCount, LPRECT lpClipList) const
{
	BLUEASSERT(m_p);

	HRESULT hr;
	LPRGNDATA regionData;
	DWORD i;

	regionData = (LPRGNDATA)::malloc(sizeof(RGNDATAHEADER) + dwRectCount * sizeof(RECT));
	::memcpy(regionData->Buffer, lpClipList, sizeof(RECT) * dwRectCount);

	regionData->rdh.dwSize		= sizeof(RGNDATAHEADER);
	regionData->rdh.iType			= RDH_RECTANGLES;
	regionData->rdh.nCount		= dwRectCount;
	regionData->rdh.nRgnSize	= dwRectCount * sizeof(RECT);

	regionData->rdh.rcBound.left			=	64000;
	regionData->rdh.rcBound.top			=	64000;
	regionData->rdh.rcBound.right		=	-64000;
	regionData->rdh.rcBound.bottom	=	-64000;

	// find bounds of all clipping regions
	for (i = 0; i < dwRectCount; i++)
	{
		// test if the next rectangle unioned with the current bound is larger
		if (lpClipList[i].left < regionData->rdh.rcBound.left)
			regionData->rdh.rcBound.left = lpClipList[i].left;

		if (lpClipList[i].right > regionData->rdh.rcBound.right)
			regionData->rdh.rcBound.right = lpClipList[i].right;

		if (lpClipList[i].top < regionData->rdh.rcBound.top)
			regionData->rdh.rcBound.top = lpClipList[i].top;

		if (lpClipList[i].bottom > regionData->rdh.rcBound.bottom)
			regionData->rdh.rcBound.bottom = lpClipList[i].bottom;
	}

	hr = m_p->SetClipList(regionData, 0);
	::free(regionData);

	return hr;
}

///////////////CDDrawSprite////////////////////////
CDDrawSprite::CDDrawSprite(void) :
m_pSrcSurface(NULL),
m_nPosX(0), m_nPosY(0),
m_nRows(0), m_nCols(0),
m_nCellWidth(0), m_nCellHeight(0),
m_nFrameNumber(0),
m_nCurrentFrameIndex(0)
{
}

bool CDDrawSprite::Create(const CDirectDrawSurface7& srcSurface, int nCellWidth, int nCellHeight)
{
	BLUEASSERT(m_pSrcSurface == NULL);
	BLUEASSERT(srcSurface);

	if (nCellWidth <= 0 || nCellHeight <= 0)
		return false;

	m_pSrcSurface = &srcSurface;

	//分块
	return Split(nCellWidth, nCellHeight);
}

void CDDrawSprite::Destroy(void)
{
	m_nFrameNumber = 0;
	m_nCurrentFrameIndex = 0;
	m_pSrcSurface = NULL;
}

bool CDDrawSprite::Split(int nCellWidth, int nCellHeight)
{
	BLUEASSERT(m_pSrcSurface);
	BLUEASSERT(nCellWidth > 0);
	BLUEASSERT(nCellHeight > 0);

	m_nCellWidth = nCellWidth;
	m_nCellHeight = nCellHeight;

	//设置分块
	DWORD dwWidth, dwHeight;
	if (FAILED(m_pSrcSurface->GetBaseDesc(dwWidth, dwHeight)))
		return false;
	m_nRows = dwHeight / m_nCellHeight;
	m_nCols = dwWidth / m_nCellWidth;
	m_nFrameNumber = m_nRows * m_nCols;

	return (m_nRows > 0 && m_nCols > 0);
}

void CDDrawSprite::Paint(const CDirectDrawSurface7& destSurface, LPCRECT lpRectDest)
{
	BLUEASSERT(m_pSrcSurface);

	RECT rcSrc = {
		GetDrawXFromCurFrameIndex(),
		GetDrawYFromCurFrameIndex(),
		rcSrc.left + m_nCellWidth,
		rcSrc.top + m_nCellHeight
	};

	m_pSrcSurface->Blt(
		destSurface,
		lpRectDest,
		&rcSrc
		);
}

void CDDrawSprite::Paint(const CDirectDrawSurface7& destSurface, int nPosX, int nPosY)
{
	BLUEASSERT(m_pSrcSurface);

	m_pSrcSurface->Blt(
		destSurface,
		nPosX, nPosY, m_nCellWidth, m_nCellHeight,
		GetDrawXFromCurFrameIndex(),
		GetDrawYFromCurFrameIndex(),
		m_nCellWidth,
		m_nCellHeight
		);
}

//////////////////////////////////////////////////////////////////////////
/////////////////////////CDDrawSpriteManager///////////////////////
//////////////////////////////////////////////////////////////////////////
CDDSpriteActorManager::CDDSpriteActorManager(void) :
m_pListener(BLUENULL)
{

}

CDDSpriteActorManager::~CDDSpriteActorManager(void)
{
	Clear();
}

void CDDSpriteActorManager::Clear(void)
{
	DSPRITECOLL::iterator it = m_spriteColl.begin();
	DSPRITECOLL::iterator itEnd = m_spriteColl.end();
	while (it != itEnd)
	{
		if (m_pListener)
			m_pListener->OnDDSMRemoveSprite(*it);
		it++;
	}
	m_spriteColl.clear();
}

void CDDSpriteActorManager::Actor(const CDirectDrawSurface7& surfaceCanvas)
{
	BLUEASSERT(surfaceCanvas);

	DSPRITECOLL::iterator it = m_spriteColl.begin();
	DSPRITECOLL::iterator itEnd = m_spriteColl.end();
	DSPRITECOLL::iterator itDel;
	while (it != itEnd)
	{
		if ((*it)->Actor(surfaceCanvas))
		{
			if (m_pListener)
				m_pListener->OnDDSMRemoveSprite(*it);

			itDel = it;
			it++;

			m_spriteColl.erase(itDel);
		}
		else
		{
			it++;
		}
	}
}

//////////////CBitmapBase//////////////////
CImageAbstract::CImageAbstract(void) :
m_dwWidth(0),
m_dwHeight(0),
m_dwBit(0),
m_dwPALCount(0),
m_pBitData(NULL),
m_pPalette(NULL)
{
}

CImageAbstract::~CImageAbstract(void)
{
}

/////////////CBMPBitmap///////////////
CBMPBitmap::CBMPBitmap(void)
{
}

CBMPBitmap::~CBMPBitmap(void)
{
	Unload();
}

HRESULT CBMPBitmap::_loadFile(LPCTSTR lpstrFileName, bool bOnlyLoadPalette)
{
	////////////////////变量声明///////////////
	HANDLE hFile;//文件句柄
	BITMAPFILEHEADER bfh;//BMP文件头信息
	BITMAPINFOHEADER bih;//BMP位图头信息
	BYTE* pBitDataTemp;//BMP位图数据信息(临时的，要经过处理后存入m_pBitData)
	PALETTEENTRY* pPalette;//BMP位图调色板信息
	DWORD dwPALCount;
	DWORD dwNumberOfBytesToRead;//读取的字节数
	DWORD i;//计数器

	/////////////////变量初始////////////////
	pBitDataTemp = NULL;
	pPalette = NULL;
	dwPALCount = 0;

	//打开文件
	hFile = ::CreateFile(lpstrFileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		0);
	//如果打开失败则退出
	if (hFile == INVALID_HANDLE_VALUE)
		return E2D_ERR_OPENFILEFAIL;//无法打开文件

	//读取文件头，类型为BITMAPFILEHEADER
	if (!::ReadFile(hFile, &bfh, sizeof(bfh), &dwNumberOfBytesToRead, NULL) ||
		dwNumberOfBytesToRead != sizeof(bfh))
	{
		::CloseHandle(hFile);
		return E2D_ERR_READFILEFAIL;//读取文件失败
	}

	//验证文件头
	if (bfh.bfType != 0x4D42)
	{
		::CloseHandle(hFile);
		return E2D_ERR_FILEFORMATERROR;//文件格式错误
	}

	//读取位图头
	if (!::ReadFile(hFile, &bih, sizeof(bih), &dwNumberOfBytesToRead, NULL) ||
		dwNumberOfBytesToRead != sizeof(bih))
	{
		::CloseHandle(hFile);
		return E2D_ERR_READFILEFAIL;
	}

	//验证位图头
	if (bih.biSizeImage < 0 ||
		bih.biWidth <= 0 ||
		bih.biHeight <= 0)
	{
		::CloseHandle(hFile);
		return E2D_ERR_FILEFORMATERROR;//文件格式错误
	}

	//验证色深，小于8位的色深，如1,2,4位的位图都不支持
	if (bih.biBitCount < 8)
	{
		::CloseHandle(hFile);
		return E2D_ERR_NOSUPPORTCOLORBIT;//不支持的色彩深度
	}

	//如果色深小于等于8位，则要先读取调色板信息
	BLUEASSERT(bih.biBitCount >= 8);//前面一句已经将低于8位的去除
	if (bih.biBitCount == 8)
	{
		DWORD dwPALBytesLen;//调色板总长度

		//确定调色板数量，如果bih.biClrUsed不为0则直接使用，如果为0则自己计算
		dwPALCount = (bih.biClrUsed > 0) ? bih.biClrUsed : (1 << bih.biBitCount);//调色板数量
		pPalette = new PALETTEENTRY[dwPALCount];//为调色板申请内在空间
		dwPALBytesLen = sizeof(PALETTEENTRY) * dwPALCount;

		//读取调色板
		if (!::ReadFile(hFile, pPalette, dwPALBytesLen, &dwNumberOfBytesToRead, NULL) ||
			dwNumberOfBytesToRead != dwPALBytesLen)
		{
			delete[] pPalette;//删除临时申请的用于调色板的内存
			::CloseHandle(hFile);
			return E2D_ERR_READFILEFAIL;
		}

		//整理读取的调色板信息，因为文件中的信息是颠倒的，文件中的是RGBQUAD结构
		for (i = 0; i < dwPALCount; i++)
		{
			int iTempColor = pPalette[i].peRed;
			pPalette[i].peRed  = pPalette[i].peBlue;
			pPalette[i].peBlue = iTempColor;
			pPalette[i].peFlags = PC_NOCOLLAPSE;
		}
	}

	if (bOnlyLoadPalette)//如果只加载调色板信息
	{
		if (pPalette)//如果该位图有调色板
		{
			m_dwPALCount = dwPALCount;
			if (m_pPalette) delete[] m_pPalette;//赋值前先删除内存空间
			m_pPalette = pPalette;
			return E2D_OK;
		}
		return E2D_ERR_NOPALETTE;
	}

	//申请内存并读取实际位图数据
	pBitDataTemp = new BYTE[bih.biSizeImage];
	if (!::ReadFile(hFile, pBitDataTemp, bih.biSizeImage, &dwNumberOfBytesToRead, NULL) ||
		dwNumberOfBytesToRead != bih.biSizeImage)
	{
		delete[] pBitDataTemp;//需要先删除临时申请的内存
		if (pPalette) delete[] pPalette;//删除临时申请的内存
		::CloseHandle(hFile);
		return E2D_ERR_READFILEFAIL;
	}

	///////////////////成功后关键变量全部赋值////////////////
	//先赋值m_pBitData，这个有点复杂
	//将pBitDataTemp做处理然后复制给m_pBitData
	{
		DWORD dwLineBytes;//实际数据中一行的字节长，这个长可能与bih.biWidth*bih.biBitCount / 8不同
		DWORD dwBitmapDataLineBytes;//位图真正有效的一行字节长
		DWORD dwBitmapDataAllBytes;//需要为位图申请的真实内存总长

		//计算三个数值
		BLUEASSERT(bih.biSizeImage % bih.biHeight == 0);//不可能无法整除
		dwLineBytes = bih.biSizeImage / bih.biHeight;
		//位图一个像素占bih.biBitCount/8个字节
		dwBitmapDataLineBytes = bih.biWidth * bih.biBitCount / 8;
		dwBitmapDataAllBytes = dwBitmapDataLineBytes * bih.biHeight;

		//先删除旧的申请的空间
		if (m_pBitData) delete[] m_pBitData;

		//为位图真实数据申请内在空间，并且要倒过来显示图像
		m_pBitData = new BYTE[dwBitmapDataAllBytes];
		for (i = 0; i < (DWORD)bih.biHeight; i++)
		{
			::memcpy(m_pBitData + i * dwBitmapDataLineBytes,
				pBitDataTemp + (bih.biHeight - i - 1) * dwLineBytes,
				dwBitmapDataLineBytes);
		}
	}

	//其余赋值
	m_dwWidth = bih.biWidth;
	m_dwHeight = bih.biHeight;
	m_dwBit = bih.biBitCount;
	m_dwPALCount = dwPALCount;
	if (m_pPalette) delete[] m_pPalette;//删除临时申请的内存
	m_pPalette = pPalette;

	delete[] pBitDataTemp;//删除临时申请的内存
	::CloseHandle(hFile);
	return E2D_OK;//读取成功
}

HRESULT CBMPBitmap::LoadFile(LPCTSTR lpstrFileName)
{
	if (IsValidate())//如果已经加载过
		return E2D_OK;

	return _loadFile(lpstrFileName, false);
}

HRESULT CBMPBitmap::LoadFileOnlyPalette(LPCTSTR lpstrFileName)
{
	if (IsHasPal())//如果已经加载过调色板
		return E2D_OK;

	return _loadFile(lpstrFileName, true);
}

HRESULT CBMPBitmap::Unload()
{
	if (m_pPalette)
	{
		BLUEASSERT(m_dwPALCount > 0);
		delete[] m_pPalette;
		m_pPalette = NULL;
	}

	if (m_pBitData)
	{
		BLUEASSERT(IsValidate());
		delete[] m_pBitData;
		m_pBitData = NULL;
	}

	m_dwPALCount = 0;
	m_dwWidth = 0;
	m_dwHeight = 0;
	m_dwBit = 0;

	return E2D_OK;
}

HRESULT CDirectInput8::CreateKeyboard(LPDIRECTINPUTDEVICE8* pp)
{
	BLUEASSERT(m_p);
	BLUEASSERT(pp);

	return m_p->CreateDevice(GUID_SysKeyboard, pp, NULL);
}

HRESULT CDirectInput8::CreateMouse(LPDIRECTINPUTDEVICE8 *pp)
{
	BLUEASSERT(m_p);
	BLUEASSERT(pp);

	return m_p->CreateDevice(GUID_SysMouse, pp, NULL);
}

HRESULT CDirectInputDevice8::Init(HWND hWnd, DWORD dwCLFlags, LPCDIDATAFORMAT pcdiFormat)
{
	BLUEASSERT(m_p);
	BLUEASSERT(pcdiFormat);

	HRESULT hr;

	if (FAILED(hr = m_p->SetCooperativeLevel(hWnd, dwCLFlags)))
		return hr;

	return m_p->SetDataFormat(pcdiFormat);
}

HRESULT CDirectInputKeyboard::Init(HWND hWnd, LPCDIDATAFORMAT pcdiFormat)
{
	if (pcdiFormat == NULL)
		pcdiFormat = &c_dfDIKeyboard;

	return CDirectInputDevice8::Init(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE, pcdiFormat);
}

HRESULT CDirectInputMouse::Init(HWND hWnd, LPCDIDATAFORMAT pcdiFormat)
{
	if (pcdiFormat == NULL)
		pcdiFormat = &c_dfDIMouse;

	return CDirectInputDevice8::Init(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE, pcdiFormat);
}