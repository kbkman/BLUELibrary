#ifndef __BLUECLIENTNETWORKMODEL_H__
#define __BLUECLIENTNETWORKMODEL_H__

#include <BLUEDef.h>
#include <BLUEAdv.h>


#ifdef BLUECLIENTNETWORKMODEL_EXPORTS
#define BLUECLIENTNETWORKMODEL_API _declspec(dllexport)
#else
#define BLUECLIENTNETWORKMODEL_API _declspec(dllimport)
#endif

//客户端网络模型发送给消息队列第一个参数的值表
#define CNWM_MSG_FP_RECVDATACOMPLETE       1              //接受数据完成
#define CNWM_MSG_FP_SENDDATACOMPLETE       2             //发送数据完成
#define CNWM_MSG_FP_ERROR                              3             //出现错误（如果是对方关闭，也使用本消息）

//网络服务器模型函数返回值
#define CNWMRESULT_OK                                       0       //正确返回
#define CNWMRESULT_E_CREATED                         1       //已经创建了，必须先Destroy
#define CNWMRESULT_E_CONNECT                        2       //连接失败
#define CNWMRESULT_E_THREAD                           3       //线程出错
#define CNWMRESULT_E_NETWORKINIT                 4       //网络初始化失败
#define CNWMRESULT_E_OTHER                             5       //其他未知错误
/**
接口名：客户端网络模型
功能：略
说明：无
版本：1.0.0.1
作者：BLUE
*/
class IBLUEClientNetWorkModel
{
public:
	virtual ~IBLUEClientNetWorkModel(void) {}

public:
	/*
	函数名：创建一个客户端网络模型
	功能：略
	说明：无
	参数：pMSMQ 使用的多线程安全消息队列
	　　　dwMSMQMessageType 使用的消息类型，到时在pMSMQ中获取的消息都是该类型，用于区别别的消息
	　　　lpstrConnectIP 连接的IP地址
	　　　nConnectPort 连接的端口地址
	返回值：具体创建结果
	*/
	virtual int Create(
		IBLUEMTSafeMessageQueue* pMSMQ,
		const BLUEDWORD dwMSMQMessageType,
		const BLUELPCSTR lpstrConnectIP,
		const BLUEWORD nConnectPort
		) = 0;

	/*
	函数名：销毁客户端网络模型
	功能：略
	说明：无
	参数：无
	返回值：无
	*/
	virtual void Destroy(void) = 0;

	/*
	函数名：接受数据
	功能：略
	说明：无
	参数：pRecvData 接受的数据接口，参阅IBLUESessionIOData
	返回值：无
	*/
	virtual void RecvData(IBLUESessionIOData* pRecvData) = 0;

	/*
	函数名：发送数据
	功能：略
	说明：无
	参数：pSendData 发送的数据接口，参阅IBLUESessionIOData
	返回值：无
	*/
	virtual void SendData(IBLUESessionIOData* pSendData) = 0;
};

/*
函数名：创建网络客户端模型对象
功能：略
说明：无
参数：无
返回值：网络客户端模型对象接口
*/
BLUECLIENTNETWORKMODEL_API IBLUEClientNetWorkModel* BLUECreateCNWM(void);

/*
函数名：销毁网络客户端模型对象
功能：略
说明：无
参数：p 网络客户端模型对象接口
返回值：无
*/
BLUECLIENTNETWORKMODEL_API void BLUEDestroyCNWM(IBLUEClientNetWorkModel* p);

#endif
