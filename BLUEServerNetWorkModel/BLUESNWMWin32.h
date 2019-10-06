#pragma once
#include "BLUEDef.h"
#include "BLUEDebug.h"
#include "BLUEServerNetWorkModel.h"

#include "BLUEAdv.h"
#include "BLUEWin32.h"
#include <WinSock2.h>
#include <vector>
#include <set>

#pragma comment(lib, "ws2_32.lib")

//重叠模型IO类型，用于填充WOLPSOCKETIO结构的nIOType成员
#define WOLPSOCKETIO_TYPE_RECV       0       //此IO为接受数据IO
#define WOLPSOCKETIO_TYPE_SEND       1       //此IO为发送数据IO
/**
结构名：重叠模型中套接字每次收发IO单元
功能：略
说明：无
版本：1.0.0.1
作者：BLUE
*/
struct WOLPSOCKETIO
{
	WOLPSOCKETIO(void) : bIsUsing(false) {}
	void SetUsing(const bool b) {bIsUsing = b;}//设置正在使用
	bool IsUsing(void) {return bIsUsing;}//是否正在使用

	WSAOVERLAPPED ola;   			                 //基本的WSAOVERLAPPED结构(必须的)
	BLUEDWORD nIOType;	                             //IO操作的类型
	WSABUF wsaBuf;		                                 //缓存
	IBLUESessionIOData* pSessionIOData;  //用户自定义IO数据

private:
	bool bIsUsing;                         //是否正在使用中
};

/**
接口名：服务器网络会话处理器接口
功能：略
说明：无
版本：1.0.0.1
作者：BLUE
*/
class IBLUESNWSessionProcessor 
{
public:
	/*
	函数名：删去一个会话
	功能：让服务器网络模型对象删去一个会话
	说明：删除一个会话后，使用者不应该再去使用该会话对象，否则可能会造成异常，
	　　　程序会检测参数的正确性
	参数：pServerNetWorkSession 会话对象
	返回值：无
	*/
	virtual void EraseSession(IBLUEServerNetWorkSession* pServerNetWorkSession) = 0;
	virtual void SessionCloseProcess(IBLUEServerNetWorkSession* pServerNetWorkSession, IBLUESessionIOData* pUserCustomIOData) = 0;
	virtual void SessionErrorProcess(IBLUEServerNetWorkSession* pServerNetWorkSession, IBLUESessionIOData* pUserCustomIOData) = 0;
};

/**
类名：服务器网络会话
功能：略
说明：无
版本：1.0.0.1
作者：BLUE
*/
class CBLUEServerNetWorkSession : public IBLUEServerNetWorkSession
{
	typedef std::vector<WOLPSOCKETIO*> WOLPSOCKETIOCOLL;

public:
	CBLUEServerNetWorkSession(SOCKET socket, IBLUESNWSessionProcessor* pSEP);
	virtual ~CBLUEServerNetWorkSession(void);

public:
	virtual void Delete(void);
	virtual void RecvData(IBLUESessionIOData* pRecvData);
	virtual void SendData(IBLUESessionIOData* pSendData);
	virtual void Close(void);

public:
	/*
	函数名：获取会话的套接字句柄
	功能：略
	说明：无
	参数：无
	返回值：套接字句柄
	*/
	SOCKET GetSocketHandle(void) const {return m_socket;}

private:
	SOCKET m_socket;
	IBLUESNWSessionProcessor* m_pSEP;
	WOLPSOCKETIOCOLL m_recvSocketIOColl;//接受SOCKETIO集合
	WOLPSOCKETIOCOLL m_sendSocketIOColl;//发送SOCKETIO集合
	CBLUEWinCriticalSection m_recvSocketIOCollCS;//接受SOCKETIO临界区
	CBLUEWinCriticalSection m_sendSocketIOCollCS;//发送SOCKETIO临界区
};

/**
类名：服务器网络模型
功能：略
说明：无
版本：1.0.0.1
作者：BLUE
*/
class CBLUEServerNetWorkModel :
	public IBLUEServerNetWorkModel,
	public IBLUESNWSessionProcessor
{
	typedef std::vector<IBLUEThread*> BLUEThreadColl;
	typedef std::set<IBLUEServerNetWorkSession*> BLUESessionColl;

public:
	CBLUEServerNetWorkModel(void);
	~CBLUEServerNetWorkModel(void);

public:
	//IBLUEServerNetWorkModel接口实现
	virtual int Create(
		IBLUEMTSafeMessageQueue* pMSMQ,
		const BLUEDWORD dwMSMQMessageType,
		const BLUELPCSTR lpstrListenIP,
		const BLUEWORD nListenPort);
	virtual void Destroy(void);
	virtual void Run(void);
	virtual void Pause(void);

	//IBLUESNWSessionProcessor接口实现
	virtual void EraseSession(IBLUEServerNetWorkSession* pServerNetWorkSession);
	virtual void SessionCloseProcess(IBLUEServerNetWorkSession* pServerNetWorkSession, IBLUESessionIOData* pSessionIOData);
	virtual void SessionErrorProcess(IBLUEServerNetWorkSession* pServerNetWorkSession, IBLUESessionIOData* pSessionIOData);

private:
	BLUEDWORD ListenThread(void);
	BLUEDWORD WorkThread(void);
	static BLUEDWORD stListenThread(void* p);
	static BLUEDWORD stWorkThread(void* p);

private:
	IBLUEMTSafeMessageQueue* m_pMSMQ;//安全消息队列
	BLUEDWORD m_dwMSMQMessageType;//使用安全消息队列时使用的消息类型值
	HANDLE	m_hCompletionPort;//完成端口
	SOCKADDR_IN m_sddrListen;//监听地址
	SOCKET m_listenSocket;//监听使用的套接字

	IBLUEThread* m_pListenThread;//监听线程
	BLUEThreadColl m_workThreadColl;//工作者线程集合
	BLUESessionColl m_sessionColl;//用户会话集合
	CBLUEWinCriticalSection m_sessionCollCS;//用户会话集合临界区，用于保证m_sessionColl在多线程下安全

	/*
	变量名：用户会话关闭与出错时临界区
	功能：略
	说明：当会话出错时，要先检测该会话是否已经关闭，如果没有关闭，才会给使用者发送一条
	　　　会话关闭或出错的信息，这样可以保证会话只被通知一次关闭或出错。但在多线程情况下
	　　　可能同时好几个线程发现出错或关闭，同时检查一个会话是否关闭，这样会造成两个多线程
	　　　都判断出会话未关闭，所以需要一个临界区来限制同一时只有一个线程可以检查会话是否未
	　　　关闭。
	　　　按理来说应该使每个会话都配备一个临界区，但这样过于浪费内存资源，因为只使用统一
	　　　一个临界区，这样会带来非同一个会话要限制在同一个临界区，但由于出错和关闭相于操作
	　　　较少，所以基本不会影响性能，所以只配备了一个临界区。
	*/
	CBLUEWinCriticalSection m_sessionCloseErrorCS;
};