#pragma once
#include "BLUEDef.h"
#include "BLUEDebug.h"

/**
类名：数组（向量）
功能：拥有可变长的数组
注意：不支持多线程
*/
template<class T>
class CBLUEArray
{
public:
	/**
	函数名：构造一个数组
	*/
	CBLUEArray(const int nInitialCapacity, const int nCapacityIncrement, const int nInitialSize, const T initialValue) {Init(nInitialCapacity, nCapacityIncrement, nInitialSize, initialValue);}
	CBLUEArray(const int nInitialCapacity, const int nCapacityIncrement, const int nInitialSize) {Init(nInitialCapacity, nCapacityIncrement, nInitialSize);}
	CBLUEArray(const int nInitialCapacity, const int nCapacityIncrement) {Init(nInitialCapacity, nCapacityIncrement);}
	CBLUEArray(const int nInitialCapacity) {Init(nInitialCapacity);}
	CBLUEArray(void) {Init(30);}
	~CBLUEArray(void);

public:
	/*
	函数名：获取当前数组元素个数
	说明：该返回值为元素有效个数，区别与容量
	*/
	int GetSize(void) const {return m_nSize;}
	
	/*
	函数名：获取当前数组容量
	说明：该返回值为容量，区别与元素个数
	*/
	int GetCapacity(void) const {return m_nCapacity;}

	/*
	函数名：重载[]下标符
	参数：nIndex 下标
	*/
	 T& operator[](const int nIndex) const {BLUEASSERT(nIndex >= 0 && nIndex < m_nSize);return m_pData[nIndex];}

	/*
	函数名：在数组最后添加一个元素
	参数：value 元素值
	*/
	void Add(const T value);

	/*
	函数名：删除最后一个元素
	*/
	void Del(void);

	/*
	函数名：插入一个元素
	参数：nIndex 在该索引之后插入
	　　　value 元素值
	*/
	void Insert(const int nIndex, const T value);

	/*
	函数名：删除一个元素
	参数：nIndex 删除该索引的元素
	*/
	void Remove(const int nIndex);


private:
	/**
	函数名：初始一个数组
	参数：nInitialCapacity 初始容量，实际分配字节数为sizeof(T) * nInitialCapacity
	　　　nCapacityIncrement 超出容量出，一次性增加多少容量，实际增加字节数为sizeof(T) * nCapacityIncrement
	　　　nInitialSize 初始元素个数，可以用该参数事先预备几个可用元素，默认为0
	　　　initialValue 初始元素的默认值
	说明：
	*/
	void Init(const int nInitialCapacity, const int nCapacityIncrement, const int nInitialSize, const T initialValue);

	/*
	参阅Init
	*/
	void Init(const int nInitialCapacity, const int nCapacityIncrement, const int nInitialSize);

	/*
	参阅Init
	*/
	void Init(const int nInitialCapacity, const int nCapacityIncrement);

	/*
	参阅Init
	*/
	void Init(const int nInitialCapacity);

	/*
	函数名：是否为满
	*/
	bool IsFull(void) const {return m_nCapacity <= m_nSize;}

	/*
	函数名：重新分配内存
	*/
	void ReAlloc(void);

private:
	T* m_pData;//数组实际数据区
	int m_nSize;//当前元素个数
	int m_nCapacity;//当前的容量
	int m_nCapacityIncrement;//每次容量增加的数量
};

template<class T>
CBLUEArray<T>::~CBLUEArray(void)
{
	delete[] m_pData;
}

template<class T>
void CBLUEArray<T>::Init(const int nInitialCapacity, const int nCapacityIncrement, const int nInitialSize, const T initialValue)
{
	BLUEASSERT(nInitialCapacity > 0);
	BLUEASSERT(nCapacityIncrement > 0);
	BLUEASSERT(nInitialSize >= 0);

	m_nSize = nInitialSize;//初始元素个数
	m_nCapacity = (nInitialSize > nInitialCapacity) ? nInitialSize : nInitialCapacity;//初始容量
	m_nCapacityIncrement = nCapacityIncrement;//初始容量增加量

	m_pData = new T[m_nCapacity];

	//初始要元素
	for (int i = 0; i < m_nSize; i++)
		m_pData[i] = initialValue;
}

template<class T>
void CBLUEArray<T>::Init(const int nInitialCapacity, const int nCapacityIncrement, const int nInitialSize)
{
	BLUEASSERT(nInitialCapacity > 0);
	BLUEASSERT(nCapacityIncrement > 0);
	BLUEASSERT(nInitialSize >= 0);

	m_nSize = nInitialSize;//初始元素个数
	m_nCapacity = (nInitialSize > nInitialCapacity) ? nInitialSize : nInitialCapacity;//初始容量
	m_nCapacityIncrement = nCapacityIncrement;//初始容量增加量

	m_pData = new T[m_nCapacity];
}

template<class T>
void CBLUEArray<T>::Init(const int nInitialCapacity, const int nCapacityIncrement)
{
	Init(nInitialCapacity, nCapacityIncrement, 0);
}

template<class T>
void CBLUEArray<T>::Init(const int nInitialCapacity)
{
	Init(nInitialCapacity, 30);
}

template<class T>
void CBLUEArray<T>::ReAlloc(void)
{
	BLUEASSERT(IsFull());

	//新容量大小（旧内存加上每扩展的大小）
	const int nNewCapacity = m_nCapacity + m_nCapacityIncrement;

	//根据新容量重新申请新的内存
	T* pNewData = new T[nNewCapacity];

	//将旧内存中的内容复制到新内存中
	::memcpy(pNewData, m_pData, m_nCapacity * sizeof(T));

	//删除旧内存
	delete[] m_pData;

	//改变成员
	m_pData = pNewData;
	m_nCapacity = nNewCapacity;
}

template<class T>
inline void CBLUEArray<T>::Add(const T value)
{
	//如果已经满了，则重新分配
	if (IsFull())
		ReAlloc();

	m_pData[m_nSize++] = value;
}

template<class T>
inline void CBLUEArray<T>::Del(void)
{
	//如果元素个数大于0个，则减1
	if (m_nSize > 0)
		m_nSize--;
}

template<class T>
void CBLUEArray<T>::Insert(const int nIndex, const T value)
{
	BLUEASSERT(nIndex >= 0 && nIndex < m_nSize);

	//如果已经满了，则多分配新内存
	if (IsFull())
		ReAlloc();

	//将插入点之后所有元素向后移一个单位
	for (int i = m_nSize; i > nIndex; i--)
		m_pData[i] = m_pData[i - 1];

	//写入插入值
	m_pData[nIndex] = value;

	//元素个数加1
	m_nSize++;
}

template<class T>
void CBLUEArray<T>::Remove(const int nIndex)
{
	BLUEASSERT(nIndex >= 0 && nIndex < m_nSize);

	//将nIndex后的所有元素都向前移一个元素
	for (int i = nIndex + 1; i < m_nSize; i++)
		m_pData[i - 1] = m_pData[i];

	//如果元素个数大于0个，则减1
	if (m_nSize > 0)
		m_nSize--;
}