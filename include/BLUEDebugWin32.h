#ifndef __BLUEDEBUGWIN32_H__
#define __BLUEDEBUGWIN32_H__

/*
BLUETRACE:在debug输出栏输出调试信息
BLUEASSERT:断言
BLUECHECKMEMLEAK:检测内存泄漏
*/

#if defined(DEBUG) | defined(_DEBUG)//该宏只有在DEBUG版本下才有效
#include "BLUEDefWin32.h"
#include <stdarg.h>
#include <assert.h>
#define WIN32_LEAN_AND_MEAN//从Windows 头中排除极少使用的资料
#include <windows.h>
#include <tchar.h>
#define _CRTDBG_MAP_ALLOC//要在包含crtdbg.h前定义
#include <crtdbg.h>

#define BLUETRACE                   CBLUETrace()
#define BLUEASSERT(f)              assert(f)
#define BLUECHECKMEMLEAK() ::_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF)

class CBLUETrace
{
public:
	CBLUETrace() {}
	void __cdecl operator()(BLUELPCTSTR pszFmt, ...) const
	{
		BLUETCHAR strOut[1024] = {};
		va_list ptr;
		va_start(ptr, pszFmt);
		::_vstprintf_s(strOut, pszFmt, ptr);
		::OutputDebugString(strOut);
		va_end(ptr);
	}
};

#else
#define BLUETRACE                    __noop
#define BLUEASSERT(f)               ((void)(f))
#define BLUECHECKMEMLEAK()  ((void)(0))
#endif

#endif