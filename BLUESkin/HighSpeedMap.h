#pragma once
/*
高速图查找算法

HSMAPITEM结构为查找的基本数据结构
CHighSpeedMap类负责查找

*/

struct HSMAPITEM
{
	void* key;
	void* value;
};

#define DEFAULT_HSMAP_ITEM_ARRAY_NUM		20//默认初始图的项数
class CHighSpeedMap
{
	/////////////公开///////////////////
public://构造
	CHighSpeedMap(DWORD dwInitHsmapItemNum);
	CHighSpeedMap(void);
	virtual ~CHighSpeedMap(void);

public://主要操作
	bool		Add(void* key, void* value);//添加一个图，返回false表示已经存在该图项
	bool		Edit(void* key, void* value);//编辑一个图,返回false表示该图项不存在
	void*	Remove(void* key);//删除一个图,返回NULL表示该图项不存在
	void*	Search(void* key);//搜索一个图,返回NULL表示该图项不存在

	/////////////私有///////////////////
private:
	void Init(DWORD dwInitHsmapItemNum);//初始化所有高速图
	void FullHsmapItemArrayToZero(void);//将所有图至空

private:
	HSMAPITEM* m_hsmapItemArray;
	DWORD m_dwHsmapItemNum;
};
