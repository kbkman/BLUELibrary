#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

#include <cstdio>
#include <string>
#include <cassert>
#include <ctime>

/**
类名：一个简单的文件类（针对ANSI版）
功能：代表一个文件
版本：1.0.0.1
*/
class CBLUETZFileA
{
public:
	enum FSEEKORIGIN//文件指针类型
	{
		FSO_CUR,
		FSO_START,
		FSO_END
	};

public:
	CBLUETZFileA(void) : m_file(NULL) {}
	~CBLUETZFileA(void) {Close();}

public:
	/*
	函数名：打开文件
	参数：
	　　lpStrFileName 文件名
	　　lpstrMode 文件模式
	*/
	bool Open(const char* lpstrFileName, const char* lpstrMode);

	/*
	函数名：关闭文件
	*/
	void Close(void);

	/*
	函数名：文件是否打开了
	*/
	bool IsOpen(void) const {return m_file != NULL;}

	/*
	函数名：读取
	参数：
	　　lpBuf 接收缓冲区
	　　nSize 接收的大小（字节）
	返回：实际读出的大小（字节）
	*/
	size_t Read(void* lpBuf, size_t nSize) const;

	/*
	函数名：写入
	参数：
	　　lpBuf 写入缓冲区
	　　nSize 写入的大小（字节）
	返回：实际写入大小（字节）
	*/
	size_t Write(const void* lpBuf, size_t nSize) const;

	/*
	函数名：输出到文件
	*/
	void Flush(void) const {assert(m_file);::fflush(m_file);}

	/*
	函数名：获取文件大小
	*/
	long GetLength(void) const;

	/*
	函数名：获取当前文件指针的位置
	*/
	long GetCurPos(void) const {assert(m_file);return ::ftell(m_file);}

	/*
	函数名：移动文件指针
	*/
	bool Seek(long nOffset, FSEEKORIGIN origin = FSO_START) const;

	/*
	函数名：文件指针是否到了文件最后
	*/
	bool IsEOF(void) const {assert(m_file);return feof(m_file) != 0;}

private:
	FILE* m_file;//文件操作符
};

#define TZLOG_WRITEFLAG_DEFAULT     0x00000001    //默认的写入日志标志
#define TZLOG_WRITEMASK_DEFAULT    0xFFFFFFFF      //默认的写入日志掩码（即都写）

/**
类名：日志类
功能：无
说明：无
版本：1.0.0.1
作者：BLUE
*/
class CBLUETZLog
{
public:
	CBLUETZLog(void) {}
	~CBLUETZLog(void) {Close();}

public:
	bool Create(const char* lpstrLogFileName, int nWriteFlag = TZLOG_WRITEFLAG_DEFAULT);
	void Close(void);

	//是否可用
	bool IsVaild(void) const {return m_file.IsOpen();}

	//设置默认写入标志
	int GetWriteFlag(void) const {return m_nWriteFlag;}
	void SetWriteFlag(int nWriteFlag) {m_nWriteFlag = nWriteFlag;}

	/*
	函数名：写日志
	功能：略
	参数：
	　　lpstrLog 要写入日志的ascii码
	　　bAddTime 是否加入时间日志
	　　nWriteFlag 日志写入标志，用于控制是否写入日志
	返回值：无
	*/
	bool WriteLog(const char* lpstrLog, bool bAddTime = true, int nWriteFlag = 0);

	/*
	函数名：写入一个换行标志
	功能：略
	返回值：无
	*/
	bool WriteLineSymbol(int nWriteFlag = 0) {return WriteLog("\n", false, nWriteFlag);}

	//写入带省略参数的行日志
	bool  WriteLineLog(const char* lpstrLogFmt, ...);

	//写入带省略参数的行日志（带标志）
	bool WriteLineLog(int nFlag, const char* lpstrLogFmt, ...);

private:
	//根据参数t创建日志文件，文件名为m_strCurFileName+.日期
	bool _createLogFile(const tm& t);

	//获取本地时间
	static tm* _getLocalTime(void)
	{
		static tm* pTmLocal;
		time_t t = ::time(NULL);
		pTmLocal = ::localtime(&t);
		return pTmLocal;
	}

private:
	CBLUETZFileA m_file;
	std::string m_strCurFileName;
	tm m_tmCurrentFile;//当前日志文件所使用的日期
	int m_nWriteFlag;//当前日志文件写入日志的默认标志

public:
	/*
	日志掩码：日志掩码用于写入日志时根据flag来决定是否写入日志
	*/
	static int GetWriteMask(void) {return s_nWriteMask;}//获取当产日志掩码
	static void SetWriteMask(int nWriteMask) {s_nWriteMask = nWriteMask;}//设置当前日志掩码

private:
	static int s_nWriteMask;//写入日志的掩码
};