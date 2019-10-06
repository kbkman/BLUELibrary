#ifndef _BLUETZDEBUG_H_
#define _BLUETZDEBUG_H_

/*
TZTRACE:在debug输出栏输出调试信息
TZASSERT:断言
TZCHECKMEMLEAK:检测内存泄漏
*/

#if defined(DEBUG) | defined(_DEBUG)//该宏只有在DEBUG版本下才有效
//添加头文件
#include <stdarg.h>
#include <cassert>
#include "BLUETZLog.h"
#define WIN32_LEAN_AND_MEAN//从Windows头中排除极少使用资料
#include <windows.h>
#define _CRTDBG_MAP_ALLOC//要在包含crtdbg.h前定义
#include <crtdbg.h>

#if defined(TZWINDOWASSERT) //如果定义TZWINDOWASSERT则不会调用assert而弹出窗口
#define TZASSERT(f) if (!(f)) {CTZAssert()(#f, __FILE__, __LINE__);assert(f);}
#else
#define TZASSERT(f) if (!(f)) {CTZAssert()(#f, __FILE__, __LINE__);}
#endif

#define TZTRACE CTZTrace()
#define TZCHECKMEMLEAK() ::_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF)

class CTZTrace
{
public:
	CTZTrace(void) {}
	void __cdecl operator()(const char* lpstrFmt, ...) const
	{
		assert(lpstrFmt);

		char strOut[1024];
		va_list ptr;
		va_start(ptr, lpstrFmt);
		::vsprintf(strOut, lpstrFmt, ptr);
		::OutputDebugStringA(strOut);
		va_end(ptr);

		static CBLUETZLog tzlog;

		//先判断是否建立了日志文件
		if (!tzlog.IsVaild())
			if (!tzlog.Create("Debug"))
				return;

		//写入日志
		tzlog.WriteLog(strOut);
	}
};

/*
函数名：断言的日志形式实现
功能：略
说明：无
参数：无
*/
class CTZAssert
{
public:
	CTZAssert(void) {}
	void __cdecl operator()(const char* lpstrLog, const char* lpstrFile, int nLine) const
	{
		assert(lpstrLog);
		assert(lpstrFile);
		assert(nLine > 0);

		//日志头信息
		char strAssertFileInfo[512];
		::sprintf(strAssertFileInfo, "[File=%s Line=%d]", lpstrFile, nLine);

		/*
		Debug文件和DebugAssert都要写入
		*/
		//先判断是否建立了Err日志文件
		static CBLUETZLog tzDebugLog;
		if (!tzDebugLog.IsVaild())
			if (!tzDebugLog.Create("Debug"))
				return;

		//写入日志文件
		tzDebugLog.WriteLog(strAssertFileInfo);
		tzDebugLog.WriteLog(lpstrLog, false);
		tzDebugLog.WriteLineSymbol();

		//先判断是否建立了DebugAssert日志文件
		static CBLUETZLog tzDebugAssertLog;
		if (!tzDebugAssertLog.IsVaild())
			if (!tzDebugAssertLog.Create("DebugAssert"))
				return;

		//写入日志文件
		tzDebugAssertLog.WriteLog(strAssertFileInfo);
		tzDebugAssertLog.WriteLog(lpstrLog, false);
		tzDebugAssertLog.WriteLineSymbol();
	}
};

#else
#define TZTRACE __noop
#define TZASSERT(f) ((void)(f))
#define TZCHECKMEMLEAK((void)(0))
#endif

#endif