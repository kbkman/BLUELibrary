#ifndef __BLUEDEFWIN32_H__
#define __BLUEDEFWIN32_H__
#include <string>

//基本宏
#define BLUENULL           0

//特定长度的数据类型
typedef char                      BLUET8;             //8位(1字节)
typedef short                     BLUET16;          //16位(2字节)
typedef int                         BLUET32;          //32位(4字节)
typedef __int64                  BLUET64;         //64位(8字节)
typedef unsigned char         BLUEUT8;       //无符号8位(1字节)
typedef unsigned short        BLUEUT16;     //无符号16位(2字节)
typedef unsigned int             BLUEUT32;    //无符号32位(4字节)
typedef unsigned __int64      BLUEUT64;   //无符号64位(8字节)

//一些无符号数据常规定义
typedef unsigned long        BLUEDWORD;       //无符号长整形
typedef unsigned short       BLUEWORD;         //无符号短整形
typedef unsigned char         BLUEBYTE;           //无符号字节（0~255）

//字符串类型定义
typedef char                  BLUECHAR, *BLUELPSTR;
typedef wchar_t            BLUEWCHAR, *BLUELPWSTR;
typedef const char         *BLUELPCSTR;
typedef const wchar_t   *BLUELPCWSTR;
typedef std::string          BLUEStringA;
typedef std::wstring       BLUEStringW;

//受是否使用Unicode影响的字符串类型定义
#ifdef  UNICODE
typedef wchar_t             BLUETCHAR;
typedef BLUELPWSTR     BLUELPTSTR;
typedef BLUELPCWSTR   BLUELPCTSTR;
typedef BLUEStringW      BLUEString;
#define _BLUET(x)          L##x
#else
typedef char                   BLUETCHAR;
typedef BLUELPSTR         BLUELPTSTR;
typedef BLUELPCSTR       BLUELPCTSTR;
typedef BLUEStringA        BLUEString;
#define _BLUET(x)           x
#endif

#endif