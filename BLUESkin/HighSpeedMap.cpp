#include "StdAfx.h"
#include "HighSpeedMap.h"

CHighSpeedMap::CHighSpeedMap(DWORD dwInitHsmapItemNum)
{
	Init(dwInitHsmapItemNum);
}

CHighSpeedMap::CHighSpeedMap(void)
{
	Init(DEFAULT_HSMAP_ITEM_ARRAY_NUM);
}

CHighSpeedMap::~CHighSpeedMap(void)
{
	delete[] m_hsmapItemArray;
}

void CHighSpeedMap::Init(DWORD dwInitHsmapItemNum)
{
	if (dwInitHsmapItemNum < 1)
		dwInitHsmapItemNum = 1;

	m_dwHsmapItemNum = dwInitHsmapItemNum;
	m_hsmapItemArray = new HSMAPITEM[m_dwHsmapItemNum];
	FullHsmapItemArrayToZero();
}

void CHighSpeedMap::FullHsmapItemArrayToZero(void)
{
	DWORD i;
	for (i = 0; i < m_dwHsmapItemNum; i++)
	{
		m_hsmapItemArray[i].key = NULL;
		m_hsmapItemArray[i].value = NULL;
	}
}

bool CHighSpeedMap::Add(void* key, void* value)
{
	if (Search(key) == NULL)//先要查找原来有没有相同的图项，如果有则直接返回false
	{
		HSMAPITEM* oldHsMapItemArray;
		DWORD dwOldHsmapItemNum;
		DWORD i;

		//先查找是否有空位，如果有则填充
		for (i = 0; i < m_dwHsmapItemNum; i++)
		{
			if (m_hsmapItemArray[i].key == NULL)
			{
				m_hsmapItemArray[i].key = key;
				m_hsmapItemArray[i].value = value;
				return true;
			}
		}

		//未找到空位则要重新申请数组
		dwOldHsmapItemNum = m_dwHsmapItemNum;//先把原图项个数赋值到临时变量
		oldHsMapItemArray = m_hsmapItemArray;//先把原图数据赋值到临时变量

		m_dwHsmapItemNum = dwOldHsmapItemNum + DEFAULT_HSMAP_ITEM_ARRAY_NUM;//赋值新图数组数量
		m_hsmapItemArray = new HSMAPITEM[m_dwHsmapItemNum];//申请新图数组内存

		FullHsmapItemArrayToZero();//新图数组清零
		CopyMemory(m_hsmapItemArray, oldHsMapItemArray, dwOldHsmapItemNum * sizeof(HSMAPITEM));//将旧的复制到新的
		delete[] oldHsMapItemArray;//删除旧的图数组

		m_hsmapItemArray[dwOldHsmapItemNum].key = key;
		m_hsmapItemArray[dwOldHsmapItemNum].value = value;

		return true;
	}

	return false;
}

void* CHighSpeedMap::Remove(void *key)
{
	DWORD i;
	void* ret;

	ret = NULL;
	for (i = 0; i < m_dwHsmapItemNum; i++)
	{
		if (m_hsmapItemArray[i].key == key)
		{
			ret = m_hsmapItemArray[i].value;
			m_hsmapItemArray[i].key = NULL;
			m_hsmapItemArray[i].value = NULL;
			return ret;
		}
	}
	return ret;
}

bool CHighSpeedMap::Edit(void* key, void* value)
{
	DWORD i;
	for (i = 0; i < m_dwHsmapItemNum; i++)
	{
		if (m_hsmapItemArray[i].key == key)
		{
			m_hsmapItemArray[i].value = value;
			return true;
		}
	}
	return false;
}

void* CHighSpeedMap::Search(void *key)
{
	DWORD i;
	for (i = 0; i < m_dwHsmapItemNum; i++)
	{
		if (m_hsmapItemArray[i].key == key)
			return m_hsmapItemArray[i].value;
	}
	return NULL;
}