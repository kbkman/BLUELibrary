#ifndef __BLUESERVERNETWORKMODEL_H__
#define __BLUESERVERNETWORKMODEL_H__

#include <BLUEDef.h>
#include <BLUEAdv.h>

#ifdef BLUESERVERNETWORKMODEL_EXPORTS
#define BLUESERVERNETWORKMODEL_API _declspec(dllexport)
#else
#define BLUESERVERNETWORKMODEL_API _declspec(dllimport)
#endif

/**
接口名：服务器端网络会话接口
功能：略
说明：无
版本：1.0.0.1
作者：BLUE
*/
class IBLUEServerNetWorkSession : public IBLUESession
{
public:
	virtual ~IBLUEServerNetWorkSession(void) {}

public:
	/*
	函数名：接受数据
	功能：略
	说明：无
	参数：无
	返回值：无
	*/
	virtual void RecvData(IBLUESessionIOData* pRecvData) = 0;

	/*
	函数名：发送数据
	功能：略
	说明：无
	参数：lpBuf 数据缓冲
	　　　nLen 数据长度
	   返回值：无
	 */
	virtual void SendData(IBLUESessionIOData* pSendData) = 0;
};

//网络服务器模型发送给消息队列第一个参数的值表
#define SNWM_MSG_FP_SESSIONCREATE              1              //一个会话创建了
#define SNWM_MSG_FP_RECVDATACOMPLETE       2              //会话接受数据完成
#define SNWM_MSG_FP_SENDDATACOMPLETE       3             //会话发送数据完成
#define SNWM_MSG_FP_SESSIONCLOSE                4             //一个会话关闭了
#define SNWM_MSG_FP_SESSIONERROR                5             //一个会话出现错误

//网络服务器模型函数返回值
#define SNWMRESULT_OK                                       0       //正确返回
#define SNWMRESULT_E_CREATED                         1       //已经创建了，必须先Destroy
#define SNWMRESULT_E_LISTEN                            2       //监听失败，可能端口已经被占用或者监听地址出错
#define SNWMRESULT_E_THREAD                           3       //线程出错
#define SNWMRESULT_E_NETWORKINIT                 4       //网络初始化失败
#define SNWMRESULT_E_OTHER                             5       //其他未知错误

/**
接口名：网络服务器模型
功能：略
说明：无
版本：1.0.0.1
作者：BLUE
*/
class IBLUEServerNetWorkModel
{
public:
	virtual ~IBLUEServerNetWorkModel(void) {}//虚析构（必写）

public:
	/*
	函数名：创建服务器网络模型
	参数：pMSMQ 多线程安全消息队列
	　　　dwMSMQMessageType
　　　　lpstrListenIP 监听IP地址（只使用ANSI，不使用Unicode）
　　　　nListenPort 监听端口
	返回：是否成功，请参阅SNWMRESULT_E_XXX宏列表
	*/
	virtual int Create(
		IBLUEMTSafeMessageQueue* pMSMQ,
		const BLUEDWORD dwMSMQMessageType,
		const BLUELPCSTR lpstrListenIP,
		const BLUEWORD nListenPort) = 0;

	/*
	函数名：销毁服务器网络模型
	说明：创建后必须要调用本函数来销毁
	*/
	virtual void Destroy(void) = 0;

	/*
	函数名：运行服务器网络模型，开始收发数据
	说明：必须Create后才有效
	*/
	virtual void Run(void) = 0;

	/*
	函数名：暂停服务器网络模型，停止收发数据
	说明：必须Create后，并且Run后才有效
	*/
	virtual void Pause(void) = 0;
};

/*
函数名：创建网络服务器模型对象
功能：略
说明：无
参数：无
返回值：网络服务器模型对象接口
*/
BLUESERVERNETWORKMODEL_API IBLUEServerNetWorkModel* BLUECreateSNWM(void);

/*
函数名：销毁网络服务器模型对象
功能：略
说明：无
参数：p 网络服务器模型对象接口
返回值：无
*/
BLUESERVERNETWORKMODEL_API void BLUEDestroySNWM(IBLUEServerNetWorkModel* p);


#endif