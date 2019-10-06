#include "BLUEAdvWin32.h"
#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
#include <BLUEWin32.h>

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//////////////////调试///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
void BLUEAdvDebugBreak(void)
{
	::DebugBreak();
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//////////////////系统/////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
BLUEDWORD BLUEAdvGetNumberOfCPU(void)
{
	SYSTEM_INFO sinfo;
	::GetSystemInfo(&sinfo);
	return sinfo.dwNumberOfProcessors;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//////////////////字符操作/////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
BLUEStringW& BLUEA2W(const BLUEStringA& strSrc, BLUEStringW& strOut)
{
	DWORD dwWStrLen = ::MultiByteToWideChar(CP_ACP, 0, strSrc.c_str(), -1, NULL, 0);
	if (dwWStrLen > 0)
	{
		wchar_t* pWStr;
		pWStr = new wchar_t[dwWStrLen];
		//转换
		::MultiByteToWideChar(CP_ACP, 0, strSrc.c_str(), -1, pWStr, dwWStrLen);
		strOut = pWStr;
		delete[] pWStr;
	}
	return strOut;
}

BLUEStringA& BLUEW2A(const BLUEStringW& strSrc, BLUEStringA& strOut)
{
	DWORD dwWStrLen = ::WideCharToMultiByte(CP_ACP, 0, strSrc.c_str(), -1, NULL, 0, NULL, FALSE);
	if (dwWStrLen > 0)
	{
		char* pAStr = new char[dwWStrLen];
		//转换
		::WideCharToMultiByte (CP_ACP, NULL, strSrc.c_str(), -1, pAStr, dwWStrLen, NULL, FALSE);
		strOut = pAStr;
		delete[] pAStr;
	}
	return strOut;
}
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//////////////////路径/////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
void BLUEAdvGetCurrentProcessFilePathA(BLUEStringA& strFilePath)
{
	BLUECHAR strTemp[MAX_PATH];
	::GetModuleFileNameA(BLUENULL, strTemp, MAX_PATH);
	strFilePath = strTemp;
}

void BLUEAdvGetCurrentProcessFilePathW(BLUEStringW& strFilePath)
{
	BLUEWCHAR strTemp[MAX_PATH];
	::GetModuleFileNameW(BLUENULL, strTemp, MAX_PATH);
	strFilePath = strTemp;
}

void BLUEAdvGetCurrentProcessFileDirectoryPathA(BLUEStringA& strDirectoryPath)
{
	BLUEAdvGetCurrentProcessFilePathA(strDirectoryPath);
	BLUEStringA::size_type nSplitIndex = strDirectoryPath.rfind('\\');
	strDirectoryPath.erase(nSplitIndex, strDirectoryPath.size() - nSplitIndex);
}

void BLUEAdvGetCurrentProcessFileDirectoryPathW(BLUEStringW& strDirectoryPath)
{
	BLUEAdvGetCurrentProcessFilePathW(strDirectoryPath);
	BLUEStringW::size_type nSplitIndex = strDirectoryPath.rfind('\\');
	strDirectoryPath.erase(nSplitIndex, strDirectoryPath.size() - nSplitIndex);
}

void BLUEAdvGetCurrentProcessFileNameA(BLUEStringA& strFileName)
{
	BLUEAdvGetCurrentProcessFilePathA(strFileName);
	strFileName.erase(0, strFileName.rfind('\\') + 1);
}

void BLUEAdvGetCurrentProcessFileNameW(BLUEStringW& strFileName)
{
	BLUEAdvGetCurrentProcessFilePathW(strFileName);
	strFileName.erase(0, strFileName.rfind('\\') + 1);
}

void BLUEAdvGetCurrentDirectoryPathA(BLUEStringA& strDirectoryPath)
{
	BLUEDWORD dwLen;
	dwLen = ::GetCurrentDirectoryA(0, BLUENULL);
	if (dwLen > 0) {
		char* p = new char[dwLen];
		if (::GetCurrentDirectoryA(dwLen, p) > 0)
			strDirectoryPath = p;
		delete[] p;
	}
}

void BLUEAdvGetCurrentDirectoryPathW(BLUEStringW& strDirectoryPath)
{
	BLUEDWORD dwLen;
	dwLen = ::GetCurrentDirectoryW(0, BLUENULL);
	if (dwLen > 0) {
		wchar_t* p = new wchar_t[dwLen];
		if (::GetCurrentDirectoryW(dwLen, p) > 0)
			strDirectoryPath = p;
		delete[] p;
	}
}

void BLUEAdvSetCurrentDirectoryPathA(BLUEStringA& strDirectoryPath)
{
	::SetCurrentDirectoryA(strDirectoryPath.c_str());
}

void BLUEAdvSetCurrentDirectoryPathW(BLUEStringW& strDirectoryPath)
{
	::SetCurrentDirectoryW(strDirectoryPath.c_str());
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//////////////////CBLUEMTSafeMessageQueue//////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
CBLUEMTSafeMessageQueue::CBLUEMTSafeMessageQueue(void)
{
	m_hHasMessageEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
}

CBLUEMTSafeMessageQueue::~CBLUEMTSafeMessageQueue(void)
{
	::CloseHandle(m_hHasMessageEvent);

	//清除所有队列中的消息
	m_msgQueue.clear();
}

void CBLUEMTSafeMessageQueue::PutOneMessage(const BLUEDWORD dwMessageType, const BLUEDWORD firstParam, void* const secondParam, void* const thirdParam)
{
	//生成新消息
	MTSAFEMSG msg;

	//给消息赋值
	msg.dwMessageType = dwMessageType;
	msg.firstParam = firstParam;
	msg.secondParam = secondParam;
	msg.thirdParam = thirdParam;

	//在消息队列尾部添加该消息
	m_msgQueueCS.Lock();//消息链表上锁
	m_msgQueue.push_back(msg);//投递消息
	::SetEvent(m_hHasMessageEvent);//设置事件开启，这样GetOneMessage不会在等待状态了
	m_msgQueueCS.Unlock();//消息链表解锁
}

bool CBLUEMTSafeMessageQueue::GetOneMessage(BLUEDWORD& dwMessageType, BLUEDWORD& firstParam, void*& secondParam, void*& thirdParam)
{
	//等待消息队列中是否有消息的通知
	if (::WaitForSingleObject(m_hHasMessageEvent, INFINITE) != WAIT_OBJECT_0)
		return false;//返回false的情况几乎为0

	m_msgQueueCS.Lock();//消息队列上锁
	if (!m_msgQueue.empty())//如果消息队列存在消息的话，则取消息
	{
		MTSAFEMSG& msg = m_msgQueue.front();//获取第一条消息
		dwMessageType =  msg.dwMessageType;
		firstParam = msg.firstParam;
		secondParam = msg.secondParam;
		thirdParam = msg.thirdParam;
		m_msgQueue.pop_front();//弹出第一条消息
		m_msgQueueCS.Unlock();//消息队列解锁
		return (dwMessageType != BLUEMSMQ_MEESAGETYPE_QUIT);
	}
	else//如果没有消息了
	{
		::ResetEvent(m_hHasMessageEvent);//重置事件，这样不会浪费CPU资源，直到有消息时开启这个事件
		m_msgQueueCS.Unlock();//消息队列解锁
		return GetOneMessage(dwMessageType, firstParam, secondParam, thirdParam);//继续调用GetOneMessage直到取得消息
	}
}

bool CBLUEMTSafeMessageQueue::PeekOneMessage(BLUEDWORD& dwMessageType, BLUEDWORD& firstParam, void*& secondParam, void*& thirdParam)
{
	m_msgQueueCS.Lock();//消息队列上锁
	if (!m_msgQueue.empty())//如果消息队列存在消息的话，则取消息
	{
		MTSAFEMSG& msg = m_msgQueue.front();//获取第一条消息
		dwMessageType =  msg.dwMessageType;
		firstParam = msg.firstParam;
		secondParam = msg.secondParam;
		thirdParam = msg.thirdParam;
		m_msgQueue.pop_front();//弹出第一条消息
		m_msgQueueCS.Unlock();//消息队列解锁
		return true;
	}
	m_msgQueueCS.Unlock();//消息队列解锁
	return false;
}

void CBLUEMTSafeMessageQueue::PutOneQuitMessage(void)
{
	return PutOneMessage(BLUEMSMQ_MEESAGETYPE_QUIT, 0, BLUENULL, BLUENULL);
}

IBLUEMTSafeMessageQueue* BLUEAdvCreateMSMQ(void)
{
	CBLUEMTSafeMessageQueue* p = new CBLUEMTSafeMessageQueue();
	return static_cast<IBLUEMTSafeMessageQueue*>(p);
}

void BLUEAdvDestroyMSMQ(IBLUEMTSafeMessageQueue* pMSMQ)
{
	BLUEASSERT(pMSMQ);
	delete pMSMQ;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//////////////////CBLUEFile/////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
CBLUEFile::CBLUEFile(void) :
m_hFile(INVALID_HANDLE_VALUE)
{
}

CBLUEFile::~CBLUEFile(void)
{
	if (m_hFile != INVALID_HANDLE_VALUE)
		Close();
}

bool CBLUEFile::Create(const BLUEString& strFileName,
						   BLUEDWORD dwAccess,
						   BLUEDWORD dwShareMode,
						   BLUEDWORD dwCreationMode,
						   BLUEDWORD dwAttributes)
{
	BLUEDWORD dwDesAccess;
	BLUEDWORD dwDesShareMode;
	BLUEDWORD dwDesCreationMode;
	BLUEDWORD dwDesAttributes;

	dwDesAccess = 0;
	dwDesShareMode = 0;
	dwDesCreationMode = 0;
	dwDesAttributes = 0;

	if (dwAccess == BLUEFILE_ACCESS_ALL) dwDesAccess = GENERIC_ALL;
	if (dwAccess & BLUEFILE_ACCESS_READ) dwDesAccess |= GENERIC_READ;
	if (dwAccess & BLUEFILE_ACCESS_WRITE) dwDesAccess |= GENERIC_WRITE;

	if (dwShareMode == BLUEFILE_SHAREMODE_NO) dwDesShareMode = 0;
	if (dwShareMode & BLUEFILE_SHAREMODE_READ) dwDesShareMode |= FILE_SHARE_READ;
	if (dwShareMode & BLUEFILE_SHAREMODE_WRITE) dwDesShareMode |= FILE_SHARE_WRITE;

	switch (dwCreationMode)
	{
	case BLUEFILE_CREATEMODE_NEW: dwDesCreationMode = CREATE_NEW;break;
	case BLUEFILE_CREATEMODE_ALWAYS: dwDesCreationMode = CREATE_ALWAYS;break;
	case BLUEFILE_CREATEMODE_OPENEXISTING: dwDesCreationMode = OPEN_EXISTING;break;
	case BLUEFILE_CREATEMODE_OPENALWAYS: dwDesCreationMode = OPEN_ALWAYS;break;
	case BLUEFILE_CREATEMODE_TRUNCATEEXISTING: dwDesCreationMode = TRUNCATE_EXISTING;break;
	}

	if (dwAttributes & BLUEFILE_ATT_ARCHIVE) dwDesAttributes |= FILE_ATTRIBUTE_ARCHIVE;
	if (dwAttributes & BLUEFILE_ATT_HIDDEN) dwDesAttributes |= FILE_ATTRIBUTE_HIDDEN;
	if (dwAttributes & BLUEFILE_ATT_NORMAL) dwDesAttributes |= FILE_ATTRIBUTE_NORMAL;
	if (dwAttributes & BLUEFILE_ATT_READONLY) dwDesAttributes |= FILE_ATTRIBUTE_READONLY;
	if (dwAttributes & BLUEFILE_ATT_SYSTEM) dwDesAttributes |= FILE_ATTRIBUTE_SYSTEM;
	if (dwAttributes & BLUEFILE_ATT_TEMP) dwDesAttributes |= FILE_ATTRIBUTE_TEMPORARY;

	m_hFile = ::CreateFile(strFileName.c_str(),
		dwDesAccess,
		dwDesShareMode,
		BLUENULL,
		dwDesCreationMode,
		dwDesAttributes,
		BLUENULL);

	return (m_hFile != INVALID_HANDLE_VALUE);
}

void CBLUEFile::Close(void)
{
	BLUEASSERT(m_hFile != INVALID_HANDLE_VALUE);
	::CloseHandle((HANDLE)m_hFile);
	m_hFile = BLUENULL;
}

BLUEDWORD CBLUEFile::Write(const void* p, BLUEDWORD dwLen)
{
	BLUEDWORD dwWrittenLen = 0;
	BLUEASSERT(m_hFile != INVALID_HANDLE_VALUE);
	::WriteFile(m_hFile, p, dwLen, &dwWrittenLen, NULL);
	return dwWrittenLen;
}

BLUEDWORD CBLUEFile::Read(void* p, BLUEDWORD dwLen)
{
	BLUEDWORD dwReadedLen = 0;
	BLUEASSERT(m_hFile != INVALID_HANDLE_VALUE);
	::ReadFile(m_hFile, p, dwLen, &dwReadedLen, NULL);
	return dwReadedLen;
}

void CBLUEFile::SeekToBegin(void)
{
	::SetFilePointer(m_hFile, 0, 0, FILE_BEGIN);
}

void CBLUEFile::SeekToEnd(void)
{
	::SetFilePointer(m_hFile, 0, 0, FILE_END);
}

IBLUEFile* BLUEAdvCreateFile(void)
{
	CBLUEFile* p = new CBLUEFile();
	return static_cast<IBLUEFile*>(p);
}

void BLUEAdvDestroyFile(IBLUEFile* pFile)
{
	BLUEASSERT(pFile);
	delete pFile;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//////////////////CBLUEFileLog////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//CBLUEFileLog::CBLUEFileLog(void) :
//m_pFile(BLUENULL)
//{
//}
//
//void CBLUEFileLog::Destruction(void)
//{
//	if (m_pFile)
//		Close();
//}
//
//bool CBLUEFileLog::Create(BLUEString& strFileName)
//{
//	m_pFile = ::BLUEAdvCreateFile();
//	if (m_pFile)
//	{
//		if (m_pFile->Create(strFileName,
//			BLUEFILE_ACCESS_WRITE,
//			BLUEFILE_SHAREMODE_READ | BLUEFILE_SHAREMODE_WRITE,
//			BLUEFILE_CREATEMODE_OPENALWAYS,
//			BLUEFILE_ATT_NORMAL))
//		{
//			if (!UnicodeProcess())//Unicode版文本处理
//				return BLUEFALSE;
//
//			m_pFile->SeekToEnd();
//			return BLUETRUE;
//		}
//	}
//	return BLUEFALSE;
//}
//
//void CBLUEFileLog::Close(void)
//{
//	BLUEASSERT(m_pFile);
//	::BLUEAdvDestroyFile(m_pFile);
//	m_pFile = BLUENULL;
//}
//
//bool CBLUEFileLog::Write(BLUEString& str)
//{
//	BLUEASSERT(m_pFile);
//	BLUEDWORD dwWriteLen;//需要写入的字节数
//	BLUEDWORD dwWrittenLen;//写入的字节数
//
//	dwWriteLen = str.GetLength() * sizeof(BLUETCHAR);
//	if (dwWriteLen > 0)
//	{
//		dwWrittenLen = m_pFile->Write((BLUELPCVOID)str, str.GetLength() * sizeof(BLUETCHAR));
//		return (dwWrittenLen == dwWriteLen);
//	}
//	return BLUETRUE;
//}
//
//bool CBLUEFileLog::WriteLine(BLUEString& str)
//{
//	return Write(str + _BLUET("\r\n"));
//}
//
//bool CBLUEFileLog::WriteEmptyLine(void)
//{
//	return WriteLine(BLUEString(_BLUET("")));
//}
//
////Unicode版文本处理
//bool CBLUEFileLog::UnicodeProcess(void)
//{
//#ifdef UNICODE
//	BLUEASSERT(m_pFile);
//
//	unsigned char byteHeadArray[2];
//	BLUEDWORD dwReadedLen;
//	BLUEDWORD dwHeadArrayLen;
//
//	byteHeadArray[0] = 0;
//	byteHeadArray[1] = 0;
//	dwHeadArrayLen = sizeof(byteHeadArray);
//	m_pFile->SeekToBegin();
//	//读到文件前两个字节
//	dwReadedLen = m_pFile->Read((BLUELPVOID)byteHeadArray, dwHeadArrayLen);
//	//如果第一和第二个字节不是0xFFFE，则表示该文本文件不是Unicode的形式，则需要转成Unicode
//	if (byteHeadArray[0] != 0xFF ||
//		byteHeadArray[1] != 0xFE)
//	{
//		byteHeadArray[0] = 0xFF;
//		byteHeadArray[1] = 0xFE;
//		m_pFile->SeekToBegin();
//		return m_pFile->Write(&byteHeadArray, dwHeadArrayLen) == dwHeadArrayLen;
//	}
//#endif
//
//	return true;
//}
//
//IBLUEFileLog* BLUEAdvCreateFileLog(void)
//{
//	CBLUEFileLog* p = new CBLUEFileLog();
//	return static_cast<IBLUEFileLog*>(p);
//}
//
//void BLUEAdvDestroyFileLog(IBLUEFileLog* pFileLog)
//{
//	BLUEASSERT(pFileLog);
//	delete pFileLog;
//}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//////////////////CBLUEThread线程/////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
CBLUEThread::CBLUEThread(void) :
m_hThread(NULL),
m_dwThreadId(0)
{
}

CBLUEThread::~CBLUEThread(void)
{
	if (m_hThread)
		Close(0);
}

bool CBLUEThread::Create(const PBLUETHREAD_FUNCTION pfn, void* lpParameter, const bool bSuspended)
{
	BLUEASSERT(m_hThread == NULL);
	BLUEASSERT(pfn);

	DWORD dwCreateFlags;
	bSuspended ? dwCreateFlags = CREATE_SUSPENDED : dwCreateFlags = NULL;

	//把函数和参数都赋值
	m_pfnThreadFunction = pfn;
	m_lpParameter = lpParameter;

	//创建线程
	m_hThread = ::CreateThread(NULL, 0,
		stThreadFunction, this, dwCreateFlags,
		&m_dwThreadId);

	return (m_hThread != BLUENULL);
}

void CBLUEThread::Close(const BLUEDWORD dwWaitThreadTime)
{
	BLUEASSERT(m_hThread);

	//开始等待线程运行结束
	BLUEDWORD dwWaitRet = ::WaitForSingleObject(m_hThread, dwWaitThreadTime);//判断线程的状态
	if (dwWaitRet == WAIT_TIMEOUT)//线程存在，但还没有结束
		::TerminateThread(m_hThread, -1);//强行终止线程

	//关闭handle
	::CloseHandle(m_hThread);

	//清理值
	m_hThread = NULL;
	m_dwThreadId = 0;
}

void CBLUEThread::SafeClose(void)
{
	Close(INFINITE);
}

bool CBLUEThread::Suspend(void) const
{
	BLUEASSERT(m_hThread);
	return (::SuspendThread(m_hThread) != -1);
}

bool CBLUEThread::Resume(void) const
{
	BLUEASSERT(m_hThread);
	return (::ResumeThread(m_hThread) != -1);
}

BLUEDWORD WINAPI CBLUEThread::stThreadFunction(void* lpParameter)
{
	BLUEASSERT(lpParameter);

	CBLUEThread* p = (CBLUEThread*)lpParameter;
	return (*p->m_pfnThreadFunction)(p->m_lpParameter);
}

IBLUEThread* BLUEAdvCreateThread(void)
{
	IBLUEThread* p = new CBLUEThread();
	return static_cast<IBLUEThread*>(p);
}

void BLUEAdvDestroyThread(IBLUEThread* p)
{
	BLUEASSERT(p);
	delete p;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//////////////////CBLUEServiceApplicationHelper///////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//CBLUEServiceApplicationHelper* CBLUEServiceApplicationHelper::s_pCurrentHelper = BLUENULL;
//CBLUEServiceApplicationHelper::CBLUEServiceApplicationHelper() :
//m_pSA(BLUENULL),
//m_ssh(NULL)
//{
//	s_pCurrentHelper = this;
//}
//
//void CBLUEServiceApplicationHelper::Destruction(void)
//{
//	if (s_pCurrentHelper == this)
//		s_pCurrentHelper = BLUENULL;
//}
//
//bool CBLUEServiceApplicationHelper::Start(IBLUEServiceApplication* pSA)
//{
//	BLUEASSERT(m_ssh == NULL);
//	BLUEASSERT(pSA);
//
//	SERVICE_TABLE_ENTRY st[2];
//
//	st[0].lpServiceName = (LPTSTR)pSA->GetServiceName();
//	st[0].lpServiceProc = stServiceMain;
//	st[1].lpServiceName = NULL;
//	st[1].lpServiceProc = NULL;
//
//	m_pSA = pSA;
//
//	return (::StartServiceCtrlDispatcher(st) == TRUE);
//}
//
//void CBLUEServiceApplicationHelper::NotifyStopped(void)
//{
//	SendStatus(SERVICE_STOPPED);
//}
//
//void CBLUEServiceApplicationHelper::NotifyPaused(void)
//{
//	SendStatus(SERVICE_PAUSED);
//}
//
//void CBLUEServiceApplicationHelper::NotifyRunning(void)
//{
//	SendStatus(SERVICE_RUNNING);
//}
//
//VOID CBLUEServiceApplicationHelper::stServiceMain(DWORD dwNumServicesArgs, LPTSTR* lpServiceArgVectors)
//{
//	BLUEASSERT(s_pCurrentHelper);
//	s_pCurrentHelper->ServiceMain(dwNumServicesArgs, lpServiceArgVectors);
//}
//
//VOID CBLUEServiceApplicationHelper::stHandler(DWORD dwControl)
//{
//	BLUEASSERT(s_pCurrentHelper);
//	s_pCurrentHelper->Handler(dwControl);
//}
//
//void CBLUEServiceApplicationHelper::ServiceMain(BLUEDWORD dwNumServicesArgs, BLUELPTSTR* lpServiceArgVectors)
//{
//	BLUEASSERT(m_pSA);
//
//	//注册一个Handler
//	m_ssh = ::RegisterServiceCtrlHandler(
//		m_pSA->GetServiceName(),
//		(LPHANDLER_FUNCTION)stHandler);
//	if (m_ssh == NULL)
//	{
//		SendStatus(SERVICE_STOPPED);
//		return;
//	}
//
//	//为服务程序设置一个被动帮助者
//	m_pSA->SetAppPassivelyHelper(static_cast<IBLUEServiceApplicationPassivelyHelper*>(this));
//
//	//运行服务程序
//	m_pSA->Run();
//}
//
//void CBLUEServiceApplicationHelper::Handler(BLUEDWORD dwControlCode)
//{
//	BLUEASSERT(m_pSA);
//	switch(dwControlCode)
//	{
//	case SERVICE_CONTROL_STOP://要求停止服务程序
//		SendStatus(SERVICE_STOP_PENDING);
//		m_pSA->Stop();
//		break;
//
//	case SERVICE_CONTROL_PAUSE://要求暂定服务程序
//		SendStatus(SERVICE_PAUSE_PENDING);
//		m_pSA->Pause();
//		break;
//
//	case SERVICE_CONTROL_CONTINUE://要求继续服务程序
//		SendStatus(SERVICE_CONTINUE_PENDING);
//		m_pSA->Continue();
//		break;
//
//	case SERVICE_CONTROL_INTERROGATE://返回服务器更新状态
//		break;
//
//	case SERVICE_CONTROL_SHUTDOWN://提示服务器关闭
//		SendStatus(SERVICE_STOP_PENDING);
//		m_pSA->Stop();
//		break;
//
//	}
//}
//
//bool CBLUEServiceApplicationHelper::SendStatus(DWORD dwCurrentState,
//		DWORD dwWin32ExitCode,
//		DWORD dwServiceSpecificExitCode,
//		DWORD dwCheckPoint,
//		DWORD dwWaitHint)
//{
//	BLUEASSERT(m_ssh);
//
//	//设置一个SERVICE_STATUS
//	SERVICE_STATUS serviceStatus;
//	serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
//	serviceStatus.dwCurrentState	= dwCurrentState;
//	if (dwCurrentState == SERVICE_START_PENDING)
//	{
//		serviceStatus.dwControlsAccepted = 0;
//	}
//	else
//	{
//		serviceStatus.dwControlsAccepted =
//			SERVICE_ACCEPT_STOP |
//			SERVICE_ACCEPT_PAUSE_CONTINUE |
//			SERVICE_ACCEPT_SHUTDOWN;
//	}
//	if (dwServiceSpecificExitCode == 0)
//	{
//		serviceStatus.dwWin32ExitCode = dwWin32ExitCode;
//	}
//	else
//	{
//		serviceStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
//	}
//	serviceStatus.dwServiceSpecificExitCode	= dwServiceSpecificExitCode;
//	serviceStatus.dwCheckPoint = dwCheckPoint;
//	serviceStatus.dwWaitHint = dwWaitHint;
//
//	// 使用SetServiceStatus将SERVICE_STATUS发送给SCM
//	return (::SetServiceStatus(m_ssh, &serviceStatus) == TRUE);
//}
//
//BLUEADV_API IBLUEServiceApplicationForwardlyHelper* BLUEAdvCreateSAH(void)
//{
//	CBLUEServiceApplicationHelper* p = new CBLUEServiceApplicationHelper();
//	return static_cast<IBLUEServiceApplicationForwardlyHelper*>(p);
//}
//
//BLUEADV_API void BLUEAdvDestroySAH(IBLUEServiceApplicationForwardlyHelper* p)
//{
//	BLUEASSERT(p);
//	delete p;
//}