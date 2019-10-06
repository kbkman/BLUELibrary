#ifndef __BLUEDEF_H__
#define __BLUEDEF_H__

#include "BLUEDefWin32.h"
#include <BLUEDebug.h>

//通用宏定义
#define BLUEMAKELONG(low, high)     (((low) & 0xffff) | (((high) & 0xffff) << 16))
#define BLUELOWORD(l)                     ((l) & 0xffff)
#define BLUEHIWORD(l)                      (((l) >> 16) & 0xffff)

//通用结构类定义
class CBLUERect;   //矩形
class CBLUEPoint;  //点
class CBLUESize;   //大小（盒子）

/**
类名：点
功能：无
说明：无
版本：1.0.0.1
作者：BLUE
*/
class CBLUEPoint
{
public:
	CBLUEPoint(int nX, int nY) : m_nX(nX), m_nY(nY) {}
	CBLUEPoint(const CBLUEPoint& pt) : m_nX(pt.m_nX), m_nY(pt.m_nY) {}
	CBLUEPoint(void) : m_nX(0), m_nY(0) {}

public:
	int m_nX;
	int m_nY;
};
/**
类名：大小
功能：无
说明：无
版本：1.0.0.1
作者：BLUE
*/
class CBLUESize
{
public:
	CBLUESize(int nWidth, int nHeight) : m_nWidth(nWidth), m_nHeight(nHeight) {}

public:
	int m_nWidth;
	int m_nHeight;
};
/**
类名：矩形
功能：无
说明：无
版本：1.0.0.1
作者：BLUE
*/
class CBLUERect
{
public:
	CBLUERect(int nLeft, int nTop, int nRight, int nBottom) : m_nLeft(nLeft), m_nTop(nTop), m_nRight(nRight), m_nBottom(nBottom) {BLUEASSERT(m_nRight >= m_nLeft && m_nBottom >= m_nTop);}
	CBLUERect(const CBLUEPoint& ltPt, const CBLUEPoint& rbPt) : m_nLeft(ltPt.m_nX), m_nTop(ltPt.m_nY), m_nRight(rbPt.m_nX), m_nBottom(rbPt.m_nY) {BLUEASSERT(m_nRight >= m_nLeft && m_nBottom >= m_nTop);}
	CBLUERect(const CBLUEPoint& ltPt, const CBLUESize& size) : m_nLeft(ltPt.m_nX), m_nTop(ltPt.m_nY), m_nRight(m_nLeft + size.m_nWidth), m_nBottom(m_nTop + size.m_nHeight) {BLUEASSERT(m_nRight >= m_nLeft && m_nBottom >= m_nTop);}
	CBLUERect(void) {}

public:
	//获取矩形长宽
	int GetWidth(void) const {return m_nRight - m_nLeft;}
	int GetHeight(void) const {return m_nBottom - m_nTop;}

	/*
	函数名：一个点是否位于本矩形内
	功能：略
	说明：无
	参数：无
	返回值：无
	*/
	bool PointIn(const CBLUEPoint& pt)
	{
		return (pt.m_nX >= m_nLeft &&
			pt.m_nX <= m_nRight &&
			pt.m_nY >= m_nTop &&
			pt.m_nY <= m_nBottom);
	}

	/*
	函数名：根据绝对坐标的一个点得到其相对于本矩形的相对点
	功能：略
	说明：无
	参数：无
	返回值：无
	*/
	CBLUEPoint& RelativePoint(int nX, int nY, CBLUEPoint& outpt)
	{
		outpt.m_nX = nX - m_nLeft;
		outpt.m_nY = nY - m_nTop;
		return outpt;
	}

	CBLUEPoint& RelativePoint(const CBLUEPoint& pt, CBLUEPoint& outpt)
	{
		return RelativePoint(pt.m_nX, pt.m_nY, outpt);
	}
	
	/*
	函数名：根据绝对坐标的一个矩形得到其相对于本矩形的相对矩形
	功能：略
	说明：无
	参数：无
	返回值：无
	*/
	CBLUERect& RelativeRect(const CBLUERect& rt, CBLUERect& outrt)
	{
		outrt.m_nLeft = rt.m_nLeft - m_nLeft;
		outrt.m_nTop = rt.m_nTop - m_nTop;
		outrt.m_nRight = rt.m_nRight - m_nLeft;
		outrt.m_nBottom = rt.m_nBottom - m_nTop;
		return outrt;
	}

	/*
	函数名：根据相对坐标的一个点得到其相对于本矩形的绝对点
	功能：略
	说明：无
	参数：无
	返回值：无
	*/
	CBLUEPoint& AbsolutePoint(int nX, int nY, CBLUEPoint& outpt)
	{
		outpt.m_nX = nX + m_nLeft;
		outpt.m_nY = nY + m_nTop;
		return outpt;
	}

	CBLUEPoint& AbsolutePoint(const CBLUEPoint& pt, CBLUEPoint& outpt)
	{
		return AbsolutePoint(pt.m_nX, pt.m_nY, outpt);
	}

	/*
	函数名：根据相对坐标的一个矩形得到其相对于本矩形的绝对矩形
	功能：略
	说明：无
	参数：无
	返回值：无
	*/
	CBLUERect& AbsoluteRect(const CBLUERect& rt, CBLUERect& outrt)
	{
		outrt.m_nLeft = rt.m_nLeft + m_nLeft;
		outrt.m_nTop = rt.m_nTop + m_nTop;
		outrt.m_nRight = rt.m_nRight + m_nLeft;
		outrt.m_nBottom = rt.m_nBottom + m_nTop;
		return outrt;
	}

public:
	int m_nLeft;
	int m_nTop;
	int m_nRight;
	int m_nBottom;
};

#endif