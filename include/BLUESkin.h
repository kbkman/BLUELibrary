#pragma once

#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
#include <windows.h>
#include <tchar.h>
#include <objbase.h>

#ifdef BLUESKIN_EXPORTS
#define BLUESKIN_API _declspec(dllexport)
#else
#define BLUESKIN_API _declspec(dllimport)
#endif

bool BLUESKIN_API BLUESkin_Initialize(void);
void BLUESKIN_API BLUESkin_Uninitialize(void);

bool BLUESKIN_API BLUESkin_BindNCSkin(HWND hWnd, LPCSTR lpstrDirectory);
bool BLUESKIN_API BLUESkin_BindNCSkin(HWND hWnd, LPCWSTR lpstrDirectory);
bool BLUESKIN_API BLUESkin_UnBindNCSkin(HWND hWnd);