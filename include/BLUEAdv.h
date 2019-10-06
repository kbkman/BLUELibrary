#ifndef __BLUEADV_H__
#define __BLUEADV_H__

#include <BLUEDef.h>

#ifdef BLUEADV_EXPORTS
#define BLUEADV_API _declspec(dllexport)
#else
#define BLUEADV_API _declspec(dllimport)
#endif


///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//////////////////调试操作/////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
BLUEADV_API void BLUEAdvDebugBreak(void);

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//////////////////系统操作/////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
/*
函数名：获取CPU处理器数量
功能：
说明：
参数：
返回值：CPU处理器数量
*/
BLUEADV_API BLUEDWORD BLUEAdvGetNumberOfCPU(void);

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//////////////////字符操作/////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
/*
函数名：将ANSI字符串转成UNICODE字符串
功能：略
说明：无
参数：strSrc ANSI字符串
　　　strOut 返回的UNICODE字符串
   返回值：等于strOut
   */
BLUEADV_API BLUEStringW& BLUEA2W(const BLUEStringA& strSrc, BLUEStringW& strOut);

/*
函数名：将UNICODE字符串转成ANSI字符串
功能：略
说明：无
参数：strSrc UNICODE字符串
　　　strOut 返回的ANSI字符串
   返回值：等于strOut
   */
BLUEADV_API BLUEStringA& BLUEW2A(const BLUEStringW& strSrc, BLUEStringA& strOut);

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//////////////////路径操作/////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
/*
函数名：得到当前应用程序文件的路径
如：c:\path\a.exe
*/
BLUEADV_API void BLUEAdvGetCurrentProcessFilePathA(BLUEStringA& strFilePath);
BLUEADV_API void BLUEAdvGetCurrentProcessFilePathW(BLUEStringW& strFilePath);

/*
函数名：得到当前应用程序文件的所有目录路径
如：c:\path
*/
BLUEADV_API void BLUEAdvGetCurrentProcessFileDirectoryPathA(BLUEStringA& strDirectoryPath);
BLUEADV_API void BLUEAdvGetCurrentProcessFileDirectoryPathW(BLUEStringW& strDirectoryPath);

/*
函数名：得到当前应用程序文件的名称
如：a.exe
*/
BLUEADV_API void BLUEAdvGetCurrentProcessFileNameA(BLUEStringA& strFileName);
BLUEADV_API void BLUEAdvGetCurrentProcessFileNameW(BLUEStringW& strFileName);

/*
函数名：得到当前目录
*/
BLUEADV_API void BLUEAdvGetCurrentDirectoryPathA(BLUEStringA& strDirectoryPath);
BLUEADV_API void BLUEAdvGetCurrentDirectoryPathW(BLUEStringW& strDirectoryPath);

/*
函数名：设置当前目录
*/
BLUEADV_API void BLUEAdvSetCurrentDirectoryPathA(const BLUEStringA& strDirectoryPath);
BLUEADV_API void BLUEAdvSetCurrentDirectoryPathW(const BLUEStringW& strDirectoryPath);

#ifdef UNICODE
#define BLUEAdvGetCurrentProcessFilePath  BLUEAdvGetCurrentProcessFilePathW
#define BLUEAdvGetCurrentProcessFileDirectoryPath  BLUEAdvGetCurrentProcessFileDirectoryPathW
#define BLUEAdvGetCurrentProcessFileName  BLUEAdvGetCurrentProcessFileNameW
#define BLUEAdvGetCurrentDirectoryPath  BLUEAdvGetCurrentDirectoryPathW
#define BLUEAdvSetCurrentDirectoryPath  BLUEAdvSetCurrentDirectoryPathW
#else
#define BLUEAdvGetCurrentProcessFilePath  BLUEAdvGetCurrentProcessFilePathA
#define BLUEAdvGetCurrentProcessFileDirectoryPath  BLUEAdvGetCurrentProcessFileDirectoryPathA
#define BLUEAdvGetCurrentProcessFileName  BLUEAdvGetCurrentProcessFileNameA
#define BLUEAdvGetCurrentDirectoryPath  BLUEAdvGetCurrentDirectoryPathA
#define BLUEAdvSetCurrentDirectoryPath  BLUEAdvSetCurrentDirectoryPathA
#endif // !UNICODE

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//////////////////安全线程队列/////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
#define BLUEMSMQ_MEESAGETYPE_QUIT				0
#define BLUEMSMQ_MESSAGETYPE_BASE				0

/**
接口名：多线程安全消息队列
功能：
说明：
版本：1.0.0.1
作者：BLUE
*/
class IBLUEMTSafeMessageQueue
{
public:
	virtual ~IBLUEMTSafeMessageQueue(void) {}

public:
	/*
	函数名：投递一条消息
	功能：略
	说明：无
	参数：dwMessageType 消息类型
	　　　firstParam 消息具体参数1
　　　　secondParam 消息具体参数2
	　　　thirdParam 消息具体参数3
	返回值：无（肯定成功）
	*/
	virtual void PutOneMessage(const BLUEDWORD dwMessageType, const BLUEDWORD firstParam, void* const secondParam, void* const thirdParam) = 0;


	/*
	函数名：取出一条消息
	功能：略
	说明：无
	参数：dwMessageType 消息类型
	　　　firstParam 消息具体参数1
	　　　secondParam 消息具体参数2
	　　　thirdParam 消息具体参数3
	返回值：取出的消息是否为QUIT退出消息
	*/
	virtual bool GetOneMessage(BLUEDWORD& dwMessageType, BLUEDWORD& firstParam, void*& secondParam, void*& thirdParam) = 0;

	/*
	函数名：视探一条消息
	功能：从消息队列中视探有没有消息，如果有则取出，如果没有则马上返回
	说明：无
	参数：dwMessageType 消息类型
	　　　firstParam 消息具体参数1
	　　　secondParam 消息具体参数2
	　　　thirdParam 消息具体参数3
	返回值：是否取到消息
	*/
	virtual bool PeekOneMessage(BLUEDWORD& dwMessageType, BLUEDWORD& firstParam, void*& secondParam, void*& thirdParam) = 0;

	/*
	函数名：投递一条退出消息
	功能：略
	说明：该消息的作用会让GetOneMessage返回false
	参数：无
	返回值：无
	*/
	virtual void PutOneQuitMessage(void) = 0;
};

/*
函数名：创建一个多线程安全消息队列对象
功能：无
说明：无
参数：
返回值：多线程安全消息队列对象接口
*/
BLUEADV_API IBLUEMTSafeMessageQueue* BLUEAdvCreateMSMQ(void);

/*
函数名：销毁一个多线程安全消息队列对象
功能：
说明：
参数：pMSMQ 要销毁的多线程安全消息队列对象
返回值：无
*/
BLUEADV_API void BLUEAdvDestroyMSMQ(IBLUEMTSafeMessageQueue* pMSMQ);


///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////会话/////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
/**
接口名：会话接口
功能：略
说明：无
版本：1.0.0.1
作者：BLUE
*/
class IBLUESession
{
public:
	IBLUESession(void) : m_userKey(0), m_bIsClosed(false)/*新创建的会话一定是连接着的，不可能为关闭着的，实现者必须保证*/ {}
	~IBLUESession(void) {}

public:
	/*
	函数名：删除用户会话
	功能：略
	说明：删除后，该会话接口不可以再使用（是否真正删除由实现者决定）
	参数：无
	返回值：无
	*/
	virtual void Delete(void) = 0;

	/*
	函数名：关闭用户会话
	功能：略
	说明：大多数情况下是断开用户，具体根据实现者定义
	参数：无
	返回值：无
	*/
	virtual void Close(void) = 0;

public:
	/*
	函数名：会话是否关闭了
	功能：略
	说明：无
	参数：无
	返回值：无
	*/
	bool IsClose(void) {return m_bIsClosed;}

	/*
	函数名：获取会话用户键值
	功能：用户通过使用该参数来存在自定义数据
	说明：无
	参数：无
	返回值：用户键值
	*/
	void* GetUserKey() const {return m_userKey;}

	/*
	函数名：设置会话用户键值
	功能：参阅GetUserKey
	说明：无
	参数：dwUserKey 用户键值
	返回值：无
	*/
	void SetUserKey(void* const userKey) {m_userKey = userKey;}

private:
	void* m_userKey;//附带参数，用户可以根据自己需要保存不同的值

protected:
	bool m_bIsClosed;//是否已经关闭，初始为false
};


/**
接口名：会话端IO数据接口
功能：略
说明：无
版本：1.0.0.1
作者：BLUE
*/
class IBLUESessionIOData
{
public:
	/*
	函数名：获取缓冲区信息
	功能：略
	说明：无
	参数：lpBuf 返回的缓冲区地址
	　　　nBufLen 返回的缓冲区长度
	返回值：无
	*/
	virtual void GetBufferInfo(char*& lpBuf, unsigned long& nBufLen) = 0;
};

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//////////////////文件操作/////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//文件操作接口
#define BLUEFILE_ACCESS_ALL                  0x00000000L
#define BLUEFILE_ACCESS_READ               0x00000001L
#define BLUEFILE_ACCESS_WRITE              0x00000002L

#define BLUEFILE_SHAREMODE_NO            0x00000000L
#define BLUEFILE_SHAREMODE_DELETE     0x00000001L
#define BLUEFILE_SHAREMODE_READ         0x00000002L
#define BLUEFILE_SHAREMODE_WRITE       0x00000004L

#define BLUEFILE_CREATEMODE_NEW                           1
#define BLUEFILE_CREATEMODE_ALWAYS                      2
#define BLUEFILE_CREATEMODE_OPENEXISTING            3
#define BLUEFILE_CREATEMODE_OPENALWAYS              4
#define BLUEFILE_CREATEMODE_TRUNCATEEXISTING     5

#define BLUEFILE_ATT_ARCHIVE                 0x00000001L
#define BLUEFILE_ATT_HIDDEN                  0x00000002L
#define BLUEFILE_ATT_NORMAL                  0x00000004L
#define BLUEFILE_ATT_READONLY              0x00000008L
#define BLUEFILE_ATT_SYSTEM                  0x00000010L
#define BLUEFILE_ATT_TEMP                      0x00000020L

/**
接口名：文件对象接口
功能：略
说明：无
版本：1.0.0.1
作者：BLUE
*/
class IBLUEFile
{
public:
	virtual ~IBLUEFile(void) {}//虚析构（必写）

public:
	virtual bool Create(
		const BLUEString& strFileName,
		BLUEDWORD dwAccess,
		BLUEDWORD dwShareMode,
		BLUEDWORD dwCreationMode,
		BLUEDWORD dwAttributes) = 0;
	virtual void Close(void) = 0;
	virtual BLUEDWORD Write(const void* p, BLUEDWORD dwLen) = 0;
	virtual BLUEDWORD Read(void* p, BLUEDWORD dwLen) = 0;
	virtual void SeekToEnd(void) = 0;
	virtual void SeekToBegin(void) = 0;
	//virtual void Seek(BLUEDWORD dwSeek) = 0;
};

/*
函数名：创建一个文件对象接口
功能：略
说明：无
参数：无
返回值：文件对象接口
*/
BLUEADV_API IBLUEFile* BLUEAdvCreateFile(void);

/*
函数名：销毁一个文件对象
功能：略
说明：无
参数：pFile 文件对象接口
返回值：无
*/
BLUEADV_API void BLUEAdvDestroyFile(IBLUEFile* pFile);

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//////////////////IBLUEFileLog/////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class IBLUEFileLog
{
public:
	virtual ~IBLUEFileLog(void) {}//虚析构（必写）

public:
	virtual bool Create(BLUEString& strFileName) = 0;
	virtual void Close(void) = 0;
	virtual bool Write(BLUEString& str) = 0;
	virtual bool WriteLine(BLUEString& str) = 0;
	virtual bool WriteEmptyLine(void) = 0;
};

BLUEADV_API IBLUEFileLog* BLUEAdvCreateFileLog(void);
BLUEADV_API void BLUEAdvDestroyFileLog(IBLUEFileLog* pLog);

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//////////////////线程操作/////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//线程操作接口
typedef BLUEDWORD (*PBLUETHREAD_FUNCTION)(void* lpThreadParameter);//线程的回调函数格式

/**
接口名：线程对象接口
功能：操作线程对象
说明：并发基础对象
版本：1.0.0.1
作者：BLUE
*/
class IBLUEThread
{
public:
	virtual ~IBLUEThread(void) {}//虚析构（必写）

public:
	/*
	函数名：创建线程
	功能：创建一个实际的线程
	说明：无
	参数：pfn 线程运行的函数指针
	　　　lpParameter 线程运行函数运行时传入的一个参数（一般用作额外信息传递）
　　　　bSuspended 创建后是否先挂起（即不马上运行），如果挂起则调用Resume后再运行
	返回值：是否创建成功
	*/
	virtual bool Create(const PBLUETHREAD_FUNCTION pfn, void* lpParameter, const bool bSuspended) = 0;

	/*
	函数名：关闭线程
	说明：无
	参数：dwWaitThreadTime 关闭线程时等待的时间
	返回值：无
	*/
	virtual void Close(const BLUEDWORD dwWaitThreadTime) = 0;//在指定时间内结束线程销毁资源

	/*
	函数名：等线程退出后结束线程和销毁资源
	说明：安全关闭线程
	参数：无
	返回值：无
	*/
	virtual void SafeClose(void) = 0;

	/*
	函数名：挂起一个线程
	说明：无
	参数：无
	返回值：无
	*/
	virtual bool Suspend(void) const = 0;

	/*
	函数名：恢复一个挂起线程
	说明：无
	参数：无
	返回值：无
	*/
	virtual bool Resume(void) const = 0;
};

/*
函数名：创建线程对象
说明：无
参数：无
返回值：线程对象接口
*/
BLUEADV_API IBLUEThread* BLUEAdvCreateThread(void);

/*
函数名：销毁线程对象
说明：无
参数：p 线程对象接口
返回值：无
*/
BLUEADV_API void BLUEAdvDestroyThread(IBLUEThread* p);

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//////////////////服务程序帮助者操作/////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//BLUEADV_API IBLUEServiceApplicationForwardlyHelper* BLUEAdvCreateSAH(void);
//BLUEADV_API void BLUEAdvDestroySAH(IBLUEServiceApplicationForwardlyHelper* p);


#endif