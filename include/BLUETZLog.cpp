#include "BLUETZLog.h"
#include <stdarg.h>

bool CBLUETZFileA::Open(const char* lpstrFileName, const char* lpstrMode)
{
	assert(lpstrFileName);
	assert(lpstrMode);

	//先关闭原来的文件，否则可能存在资源泄漏
	Close();

	//打开文件
	m_file = ::fopen(lpstrFileName, lpstrMode);
	return (m_file != NULL);
}

void CBLUETZFileA::Close(void)
{
	if (m_file)
	{
		::fclose(m_file);
		m_file = NULL;
	}
}

size_t CBLUETZFileA::Read(void* lpBuf, size_t nSize) const
{
	assert(m_file);
	assert(lpBuf);
	return ::fread(lpBuf, 1, nSize, m_file);
}

size_t CBLUETZFileA::Write(const void* lpBuf, size_t nSize) const
{
	assert(m_file);
	assert(lpBuf);
	return ::fwrite(lpBuf, 1, nSize, m_file);
}

long CBLUETZFileA::GetLength(void) const
{
	assert(m_file);

	//先得到当前位置
	long lCurPos = ::ftell(m_file);
	//将文件移到最后
	::fseek(m_file, 0, SEEK_END);
	//得到文件长度
	long lResult = ::ftell(m_file);
	//将文件指针移动到最初位置
	::fseek(m_file, lCurPos, SEEK_SET);

	return lResult;
}

bool CBLUETZFileA::Seek(long nOffset, FSEEKORIGIN origin) const
{
	assert(m_file);
	int _org;
	if (origin == FSO_START)
		_org = SEEK_SET;
	else if (origin == FSO_END)
		_org = SEEK_END;
	else
		_org = SEEK_CUR;

	return ::fseek(m_file, nOffset, _org) == 0;//返回0表示移动正确
}

int CBLUETZLog::s_nWriteMask = TZLOG_WRITEMASK_DEFAULT;//默认掩码，所有日志都打印

bool CBLUETZLog::Create(const char* lpstrLogFileName, int nWriteFlag)
{
	assert(lpstrLogFileName);

	SetWriteFlag(nWriteFlag);

	//记下使用的日志文件名
	m_strCurFileName = lpstrLogFileName;

	//得出最前的日志文件日期
	m_tmCurrentFile = *_getLocalTime();

	//根据日志文件日期创建日志文件
	return _createLogFile(m_tmCurrentFile);
}

void CBLUETZLog::Close(void)
{
	m_file.Close();
}

bool CBLUETZLog::WriteLog(const char* lpstrLog, bool bAddTime, int nWriteFlag)
{
	assert(lpstrLog);

	if (nWriteFlag == 0)//如果写入标志为0，则使用变量成员变量m_nWriteFlag作为使用的写入标志
		nWriteFlag = m_nWriteFlag;

	if ((nWriteFlag & s_nWriteMask) == 0)//判断写入标志，是否要打印日志
		return false;

	/*
	先判断是否需要改换写入的日志文件（日期不一样了）
	与当前写入的日志文件的日期比较，如果不一样则先建立新的日志文件
	*/
	tm* pLocalTm = _getLocalTime();
	if (pLocalTm->tm_year != m_tmCurrentFile.tm_year ||
		pLocalTm->tm_mon != m_tmCurrentFile.tm_mon ||
		pLocalTm->tm_mday != m_tmCurrentFile.tm_mday)
	{
		Close();
		_createLogFile(*pLocalTm);
		m_tmCurrentFile = *pLocalTm;
	}

	//写入时间到日志
	size_t nStrLen;
	if (bAddTime)
	{
		char strTime[64];
		nStrLen = ::sprintf(
			strTime,
			"[%2d:%02d:%02d]",
			pLocalTm->tm_hour,
			pLocalTm->tm_min,
			pLocalTm->tm_sec
			);
		m_file.Write(strTime, nStrLen);
	}

	//写入实际内容到日志
	nStrLen = ::strlen(lpstrLog);
	if (m_file.Write(lpstrLog, nStrLen) == nStrLen)
	{
		m_file.Flush();//一定要flush，不然日志无法马上真正的写进文件
		return true;
	}
	return false;
}

bool CBLUETZLog::WriteLineLog(const char* lpstrLogFmt, ...)
{
	assert(lpstrLogFmt);

	char strOut[1024];
	va_list ptr;
	va_start(ptr, lpstrLogFmt);
	::vsprintf(strOut, lpstrLogFmt, ptr);
	va_end(ptr);

	if (WriteLog(strOut))
		if (WriteLineSymbol())
			return true;

	return false;
}

bool CBLUETZLog::WriteLineLog(int nWriteFlag, const char* lpstrLogFmt, ...)
{
	assert(lpstrLogFmt);

	char strOut[1024];
	va_list ptr;
	va_start(ptr, lpstrLogFmt);
	::vsprintf(strOut, lpstrLogFmt, ptr);
	va_end(ptr);

	if (WriteLog(strOut, true, nWriteFlag))
		if (WriteLineSymbol(nWriteFlag))
			return true;

	return false;
}

bool CBLUETZLog::_createLogFile(const tm& t)
{
	//得出日志文件名的后缀
	char strSuffix[64];
	::sprintf(
		strSuffix, ".%04d%02d%02d.log",
		t.tm_year + 1900,
		t.tm_mon + 1,
		t.tm_mday
		);

	//文件名加上后缀得出真正的文件名
	std::string strCreateFileName;
	strCreateFileName += m_strCurFileName;
	strCreateFileName += strSuffix;

	//创建日志文件
	return m_file.Open(strCreateFileName.c_str(), "at");
}