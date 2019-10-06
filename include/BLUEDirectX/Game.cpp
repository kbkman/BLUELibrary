#include "BLUEDirectX/Game.h"

#define DGR_WNDCLASS_NAME        _BLUET("DXGameRunWnd")

CDXGameRun* CDXGameRun::s_pLDGRRO = NULL;//初始静态变量
CDXGameRun::CDXGameRun(void) :
m_hMainWnd(NULL),
m_hInstance(NULL),
m_dwFrameSpeed(30)//默认为30毫秒帧间隔
{
	BLUEASSERT(s_pLDGRRO == NULL);//限制只能有一个CDXGameRun实例
	s_pLDGRRO = this;
}

CDXGameRun::~CDXGameRun(void)
{
	if (s_pLDGRRO == this)
		s_pLDGRRO = NULL;
}

bool CDXGameRun::RegWndClass(WNDCLASSEX* pWCEX)
{
	if (pWCEX == NULL)
	{
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = MainWndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = m_dxGameRunDataBase.hInstance;
		wcex.hIcon = NULL;
		wcex.hCursor = ::LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = NULL;//(HBRUSH)(COLOR_WINDOW);背景不需要重画
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = DGR_WNDCLASS_NAME;
		wcex.hIconSm = NULL;
		return (::RegisterClassEx(&wcex) != 0);
	}

	return (::RegisterClassEx(pWCEX) != 0);
}

LRESULT CALLBACK CDXGameRun::MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BLUEASSERT(s_pLDGRRO);
	return s_pLDGRRO->OnMainWndProc(hWnd, uMsg, wParam, lParam);
}

HRESULT CDXGameRun::RunInitBeforeLoop(void)
{
	return S_OK;
}

HRESULT CDXGameRun::RunEndAfterLoop(void)
{
	return S_OK;
}

void CDXGameRun::	ComputeClientRect(void)
{
	HWND hWnd = GetMainWnd();
	::GetClientRect(hWnd, &m_rcMainWndClient);

	//把客户端坐标转成屏幕坐标
	::ClientToScreen(hWnd, (POINT*)&m_rcMainWndClient);
	::ClientToScreen(hWnd, (POINT*)&m_rcMainWndClient + 1);
}

LRESULT CDXGameRun::OnMainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!OnPreMainWndProc(hWnd, uMsg, wParam, lParam))
		return 0;

	switch (uMsg)
	{
	case WM_CREATE:
		return OnCreate(uMsg, wParam, lParam);

	case WM_PAINT:
		return OnPaint(uMsg, wParam, lParam);

	case WM_MOVE:
		return OnMove(uMsg, wParam, lParam);

	case WM_SIZE:
		return OnSize(uMsg, wParam, lParam);

	case WM_DESTROY:
		::PostQuitMessage(0);
		break;

	default:
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

LRESULT CDXGameRun::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ComputeClientRect();
	return 0;
}

LRESULT CDXGameRun::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	hdc = ::BeginPaint(GetMainWnd(), &ps);
	::EndPaint(GetMainWnd(), &ps);
	return 0;
}

LRESULT CDXGameRun::OnMove(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ComputeClientRect();
	return 0;
}

LRESULT CDXGameRun::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ComputeClientRect();
	return 0;
}

bool CDXGameRun::Run(
					 DXGAMERUNDATABASE* pDXGameRunDataBase,
					 DXGAMERUNDATAEXPAND* pDXGameRunDataExpand
					 )
{
	BLUEASSERT(pDXGameRunDataBase);

	m_dxGameRunDataBase = *pDXGameRunDataBase;
	m_hInstance = m_dxGameRunDataBase.hInstance;

	//注册窗口类
	if (!RegWndClass((pDXGameRunDataExpand) ? pDXGameRunDataExpand->pWCEX : NULL))
		return false;

	//创建窗口
	if (pDXGameRunDataExpand)
	{
		m_hMainWnd = ::CreateWindow(
			(pDXGameRunDataExpand->pWCEX) ? pDXGameRunDataExpand->pWCEX->lpszClassName : DGR_WNDCLASS_NAME,
			pDXGameRunDataExpand->lpstrWindowCaption,
			pDXGameRunDataExpand->dwStyle,
			pDXGameRunDataExpand->nWndX,
			pDXGameRunDataExpand->nWndY,
			pDXGameRunDataExpand->nWndWidth,
			pDXGameRunDataExpand->nWndHeight,
			NULL, NULL, m_dxGameRunDataBase.hInstance, NULL);
	}
	else
	{
		m_hMainWnd = ::CreateWindow(
			DGR_WNDCLASS_NAME,
			NULL,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			NULL, NULL, m_dxGameRunDataBase.hInstance, NULL);
	}

	if (m_hMainWnd == NULL)
		return false;

	::ShowWindow(m_hMainWnd, m_dxGameRunDataBase.nCmdShow);
	::UpdateWindow(m_hMainWnd);

	//让派生类调用自己的初始函数
	if (FAILED(RunInitBeforeLoop()))
		goto ENDRUN;

	//重新计算一次m_rcMainWndClient，给派生类一次重新计算的机会
	//注意，必须在RunInitBeforeLoop之后调用
	ComputeClientRect();

	//调用游戏初始函数
	if (FAILED(GameInit()))
		goto ENDRUN;

	MSG msg;
	DWORD dwStartFrameClock;
	DWORD dwFrameUseTime;
	//主消息循环:
	while (true)
	{
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			//如果定义DXGAME_NONEED_QUITMSG
			//则通过只由GameMain返回错误退出游戏
#ifndef DXGAME_NONEED_QUITMSG
			if (msg.message == WM_QUIT)
				break;
#endif

			//如果没有定义DXGAME_NONEED_TRANSMSG
			//则使用TranslateMessage翻译消息
#ifndef DXGAME_NONEED_TRANSMSG
			::TranslateMessage(&msg);
#endif

			//发送消息
			::DispatchMessage(&msg);

		}

		//可以直接利用GameMain返回错误来结束游戏
		dwStartFrameClock = ::GetTickCount();
		if (FAILED(GameMain()))
			break;
		dwFrameUseTime = ::GetTickCount() - dwStartFrameClock;
		//BLUETRACE(_BLUET("use time %d\n"), dwFrameUseTime);
		if (dwFrameUseTime < m_dwFrameSpeed)
			::Sleep(m_dwFrameSpeed - dwFrameUseTime);
	}

	//调用游戏结束函数
	GameShutDown();

ENDRUN://退出Run的最后处理

	//先调用RunEndAfterLoop，派生类可以在先销毁自己的资源
	RunEndAfterLoop();

	//如果窗口还没有被销毁则要先销毁
	if (::IsWindow(m_hMainWnd))
		::DestroyWindow(m_hMainWnd);

	//返回0
	return 0;
}