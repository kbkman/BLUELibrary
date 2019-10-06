#include <BLUEDirectX\Game2D.h>

////////////////////////CDXGameRun2D////////////////////////
CDXGameRun2D::CDXGameRun2D(void)
{
}

CDXGameRun2D::~CDXGameRun2D(void)
{
}

HRESULT CDXGameRun2D::SetPaletteToPrimarySurface(LPPALETTEENTRY pPalette, DWORD dwPalCount)
{
	BLUEASSERT(pPalette);
	BLUEASSERT(m_ddsprimary);

	if (!m_ddpprimary)
		return S_OK;

	return m_ddpprimary->SetEntries(0, 0, dwPalCount, pPalette);
}

HRESULT CDXGameRun2D::SetPaletteToPrimarySurface(LPCTSTR lpstrFileName)
{
	BLUEASSERT(lpstrFileName);

	HRESULT hr;
	CImageAbstract* pBmp;

	if (FAILED(hr = ::E2DCreateImageFromFile(lpstrFileName, &pBmp)))
		return hr;

	hr = SetPaletteToPrimarySurface(pBmp->GetPalette(), pBmp->GetPALCount());
	::E2DDestroyImage(pBmp);

	return hr;
}

HRESULT CDXGameRun2D::RunInitBeforeLoop(void)
{
	HRESULT hr;
	HWND hMainWnd;
	DWORD dwScreenWidth;
	DWORD dwScreenHeight;
	DWORD dwScreenBit;

	//创建DirectDraw
	if (FAILED(hr = ::E2DCreateDDRAW7(&m_dd7)))
		return hr;

	//获取主窗口句柄
	hMainWnd = GetMainWnd();

#ifdef DXGAME2D_USE_EXCLUSIVE_MODE//独占模式下

	//绘制方式，从窗口起点开始绘制
	m_PSDrawType = imm;

	//设置协作模式和设置分辨率和色彩模式
	if (FAILED(hr = m_dd7.InitForExclusiveMode(
		hMainWnd,
		DXGAME2D_SCREEN_WIDTH,
		DXGAME2D_SCREEN_HEIGHT,
		DXGAME2D_SCREEN_BIT)))
		return hr;

	//用GetDisplayModeBase获取屏幕宽度高度及色深
	if (FAILED(hr = m_dd7.GetDisplayModeBase(
		dwScreenWidth,
		dwScreenHeight,
		dwScreenBit)))
		return hr;

	//如果屏幕色深小于8，即1,2,4位色则返回错误，表示不支持这些色深
	if (dwScreenBit < 8)
		return DXGR2D_ERR_NOSUPPORTCOLORBIT;

	//创建主表面
	if (FAILED(hr = m_dd7.CreateExclusiveModePrimarySurface(
		DXGAME2D_EXCLUSIVE_MODE_BACKBUFFERCOUNT,
		&m_ddsprimary,
		&m_ddsback)))
		return hr;

#else//普通模式(窗口模式)下

	//绘制方式，从窗口起点开始绘制
	m_PSDrawType = windowstart;

	if (FAILED(hr = m_dd7.InitForNormalMode(hMainWnd,
		DXGAME2D_SCREEN_WIDTH,
		DXGAME2D_SCREEN_HEIGHT,
		DXGAME2D_SCREEN_BIT)))
		return hr;

	//用GetDisplayModeBase获取屏幕宽度高度及色深
	if (FAILED(hr = m_dd7.GetDisplayModeBase(
		dwScreenWidth,
		dwScreenHeight,
		dwScreenBit)))
		return hr;

	//如果屏幕色深小于8，即1,2,4位色则返回错误，表示不支持这些色深
	if (dwScreenBit < 8)
		return DXGR2D_ERR_NOSUPPORTCOLORBIT;

	//创建主表面
	if (FAILED(hr = m_dd7.CreateNormalModePrimarySurface(&m_ddsprimary)))
		return hr;

	//自行创建一个缓存表面
	if (FAILED(hr = m_dd7.CreateVideoMemoryGeneralSurface(
		dwScreenWidth,
		dwScreenHeight,
		&m_ddsback)))
	{
		//如果在显存中无法创建则在内存中创建
		if (FAILED(hr = m_dd7.CreateSystemMemoryGeneralSurface(
			dwScreenWidth,
			dwScreenHeight,
			&m_ddsback)))
			return hr;
	}

	//主表面要栽剪进窗口客户区（不剪切的话，blt时矩形在表面之外将无法绘制）
	if (FAILED(m_ddsprimary.ClipWnd(m_dd7, hMainWnd)))
		return hr;

	//缓存表面要栽剪自己（不剪切的话，blt时矩形在表面之外将无法绘制）
	if (FAILED(m_ddsback.ClipOwn(m_dd7)))
		return hr;

#endif

	//如果是8位色
	if (dwScreenBit == 8)
	{
		//创建一个8位调色板
		if (FAILED(hr = m_dd7.Create8BitPalette(&m_ddpprimary)))
			return hr;

		//将调色板关联到主表面
		if (FAILED(hr = m_ddsprimary->SetPalette(m_ddpprimary)))
			return hr;
	}

	////////////////创建DirectInput//////////////
	//创建DirectInput8
	if (FAILED(hr = ::E2DCreateDInput8(GetInstance(), &m_di8)))
		return hr;

	//创建keyboard Device
	if (FAILED(hr = m_di8.CreateKeyboard(&m_diKeyboard)))
		return hr;
	//keyboard初始
	if (FAILED(hr = m_diKeyboard.Init(hMainWnd)))
		return hr;

	//创建Mouse Device
	if (FAILED(hr = m_di8.CreateMouse(&m_diMouse)))
		return hr;
	//Mouse初始
	if (FAILED(hr = m_diMouse.Init(hMainWnd)))
		return hr;

	return DXGR2D_OK;
}

HRESULT CDXGameRun2D::RunEndAfterLoop(void)
{
	return DXGR2D_OK;
}

void CDXGameRun2D::ComputeClientRect(void)
{
#if (DXGAME2D_COMPUTECLIENTRECT_TYPE == 0)//不作任何处理

#elif (DXGAME2D_COMPUTECLIENTRECT_TYPE == 1)//默认以窗口计算
	CDXGameRun::ComputeClientRect();

#elif (DXGAME2D_COMPUTECLIENTRECT_TYPE == 2)//以窗口和全屏计算
	if (m_ddsprimary) {
		DWORD dwWidth, dwHeight, dwBit;
		HWND hWnd;

		hWnd = GetMainWnd();
		m_ddsprimary.GetBaseDesc(dwWidth, dwHeight, dwBit);

		::GetClientRect(hWnd, &m_rcMainWndClient);
		::ClientToScreen(hWnd, (POINT*)&m_rcMainWndClient);

		m_rcMainWndClient.right = m_rcMainWndClient.left + dwWidth;
		m_rcMainWndClient.bottom = m_rcMainWndClient.top + dwHeight;
	} else {
		CDXGameRun::ComputeClientRect();
	}

#else
	BLUEASSERT(0);

#endif
}