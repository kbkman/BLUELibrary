#ifndef __BLUELIST_H__
#define __BLUELIST_H__
#include "BLUEDef.h"
#include "BLUEDebug.h"

/**
命名：位置
功能：用于链表等数据结构，供用户外部使用
*/
typedef void* BLUEPOSITION;
/**
类名：链表结点结构
功能：用于CBLUEList链表
版本：1.0.0.1
*/
template<class T>
struct BLUELISTNODE
{
	BLUELISTNODE<T>* pNext;//前一个结点
	BLUELISTNODE<T>* pPrev;//后一个结点
	T data;//当前结点存储的数据
};

/**
模板类名：链表
模板参数：T 链表内保存的类型，如果是复合类型建议使用该类型的指针类型
注意：不支持多线程
版本：1.0.0.1
*/
template<class T>
class CBLUEList
{
public:
	CBLUEList(void);
	~CBLUEList(void);

	int GetCount(void) const {return m_nCount;}//获取链表结点数量
	bool IsEmpty(void) const {return m_nCount == 0;}//判断链表是否为空

	BLUEPOSITION AddHead(const T newElement);//为链表头加个结点
	BLUEPOSITION AddTail(const T newElement);//为链表尾加个结点
	BLUEPOSITION InsertBefore(const BLUEPOSITION position, const T newElement);//在链表某个位置之前插入一个结点
	BLUEPOSITION InsertAfter(const BLUEPOSITION position, const T newElement);//在链表某个位置之后插入一个结点
	BLUEPOSITION GetHeadPosition(void) const {return m_pNodeHead;}//获取链表头结点
	BLUEPOSITION GetTailPosition(void) const {return m_pNodeTail;}//获取链表尾结点
	T RemoveAt(const BLUEPOSITION position);//删除某个结点
	T RemoveHead(void) {return RemoveAt(m_pNodeHead);}//删除头结点
	T RemoveTail(void) {return RemoveAt(m_pNodeTail);}//删除尾结点
	void RemoveAll(void);//删除所有结点
	T GetAt(const BLUEPOSITION position) const;//获取指定位置的结点值
	T GetNext(BLUEPOSITION& rPosition) const;//获取指定位置之后的一个结点值
	T GetPrev(BLUEPOSITION& rPosition) const;//获取指定位置之前的一个结点值
	T GetHead(void) const {return GetAt(GetHeadPosition());}//获取头元素
	T GetTail(void) const {return GetAt(GetTailPosition());}//获取尾元素
	void SetAt(const BLUEPOSITION position, const T newElement);//为一个结点重新设置新值

private:
	BLUELISTNODE<T>* NewNode(const T newElement) const;//新建立一个结点
	void FreeNode(const BLUELISTNODE<T>* pNode) const;//删除一个结点

private:
	BLUELISTNODE<T>* m_pNodeHead;//头结点
	BLUELISTNODE<T>* m_pNodeTail;//尾结点
	int m_nCount;//当前结点数量
};

template<class T>
CBLUEList<T>::CBLUEList(void) :
m_pNodeHead(BLUENULL),
m_pNodeTail(BLUENULL),
m_nCount(0)
{
}

template<class T>
CBLUEList<T>::~CBLUEList(void)
{
	RemoveAll();//析构时删除所有结点
}

template<class T>
BLUELISTNODE<T>* CBLUEList<T>::NewNode(const T newElement) const
{
	BLUELISTNODE<T>* pNode = new BLUELISTNODE<T>;
	pNode->data = newElement;
	pNode->pPrev = BLUENULL;
	pNode->pNext = BLUENULL;
	return pNode;
}

template<class T>
inline void CBLUEList<T>::FreeNode(const BLUELISTNODE<T>* pNode) const
{
	BLUEASSERT(pNode);
	delete pNode;
}

template<class T>
BLUEPOSITION CBLUEList<T>::AddHead(const T newElement)
{
	BLUELISTNODE<T>* pNode = NewNode(newElement);//生成一个结点
	if (m_pNodeHead)//如果头结点存在
	{
		BLUEASSERT(m_pNodeHead->pPrev == BLUENULL);//头结点的pPrev值应该为BLUENULL
		m_pNodeHead->pPrev = pNode;//当前头结点的pPrev指向生成的结点
		pNode->pNext = m_pNodeHead;//将生成的结点的pNext赋值为现在的头结点
	}
	m_pNodeHead = pNode;//头结点指向生成的结点

	if (m_pNodeTail == BLUENULL)//如果尾结点没有指向，则也指向头结点
		m_pNodeTail = m_pNodeHead;

	m_nCount++;//结点数量加1
	return pNode;//返回生成的结点
}

template<class T>
BLUEPOSITION CBLUEList<T>::AddTail(const T newElement)
{
	if (m_pNodeTail)//如果尾结点存在
	{
		BLUELISTNODE<T>* pNode = NewNode(newElement);//生成一个结点
		BLUEASSERT(m_pNodeTail->pNext == BLUENULL);//尾结点的pNext值应该为BLUENULL
		m_pNodeTail->pNext = pNode;//当前尾结点的pNext指向生成的结点
		pNode->pPrev = m_pNodeTail;//生成的结点的pPrev指向当前的尾结点
		m_pNodeTail = pNode;//尾结点指向生成的结点
		m_nCount++;//结点数量加1
		return pNode;//返回生成的结点
	}
	else//如果尾结点不存在，那么说明当前链表为空，则直接调用AddHead
	{
		return AddHead(newElement);
	}
}

template<class T>
BLUEPOSITION CBLUEList<T>::InsertBefore(const BLUEPOSITION position, const T newElement)
{
	BLUEASSERT(position);

	BLUELISTNODE<T>* pNodeInsertBefore;
	pNodeInsertBefore = (BLUELISTNODE<T>*)position;//插入点结点

	if (pNodeInsertBefore == m_pNodeHead)//如果插入对象结点是头结点，则直接调用AddHead即可
	{
		return AddHead(newElement);
	}
	else
	{

		BLUELISTNODE<T>* pNode;
		pNode = NewNode(newElement);//生成一个结点

		BLUELISTNODE<T>* pNodeInsertBeforePrev;//插入对象结点的前一个结点
		pNodeInsertBeforePrev = pNodeInsertBefore->pPrev;
		pNodeInsertBeforePrev->pNext = pNode;
		pNode->pPrev = pNodeInsertBeforePrev;
		pNode->pNext = pNodeInsertBefore;
		pNodeInsertBefore->pPrev = pNode;
		m_nCount++;//结点数量加1
		return pNode;
	}
}

template<class T>
BLUEPOSITION CBLUEList<T>::InsertAfter(const BLUEPOSITION position, const T newElement)
{
	BLUEASSERT(position);

	BLUELISTNODE<T>* pNodeInsertAfter;
	pNodeInsertAfter = (BLUELISTNODE<T>*)position;//插入点结点

	if (pNodeInsertAfter == m_pNodeTail)//如果插入对象结点是尾结点，则直接调用AddTail即可
	{
		return AddTail(newElement);
	}
	else
	{
		BLUELISTNODE<T>* pNode;
		pNode = NewNode(newElement);//生成一个结点
		BLUELISTNODE<T>* pNodeInsertAfterNext;//插入对象结点的后一个结点
		pNodeInsertAfterNext = pNodeInsertAfter->pNext;
		pNodeInsertAfterNext->pPrev = pNode;
		pNode->pNext = pNodeInsertAfterNext;
		pNode->pPrev = pNodeInsertAfter;
		pNodeInsertAfter->pNext = pNode;
		m_nCount++;//结点数量加1
		return pNode;
	}
}

template<class T>
T CBLUEList<T>::RemoveAt(const BLUEPOSITION position)
{
	BLUEASSERT(position);

	BLUELISTNODE<T>* pNode;
	BLUELISTNODE<T>* pNodePrev;
	BLUELISTNODE<T>* pNodeNext;

	pNode = (BLUELISTNODE<T>*)position;//要删除的结点
	pNodePrev = pNode->pPrev;//要删除的结点的前一个结点
	pNodeNext = pNode->pNext;//要删除的结点的后一个结点
	T returnValue = pNode->data;

	if (pNodePrev == BLUENULL && pNodeNext == BLUENULL)//删除结点前后都无结点
	{
		BLUEASSERT(m_pNodeHead == pNode);
		BLUEASSERT(m_pNodeTail == pNode);
		FreeNode(pNode);
		m_pNodeHead = BLUENULL;
		m_pNodeTail = BLUENULL;
	}
	else if (pNodePrev != BLUENULL && pNodeNext == BLUENULL)//删除结点有前结点，无后结点
	{
		BLUEASSERT(m_pNodeHead != pNode);
		BLUEASSERT(m_pNodeTail == pNode);
		pNodePrev->pNext = BLUENULL;
		FreeNode(pNode);
		m_pNodeTail = pNodePrev;
	}
	else if (pNodePrev == BLUENULL && pNodeNext != BLUENULL)//删除结点无前结点，有后结点
	{
		BLUEASSERT(m_pNodeHead == pNode);
		BLUEASSERT(m_pNodeTail != pNode);
		pNodeNext->pPrev = BLUENULL;
		FreeNode(pNode);
		m_pNodeHead = pNodeNext;
	}
	else if (pNodePrev != BLUENULL && pNodeNext != BLUENULL)//删除结点有前结点，有后结点
	{
		BLUEASSERT(m_pNodeHead != pNode);
		BLUEASSERT(m_pNodeTail != pNode);
		pNodePrev->pNext = pNodeNext;
		pNodeNext->pPrev = pNodePrev;
		FreeNode(pNode);
	}

	m_nCount--;//结点数量减1
	return returnValue;
}

template<class T>
void CBLUEList<T>::RemoveAll(void)
{
	BLUELISTNODE<T>* pDeleteNode;
	BLUELISTNODE<T>* pTempNode;

	//遍历所有结点并删除
	pTempNode = m_pNodeHead;
	while (pTempNode != BLUENULL)
	{
		pDeleteNode = pTempNode;
		pTempNode = pTempNode->pNext;
		FreeNode(pDeleteNode);
	}

	//头尾结点设置成BLUENULL并设置结点数量为0
	m_pNodeHead = BLUENULL;
	m_pNodeTail = BLUENULL;
	m_nCount = 0;
}

template<class T>
T CBLUEList<T>::GetAt(const BLUEPOSITION position) const
{
	BLUEASSERT(position);

	BLUELISTNODE<T>* pNode;
	pNode = (BLUELISTNODE<T>*)position;
	return pNode->data;
}

template<class T>
T CBLUEList<T>::GetNext(BLUEPOSITION& rPosition) const
{
	BLUEASSERT(rPosition);

	BLUELISTNODE<T>* pNode;
	pNode = (BLUELISTNODE<T>*)rPosition;
	rPosition = pNode->pNext;
	return pNode->data;
}

template<class T>
T CBLUEList<T>::GetPrev(BLUEPOSITION& rPosition) const
{
	BLUEASSERT(rPosition);

	BLUELISTNODE<T>* pNode;
	pNode = (BLUELISTNODE<T>*)rPosition;
	rPosition = pNode->pPrev;
	return pNode->data;
}

template<class T>
void CBLUEList<T>::SetAt(const BLUEPOSITION position, const T newElement)
{
	BLUEASSERT(position);

	BLUELISTNODE<T>* pNode;
	pNode = (BLUELISTNODE<T>*)position;
	pNode->data = newElement;
}

#endif