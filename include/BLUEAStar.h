#ifndef __BLUEASTAR_H__
#define __BLUEASTAR_H__

#include "BLUEDef.h"
#include "BLUEDebug.h"
#include "BLUEList.h"


/**************************************
***************************************
*****************声明部分***************
***************************************
***************************************/
/**
函数指针：A星寻路输出回调函数
说明：搜索完路径后，使用该类型的回调函数来打印
*/
typedef void (*BLUEAStarPathOutputCallBack)(int nRowIndex, int nColIndex);

/**
类名：A星寻路算法封装类
功能：使用A星算法在地图上找到一点到另外一点的最佳路径
*/
class CBLUEAStar
{
private:
	/**
	枚举名：A星结点状态
	枚举列表：none 无状态（可通过）
	　　　　　close 关闭状态
	　　　　　open 开启状态
	　　　　　nopass 不可通过（有障碍物）
	*/
	enum ASTARState {none, close, open, nopass};

	/**
	结构名：A星结点结构
	*/
	struct BLUEASTARNODE
	{
		ASTARState state;//当前状态
		BLUEASTARNODE* pFather;//父结构
		int nRowIndex;//该结构在地图上的行号
		int nColIndex;//该结构在地图上的列号
		int nGCost;//起点到该点的消耗（这是一个实际值）
		int nHCost;//该点到终点的消耗（这是一个估算值）
		int nCost;//总消耗（以上两者相加）
	};
public:
	/**
	函数名：构造一个A星对象
	参数：pMapData 地图矩阵数组
	　　　nRows 地图的行数
	   　　nCols 地图的列数
	*/
	CBLUEAStar(const int* pMapData, const int nRows, const int nCols);
	~CBLUEAStar(void);

public:
	/**
	函数名：路径搜索
	参数：srcRowIndex 起点行号
	　　　srcColIndex 起点列号
	　　　destRowIndex 目标行号
	　　　destColIndex 目标列号
	返回：是否成功搜索到路径
	*/
	bool PathSearch(const int srcRowIndex, const int srcColIndex, const int destRowIndex, const int destColIndex); 

	/**
	函数名：输出搜索到的路径（比反向输出要快，但得到的是终点到起点）
	参数：回调函数
	*/
	void PathOutput(BLUEAStarPathOutputCallBack pfnPathOutput) const;

	/**
	函数名：输出搜索到的路径，反向输出（即得到起点到终点）
	参数：回调函数
	*/
	void PathOutputRev(BLUEAStarPathOutputCallBack pfnPathOutput) const;
	
private:
	/**
	函数名：根据行号和列号得到在数组中的序号
	*/
	int GetIndexFromRC(const int nRowIndex, const int nColIndex) const {return nRowIndex * m_nCols + nColIndex;}

	/**
	函数名：清空开启表中所有数据
	*/
	void ClearOpenTable(void) {m_openTable.RemoveAll();}

	/**
	函数名：重置所有A星结构
	*/
	void ResetAllAStarNode(void);

	/**
	函数名：打开一个起点的A星结点
	说明：该函数只能第一次搜索前调用，为了不让OpenAStarNode每次都多判断一次
	　　　是否为起始结点，所以特别单独写出来，提高效率
	*/
	void OpenStartAStarNode(void);

	/**
	函数名：打开一个A星结点
	参数：nRowIndex 结点行号
	　　　nColIndex 结点列号
	　　　pFatherNode 父结点（为0表示无父结点）
	*/
	void OpenAStarNode(const int nRowIndex, const int nColIndex, const BLUEASTARNODE* pFatherNode);

	/**
	函数名：关闭一个A星结点
	参数：pNode A星结点
	*/
	void CloseAStarNode(BLUEASTARNODE* pNode);

private:
	const int m_nRows;//地图行数
	const int m_nCols;//地图列数
	const int m_nTotalNode;//总结点数（m_nRows * m_nCols）
	BLUEASTARNODE* m_pMapAStarNode;//A星结点地图指针

	int m_nSrcRowIndex;//搜索起点行号
	int m_nSrcColIndex;//搜索起点列号
	int m_nDestRowIndex;//搜索终点行号
	int m_nDestColIndex;//搜索终点列号

	/**
	变量名：最后一个最小消耗的节点
	说明：该变量用于跟踪最后一个行走到的结点，如果路径搜索成功，则该结点为目标结点，
	　　　否则为最后一个行走到的结点，有利用画出断开的结点

	*/
	BLUEASTARNODE* m_pLastMinCostNode;

	CBLUEList<BLUEASTARNODE*> m_openTable;//开启表
};

/**************************************
***************************************
*****************实现部分***************
***************************************
***************************************/
CBLUEAStar::CBLUEAStar(const int* pMapData, const int nRows, const int nCols) :
m_nRows(nRows),
m_nCols(nCols),
m_nTotalNode(m_nRows * m_nCols),
m_pLastMinCostNode(BLUENULL)
{
	BLUEASSERT(pMapData);
	BLUEASSERT(m_nRows > 0);
	BLUEASSERT(m_nCols > 0);

	//A星结点地图申请内存
	m_pMapAStarNode = new BLUEASTARNODE[m_nTotalNode];

	//设置A星结点地图
	for (int i = 0; i < m_nTotalNode; i++)
	{
		(pMapData[i] == 0) ?
			m_pMapAStarNode[i].state = none :
			m_pMapAStarNode[i].state = nopass;
	}
}

CBLUEAStar::~CBLUEAStar(void)
{
	delete[] m_pMapAStarNode;
}

void CBLUEAStar::ResetAllAStarNode(void)
{
	BLUEASSERT(m_pMapAStarNode);
	BLUEASSERT(m_nTotalNode > 0);

	for (int i = 0; i < m_nTotalNode; i++)
		if (m_pMapAStarNode[i].state != nopass)//非障碍全部归为none
			m_pMapAStarNode[i].state = none;
}

void CBLUEAStar::OpenStartAStarNode(void)
{
	BLUEASSERT(m_pMapAStarNode);
	BLUEASSERT(m_nRows > 0);
	BLUEASSERT(m_nCols > 0);
	BLUEASSERT(m_nSrcRowIndex >= 0 && m_nSrcRowIndex < m_nRows);
	BLUEASSERT(m_nSrcColIndex >= 0 && m_nSrcColIndex < m_nCols);
	BLUEASSERT(m_nDestRowIndex >= 0 && m_nDestRowIndex < m_nRows);
	BLUEASSERT(m_nDestColIndex >= 0 && m_nDestColIndex < m_nCols);
	BLUEASSERT(m_openTable.IsEmpty());

	BLUEASTARNODE* pSrcNode = &m_pMapAStarNode[GetIndexFromRC(m_nSrcRowIndex, m_nSrcColIndex)];
	pSrcNode->nRowIndex = m_nSrcRowIndex;
	pSrcNode->nColIndex = m_nSrcColIndex;
	pSrcNode->nGCost = 0;
	pSrcNode->nHCost = ::abs(m_nDestRowIndex - m_nSrcRowIndex) + ::abs(m_nDestColIndex - m_nSrcColIndex);
	pSrcNode->nCost = pSrcNode->nGCost + pSrcNode->nHCost;//以上两者相加
	pSrcNode->pFather = BLUENULL;
	pSrcNode->state = open;
	m_openTable.AddTail(pSrcNode);
	//设置最后一个min cost node，参阅声明部分
	m_pLastMinCostNode = pSrcNode;
}


void CBLUEAStar::OpenAStarNode(
	const int nRowIndex,
	const int nColIndex,
	const BLUEASTARNODE* pFatherNode
	)
{
	BLUEASSERT(pFatherNode);
	BLUEASSERT(m_pMapAStarNode);
	BLUEASSERT(m_nRows > 0);
	BLUEASSERT(m_nCols > 0);

	//在地图以外的点要忽略（可能会产生这样的数据的）
	if (nRowIndex < 0 || nRowIndex >= m_nRows)
		return;
	if (nColIndex < 0 || nColIndex >= m_nCols)
		return;

	//根据行列号得到结点
	BLUEASTARNODE* pNode = &m_pMapAStarNode[GetIndexFromRC(nRowIndex, nColIndex)];

	//如果状态为none的结点才可以加入到开启表中，
	//open为已经在开启表中，所以不能加入
	//close为已经关闭的结点，也不能加入
	//nopass为原地图中的障碍物，也不能加入
	if (pNode->state == none)
	{
		//初始该结点数据
		pNode->nRowIndex = nRowIndex;
		pNode->nColIndex = nColIndex;
		pNode->pFather = const_cast<BLUEASTARNODE*>(pFatherNode);
		pNode->nGCost = pFatherNode->nGCost + 1;//（因为没有斜向，所以只需要加1，如果有斜向，请另外计算）
		pNode->nHCost = ::abs(m_nDestRowIndex - nRowIndex) + ::abs(m_nDestColIndex - nColIndex);//从该点到终点的距离（估算）
		pNode->nCost = pNode->nGCost + pNode->nHCost;//以上两者相加

		//将该结点添加入开启表中
		BLUEPOSITION pos = m_openTable.GetHeadPosition();
		bool bIsInsert = false;//添加到了开启中
		while (pos)
		{
			BLUEPOSITION curPos = pos;
			BLUEASTARNODE* pTempNode = m_openTable.GetNext(pos);
			//开启表中放的结点，是按其const值由小到大排序
			//所以，只需要从头查找，只要找到比自己cost值大的结点，即可插入在其前面
			if (pTempNode->nCost >= pNode->nCost)//如果相等，则应该把新打开的结点放在之前，这样会快一些
			{
				m_openTable.InsertBefore(curPos, pNode);
				bIsInsert = true;
				break;
			}
		}

		//如果从头到尾都没有插入，说明所有结点的cost值均小于本结点
		//所以要插在未尾
		if (!bIsInsert)
			m_openTable.AddTail(pNode);

		//结点状态设置为开启状态
		pNode->state = open;
	}
}

void CBLUEAStar::CloseAStarNode(BLUEASTARNODE* pNode)
{
	BLUEASSERT(pNode);

	//关闭该结构
	pNode->state = close;

	//将该结点周围的四个点全部添加到开启表中
	//添加上结点
	OpenAStarNode(pNode->nRowIndex, pNode->nColIndex - 1, pNode);
	//添加下结点
	OpenAStarNode(pNode->nRowIndex, pNode->nColIndex + 1, pNode);
	//添加左结点
	OpenAStarNode(pNode->nRowIndex - 1, pNode->nColIndex, pNode);
	//添加右结点
	OpenAStarNode(pNode->nRowIndex + 1, pNode->nColIndex, pNode);
}

bool CBLUEAStar::PathSearch(
							const int nSrcRowIndex,
							const int nSrcColIndex,
							const int nDestRowIndex,
							const int nDestColIndex)
{
	BLUEASSERT(m_pMapAStarNode);
	BLUEASSERT(m_nRows > 0);
	BLUEASSERT(m_nCols > 0);
	BLUEASSERT(nSrcRowIndex >= 0 && nSrcRowIndex < m_nRows);
	BLUEASSERT(nSrcColIndex >= 0 && nSrcColIndex < m_nCols);
	BLUEASSERT(nDestRowIndex >= 0 && nDestRowIndex < m_nRows);
	BLUEASSERT(nDestColIndex >= 0 && nDestColIndex < m_nCols);

	/**
	大致流程如下
		||
		||
		||
		V
	*/
	//初始成员
	//清空开启表中的数据
	//所有结点内容恢复
	//计算起点的cost值（消耗值）
	//将起点加入开启表中
	//开始循环查找
		//取出开启表中cost消耗值最小的结点
		//如果开启表已经空了，则结束循环，说明无法找到移动到目的的路径
		//如果最小的结点就是终点的话，则表示已经搜索成功，结束循环
		//关闭该结点
		//将该结点周围的所有可开启的结点开启，并计算这些结点的cost消耗值
	//结束循环


	/************************
	**********实现***********
	*************************/
	//初始成员
	m_nSrcRowIndex = nSrcRowIndex;
	m_nSrcColIndex = nSrcColIndex;
	m_nDestRowIndex = nDestRowIndex;
	m_nDestColIndex = nDestColIndex;
	
	//清空开启表中的数据
	ClearOpenTable();

	//所有结点内容恢复
	ResetAllAStarNode();

	//将起点打开（将其加入到开启表中）
	OpenStartAStarNode();

	//开始循环查找
	while (true)
	{
		//取出开启表中cost值最小的结点
		BLUEASTARNODE* pMinCostNode = m_openTable.RemoveHead();

		//如果开启表已经空了，则结束循环，表示找不到有效路径
		if (pMinCostNode == BLUENULL)
			return false;

		//设置最后一个min cost node，参阅声明部分
		if (m_pLastMinCostNode->nHCost > pMinCostNode->nHCost)
			m_pLastMinCostNode = pMinCostNode;

		//如果已经到达了终点（与终点坐标一样时），则返回
		if (pMinCostNode->nRowIndex == m_nDestRowIndex &&
			pMinCostNode->nColIndex == m_nDestColIndex)
			break;

		//关闭该结点（同时打开其周围可以打开的结点）
		CloseAStarNode(pMinCostNode);
	}

	//搜索到完整路径
	return true;
}

void CBLUEAStar::PathOutput(BLUEAStarPathOutputCallBack pfnPathOutput) const
{
	BLUEASTARNODE* pNode = m_pLastMinCostNode;
	while (pNode)
	{
		pfnPathOutput(pNode->nRowIndex, pNode->nColIndex);
		pNode = pNode->pFather;
	}
}

void CBLUEAStar::PathOutputRev(BLUEAStarPathOutputCallBack pfnPathOutput) const
{
	CBLUEList<BLUEASTARNODE*> atnList;
	BLUEASTARNODE* pNode = m_pLastMinCostNode;
	while (pNode)
	{
		atnList.AddHead(pNode);
		pNode = pNode->pFather;
	}

	BLUEPOSITION pos = atnList.GetHeadPosition();
	while (pos)
	{
		pNode = atnList.GetNext(pos);
		pfnPathOutput(pNode->nRowIndex, pNode->nColIndex);
	}
}

#endif