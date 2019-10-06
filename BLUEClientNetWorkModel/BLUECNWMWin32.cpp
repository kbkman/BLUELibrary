#include "BLUECNWMWin32.h"

CBLUEClientNetWorkModel::CBLUEClientNetWorkModel(void) :
m_pMSMQ(BLUENULL),
m_socket(INVALID_SOCKET),
m_pRecvMSMQ(BLUENULL),
m_pSendMSMQ(BLUENULL),
m_pRecvThread(BLUENULL),
m_pSendThread(BLUENULL)
{

}

CBLUEClientNetWorkModel::~CBLUEClientNetWorkModel(void)
{
	Destroy();
}

int CBLUEClientNetWorkModel::Create(
									 IBLUEMTSafeMessageQueue* pMSMQ,
									 const BLUEDWORD dwMSMQMessageType,
									 const BLUELPCSTR lpstrConnectIP,
									 const BLUEWORD nConnectPort)
{

	BLUEASSERT(pMSMQ);
	m_pMSMQ = pMSMQ;
	m_dwMSMQMessageType = dwMSMQMessageType;

	int nResult = CNWMRESULT_E_OTHER;

	//启动winsock2.2
	WSADATA wsaData;
	if (::WSAStartup(0x0202, &wsaData) != NO_ERROR)
	{
		nResult = CNWMRESULT_E_NETWORKINIT;
		goto ERROR_PROCESS;
	}

	//创建套接字
	m_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket == INVALID_SOCKET)
	{
		nResult = CNWMRESULT_E_NETWORKINIT;
		goto ERROR_PROCESS;
	}

	//连接服务器
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = ::htons(nConnectPort);
	serverAddr.sin_addr.s_addr = ::inet_addr(lpstrConnectIP);
	BLUEASSERT(lpstrConnectIP);
	BLUEASSERT(nConnectPort > 0);
	if (::connect(m_socket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		nResult = CNWMRESULT_E_CONNECT;
		goto ERROR_PROCESS;
	}

	//创建接受线程
	nResult = CNWMRESULT_E_OTHER;
	m_pRecvMSMQ = ::BLUEAdvCreateMSMQ();
	if (m_pRecvMSMQ == BLUENULL)
		goto ERROR_PROCESS;
	nResult = CNWMRESULT_E_THREAD;
	m_pRecvThread = ::BLUEAdvCreateThread();
	if (m_pRecvThread == BLUENULL)
		goto ERROR_PROCESS;
	if (!m_pRecvThread->Create(stRecvThread, this, false))
		goto ERROR_PROCESS;

	//创建发送线程
	nResult = CNWMRESULT_E_OTHER;
	m_pSendMSMQ = ::BLUEAdvCreateMSMQ();
	if (m_pSendMSMQ == BLUENULL)
		goto ERROR_PROCESS;
	nResult = CNWMRESULT_E_THREAD;
	m_pSendThread = ::BLUEAdvCreateThread();
	if (m_pSendThread == BLUENULL)
		goto ERROR_PROCESS;
	if (!m_pSendThread->Create(stSendThread, this, false))
		goto ERROR_PROCESS;

	return CNWMRESULT_OK;

ERROR_PROCESS:
	Destroy();
	return nResult;
}

void CBLUEClientNetWorkModel::Destroy(void)
{
	//关闭socket
	if (m_socket != INVALID_SOCKET)
	{
		::closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}

	//关闭发送数据线程
	if (m_pSendMSMQ)
	{
		m_pSendMSMQ->PutOneQuitMessage();
		if (m_pSendThread)
		{
			m_pSendThread->Close(3000);
			::BLUEAdvDestroyThread(m_pSendThread);
			m_pSendThread = BLUENULL;
		}
		::BLUEAdvDestroyMSMQ(m_pSendMSMQ);
		m_pSendMSMQ = BLUENULL;
	}

	//关闭接受数据线程
	if (m_pRecvMSMQ)
	{
		m_pRecvMSMQ->PutOneQuitMessage();
		if (m_pRecvThread)
		{
			m_pRecvThread->Close(3000);
			::BLUEAdvDestroyThread(m_pRecvThread);
			m_pRecvThread = BLUENULL;
		}
		::BLUEAdvDestroyMSMQ(m_pRecvMSMQ);
		m_pRecvMSMQ = BLUENULL;
	}

	//清理WSA
	::WSACleanup();
}
#define RECVDATAMSMQ_MESSAGETYPE     (BLUEMSMQ_MESSAGETYPE_BASE + 1)
void CBLUEClientNetWorkModel::RecvData(IBLUESessionIOData* pRecvData)
{
	BLUEASSERT(pRecvData);
	char* lpBuf;
	unsigned long nBufLen;
	pRecvData->GetBufferInfo(lpBuf, nBufLen);
	m_pRecvMSMQ->PutOneMessage(
		RECVDATAMSMQ_MESSAGETYPE,
		nBufLen,
		lpBuf,
		pRecvData
		);
}

#define SENDDATAMSMQ_MESSAGETYPE     (BLUEMSMQ_MESSAGETYPE_BASE + 2)
void CBLUEClientNetWorkModel::SendData(IBLUESessionIOData* pSendData)
{
	BLUEASSERT(pSendData);
	char* lpBuf;
	unsigned long nBufLen;
	pSendData->GetBufferInfo(lpBuf, nBufLen);
	m_pSendMSMQ->PutOneMessage(
		SENDDATAMSMQ_MESSAGETYPE,
		nBufLen,
		lpBuf,
		pSendData
		);
}

BLUEDWORD CBLUEClientNetWorkModel::RecvThread(void)
{
	BLUEASSERT(m_socket != INVALID_SOCKET);
	BLUEASSERT(m_pRecvMSMQ);

	BLUEDWORD dwMessageType;
	BLUEDWORD fp;
	void* sp;
	void* tp;

	while (m_pRecvMSMQ->GetOneMessage(dwMessageType, fp, sp, tp))
	{
		BLUEASSERT(dwMessageType == RECVDATAMSMQ_MESSAGETYPE);
		BLUEASSERT(fp > 0);
		BLUEASSERT(sp);
		BLUEASSERT(tp);

		if (::recv(m_socket, (char*)sp, fp, 0) == SOCKET_ERROR)
		{
			m_pMSMQ->PutOneMessage(
				m_dwMSMQMessageType,
				CNWM_MSG_FP_ERROR,
				(void*)(__int64)::WSAGetLastError(),
				tp);
		}
		else
		{
			m_pMSMQ->PutOneMessage(
				m_dwMSMQMessageType,
				CNWM_MSG_FP_RECVDATACOMPLETE,
				BLUENULL,
				tp);
		}

	}

	return 0;
}

BLUEDWORD CBLUEClientNetWorkModel::SendThread(void)
{
	BLUEASSERT(m_socket != INVALID_SOCKET);
	BLUEASSERT(m_pSendMSMQ);

	BLUEDWORD dwMessageType;
	BLUEDWORD fp;
	void* sp;
	void* tp;

	while (m_pSendMSMQ->GetOneMessage(dwMessageType, fp, sp, tp))
	{
		BLUEASSERT(dwMessageType == SENDDATAMSMQ_MESSAGETYPE);
		BLUEASSERT(fp > 0);
		BLUEASSERT(sp);
		BLUEASSERT(tp);

		if (::send(m_socket, (char*)sp, fp, 0) == SOCKET_ERROR)
		{
			m_pMSMQ->PutOneMessage(
				m_dwMSMQMessageType,
				CNWM_MSG_FP_ERROR,
				(void*)(__int64)::WSAGetLastError(),
				tp);
		}
		else
		{
			m_pMSMQ->PutOneMessage(
				m_dwMSMQMessageType,
				CNWM_MSG_FP_SENDDATACOMPLETE,
				BLUENULL,
				tp);
		}
	}

	return 0;
}

BLUEDWORD CBLUEClientNetWorkModel::stRecvThread(void* p)
{
	BLUEASSERT(p);
	CBLUEClientNetWorkModel* pBCNWM = (CBLUEClientNetWorkModel*)p;
	return pBCNWM->RecvThread();
}

BLUEDWORD CBLUEClientNetWorkModel::stSendThread(void* p)
{
	BLUEASSERT(p);
	CBLUEClientNetWorkModel* pBCNWM = (CBLUEClientNetWorkModel*)p;
	return pBCNWM->SendThread();
}

IBLUEClientNetWorkModel* BLUECreateCNWM(void)
{
	IBLUEClientNetWorkModel* p = new CBLUEClientNetWorkModel();
	return static_cast<IBLUEClientNetWorkModel*>(p);
}

void BLUEDestroyCNWM(IBLUEClientNetWorkModel* p)
{
	BLUEASSERT(p);
	delete p;
}