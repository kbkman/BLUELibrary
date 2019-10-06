#ifndef __BLUENETWORKMODELHELPER_H__
#define __BLUENETWORKMODELHELPER_H__

#include <BLUEDef.h>
#include <BLUEDebug.h>
#include <BLUEAdv.h>
#include <BLUEServerNetWorkModel.h>
#include <BLUEClientNetWorkModel.h>

/**
结构名：基本网络IO数据包（BLUE Base NetWork IO Data Package）
功能：略
说明：无
版本：1.0.0.1
作者：BLUE
*/
struct  BLUEBNWIODP
{
	short nVer;    //版本标识
	int nOrder;    //命令
	int nFParam; //参数一
	int nSParam; //参数二
	BLUEDWORD nExtendedLength; //负载长度
};

/**
类名：网络IO数据包
功能：略
说明：不支持多线程，需要支持的话请自行修改
版本：1.0.0.1
作者：BLUE
*/
class CBLUENetWorkIODataPackage : public IBLUESessionIOData
{
public:
	enum MODE {store, load};
	enum LOADSTEP {empty, base, complete};

private:
	/*
	函数名：构造一个网络IO数据包
	功能：略
	说明：无
	参数：nMode 模型（可以是store储存模型或load读取模型）
	　　　nPreBufSize 预分配的内存容量，事先分配足够用的容量可防止多次的重新分配
	返回值：无
	*/
	CBLUENetWorkIODataPackage(const MODE mode, const int nPreBufSize) :
	   m_pBuf(BLUENULL), m_nBufLen(0), m_loadStep(empty)
	{
		Reset(mode, nPreBufSize);
	}

	/*
	函数名：析构一个网络IO数据包
	功能：略
	说明：无
	参数：无
	返回值：无
	*/
	~CBLUENetWorkIODataPackage(void)
	{
		BLUEASSERT(m_nRef == 0);
		BLUEASSERT(m_pBuf);
		delete[] m_pBuf;
	}

private:

	/*
	参阅：IBLUESessionIOData::GetBufferInfo说明
	*/
	virtual void GetBufferInfo(char*& lpBuf, unsigned long& nBufLen);

	/*
	函数名：重设一个网络IO数据包
	功能：略
	说明：无
	参数：nMode 模型（可以是store储存模型或load读取模型）
	　　　nPreBufSize 预分配的内存容量，事先分配足够用的容量可防止多次的重新分配
	返回值：无
	*/
	void Reset(const MODE mode, int nPreBufSize);

	/*
	函数名：重新分配使用的内存
	功能：略
	说明：只有重新分配的内存大小大小现在的内存大小才会分配，否则会引发assert
	参数：无
	返回值：无
	*/
	void ReMallocBuf(const int nBufSize);

public:

	MODE GetMode(void) const {return m_mode;}
	bool IsLoadComplete(void)//在load模式下是否已经全部加载
	{
		if (m_loadStep == base && GetExtendedLength() == 0)
		{
			m_loadStep = complete;
			return true;
		}
		return m_loadStep == complete;
	}

	short GetVer(void) const {return ((BLUEBNWIODP*)m_pBuf)->nVer;}
	int GetOrder(void) const {return ((BLUEBNWIODP*)m_pBuf)->nOrder;}
	int GetFParam(void) const {return ((BLUEBNWIODP*)m_pBuf)->nFParam;}
	int GetSParam(void) const {return ((BLUEBNWIODP*)m_pBuf)->nSParam;}
	int GetLength(void) const {return GetExtendedLength() + sizeof(BLUEBNWIODP);}
	BLUEDWORD GetExtendedLength() const {return ((BLUEBNWIODP*)m_pBuf)->nExtendedLength;}

	void SetVer(const short ver) {((BLUEBNWIODP*)m_pBuf)->nVer = ver;}
	void SetFParam(const int fp) {((BLUEBNWIODP*)m_pBuf)->nFParam = fp;}
	void SetSParam(const int sp) {((BLUEBNWIODP*)m_pBuf)->nSParam = sp;}
	void SetOrder(const int order)  {((BLUEBNWIODP*)m_pBuf)->nOrder = order;}
	void SetBaseData(const int order, const int fp, const int sp = 0)
	{
		SetOrder(order);
		SetFParam(fp);
		SetSParam(sp);
	}

	/*
	函数名：写入扩展数据
	功能：略
	说明：无
	参数：p 写入的数据地址
	　　　nSize 要写入数据的长度
	返回值：无
	*/
	void Write(const void* p, const BLUEWORD& nSize);
	CBLUENetWorkIODataPackage& operator<<(const bool b) {Write(&b, sizeof(b));return *this;}
	CBLUENetWorkIODataPackage& operator<<(const char cb) {Write(&cb, sizeof(cb));return *this;}
	CBLUENetWorkIODataPackage& operator<<(const short s) {Write(&s, sizeof(s));return *this;}
	CBLUENetWorkIODataPackage& operator<<(const int i)	{Write(&i, sizeof(i));return *this;}
	CBLUENetWorkIODataPackage& operator<<(const long l) {Write(&l, sizeof(l));return *this;}
	CBLUENetWorkIODataPackage& operator<<(const BLUEBYTE by) {Write(&by, sizeof(by));return *this;}
	CBLUENetWorkIODataPackage& operator<<(const BLUEWORD w) {Write(&w, sizeof(w));return *this;}
	CBLUENetWorkIODataPackage& operator<<(const BLUEDWORD dw)	{Write(&dw, sizeof(dw));return *this;}
	CBLUENetWorkIODataPackage& operator<<(const __int64 i64) {Write(&i64, sizeof(i64));return *this;}//暂时不支持64位长
	CBLUENetWorkIODataPackage& operator<<(const BLUEStringW& str);
	CBLUENetWorkIODataPackage& operator<<(const BLUEStringA& str);

	/*
	函数名：读取扩展数据
	功能：略
	说明：无
	参数：p 读出的数据存入的地址
	　　　nSize 要读出数据的长度
	   返回值：无
	   */
	void Read(void* p, const BLUEWORD& nSize);
	CBLUENetWorkIODataPackage& operator>>(bool& b) {Read(&b, sizeof(b));return *this;}
	CBLUENetWorkIODataPackage& operator>>(char& cb) {Read(&cb, sizeof(cb));return *this;}
	CBLUENetWorkIODataPackage& operator>>(short& s) {Read(&s, sizeof(s));return *this;}
	CBLUENetWorkIODataPackage& operator>>(int& i) {Read(&i, sizeof(i));return *this;}
	CBLUENetWorkIODataPackage& operator>>(long& l)	{Read(&l, sizeof(l));return *this;}
	CBLUENetWorkIODataPackage& operator>>(BLUEBYTE& by)	{Read(&by, sizeof(by));return *this;}
	CBLUENetWorkIODataPackage& operator>>(BLUEWORD& w)	{Read(&w, sizeof(w));return *this;}
	CBLUENetWorkIODataPackage& operator>>(BLUEDWORD& dw) {Read(&dw, sizeof(dw));return *this;}
	CBLUENetWorkIODataPackage& operator>>(__int64& i64) {Read(&i64, sizeof(i64));return *this;}//暂时不支持64位长
	CBLUENetWorkIODataPackage& operator>>(BLUEStringW& str);
	CBLUENetWorkIODataPackage& operator>>(BLUEStringA& str);

	/*
	函数名：添加引用
	功能：略
	说明：无
	参数：无
	返回值：无
	*/
	void AddRef(void)
	{
		m_nRef++;
	}

	/*
	函数名：减少引用
	功能：略
	说明：当引用为0时，会自动删除本对象
	参数：无
	返回值：无
	*/
	void Release(void)
	{
		m_nRef--;
		if (m_nRef == 0)
			delete this;
	}

private:
	MODE m_mode;                //模式store或load
	char* m_pBuf;                 //缓冲区
	int m_nBufLen;                //缓冲区长度
	int m_nExtendedSeek;    //在load模式时，当前读取扩展数据的位置
	BLUEDWORD m_nRef;     //引用计数

	/*
	变量名：load模式下，当前读入数据的步骤
	功能：略
	说明：empty代表还没有读入任何数据，
	　　　base代表已经读入基本（即读入了BLUEBNWIODP），
	　　　complete代表已经全部读入
	*/
	LOADSTEP m_loadStep;

private:
	friend class CBLUENetWorkIODataPackageManager;
};

/**
类名：网络IO数据包管理器
功能：略
说明：不支持多线程，需要支持的话请自行修改
版本：1.0.0.1
作者：BLUE
*/
class CBLUENetWorkIODataPackageManager
{
public:
	CBLUENetWorkIODataPackageManager(const short nVer) : m_nVer(nVer) {}
	~CBLUENetWorkIODataPackageManager(void) {}

public:
	CBLUENetWorkIODataPackage* CreateStoreNWIOData(int nPreBufSize = 100)
	{
		CBLUENetWorkIODataPackage* pIOD = new CBLUENetWorkIODataPackage(CBLUENetWorkIODataPackage::store, nPreBufSize);
		pIOD->SetVer(m_nVer);
		return pIOD;
	}

	CBLUENetWorkIODataPackage* CreateLoadNWIOData(int nPreBufSize = 100)
	{
		return new CBLUENetWorkIODataPackage(CBLUENetWorkIODataPackage::load, nPreBufSize);
	}

private:
	const short m_nVer;
};

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//////////////////服务器网络模型帮助者/////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
/**
接口名：服务器网络模型处理器回调接口
功能：参阅BLUEProcessSNWMMsg函数说明
说明：无
版本：1.0.0.1
作者：BLUE
*/
class IBLUESNWMMsgProcessorCallBack
{
public:

	/*
	函数名：有新的会话创建
	功能：略
	说明：无
	参数：pSNWSession 会话
	返回值：无
	*/
	virtual void OnSNWSessionCreate(IBLUEServerNetWorkSession* pSNWSession) = 0;

	/*
	函数名：有会话关闭
	功能：略
	说明：无
	参数：pSNWSession 会话
	返回值：无
	*/
	virtual void OnSNWSessionClose(IBLUEServerNetWorkSession* pSNWSession) = 0;

	/*
	函数名：有会话收到数据
	功能：略
	说明：无
	参数：pSNWSession 会话
	　　　pSessionIOData 收到的IO数据
	返回值：无
	*/
	virtual void OnSNWSessionRecvData(IBLUEServerNetWorkSession* pSNWSession, CBLUENetWorkIODataPackage* pSessionIOData) = 0;

	/*
	函数名：有会话发生错误
	功能：略
	说明：无
	参数：pSNWSession 会话
	返回值：无
	*/
	virtual void OnSNWSessionError(IBLUEServerNetWorkSession* pSNWSession) = 0;
};

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
						IBLUESNWMMsgProcessorCallBack* pCallBack);



///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//////////////////客户端网络模型帮助者/////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

/**
接口名：客户端网络模型处理器回调接口
功能：参阅BLUEProcessCNWMMsg函数说明
说明：无
版本：1.0.0.1
作者：BLUE
*/
class IBLUECNWMMsgProcessorCallBack
{
public:
	virtual void OnCNWRecvData(CBLUENetWorkIODataPackage& iod) = 0;
	virtual void OnCNWError(int nErrorCode) = 0;
};

/*
函数名：处理网络客户模型消息
功能：用于处理网络客户模型给IBLUEMTSafeMessageQueue发送的消息，
　　　如果用户没有特殊要求可以用该函数来帮助分解消息，如果用户有特殊要求则必须自己来
   　　解析消息
说明：无
参数：pCNWM 客户端网络模型对象
　　　firstParam 消息的第一个参数
　　　sencondParam 消息的第一个参数
　　　thirdParam 消息的第一个参数
　　　pCallBack 解析后处理将回调IBLUECNWMMsgProcessorCallBack接口，用户只要实现该接口即可
返回值：无
*/
void BLUEProcessCNWMMsg(
	IBLUEClientNetWorkModel* pCNWM,
	const BLUEDWORD firstParam,
	void* secondParam,
	void* thirdParam,
	IBLUECNWMMsgProcessorCallBack* pCallBack);


#endif