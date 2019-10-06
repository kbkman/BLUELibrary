#pragma once
#include "BLUEDef.h"
#include "BLUEDebug.h"
#include "BLUEList.h"

/**
类名：栈
说明：固定大小的栈，如果要动态大小，请使用CBLUELinkStack（链式栈）
注意：不支持多线程
*/
template<class T>
class CBLUEStack
{
public:
	/**
	函数名：构造一个栈
	参数：nInitMaxCount 栈的初始元素个数（不能<= 0)，默认为512*sizeof(T)个字节
	*/
	CBLUEStack(const int nInitMaxCount = 512);

	/**
	函数名：析构一个栈
	*/
	~CBLUEStack(void);

public:
	int GetCount(void) const {return m_pTop - m_pData;}//获取栈元素个数
	bool IsEmpty(void) const {return m_pTop == m_pData;}//栈是否为空
	bool IsFull(void) const {return m_pTop == (m_pData + m_nMaxCount);}//栈满
	bool Push(const T value);//入栈
	bool Pop(T& value);//出栈

private:
	const int m_nMaxCount;//栈内可以存储压入个T型数据
	T* m_pData;//栈数据
	T* m_pTop;//当前的栈顶指针
};

template<class T>
CBLUEStack<T>::CBLUEStack(const int nInitMaxCount) : m_nMaxCount(nInitMaxCount)
{
	BLUEASSERT(m_nCount > 0);
	m_pData = new T[m_nMaxCount];
	m_pTop = m_pData;
}

template<class T>
CBLUEStack<T>::~CBLUEStack(void)
{
	delete[] m_pData;
}

template<class T>
bool CBLUEStack<T>::Push(const T value)
{
	if (!IsFull())
	{
		*m_pTop = value;
		m_pTop++;
		return true;
	}
	return false;
}

template<class T>
bool CBLUEStack<T>::Pop(T& value)
{
	if (!IsEmpty())
	{
		m_pTop--;
		value = *m_pTop;
		return true;
	}
	return false;
}

/**
类名：链式栈（没有大小限制）
说明：为了效率且大小可知情况下请使用CBLUEStack栈（固定大小的栈）
注意：不支持多线程
*/
template<class T>
class CBLUELinkStack
{
public:
	/**
	函数名：构造一个链式栈
	*/
	CBLUELinkStack(void) {}

	/**
	函数名：析构一个链式栈
	*/
	~CBLUELinkStack(void) {}

public:
	int GetCount(void) const {return m_list.GetCount();}//获取栈元素个数
	bool IsEmpty(void) const {return m_list.IsEmpty();}//栈是否为空
	void Push(const T value) {m_list.AddHead(value);}//入栈
	T Pop(void) {return m_list.RemoveHead();}//出栈

private:
	CBLUEList<T> m_list;
};