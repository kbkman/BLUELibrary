#pragma once
#include <BLUENetWorkModelHelper.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void CBLUENetWorkIODataPackage::GetBufferInfo(char*& lpBuf, unsigned long& nBufLen)
{
	//添加一个引用
	AddRef();

	BLUEASSERT(m_pBuf);

	//保存模型下
	if (m_mode == store)
	{
		//返回发送内存指针和发送长度
		lpBuf = m_pBuf;
		nBufLen = GetLength();
	}
	else//读取模型下
	{
		BLUEASSERT(m_mode == load);
		BLUEASSERT(
			m_loadStep == empty ||
			m_loadStep == base ||
			m_loadStep == complete
			);

		//如果还没有读取过，则只读取基本部分数据
		if (m_loadStep == empty)
		{
			//返回内存指针和加载长度
			lpBuf = m_pBuf;
			nBufLen = sizeof(BLUEBNWIODP);

			//设置读取步骤标记
			m_loadStep = base;
		}
		else if (m_loadStep == base)//如果已经读取了基本部分，则读取扩展部分
		{
			//先根据大小判断是否要扩展buf
			if (GetLength() > m_nBufLen)
				ReMallocBuf(GetLength());

			//返回扩展部分内存指针和加载长度
			lpBuf = m_pBuf + sizeof(BLUEBNWIODP);
			nBufLen = GetExtendedLength();

			//设置读取步骤标记
			m_loadStep = complete;
		}
		else
		{
			lpBuf = m_pBuf;
			nBufLen = GetLength();
		}
	}
}

void CBLUENetWorkIODataPackage::Reset(const MODE mode, int nPreBufSize)
{
	//先处理内存分配
	//是否之前已经分配过还是第一次分配
	if (m_pBuf)
	{
		if (nPreBufSize > m_nBufLen)
			ReMallocBuf(nPreBufSize);
	}
	else
	{
		//是否第一次，则直接为缓存分配nPreBufSize的内存
		BLUEASSERT(m_nBufLen == 0);

		//不能小于基本长度
		if (nPreBufSize < sizeof(BLUEBNWIODP))
			nPreBufSize = sizeof(BLUEBNWIODP);

		m_pBuf = new char[nPreBufSize];
		m_nBufLen = nPreBufSize;
	}

	//处理模式
	BLUEASSERT(mode == store || mode == load);
	m_mode = mode;
	m_loadStep = empty;
	m_nExtendedSeek = sizeof(BLUEBNWIODP);
	((BLUEBNWIODP*)m_pBuf)->nExtendedLength = 0;//扩展数据区初始应该为0长度

	//处理引用
	m_nRef	 = 1;	
}

void CBLUENetWorkIODataPackage::ReMallocBuf(const int nBufSize)
{
	BLUEASSERT(nBufSize > m_nBufLen);
	BLUEASSERT(m_pBuf);
	BLUEASSERT(m_nBufLen > 0);

	//申请一个新的内存空间，长度为n(GetLength())
	char* pNewBuf = new char[nBufSize];

	//将m_pBuf中的数据全部copy到pNewBuf中
	::memcpy(pNewBuf, m_pBuf, m_nBufLen);

	//删除旧的内存块
	delete[] m_pBuf;

	//得到新内存块
	m_pBuf = pNewBuf;

	//内存大小改为新的大小
	m_nBufLen = nBufSize;
}

void CBLUENetWorkIODataPackage::Write(const void* p, const BLUEWORD& nSize)
{
	BLUEASSERT(p);
	//BLUEASSERT(nSize > 0);
	BLUEASSERT(m_mode == store);//store模式才能Write数据

	int nNewBufLen = nSize + GetLength();
	if (nNewBufLen > m_nBufLen)//内存不够写入数据，要扩展内存
		ReMallocBuf(nNewBufLen);

	//写入
	::memcpy(&m_pBuf[GetLength()], p, nSize);

	//扩展数据量增加
	((BLUEBNWIODP*)m_pBuf)->nExtendedLength += nSize;
}

void CBLUENetWorkIODataPackage::Read(void* p, const BLUEWORD& nSize)
{
	BLUEASSERT(p);
	//BLUEASSERT(nSize > 0);
	BLUEASSERT(m_mode == load);//load模式才可以读取

	//读取如果已经超出了内存范围则为非法读取，直接返回（不给出任何出错信息）
	if (m_nExtendedSeek + nSize > GetLength())
		return;

	//读出
	::memcpy(p, &m_pBuf[m_nExtendedSeek], nSize);

	//改变读到的位置
	m_nExtendedSeek += nSize;
}

/************************************************************************/
/*无论字符读出与写入是否用宽字符还是多字节，
*在内部均使用宽字符处理
*注意：宽字符与多字节之间的互相转换使用了windows API，效率会降低，建议使用者使用Unicode版本
*/
/************************************************************************/
CBLUENetWorkIODataPackage& CBLUENetWorkIODataPackage::operator<<(const BLUEStringW& str)
{
	short nStrLen = (short)str.length();
	Write(&nStrLen, sizeof(nStrLen));
	Write(str.c_str(), nStrLen * sizeof(wchar_t));
	return *this;
}

CBLUENetWorkIODataPackage& CBLUENetWorkIODataPackage::operator<<(const BLUEStringA& str)
{
	//多字节版写入要先转成宽字节
	//先得到转成宽字节后需要多少个字节长度
	DWORD dwWStrLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
	if (dwWStrLen > 0)
	{
		wchar_t* pWStr;
		pWStr = new wchar_t[dwWStrLen];
		//转换
		::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, pWStr, dwWStrLen);
		BLUEStringW strW(pWStr);
		*this << strW;
		delete[] pWStr;
	}
	else
	{
		//字符串长度为0也要写入
		*this << BLUEString();
	}

	return *this;
}

CBLUENetWorkIODataPackage& CBLUENetWorkIODataPackage::operator>>(BLUEStringW& str)
{
	short nStrLen = 0;
	Read(&nStrLen, sizeof(nStrLen));
	if (nStrLen > 0)
	{
		str.resize(nStrLen);
		for (short i = 0; i < nStrLen; i++)
			Read(&str[i], sizeof(wchar_t));
	}
	return *this;
}

CBLUENetWorkIODataPackage& CBLUENetWorkIODataPackage::operator>>(BLUEStringA& str)
{
	//多字节版读出要先从宽字节读入，再将宽字节转成多字节
	BLUEStringW strW;
	*this >> strW;

	//先得到转成多字节后需要多少个字节长度
	DWORD dwWStrLen = ::WideCharToMultiByte(CP_ACP, 0, strW.c_str(), -1, NULL, 0, NULL, FALSE);
	if (dwWStrLen > 0)
	{
		char* pAStr = new char[dwWStrLen];
		//转换
		::WideCharToMultiByte (CP_ACP, NULL, strW.c_str(), -1, pAStr, dwWStrLen, NULL, FALSE);
		str = pAStr;
		delete[] pAStr;
	}
	else
	{
		//无长度也要返回
		str.clear();
	}
	return *this;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//////////////////服务器网络模型帮助者/////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
/*
函数名：处理网络服务器模型消息
功能：用于处理网络服务器模型给IBLUEMTSafeMessageQueue发送的消息，
　　　如果用户没有特殊要求可以用该函数来帮助分解消息，如果用户有特殊要求则必须自己来
   　　解析消息
说明：无
参数：firstParam 消息的第一个参数
　　　sencondParam 消息的第一个参数
　　　thirdParam 消息的第一个参数
　　　pCallBack 解析后处理将回调IBLUESNWMMsgProcessorCallBack接口，用户只要实现该接口即可
返回值：无
*/
void BLUEProcessSNWMMsg(
						const BLUEDWORD firstParam,
						void* secondParam,
						void* thirdParam,
						IBLUESNWMMsgProcessorCallBack* pCallBack)
{
	BLUEASSERT(pCallBack);

	switch(firstParam)
	{
	case SNWM_MSG_FP_RECVDATACOMPLETE://会话收到数据（接受与发送消息是最多的，所以放在switch最前两位）
		{
			BLUEASSERT(secondParam);
			BLUEASSERT(thirdParam);
			IBLUEServerNetWorkSession* pSNWSession = (IBLUEServerNetWorkSession*)secondParam;
			CBLUENetWorkIODataPackage* pNWIODataPackage = (CBLUENetWorkIODataPackage*)thirdParam;
			BLUEASSERT(pNWIODataPackage->GetMode() == CBLUENetWorkIODataPackage::load);

			//如果收到的数据是完整的，则提示使用者收到数据
			if (pNWIODataPackage->IsLoadComplete())
			{
				pCallBack->OnSNWSessionRecvData(
					pSNWSession,
					pNWIODataPackage
					);
			}
			else//如果没有完整收到，则继续接受
			{
				//如果包的扩展长度太长，则忽略，×××××此处可能需要再次修改，暂时作此处理
				if (pNWIODataPackage->GetExtendedLength() <= 1024)
					pSNWSession->RecvData(pNWIODataPackage);
			}

			//最后对IO数据减少一次引用，不管有没有收全数据
			//因为没有接受完数据的情况下会再接受，所以会内部再加一次引用
			pNWIODataPackage->Release();
		}
		break;

	case SNWM_MSG_FP_SENDDATACOMPLETE:
		BLUEASSERT(thirdParam);
		((CBLUENetWorkIODataPackage*)thirdParam)->Release();//IO数据减少一次引用
		break;

	case SNWM_MSG_FP_SESSIONCREATE://新会话
		BLUEASSERT(secondParam);
		BLUEASSERT(thirdParam == BLUENULL);
		pCallBack->OnSNWSessionCreate((IBLUEServerNetWorkSession*)secondParam);
		break;

	case SNWM_MSG_FP_SESSIONCLOSE://会话关闭
		BLUEASSERT(secondParam);
		BLUEASSERT(thirdParam);
		pCallBack->OnSNWSessionClose((IBLUEServerNetWorkSession*)secondParam);
		((CBLUENetWorkIODataPackage*)thirdParam)->Release();//减少一次引用
		break;


	case SNWM_MSG_FP_SESSIONERROR:
		BLUEASSERT(secondParam);
		BLUEASSERT(thirdParam);
		pCallBack->OnSNWSessionError((IBLUEServerNetWorkSession*)secondParam);
		((CBLUENetWorkIODataPackage*)thirdParam)->Release();//IO数据减少一次引用
		break;

	default:
		BLUEASSERT(0);
		break;
	}
}



///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//////////////////客户端网络模型帮助者/////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
void BLUEProcessCNWMMsg(
						IBLUEClientNetWorkModel* pCNWM,
						const BLUEDWORD firstParam,
						void* secondParam,
						void* thirdParam,
						IBLUECNWMMsgProcessorCallBack* pCallBack)
{
	BLUEASSERT(pCNWM);
	BLUEASSERT(pCallBack);

	switch(firstParam)
	{
	case CNWM_MSG_FP_RECVDATACOMPLETE://会话收到数据（接受与发送消息是最多的，所以放在switch最前两位）
		{
			BLUEASSERT(secondParam == BLUENULL);
			BLUEASSERT(thirdParam);
			CBLUENetWorkIODataPackage* pNWIODataPackage = (CBLUENetWorkIODataPackage*)thirdParam;
			BLUEASSERT(pNWIODataPackage->GetMode() == CBLUENetWorkIODataPackage::load);

			//如果收到的数据是完整的，则提示使用者收到数据
			if (pNWIODataPackage->IsLoadComplete())
			{
				pCallBack->OnCNWRecvData(*pNWIODataPackage);
			}
			else//如果没有完整收到，则继续接受
			{
				pCNWM->RecvData(pNWIODataPackage);
			}
			//最后对IO数据减少一次引用，不管有没有收全数据
			pNWIODataPackage->Release();
		}
		break;

	case CNWM_MSG_FP_SENDDATACOMPLETE:
		BLUEASSERT(secondParam == BLUENULL);
		BLUEASSERT(thirdParam);
		((CBLUENetWorkIODataPackage*)thirdParam)->Release();//IO数据减少一次引用
		break;

	case CNWM_MSG_FP_ERROR://连接出错
		BLUEASSERT(secondParam);
		BLUEASSERT(thirdParam);
		pCallBack->OnCNWError((int)(__int64)secondParam);
		((CBLUENetWorkIODataPackage*)thirdParam)->Release();//IO数据减少一次引用
		break;

	default:
		BLUEASSERT(0);
		break;
	}
}