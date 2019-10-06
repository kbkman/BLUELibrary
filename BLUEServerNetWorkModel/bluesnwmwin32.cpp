#include "BLUESNWMWin32.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
////////////CBLUEServerNetWorkSession/////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
CBLUEServerNetWorkSession::CBLUEServerNetWorkSession(SOCKET socket, IBLUESNWSessionProcessor* pSEP) :
m_socket(socket),
m_pSEP(pSEP)
{
	BLUEASSERT(socket != INVALID_SOCKET);
	BLUEASSERT(pSEP);

	m_recvSocketIOColl.reserve(5);//缓冲区先扩展的大一些
	m_sendSocketIOColl.reserve(15);//缓冲区先扩展的大一些
}

CBLUEServerNetWorkSession::~CBLUEServerNetWorkSession(void)
{
	Close();

	//清除所有发IO信息发
	for (WOLPSOCKETIOCOLL::size_type i = 0; i < m_sendSocketIOColl.size(); i++)
		if (m_sendSocketIOColl[i])
			delete m_sendSocketIOColl[i];
	m_sendSocketIOColl.clear();

	//清除所有收IO信息发
	for (WOLPSOCKETIOCOLL::size_type i = 0; i < m_recvSocketIOColl.size(); i++)
		if (m_recvSocketIOColl[i])
			delete m_recvSocketIOColl[i];
	m_recvSocketIOColl.clear();
}

void CBLUEServerNetWorkSession::Close(void)
{
	if (!m_bIsClosed)
	{
		BLUEASSERT(m_socket != INVALID_SOCKET);
		::closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		m_bIsClosed = true;
	}
}

void CBLUEServerNetWorkSession::Delete(void)
{
	BLUEASSERT(m_pSEP);
	m_pSEP->EraseSession(this);
}

void CBLUEServerNetWorkSession::RecvData(IBLUESessionIOData* pRecvData)
{
	//BLUEASSERT(m_socket != INVALID_SOCKET);
	BLUEASSERT(m_pSEP);
	BLUEASSERT(pRecvData);

	//找一个可以使用的接受SOCKETIO
	WOLPSOCKETIO* pSocketIO = BLUENULL;

	//不能同时多个线程进入可用SOCKETIO检索，故要加锁
	m_recvSocketIOCollCS.Lock();
	BLUETRACE(_BLUET("m_recvSocketIOColl.size=%d\n"), m_recvSocketIOColl.size());
	//开始检索
	for (WOLPSOCKETIOCOLL::size_type nCollSize = 0; nCollSize < m_recvSocketIOColl.size(); nCollSize++)
	{
		//如果该位置没有IO信息包，则申请后返回
		if (m_recvSocketIOColl[nCollSize] == BLUENULL)
		{
			m_recvSocketIOColl[nCollSize] = new WOLPSOCKETIO;
			m_recvSocketIOColl[nCollSize]->nIOType = WOLPSOCKETIO_TYPE_RECV;
			pSocketIO = m_recvSocketIOColl[nCollSize];
			pSocketIO->SetUsing(true);//SOCKETIO要设置为使用中
			break;
		}

		//如果存在且不在使用中则返回该IO信息包
		if (!m_recvSocketIOColl[nCollSize]->IsUsing())
		{
			pSocketIO = m_recvSocketIOColl[nCollSize];
			BLUEASSERT(pSocketIO->nIOType == WOLPSOCKETIO_TYPE_RECV);
			pSocketIO->SetUsing(true);//SOCKETIO要设置为使用中
			break;
		}
	}

	//如果没有找到，只能在未尾添加一个并使用
	if (pSocketIO == BLUENULL)
	{
		pSocketIO = new WOLPSOCKETIO;
		pSocketIO->nIOType = WOLPSOCKETIO_TYPE_RECV;
		pSocketIO->SetUsing(true);//SOCKETIO要设置为使用中
		m_recvSocketIOColl.push_back(pSocketIO);
	}
	//解锁
	m_recvSocketIOCollCS.Unlock();

	//填充buf len other
	pRecvData->GetBufferInfo(pSocketIO->wsaBuf.buf, pSocketIO->wsaBuf.len);
	pSocketIO->pSessionIOData = pRecvData;
	BLUEASSERT(pSocketIO->wsaBuf.buf);
	BLUEASSERT(pSocketIO->wsaBuf.len > 0);

	//发送数据
	DWORD dwRecvBytes;
	DWORD dwFlags = 0;
	::memset(&pSocketIO->ola, 0, sizeof(pSocketIO->ola));//ola要置0
	BLUEASSERT(pSocketIO->nIOType == WOLPSOCKETIO_TYPE_RECV);
	//调用发送函数发生数据
	if (::WSARecv(
		m_socket,
		&pSocketIO->wsaBuf, 1,
		&dwRecvBytes,
		&dwFlags,
		&pSocketIO->ola,
		NULL
		) == SOCKET_ERROR)
	{
		//接受发生错误（非阻塞情况）时
		if (::WSAGetLastError() != WSA_IO_PENDING)
		{
			pSocketIO->SetUsing(false);//发生时候，则该发送IO要置为非使用中
			m_pSEP->SessionErrorProcess(this, pRecvData);//让错误处理器来处理本会话出错
		}
	}
}

void CBLUEServerNetWorkSession::SendData(IBLUESessionIOData* pSendData)
{
	//BLUEASSERT(m_socket != INVALID_SOCKET);
	BLUEASSERT(m_pSEP);
	BLUEASSERT(pSendData);

	//找一个可以使用的发送SOCKETIO
	WOLPSOCKETIO* pSocketIO = BLUENULL;

	//不能同时多个线程进入可用SOCKETIO检索，故要加锁
	m_sendSocketIOCollCS.Lock();
	BLUETRACE(_BLUET("m_sendSocketIOColl.size=%d\n"), m_sendSocketIOColl.size());
	//开始检索
	for (WOLPSOCKETIOCOLL::size_type nCollSize = 0; nCollSize < m_sendSocketIOColl.size(); nCollSize++)
	{
		//如果该位置没有IO信息包，则申请后返回
		if (m_sendSocketIOColl[nCollSize] == BLUENULL)
		{
			m_sendSocketIOColl[nCollSize] = new WOLPSOCKETIO;
			m_sendSocketIOColl[nCollSize]->nIOType = WOLPSOCKETIO_TYPE_SEND;
			pSocketIO = m_sendSocketIOColl[nCollSize];
			pSocketIO->SetUsing(true);//SOCKETIO要设置为使用中
			break;
		}

		//如果存在且不在使用中则返回该IO信息包
		if (!m_sendSocketIOColl[nCollSize]->IsUsing())
		{
			pSocketIO = m_sendSocketIOColl[nCollSize];
			BLUEASSERT(pSocketIO->nIOType == WOLPSOCKETIO_TYPE_SEND);
			pSocketIO->SetUsing(true);//SOCKETIO要设置为使用中
			break;
		}
	}

	//如果没有找到，只能在未尾添加一个并使用
	if (pSocketIO == BLUENULL)
	{
		pSocketIO = new WOLPSOCKETIO;
		pSocketIO->nIOType = WOLPSOCKETIO_TYPE_SEND;
		pSocketIO->SetUsing(true);//SOCKETIO要设置为使用中
		m_sendSocketIOColl.push_back(pSocketIO);
	}
	//解锁
	m_sendSocketIOCollCS.Unlock();

	//填充buf len other
	pSendData->GetBufferInfo(pSocketIO->wsaBuf.buf, pSocketIO->wsaBuf.len);
	pSocketIO->pSessionIOData = pSendData;
	BLUEASSERT(pSocketIO->wsaBuf.buf);
	BLUEASSERT(pSocketIO->wsaBuf.len > 0);

	//发送数据
	DWORD dwSendBytes;
	::memset(&pSocketIO->ola, 0, sizeof(pSocketIO->ola));//ola要置0
	BLUEASSERT(pSocketIO->nIOType == WOLPSOCKETIO_TYPE_SEND);
	//调用发送函数发生数据
	if (::WSASend(
		m_socket,
		&pSocketIO->wsaBuf, 1,
		&dwSendBytes,
		0,
		&pSocketIO->ola,
		NULL
		) == SOCKET_ERROR)
	{
		//接受发生错误（非阻塞情况）时
		if (::WSAGetLastError() != WSA_IO_PENDING)
		{
			pSocketIO->SetUsing(false);//发生时候，则该发送IO要置为非使用中
			m_pSEP->SessionErrorProcess(this, pSendData);//让错误处理器来处理本会话出错
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
////////////CBLUEServerNetWorkModel///////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
CBLUEServerNetWorkModel::CBLUEServerNetWorkModel(void) :
m_pMSMQ(BLUENULL),
m_hCompletionPort(NULL),
m_pListenThread(BLUENULL),
m_listenSocket(INVALID_SOCKET)
{
}

CBLUEServerNetWorkModel::~CBLUEServerNetWorkModel(void)
{
	Destroy();
}

int CBLUEServerNetWorkModel::Create(
	IBLUEMTSafeMessageQueue* pMSMQ,
	const BLUEDWORD dwMSMQMessageType,
	const BLUELPCSTR lpstrListenIP,
	const BLUEWORD nListenPort)
{
	BLUEASSERT(pMSMQ);

	int nResult = SNWMRESULT_E_OTHER;

	//已经创建了
	if (m_hCompletionPort)
		return SNWMRESULT_E_CREATED;

	//启动winsock2.2
	WSADATA wsaData;
	if (::WSAStartup(0x0202, &wsaData) != NO_ERROR)
	{
		nResult = SNWMRESULT_E_NETWORKINIT;
		goto ERROR_PROCESS;
	}

	//创建完成端口
	BLUEASSERT(m_hCompletionPort == NULL);
	m_hCompletionPort = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, BLUENULL, 0, 0);
	if (m_hCompletionPort == NULL)
	{
		nResult = SNWMRESULT_E_NETWORKINIT;
		goto ERROR_PROCESS;
	}
	
	BLUEASSERT(nListenPort > 0);
	BLUEASSERT(m_listenSocket == INVALID_SOCKET);

	//监听任务
	//监听地址处理
	m_sddrListen.sin_family = AF_INET;//协议赋值
	m_sddrListen.sin_port = ::htons(nListenPort);//端口赋值
	m_sddrListen.sin_addr.s_addr = (lpstrListenIP == BLUENULL) ? ::htonl(INADDR_ANY) : ::inet_addr(lpstrListenIP);//IP赋值

	//创建监听套接字
	if ((m_listenSocket = ::WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
		WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)//创建监听套接字
	{
		nResult = SNWMRESULT_E_NETWORKINIT;
		goto ERROR_PROCESS;
	}

	//绑定套接字
	if (::bind(m_listenSocket, (sockaddr*)&m_sddrListen, sizeof(m_sddrListen)) == SOCKET_ERROR)
	{
		nResult = SNWMRESULT_E_LISTEN;
		goto ERROR_PROCESS;
	}

	//监听套接字
	if (::listen(m_listenSocket, 10) == SOCKET_ERROR)
	{
		nResult = SNWMRESULT_E_NETWORKINIT;
		goto ERROR_PROCESS;
	}

	//创建监听端口线程
	if ((m_pListenThread = ::BLUEAdvCreateThread()) == BLUENULL)
	{
		nResult = SNWMRESULT_E_THREAD;
		goto ERROR_PROCESS;
	}
	if (!m_pListenThread->Create(stListenThread, this, true))
	{
		::BLUEAdvDestroyThread(m_pListenThread);
		nResult = SNWMRESULT_E_THREAD;
		goto ERROR_PROCESS;
	}

	//获取CPU数量
	BLUEDWORD dwNumberOfCPU = ::BLUEAdvGetNumberOfCPU();
	BLUEASSERT(dwNumberOfCPU > 0);
	if (dwNumberOfCPU < 1)
	{
		nResult = SNWMRESULT_E_OTHER;
		goto ERROR_PROCESS;
	}

	//创建工作线程
	m_workThreadColl.assign(dwNumberOfCPU * 2, BLUENULL);//创建两倍于cpu数量的线程
	for (BLUESessionColl::size_type nCollSize = 0; nCollSize < m_workThreadColl.size(); nCollSize++)
	{
		if ((m_workThreadColl[nCollSize] = ::BLUEAdvCreateThread()) == BLUENULL)
		{
			nResult = SNWMRESULT_E_THREAD;
			goto ERROR_PROCESS;
		}

		if (!m_workThreadColl[nCollSize]->Create(stWorkThread, this, true))
		{
			::BLUEAdvDestroyThread(m_workThreadColl[nCollSize]);
			m_workThreadColl[nCollSize] = BLUENULL;
			nResult = SNWMRESULT_E_THREAD;
			goto ERROR_PROCESS;
		}
	}

	m_pMSMQ = pMSMQ;//安全消息队列赋值
	m_dwMSMQMessageType = dwMSMQMessageType;
	return SNWMRESULT_OK;


ERROR_PROCESS:
	Destroy();
	return nResult;
}

void CBLUEServerNetWorkModel::Destroy(void)
{
	//通知道所有工作线程可以结束了
	for (BLUEThreadColl::size_type nCollSize = 0; nCollSize < m_workThreadColl.size(); nCollSize++)
		::PostQueuedCompletionStatus(m_hCompletionPort, 0, BLUENULL, BLUENULL);
	

	//关闭所有工作线程
	for (BLUEThreadColl::size_type nCollSize = 0; nCollSize < m_workThreadColl.size(); nCollSize++)
	{
		if (m_workThreadColl[nCollSize])
		{
			m_workThreadColl[nCollSize]->Close(3000);//终止线程运行，等待时间5秒
			::BLUEAdvDestroyThread(m_workThreadColl[nCollSize]);
		}
	}
	m_workThreadColl.clear();

	//关闭监听线程
	if (m_pListenThread)
	{
		::closesocket(m_listenSocket);//关闭监听套接字
		m_pListenThread->Close(3000);
		::BLUEAdvDestroyThread(m_pListenThread);
		m_pListenThread = BLUENULL;
		m_listenSocket = INVALID_SOCKET;
	}

	//删除所有用户会话（退出时，其他线程已经部分关闭，所以不再调用临界区保证多线程安全
	for (BLUESessionColl::iterator it = m_sessionColl.begin(), itEnd = m_sessionColl.end(); it != itEnd; it++)
	{
		(*it)->Close();
		delete *it;
	}
	m_sessionColl.clear();

	//关闭完成端口
	if (m_hCompletionPort)
	{
		::CloseHandle(m_hCompletionPort);
		m_hCompletionPort = NULL;
	}

	//清理WinSock
	::WSACleanup();
}

void CBLUEServerNetWorkModel::Run(void)
{
	BLUEASSERT(m_hCompletionPort);
	BLUEASSERT(m_pListenThread);

	//监听线程运行
	m_pListenThread->Resume();

	//所有工作线程运行
	for (BLUESessionColl::size_type nCollSize = 0; nCollSize < m_workThreadColl.size(); nCollSize++)
		m_workThreadColl[nCollSize]->Resume();
}

void CBLUEServerNetWorkModel::Pause(void)
{
	BLUEASSERT(m_hCompletionPort);
	BLUEASSERT(m_pListenThread);

	//所有工作线程挂起
	for (BLUESessionColl::size_type nCollSize = 0; nCollSize < m_workThreadColl.size(); nCollSize++)
		m_workThreadColl[nCollSize]->Suspend();

	//监听线程挂起
	m_pListenThread->Suspend();
}

void CBLUEServerNetWorkModel::EraseSession(IBLUEServerNetWorkSession* pServerNetWorkSession)
{
	BLUEASSERT(pServerNetWorkSession);

	//关闭会话，将会话从会话集合中删除，并删除会话资源
	BLUESessionColl::size_type nEraseNumber;//删除会话的个数
	m_sessionCollCS.Lock();
	nEraseNumber = m_sessionColl.erase(pServerNetWorkSession);//在集合中删除该会话，返回删除的个数（只可能为0或1）
	m_sessionCollCS.Unlock();

	//如果集合中有该会话，则关闭该会话，并且删除会话资源
	//判断nEraseNumber是为了避免反复删除同一个会话资源（否则多线程中可能会有危险）
	if (nEraseNumber > 0)
	{
		pServerNetWorkSession->Close();
		delete pServerNetWorkSession;
	}
}

void CBLUEServerNetWorkModel::SessionCloseProcess(
	IBLUEServerNetWorkSession* pServerNetWorkSession,
	IBLUESessionIOData* pSessionIOData
	)
{
	BLUEASSERT(pServerNetWorkSession);

	//如果会话还没有关闭，则给使用者发送一条会话关闭信息
	m_sessionCloseErrorCS.Lock();
	if (!pServerNetWorkSession->IsClose())
	{
		BLUEASSERT(m_pMSMQ);
		pServerNetWorkSession->Close();//不要忘记关闭会话
		m_pMSMQ->PutOneMessage(
			m_dwMSMQMessageType,
			SNWM_MSG_FP_SESSIONCLOSE,
			pServerNetWorkSession,
			pSessionIOData
			);
	}
	m_sessionCloseErrorCS.Unlock();
}

void CBLUEServerNetWorkModel::SessionErrorProcess(
	IBLUEServerNetWorkSession* pServerNetWorkSession,
	IBLUESessionIOData* pSessionIOData
	)
{
	BLUEASSERT(pServerNetWorkSession);

	m_sessionCloseErrorCS.Lock();
	//如果会话还没有关闭，则给使用者发送一条会话出错信息
	if (!pServerNetWorkSession->IsClose())
	{
		BLUEASSERT(m_pMSMQ);
		pServerNetWorkSession->Close();//不要忘记关闭会话
		m_pMSMQ->PutOneMessage(
			m_dwMSMQMessageType,
			SNWM_MSG_FP_SESSIONERROR,
			pServerNetWorkSession,
			pSessionIOData
			);
	}
	m_sessionCloseErrorCS.Unlock();
}

BLUEDWORD CBLUEServerNetWorkModel::ListenThread(void)
{
	BLUEASSERT(m_hCompletionPort);
	BLUEASSERT(m_listenSocket != INVALID_SOCKET);
	BLUETRACE(_BLUET("服务端网络模型监听线程开始\n"));

	//开始循环监听有没有连接
	SOCKET as;
	while (true)
	{
		as = ::WSAAccept(m_listenSocket, NULL, NULL, NULL, 0);
		if (as == INVALID_SOCKET)//监听出错
		{
			//如果出错，预料情况错误为WSAINTR，这是由于关闭了监听套字导致
			if (::WSAGetLastError() == WSAEINTR)
			{
				BLUETRACE(_BLUET("监听端口关闭，退出监听循环\n"));
				break;
			}
			else//出错，但错误不是预料，这种情况一般不会发生
			{
				BLUETRACE(_BLUET("监听失败，非预料情况，ERROR=%d\n"), ::WSAGetLastError());
				continue;
			}
		}
		else//监听成功
		{
			//如果监听成功，则要新创建一个用户会话
			IBLUEServerNetWorkSession* pServerNetWorkSession = new CBLUEServerNetWorkSession(as, this);

#pragma warning(disable:4311)//不让(DWORD)pServerNetWorkSession产生警告（编译器命令）
			//将该套接字与完成端口关联
			if (::CreateIoCompletionPort(
				(HANDLE)as,
				m_hCompletionPort,
				(DWORD)pServerNetWorkSession,
				0) == NULL)
			{
				//如果关联失败，则直接删除该会话，不再作任何处理
				BLUETRACE(_BLUET("关联完成端口失败"), ::WSAGetLastError());
				delete pServerNetWorkSession;
				continue;
			}
#pragma warning(default:4311)

			//为了保证会话集合在多线程下安全，使用临界区上锁
			m_sessionCollCS.Lock();
			//不可能会重复
			BLUEASSERT(m_sessionColl.find(pServerNetWorkSession) == m_sessionColl.end());
			//集合中添加这个
			m_sessionColl.insert(pServerNetWorkSession);
			m_sessionCollCS.Unlock();

			//通知使用者，创建了一个会话
			BLUEASSERT(m_pMSMQ);
			m_pMSMQ->PutOneMessage(
				m_dwMSMQMessageType,
				SNWM_MSG_FP_SESSIONCREATE,
				pServerNetWorkSession,
				BLUENULL
				);
		}
	}

	BLUETRACE(_BLUET("服务端网络模型监听线程结束\n"));
	return 0;
}

BLUEDWORD CBLUEServerNetWorkModel::WorkThread(void)
{
	BLUEASSERT(m_hCompletionPort);
	BLUETRACE(_BLUET("服务端网络模型一个工作线程开始\n"));

	DWORD dwBytesTrans;//接受或发送的数据量
	CBLUEServerNetWorkSession* pServerNetWorkSession;//发生数据的会话
	WOLPSOCKETIO* pSocketIO;//发生数量的IO
	DWORD dwRecvOrSendBytes, dwFlag;

	//开始在完成端口上接受信息(循环)
	while (true)
	{
		if (::GetQueuedCompletionStatus(
					m_hCompletionPort,
					&dwBytesTrans,
					(PULONG_PTR)&pServerNetWorkSession,
					(LPOVERLAPPED*)&pSocketIO,
					INFINITE) == FALSE)
		{
			if (pSocketIO)
			{
				pSocketIO->SetUsing(false);
				SessionCloseProcess(pServerNetWorkSession, pSocketIO->pSessionIOData);
				BLUETRACE(_BLUET("GQCS返回false，一个会话关闭连接，调用了SessionCloseProcess\n"));
				continue;
			}
			else//pSocketIO为NULL为非正常情况
			{
				BLUETRACE(_BLUET("服务端网络模型一个工作线程非正常结束，错误代码为=%d\n"), ::GetLastError());
				return 0;
			}
			continue;
		}

		//socket关闭或主动要求结束工作线程的情况下dwBytesTrans会为0
		if (dwBytesTrans == 0)
		{
			//所有参数均返回0时表示主要结束线程
			if (pServerNetWorkSession == BLUENULL)
			{
				BLUEASSERT(pSocketIO == BLUENULL);
				BLUETRACE(_BLUET("服务端网络模型一个工作线程正常结束\n"));
				return 0;
			}

			//否则表示有socket关闭
			BLUEASSERT(pSocketIO);
			pSocketIO->SetUsing(false);
			SessionCloseProcess(pServerNetWorkSession, pSocketIO->pSessionIOData);
			BLUETRACE(_BLUET("GQCS返回true，一个会话关闭连接，调用了SessionCloseProcess\n"));
			continue;
		}

		//会话指针不可能为空
		BLUEASSERT(pServerNetWorkSession);
		BLUEASSERT(pServerNetWorkSession->GetSocketHandle() != INVALID_SOCKET);
		BLUEASSERT(pSocketIO);
		BLUEASSERT(pSocketIO->IsUsing());
		BLUEASSERT(pSocketIO->wsaBuf.buf);
		BLUEASSERT(dwBytesTrans > 0);
		BLUEASSERT(dwBytesTrans <= pSocketIO->wsaBuf.len);

		//收/发数据进行偏移操作
		pSocketIO->wsaBuf.len -= dwBytesTrans;
		pSocketIO->wsaBuf.buf += dwBytesTrans;

		BLUEASSERT(pSocketIO->wsaBuf.len >= 0);
		//如果收/发数据已经完成
		if (pSocketIO->wsaBuf.len == 0)
		{
			//收数据IO情况
			if (pSocketIO->nIOType == WOLPSOCKETIO_TYPE_RECV)
			{
				m_pMSMQ->PutOneMessage(m_dwMSMQMessageType,
					SNWM_MSG_FP_RECVDATACOMPLETE,
					pServerNetWorkSession,
					pSocketIO->pSessionIOData);
				//设置该IO已经不在使用状态了
				pSocketIO->SetUsing(false);
			}
			else//发数据IO情况
			{
				m_pMSMQ->PutOneMessage(m_dwMSMQMessageType,
					SNWM_MSG_FP_SENDDATACOMPLETE,
					pServerNetWorkSession,
					pSocketIO->pSessionIOData);
				//设置该IO已经不在使用状态了
				pSocketIO->SetUsing(false);
			}
		}
		else//未完成数据的收/发情况下则继续收/发
		{
			//无论收还是发，都要先将ola结构体全部置0
			::memset(&pSocketIO->ola, 0, sizeof(pSocketIO->ola));

			//收数据IO情况
			if (pSocketIO->nIOType == WOLPSOCKETIO_TYPE_RECV)
			{
				//收数据时dwFlag要置0
				dwFlag = 0;

				//继续发送
				if (::WSARecv(
					pServerNetWorkSession->GetSocketHandle(),
					&pSocketIO->wsaBuf, 1,
					&dwRecvOrSendBytes,
					&dwFlag,
					&pSocketIO->ola,
					NULL
					) == SOCKET_ERROR)
				{
					//接受发生错误（非阻塞情况）时
					if (::WSAGetLastError() != WSA_IO_PENDING)
					{
						pSocketIO->SetUsing(false);//发生时候，则该发送IO要置为非使用中
						SessionErrorProcess(
							pServerNetWorkSession,
							pSocketIO->pSessionIOData
							);//处理会话出错
					}
				}
			}
			else//发数据IO情况
			{
				//继续发送
				if (::WSASend(
					pServerNetWorkSession->GetSocketHandle(),
					&pSocketIO->wsaBuf, 1,
					&dwRecvOrSendBytes,
					0,
					&pSocketIO->ola,
					NULL
					) == SOCKET_ERROR)
				{
					//接受发生错误（非阻塞情况）时
					if (::WSAGetLastError() != WSA_IO_PENDING)
					{
						pSocketIO->SetUsing(false);//发生时候，则该发送IO要置为非使用中
						SessionErrorProcess(
							pServerNetWorkSession,
							pSocketIO->pSessionIOData
							);//处理会话出错
					}
				}
			}
		}
	}

	return 0;
}

BLUEDWORD CBLUEServerNetWorkModel::stListenThread(void* p)
{
	BLUEASSERT(p);
	CBLUEServerNetWorkModel* pBSNWM = (CBLUEServerNetWorkModel*)p;
	return pBSNWM->ListenThread();
}

BLUEDWORD CBLUEServerNetWorkModel::stWorkThread(void* p)
{
	BLUEASSERT(p);
	CBLUEServerNetWorkModel* pBSNWM = (CBLUEServerNetWorkModel*)p;
	return pBSNWM->WorkThread();
}

IBLUEServerNetWorkModel* BLUECreateSNWM(void)
{
	IBLUEServerNetWorkModel* p = new CBLUEServerNetWorkModel();
	return static_cast<IBLUEServerNetWorkModel*>(p);
}

void BLUEDestroySNWM(IBLUEServerNetWorkModel* p)
{
	BLUEASSERT(p);
	delete p;
}