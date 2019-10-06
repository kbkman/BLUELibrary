#pragma once
#include <BLUEClientNetWorkModel.h>
#include <BLUEDebug.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

class CBLUEClientNetWorkModel : public IBLUEClientNetWorkModel
{
public:
	CBLUEClientNetWorkModel(void);
	~CBLUEClientNetWorkModel(void);

public:
	virtual int Create(
		IBLUEMTSafeMessageQueue* pMSMQ,
		const BLUEDWORD dwMSMQMessageType,
		const BLUELPCSTR lpstrConnectIP,
		const BLUEWORD nConnectPort
		);
	virtual void Destroy(void);
	virtual void RecvData(IBLUESessionIOData* pRecvData);
	virtual void SendData(IBLUESessionIOData* pSendData);

public:
	BLUEDWORD RecvThread(void);
	BLUEDWORD SendThread(void);
	static BLUEDWORD stRecvThread(void* p);
	static BLUEDWORD stSendThread(void* p);

private:
	IBLUEMTSafeMessageQueue* m_pMSMQ;
	BLUEDWORD m_dwMSMQMessageType;//ʹ�ð�ȫ��Ϣ����ʱʹ�õ���Ϣ����ֵ
	SOCKET m_socket;

	IBLUEMTSafeMessageQueue* m_pRecvMSMQ;
	IBLUEMTSafeMessageQueue* m_pSendMSMQ;
	IBLUEThread* m_pRecvThread;//���������߳�
	IBLUEThread* m_pSendThread;//���������߳�
};