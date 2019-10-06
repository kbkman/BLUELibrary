#pragma once
#include <BLUEDirectX/Def.h>

struct DXGAMERUNDATABASE//DXGameRun运行时需要的基本参数
{
	//windows应用程序启动时的四个参数,这部分必填
	HINSTANCE hInstance;
	HINSTANCE hPrevInstance;
	LPTSTR lpCmdLine;
	int nCmdShow;
};

struct DXGAMERUNDATAEXPAND//DXGameRun运行时需要的扩展参数
{
	//注册窗口类的信息,如果使用默认,则可以直接设置为NULL
	WNDCLASSEX* pWCEX;

	//创建的窗口信息
	LPCTSTR lpstrWindowCaption;
	DWORD dwStyle;
	int nWndX;
	int nWndY;
	int nWndWidth;
	int nWndHeight;
};

class CDXGameRun
{
public:
	CDXGameRun(void);
	~CDXGameRun(void);

public:
	HINSTANCE GetInstance(void) {return m_dxGameRunDataBase.hInstance;}
	HWND GetMainWnd(void) {return m_hMainWnd;}
	bool Run(DXGAMERUNDATABASE* pDXGameRunDataBase, DXGAMERUNDATAEXPAND* pDXGameRunDataExpand = NULL);
	DWORD GetFrameSpeed(void) const {return m_dwFrameSpeed;}
	void SetFrameSpeed(const DWORD dwFrameSpeed) {m_dwFrameSpeed = dwFrameSpeed;}

protected:
	bool RegWndClass(WNDCLASSEX* pWCEX = NULL);//注册窗口类
	static LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);

	virtual void ComputeClientRect(void);//计算客户区矩形(相对于屏幕)
	virtual HRESULT RunInitBeforeLoop(void);//在进入游戏循环之前初始，供派生类使用
	virtual HRESULT RunEndAfterLoop(void);//在退出游戏循环之后释放，供派生类使用
	virtual HRESULT GameInit(void) = 0;
	virtual HRESULT GameMain(void) = 0;
	virtual HRESULT GameShutDown(void) = 0;

	/*
	函数名：消息预处理
	功能：略
	说明：无
	参数：无
	返回值：如果返回false表示无须再处理，即OnMainWndProc不会被调用
	*/
	virtual bool OnPreMainWndProc(HWND, UINT, WPARAM, LPARAM) {return true;}
	LRESULT OnMainWndProc(HWND, UINT, WPARAM, LPARAM);

	//特殊消息处理回调
	virtual LRESULT OnCreate(UINT, WPARAM, LPARAM);
	virtual LRESULT OnPaint(UINT, WPARAM, LPARAM);
	virtual LRESULT OnMove(UINT, WPARAM, LPARAM);
	virtual LRESULT OnSize(UINT, WPARAM, LPARAM);

protected:
	RECT m_rcMainWndClient;

private:
	HWND m_hMainWnd;
	DXGAMERUNDATABASE m_dxGameRunDataBase;
	static CDXGameRun* s_pLDGRRO;//LastDXGameRunRegObject(用于保存创建的CDXGameRun变量)
	HINSTANCE m_hInstance;
	DWORD m_dwFrameSpeed;//帧速，即两帧之间相差多少毫秒
};