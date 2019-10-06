#pragma once

#include "XMLOperation.h"
#include "HighSpeedMap.h"

#define WM_NCSKIN_UPDATED		(WM_USER + 101)//皮肤更新完毕消息
//类CBmp
class CNCBmp : public BITMAP
{
public:
	CNCBmp(void);
	virtual ~CNCBmp(void);

	virtual void Free(void);
	virtual bool LoadFile(LPCTSTR strFileName);
	virtual bool BitBltToHDC(HDC hdcDest, HDC hdcSrc, int xDest = 0, int yDest = 0, DWORD rop = SRCCOPY);
	virtual bool StretchBltToHDC(HDC hdcDest, HDC hdcSrc, int xDest, int yDest, int wDest, int hDest, DWORD rop = SRCCOPY);

	HBITMAP GetHandle(void) {return m_hBmp;}

private:
	HBITMAP m_hBmp;
};

//类CNCBmpButton
#define NCBTN_STATE_DEFAULT		0
#define NCBTN_STATE_MOVE			1
#define NCBTN_STATE_DOWN			2
class CNCBmpButton : public CNCBmp
{
private:
	CNCBmp m_bmpMouseMove;//mouseover状态使用
	CNCBmp m_bmpMouseDown;//按下时的状态使用

	RECT m_rcPos;//按钮在什么地方（屏幕坐标，非窗口坐标）
	long m_lState;//状态，有正常(NCBTN_STATE_DEFAULT)，滑过(NCBTN_STATE_MOVE)，按下(NCBTN_STATE_DOWN)

public:
	int m_iPosTopToFrame;
	int m_iPosRightToFrame;

public:
	CNCBmpButton(void);
	virtual ~CNCBmpButton(void);

public:
	virtual void Free(void);
	virtual bool LoadFile(LPCTSTR strFileName);
	virtual bool LoadFile(LPCTSTR strFileName, LPCTSTR strFileNameMouseMove, LPCTSTR strFileNameMouseDown);
	virtual bool BitBltToHDC(HDC hdcDest, HDC hdcSrc, int xDest = 0, int yDest = 0, DWORD rop = SRCCOPY);
	virtual bool StretchBltToHDC(HDC hdcDest, HDC hdcSrc, int xDest, int yDest, int wDest, int hDest, DWORD rop = SRCCOPY);

	void CalcPos(const RECT& rcWndPos);
	void GetPos(RECT& rc);
	void GetPos(HRGN& hRgn);
	bool IsPtInPos(const POINT& pt);
	bool IsPtInPos(const LONG x, const LONG y);
	long PutState(long lState);
	long GetState(void);
};

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lParam)	((int)(short)LOWORD(lParam))
#endif

#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lParam)	((int)(short)HIWORD(lParam))
#endif

class CWndNCSkinProc
{
	//////公开///////////
public:
	static bool Bind(HWND hWnd, LPCTSTR lpstrDirectory);
	static bool UnBind(HWND hWnd);

	long GetVersion(void) {return m_lVer;}
	bool BindSkin(LPCTSTR lpstrFoldName = NULL);

	///////私有/////////
private:
	static CHighSpeedMap s_hsm;
	int m_iTitleIconTopToFrame;//图标离框架上面距离
	int m_iTitleIconLeftToFrame;//图标离框架左面距离
	int m_iTitleIconWidth;//图标宽度
	int m_iTitleIconHeight;//图标高度
	int m_iTitleTextPosTopToFrame;;//标题文字离框架上面距离
	int m_iTitleTextPosLeftToFrame;;//标题文字离框架左面距离
	int m_iTitleTextPosRightToFrame;;//标题文字离框架右面距离
	int m_iTitleTextHeight;//标题文字高度
	int m_iTitleTextWeight;//标题文字粗细
	bool m_bNeedClrTransparent;//是否需要透明色
	bool m_bDrawTitleIcon;//是否画图标
	HFONT m_hTitleTextFont;//标题文字字体
	HICON m_hTitleIcon;//标题图标
	COLORREF m_clrTransparent;//透明色，用于SetWndRgn（m_bNeedClrTransparent为true时才有效）
	COLORREF m_clrTitleText;//标题文字色

	CNCBmp m_bmpTopBack;
	CNCBmp m_bmpTopLeft;
	CNCBmp m_bmpTopRight;
	CNCBmp m_bmpLeftBack;
	CNCBmp m_bmpLeftTop;
	CNCBmp m_bmpLeftBottom;
	CNCBmp m_bmpRightBack;
	CNCBmp m_bmpRightTop;
	CNCBmp m_bmpRightBottom;
	CNCBmp m_bmpBottomBack;
	CNCBmp m_bmpBottomLeft;
	CNCBmp m_bmpBottomRight;

	CNCBmpButton m_ncbbCloseBtn;
	CNCBmpButton m_ncbbRecBtn;//已经最大化时的恢复正常按钮
	CNCBmpButton m_ncbbMaxBtn;
	CNCBmpButton m_ncbbMinBtn;
	CNCBmpButton* m_pCaptureBtn;//当前获得捕获资格的按钮


	long m_lVer;//当前皮肤的版本号
	bool m_bNeedSetWndRgn;//是否需要SetWindowRgn操作

	//其他
	HWND m_hWnd;
	WNDPROC m_fnOldWndProc;

private:
	static LRESULT stWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	CWndNCSkinProc(void);
	virtual ~CWndNCSkinProc(void);

	bool UseSkin(LPCTSTR lpstrFoldName = NULL);
	bool IsUseDefaultSkin(void);
	bool UseDefaultSkin(void);
	void Free(void);
	void Init(void);
	bool ReDrawFrame(void) {return (::SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_NOCOPYBITS | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING) == TRUE);}
	bool IsHasMaxBtn(void) {return ((::GetWindowLong(m_hWnd, GWL_STYLE) & WS_MAXIMIZEBOX) > 0);}
	bool IsHasMinBtn(void) {return ((::GetWindowLong(m_hWnd, GWL_STYLE) & WS_MINIMIZEBOX) > 0);}
	void SetCaptureBtn(CNCBmpButton* p);
	void ReleaseCaptureBtn(void);
	void DrawNCBmpBtn(CNCBmpButton* pNcbb);
	void DrawNCBmpBtnByState(CNCBmpButton* pNcbb, long lState);

	LRESULT HitTest(POINT pt);
	HRGN CreateTempRgn() {return ::CreateRectRgn(0, 0, 1, 1);}
	CNCBmpButton* GetCaptureBtn(void) {return m_pCaptureBtn;}

	LRESULT WndPorc(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
	LRESULT OnNcPaint(WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnNcCalcSize(WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnNcActivate(WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnNcHitTest(WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnNcMouseMove(WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnNcLButtonDown(WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnNcRButtonDown(WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSetText(WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnGetText(WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnGetTextLength(WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnMouseMove(WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnLButtonDown(WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnLButtonUp(WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRButtonUp(WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSysCommand(WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSetCursor(WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSetIcon(WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnGetIcon(WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnStyleChanged(WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSettingChange(WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnNcSkinUpdated(WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};
