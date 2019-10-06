#pragma once
#include "BLUEDef.h"
#include "BLUEDebug.h"

template<class T> class CBLUEBinaryTree;

/**
类名：二叉树结构
*/
template<class T>
class CBLUEBinaryTreeNode
{

private:
	T m_data;//保存的数据
	CBLUEBinaryTreeNode<T> m_leftTree;//左孩子树
	CBLUEBinaryTreeNode<T> m_rightTree;//右孩子树

	friend class CBLUEBinaryTree<T>;
};

/**
类名：二叉树
*/
template<class T>
class CBLUEBinaryTree
{
public:
	CBLUEBinaryTree(void) : m_pRoot(BLUENULL) {}
	~CBLUEBinaryTree(void) {}


private:
	CBLUEBinaryTreeNode<T>* m_pRoot;//根结点
};

/**
类名：最大堆
*/
template<class T>
class CBLUEMaxHeap : protected CBLUEBinaryTree<T>
{
public:
	CBLUEMaxHeap(void);
	~CBLUEMaxHeap(void);

public:
	void Push(const T value);//压入一个堆结点
	void Pop(void);//弹出一个堆结点
	void Output(void);//输出

};