#pragma once
#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
#include <BLUEAdv.h>
#include <BLUEDebug.h>
#include <BLUEWin32.h>
#include <queue>
#include <windows.h>

class CBLUEMTSafeMessageQueue : public IBLUEMTSafeMessageQueue
{
	struct MTSAFEMSG//一条消息的类型
	{
		BLUEDWORD dwMessageType;//消息类型
		BLUEDWORD firstParam;//firstParam 消息具体参数1
		void* secondParam;//secondParam 消息具体参数2
		void* thirdParam;//secondParam 消息具体参数3
	};

public:
	CBLUEMTSafeMessageQueue(void);
	~CBLUEMTSafeMessageQueue(void);

public:
	virtual void PutOneMessage(const BLUEDWORD dwMessageType, const BLUEDWORD firstParam, void* const secondParam, void* const thirdParam);
	virtual bool GetOneMessage(BLUEDWORD& dwMessageType, BLUEDWORD& firstParam, void*& secondParam, void*& thirdParam);
	virtual bool PeekOneMessage(BLUEDWORD& dwMessageType, BLUEDWORD& firstParam, void*& secondParam, void*& thirdParam);
	virtual void PutOneQuitMessage(void);

private:
	/*
	变量名：内部消息队列
	功能：略
	说明：由MTSAFEMSG结构体比较小，所以直接用结构体，
	　　　而不用结构体指针，这样避免了频繁的new和delete
	*/
	std::deque<MTSAFEMSG> m_msgQueue;

	/*
	变量名：操作消息队列时使用的临界区
	功能：略
	说明：无
	*/
	CBLUEWinCriticalSection m_msgQueueCS;

	/*
	变量名：消息队列中是否有消息的事件通知句柄
	功能：略
	说明：无
	*/
	HANDLE m_hHasMessageEvent;
};

class CBLUEFile : public IBLUEFile
{
public:
	CBLUEFile(void);
	virtual ~CBLUEFile(void);
	virtual bool Create(
		const BLUEString& strFileName,
		BLUEDWORD dwAccess,
		BLUEDWORD dwShareMode,
		BLUEDWORD dwCreationMode,
		BLUEDWORD dwAttributes
		);
	virtual void Close(void);
	virtual BLUEDWORD Write(const void* p, BLUEDWORD dwLen);
	virtual BLUEDWORD Read(void* p, BLUEDWORD dwLen);
	virtual void SeekToEnd(void);
	virtual void SeekToBegin(void);

protected:
	HANDLE m_hFile;
};

//class CBLUEFileLog : public IBLUEFileLog
//{
//public:
//	CBLUEFileLog(void);
//	virtual bool Create(BLUEString& strFileName);
//	virtual void Close(void);
//	virtual bool Write(BLUEString& str);
//	virtual bool WriteLine(BLUEString& str);
//	virtual bool WriteEmptyLine(void);
//
//protected:
//	bool UnicodeProcess(void);//Unicode版处理
//
//protected:
//	IBLUEFile* m_pFile;
//};

class CBLUEThread : public IBLUEThread
{
public:
	CBLUEThread(void);
	~CBLUEThread(void);
	virtual bool Create(const PBLUETHREAD_FUNCTION pfn, void* lpParameter, const bool bSuspended);
	virtual void Close(const BLUEDWORD dwWaitThreadTime);
	virtual void SafeClose(void);
	virtual bool Suspend(void) const;
	virtual bool Resume(void) const;

protected:
	static BLUEDWORD WINAPI stThreadFunction(void* lpParameter);//默认线程函数

protected:
	HANDLE m_hThread;                                                     //线程的handle
	BLUEDWORD m_dwThreadId;                                       //线程的id号
	PBLUETHREAD_FUNCTION m_pfnThreadFunction;        //线程所要运行的函数
	void* m_lpParameter;                                                  //线程所要运行的参数
};

//
//class CBLUEServiceApplicationHelper : public IBLUEServiceApplicationForwardlyHelper, public IBLUEServiceApplicationPassivelyHelper
//{
//public:
//	CBLUEServiceApplicationHelper(void);
//	virtual void Destruction(void);
//	virtual bool Start(IBLUEServiceApplication* pSA);
//	virtual void NotifyStopped(void);
//	virtual void NotifyPaused(void);
//	virtual void NotifyRunning(void);
//
//protected:
//	static VOID WINAPI stServiceMain(DWORD dwNumServicesArgs, LPTSTR* lpServiceArgVectors);
//	static VOID WINAPI stHandler(DWORD dwControl);
//	static CBLUEServiceApplicationHelper* s_pCurrentHelper;
//
//	BLUEVOID ServiceMain(BLUEDWORD dwNumServicesArgs, BLUELPTSTR* lpServiceArgVectors);
//	BLUEVOID Handler(BLUEDWORD dwControlCode);
//	BLUEBOOL SendStatus(DWORD dwCurrentState,
//		DWORD dwWin32ExitCode = NO_ERROR,
//		DWORD dwServiceSpecificExitCode = 0,
//		DWORD dwCheckPoint = 0,
//		DWORD dwWaitHint = 0);
//
//private:
//	IBLUEServiceApplication* m_pSA;//指向服务程序对象
//	SERVICE_STATUS_HANDLE m_ssh;//服务状态发送器
//};