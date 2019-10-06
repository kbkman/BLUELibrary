#pragma once
#include "BLUEDef.h"
#include "BLUEDebug.h"
#include "BLUEList.h"

/**
类名：队列
说明：固定大小的队列，如果要动态大小，请使用CBLUELinkQueue（链式队列）
注意：不支持多线程
*/
template<class T>
class CBLUEQueue
{
public:
	/**
	函数名：构造一个队列
	参数：nInitMaxCount 队列的初始元素个数（不能<= 0)，默认为512*sizeof(T)个字节
	*/
	CBLUEQueue(const int nInitMaxCount = 512);

	/**
	函数名：析构一个队列
	*/
	~CBLUEQueue(void);

public:
	/*
	函数名：获取队列的当前长度
	*/
	int GetCount(void) const {return m_nCount;}

	/*
	函数名：队列是否为空
	*/
	bool IsEmpty(void) const {return GetCount() == 0;}

	/*
	函数名：队列是否满了
	*/
	bool IsFull(void) const {return GetCount() == m_nMaxCount;}

	/*
	函数名：入队
	参数：value 入队元素
	返回：是否成功（如果队列满了，会返回false）
	*/
	bool Put(const T value);

	/*
	函数名：出队
	参数：value 出队元素（被填充）
	返回：是否成功（如果队列满了，会返回false）
	*/
	bool Get(T& value);

	/*
	函数名：查看队首元素
	参数：value 填充元素
	*/
	bool First(T& value);

	/*
	函数名：查看队尾元素
	参数：value 填充元素
	*/
	bool Last(T& value);

private:
	const int m_nMaxCount;//栈内可以存储压入个T型数据
	int m_nCount;//当前队列中元素个数
	T* m_pData;//队列数据
	T* m_pHead;//当前的队列头
	T* m_pTail;//当前队列尾
};

template<class T>
CBLUEQueue<T>::CBLUEQueue(const int nInitMaxCount) :
m_nMaxCount(nInitMaxCount),
m_nCount(0)
{
	BLUEASSERT(m_nMaxCount > 0);
	m_pData = new T[m_nMaxCount];
	m_pHead = m_pData;
	m_pTail = m_pData;
}

template<class T>
CBLUEQueue<T>::~CBLUEQueue(void)
{
	delete[] m_pData;
}

template<class T>
bool CBLUEQueue<T>::Put(const T value)
{
	//队列未满，则可以入队
	if (!IsFull())
	{
		*m_pTail = value;
		m_nCount++;
		if (++m_pTail - m_pData >= m_nMaxCount)
			m_pTail = m_pData;
		return true;
	}
	return false;
}

template<class T>
bool CBLUEQueue<T>::Get(T& value)
{
	//队列未空，可以出队
	if (!IsEmpty())
	{
		value = *m_pHead;
		m_nCount--;
		if (++m_pHead - m_pData >= m_nMaxCount)
			m_pHead = m_pData;
		return true;
	}
	return false;
}

template<class T>
bool CBLUEQueue<T>::First(T& value)
{
	//队列未空，可以查看队首
	if (!IsEmpty())
	{
		value = *m_pHead;
		return true;
	}
	return false;
}

template<class T>
bool CBLUEQueue<T>::Last(T& value)
{
	//队列未空，可以查看队尾
	if (!IsEmpty())
	{
		value = *(m_pTail - 1);
		return true;
	}
	return false;
}

/**
类名：链式队列
说明：可扩展队列，比固定队列CBLUEQueue灵活，但效率相对低
注意：不支持多线程
*/
template<class T>
class CBLUELinkQueue
{
public:
	CBLUELinkQueue(void) {}
	~CBLUELinkQueue(void) {}

public:
	int GetCount(void) const {return m_list.GetCount();}//获取队列的当前长度
	bool IsEmpty(void) const {return m_list.IsEmpty();}//队列是否为空
	void Put(const T value) {m_list.AddTail(value);};//入队（队尾添加）
	T Get(void) {return m_list.RemoveHead();}//出队（队首删除）注：使用前请先确定队列不为空
	T First(void) const {return m_list.GetHead();}//获取队首元素（不删除队首）注：使用前请先确定队列不为空
	T Last(void) const {return m_list.GetTail();}//获取队尾元素（不删除队尾）注：使用前请先确定队列不为空

private:
	CBLUEList<T> m_list;
};