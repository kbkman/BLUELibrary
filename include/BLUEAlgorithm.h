#pragma once
#include "BLUEDef.h"
#include "BLUEDebug.h"

/***********
描述：数据结构与算法
版本：1.0.0.1
作者：BLUE

***********/

/**
名称：交换
功能：交换两个变量的数据
参数：t1 第一个数据变量
　　　t2 第二个数据变量
*/
template<class T>
inline void BLUESwap(T& t1, T& t2)
{
	T temp = t1;
	t1 = t2;
	t2 = temp;
}

/**
名称：冒泡排序
功能：将数组中的元素由小到大排列
参数：pArray 数组
　　　n 数组个数
*/
template<class T>
void BLUEBubbleSort(T* pArray, int n)
{
	BLUEASSERT(pArray);
	for (int i = 0, mi = n - 1; i < mi; i++)
		for (int j = 0, mj = n - i - 1; j < mj; j++)//前n-i个元素冒一次泡
			if (pArray[j] > pArray[j + 1])
				::BLUESwap(pArray[j], pArray[j + 1]);
}

/**
名称：选择排序
功能：将数组中的元素由小到大排列
参数：pArray 数组
　　　n 数组个数
*/
template<class T>
void BLUESelectionSort(T* pArray, int n)
{
	BLUEASSERT(pArray);
	for (int i = 0; i < n; i++)
		for (int j = i + 1; j < n; j++)//从第i+1个元素到最后一个元素之间找一个最小的与第i个元素交换
			if (pArray[j] < pArray[i])
				::BLUESwap(pArray[j], pArray[i]);
}

/**
名称：插入排序
功能：将数组中的元素由小到大排列
参数：pArray 数组
　　　n 数组个数
*/
template<class T>
void BLUEInsertionSort(T* pArray, int n)
{
	BLUEASSERT(pArray);
	//从第2个元素开始到最后一个元素，第一个元素默认为有序表
	for (int i = 1; i < n; i++)
	{
		T curTemp = pArray[i];//临时存储当前要处理的元素

		//当前要处理的元素插入已序部分，插入后仍然有序
		int j;
		for (j = i - 1; j >= 0; j--)
		{
			//如果找到了插入点，则本次插入寻找结束
			if (curTemp > pArray[j])
				break;

			//否则寻找的元素向后移一位
			pArray[j + 1] = pArray[j];
		}

		//将当前要处理的元素放入正确的位置
		pArray[j + 1] = curTemp;
	}
}

/*
函数名：快速排序
功能：略
说明：无
参数：pArray 数组
　　　nLeftIndex 左索引
　　　nRightIndex 右索引
返回值：无
*/
template<class T>
void BLUEQuickSort(T* pArray, const int nLeftIndex, const int nRightIndex)
{
	BLUEASSERT(pArray);
	if (nLeftIndex < nRightIndex)
	{
		//取最后边一个值为轴
		T pivot = pArray[nLeftIndex];
		int i = nLeftIndex;
		int j = nRightIndex + 1;

		//把比轴小的都转到轴的左边
		//把比轴大的都转到轴的右边
		do
		{
			do i++;while (pArray[i] < pivot);
			do j--;while (pArray[j] > pivot);
			if (i < j) ::BLUESwap(pArray[i], pArray[j]);
		} while (i < j);

		//必须和j交换，因为j当前指向的一定是比轴小的，而i指向的比轴大
		//而轴所在位置交换后应该比轴小
		::BLUESwap(pArray[nLeftIndex], pArray[j]);

		//将轴左右两边的继续递归快速排序
		::BLUEQuickSort(pArray, nLeftIndex, j - 1);
		::BLUEQuickSort(pArray, j + 1, nRightIndex);
	}
}

/*
函数名：快速排序
功能：略
说明：实现内部直接调用BLUEQuickSort(T* pArray, int nLeftIndex, int nRightIndex)
参数：pArray 数组
　　　n 数组元素个数
返回值：无
*/
template<class T>
inline void BLUEQuickSort(T* pArray, int n)
{
	::BLUEQuickSort(pArray, 0, n - 1);
}