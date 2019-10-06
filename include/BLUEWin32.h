#pragma once
#include <BLUEDef.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsvc.h>

#ifdef BLUEWIN32_EXPORTS
#define BLUEWIN32_API __declspec(dllexport)
#else
#define BLUEWIN32_API __declspec(dllimport)
#endif

//windows临界对象
class CBLUEWinCriticalSection
{
public:
	CBLUEWinCriticalSection(void) {::InitializeCriticalSection(&m_cs);}
	~CBLUEWinCriticalSection(void) {::DeleteCriticalSection(&m_cs);}
	void Lock(void) {::EnterCriticalSection(&m_cs);}
	void Unlock(void) {::LeaveCriticalSection(&m_cs);}
private:
	CRITICAL_SECTION m_cs;
};

//windows事件对象
class BLUEWIN32_API CBLUEWinEvent
{
public:
	CBLUEWinEvent(void);
	~CBLUEWinEvent(void);
	void SetEventSigned(void) {}
	void SetEventNosigned(void) {}

private:
	HANDLE m_hEvent;
};