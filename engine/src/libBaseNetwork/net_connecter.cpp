#include "stdafx.h"
#include "net_connecter.h"
#include "net_event_loop.h"

#include "libBaseCommon\logger.h"
#include "libBaseCommon\base_function.h"
#include "libBaseCommon\debug_helper.h"

namespace base
{
	void CNetConnecter::printInfo(const char* szFormat, ...)
	{
		if (nullptr == szFormat)
			return;

		char szBuf[1024] = { 0 };
		va_list arg;
		va_start(arg, szFormat);
		base::crt::vsnprintf(szBuf, _countof(szBuf), szFormat, arg);
		va_end(arg);

		PrintInfo("%s connect state: %d flag��%d local addr: %s %d remote addr: %s %d socket_id: %d error code[%d] send_index: %d send_count: %d", szBuf, this->m_eConnecterState, this->m_nFlag, this->getLocalAddr().szHost, this->getLocalAddr().nPort, this->getRemoteAddr().szHost, this->getRemoteAddr().nPort, this->GetSocketID(), getLastError(), this->m_nSendConnecterIndex, this->m_pNetEventLoop->getSendConnecterCount());
	}

	void CNetConnecter::onEvent(uint32_t nEvent)
	{
		switch (this->m_eConnecterState)
		{
		case eNCS_Connecting:
			if (nEvent&(eNET_Send|eNET_Error))
			{
				this->onConnect();
			}
			break;

		case eNCS_Connected:
		case eNCS_Disconnecting:
			if (nEvent&eNET_Error)
			{
				// ������Դ󵨵�ǿ�ƹر�����
				this->shutdown(true, "socket eNET_Error");
				return;
			}

			if (nEvent&eNET_Recv)
			{
				if ((this->m_nFlag&eNCF_CloseRecv) == 0)
					this->onRecv();
				else
					this->printInfo("socket is close recv data");
			}

			if (nEvent&eNET_Send)
			{
				this->onSend();
			}
			break;

		default:
			this->printInfo("state error��event��%d", nEvent);
		}
	}

	void CNetConnecter::release()
	{
		this->m_eConnecterState = eNCS_Disconnected;
		if (this->m_pHandler != nullptr)
		{
			if ((this->m_nFlag&eNCF_ConnectFail) == 0) 
			{
				PrintInfo("disconnect local addr: %s %d remote addr: %s %d socket_id: %d send_index: %d send_count: %d", this->getLocalAddr().szHost, this->getLocalAddr().nPort, this->getRemoteAddr().szHost, this->getRemoteAddr().nPort, this->GetSocketID(), this->m_nSendConnecterIndex, this->m_pNetEventLoop->getSendConnecterCount());
				this->m_pHandler->onDisconnect();
			}
			else
			{
				PrintInfo("connect fail local addr: %s %d remote addr: %s %d socket_id: %d send_index: %d send_count: %d", this->getLocalAddr().szHost, this->getLocalAddr().nPort, this->getRemoteAddr().szHost, this->getRemoteAddr().nPort, this->GetSocketID(), this->m_nSendConnecterIndex, this->m_pNetEventLoop->getSendConnecterCount());
				this->m_pHandler->onConnectFail();
			}
			this->m_pHandler = nullptr;
		}
		this->m_pNetEventLoop->delSendConnecter(this);
		CNetSocket::release();

		delete this;
	}

	bool CNetConnecter::init(uint32_t nSendBufferSize, uint32_t nRecvBufferSize, CNetEventLoop* pNetEventLoop)
	{
		if (!CNetSocket::init(nSendBufferSize, nRecvBufferSize, pNetEventLoop))
			return false;

		this->m_pRecvBuffer = new CNetRecvBuffer();
		if (!this->m_pRecvBuffer->init(_RECV_BUF_SIZE))
			return false;

		this->m_pSendBuffer = new CNetSendBuffer();
		if (!this->m_pSendBuffer->init(_SEND_BUF_SIZE))
			return false;

		return true;
	}

	void CNetConnecter::onConnect()
	{
		if (eNCS_Connecting != this->m_eConnecterState)
		{
			this->printInfo("connection state error");
			return;
		}
		
		this->m_nFlag &= ~eNCF_DisableWrite;
		/*
		���ֻ��д��˵�����ӳɹ������Խ�������Ĳ�����
		����������ȿɶ��ֿ�д����Ϊ���������
		��һ�������socket���ӳ��ִ��󣨲�Ҫ��Ϊʲô������ϵͳ�涨�ģ��ɶ���дʱ���п�����connect���ӳɹ���Զ�������Ͽ�������close(socket)����
		�ڶ��������connect���ӳɹ���socket���������õ���Զ���������͵����ݡ���Ҫͨ��connect���Ӻ󷵻ظ�errno��ֵ�������ж�������ͨ������ getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&error,&len); ��������ֵ���ж��Ƿ�������
		*/
		if (this->m_eConnecterType == eNCT_Initiative)
		{
			int32_t err;
			socklen_t len = sizeof(err);
			if (getsockopt(this->GetSocketID(), SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&err), (socklen_t*)&len) < 0)
			{
				this->m_nFlag |= eNCF_ConnectFail;
				this->shutdown(true, "SO_ERROR getsockopt error %d", getLastError());
				return;
			}
			if (err != 0)
			{
				this->m_nFlag |= eNCF_ConnectFail;
				this->shutdown(true, "SO_ERROR error %d", err);
				return;
			}
		}
		this->setLocalAddr();
		// ��ֹ�Լ����Լ����������
		if (this->getRemoteAddr() == this->getLocalAddr())
		{
			this->m_nFlag |= eNCF_ConnectFail;
			this->shutdown(true, "connect owner");
			return;
		}
		this->m_eConnecterState = eNCS_Connected;
		PrintInfo("connect local addr: %s %d remote addr: %s %d socket_id: %d", this->getLocalAddr().szHost, this->getLocalAddr().nPort, this->getRemoteAddr().szHost, this->getRemoteAddr().nPort, this->GetSocketID());

		if (nullptr != this->m_pHandler)
			this->m_pHandler->onConnect();
	}

	void CNetConnecter::onRecv()
	{
		while (true)
		{
			uint32_t nBufSize = this->m_pRecvBuffer->getWritableSize();
			if (0 == nBufSize)
			{
				this->m_pRecvBuffer->resize(this->m_pRecvBuffer->getBufferSize() * 2);
				nBufSize = this->m_pRecvBuffer->getWritableSize();
				PrintInfo("not have enought recv buffer append size to: %d", this->m_pRecvBuffer->getBufferSize());
			}
			char* pBuf = this->m_pRecvBuffer->getWritableBuffer();
#ifdef _WIN32
			int32_t nRet = ::recv(this->GetSocketID(), pBuf, (int32_t)nBufSize, MSG_NOSIGNAL);
#else
			int32_t nRet = ::read(this->GetSocketID(), pBuf, nBufSize);
#endif
			// �Զ��Ѿ��ر�
			if (0 == nRet)
			{
				this->printInfo("remote connection is close");
				this->m_nFlag |= eNCF_CloseRecv;
				this->m_eConnecterState = eNCS_Disconnecting;

				// ���õ��߼��������رգ�ֻҪû����Ҫ���͵����ݵ��ˣ��͹ر�����
				if (this->m_pSendBuffer->getTailDataSize() != 0)
					this->close();

				break;
			}
			else if (nRet < 0)
			{
				if (getLastError() == NW_EAGAIN || getLastError() == NW_EWOULDBLOCK)
					break;

				if (getLastError() == NW_EINTR)
					continue;

				this->shutdown(true, "recv data error");
				break;
			}
			this->m_pRecvBuffer->push(nRet);
			if (nullptr != this->m_pHandler)
			{
				uint32_t nSize = this->m_pHandler->onRecv(this->m_pRecvBuffer->getReadableBuffer(), this->m_pRecvBuffer->getReadableSize());
				this->m_pRecvBuffer->pop(nSize);
			}
		}
	}

	void CNetConnecter::onSend()
	{
		uint32_t nTotalDataSize = this->m_pSendBuffer->getTotalReadableSize();
		while (true)
		{
			uint32_t nDataSize = this->m_pSendBuffer->getTailDataSize();
			char* pData = this->m_pSendBuffer->getTailData();
			if (0 == nDataSize)
				break;

			this->m_nFlag &= ~eNCF_DisableWrite;
#ifdef _WIN32
			int32_t nRet = ::send(this->GetSocketID(), pData, (int32_t)nDataSize, MSG_NOSIGNAL);
#else
			int32_t nRet = ::write(this->GetSocketID(), pData, nDataSize);
#endif
			if (SOCKET_ERROR == nRet)
			{
				if (getLastError() == NW_EAGAIN || getLastError() == NW_EWOULDBLOCK)
				{
					this->m_nFlag |= eNCF_DisableWrite;
					break;
				}
				if (getLastError() == NW_EINTR)
					continue;

				this->shutdown(true, "send data error");
				break;
			}
			this->m_pSendBuffer->pop(nRet);

			if (nRet != (int32_t)nDataSize)
				break;
		}

		if (nTotalDataSize != 0 && this->m_pSendBuffer->getTailDataSize() == 0 && this->m_pHandler != nullptr)
			this->m_pHandler->onSendComplete(nTotalDataSize);
		
		if (this->m_eConnecterState == eNCS_Disconnecting && this->m_pSendBuffer->getTailDataSize() == 0)
			this->close();
	}

	void CNetConnecter::flushSend()
	{
		this->onSend();
	}

	CNetConnecter::CNetConnecter()
		: m_eConnecterType(eNCT_Unknown)
		, m_eConnecterState(eNCS_Disconnected)
		, m_nSendConnecterIndex(_Invalid_SendConnecterIndex)
		, m_nFlag(0)
		, m_pHandler(nullptr)
		, m_pRecvBuffer(nullptr)
		, m_pSendBuffer(nullptr)
	{
		this->m_nFlag |= eNCF_DisableWrite;
	}

	CNetConnecter::~CNetConnecter()
	{
		delete this->m_pSendBuffer;
		delete this->m_pRecvBuffer;

		DebugAst(this->getConnecterState() == eNCS_Disconnected);
	}

	int32_t CNetConnecter::getSendConnecterIndex() const
	{
		return this->m_nSendConnecterIndex;
	}

	void CNetConnecter::setSendConnecterIndex(int32_t nIndex)
	{
		this->m_nSendConnecterIndex = nIndex;
	}

	bool CNetConnecter::connect(const SNetAddr& sNetAddr)
	{
		if (!this->open())
			return false;

		if (!this->nonblock())
		{
			::closesocket(this->GetSocketID());
			return false;
		}
		if (!this->setBufferSize())
		{
			::closesocket(this->GetSocketID());
			return false;
		}
		this->m_sRemoteAddr = sNetAddr;

		struct sockaddr_in romateAddr;
		romateAddr.sin_family = AF_INET;
		// ������::htons https://bbs.archlinux.org/viewtopic.php?id=53751
		romateAddr.sin_port = htons(this->m_sRemoteAddr.nPort);
		romateAddr.sin_addr.s_addr = inet_addr(this->m_sRemoteAddr.szHost);
		memset(romateAddr.sin_zero, 0, sizeof(romateAddr.sin_zero));

		// connect���ź��жϵ�ʱ������
		int32_t nRet = ::connect(this->GetSocketID(), (sockaddr*)&romateAddr, sizeof(romateAddr));

		if (0 != nRet)
		{
			switch (getLastError())
			{
#ifdef _WIN32
			case NW_EWOULDBLOCK:
#else
			case NW_EINPROGRESS:
#endif
			case NW_EINTR:
			case NW_EISCONN:
				// ������¼�ѭ���ص���д�¼�֪ͨ���ӽ���
				this->m_eConnecterType = eNCT_Initiative;
				this->m_eConnecterState = eNCS_Connecting;

				this->m_pNetEventLoop->addSocket(this);
				break;

			default:
				PrintInfo("connect error socket_id: %d remote addr: %s %d err: %d", this->GetSocketID(), this->m_sRemoteAddr.szHost, this->m_sRemoteAddr.nPort, getLastError());
				::closesocket(this->GetSocketID());
				return false;
			}
		}
		else
		{
			this->m_eConnecterType = eNCT_Initiative;
			this->m_eConnecterState = eNCS_Connected;
			this->m_pNetEventLoop->addSocket(this);
			this->onConnect();
		}

		return true;
	}

	bool CNetConnecter::send(const void* pData, uint32_t nDataSize, bool bCache)
	{
		if (eNCS_Connecting == this->m_eConnecterState
			|| eNCS_Disconnected == this->m_eConnecterState
			|| (eNCS_Disconnecting == this->m_eConnecterState && (this->m_nFlag&eNCF_CloseSend) != 0))
			return false;

		if (!bCache)
		{
			if (this->m_pSendBuffer->getTailDataSize() == 0)
			{
				int32_t nRet = 0;
				do
				{
#ifdef _WIN32
					nRet = ::send(this->GetSocketID(), reinterpret_cast<const char*>(pData), (int32_t)nDataSize, MSG_NOSIGNAL);
#else
					nRet = ::write(this->GetSocketID(), pData, (int32_t)nDataSize);
#endif
				} while (nRet == SOCKET_ERROR && getLastError() == NW_EINTR);

				if (SOCKET_ERROR == nRet)
				{
					if (getLastError() != NW_EAGAIN && getLastError() != NW_EWOULDBLOCK)
					{
						this->shutdown(true, "send data error");
						return false;
					}

					this->m_nFlag |= eNCF_DisableWrite;
					nRet = 0;
				}

				DebugAstEx(nRet <= (int32_t)nDataSize, false);

				if (nRet != (int32_t)nDataSize)
					this->m_pSendBuffer->push(reinterpret_cast<const char*>(pData)+nRet, nDataSize - nRet);
				else if (this->m_pHandler != nullptr)
					this->m_pHandler->onSendComplete(nDataSize);
			}
			else
			{
				this->m_pSendBuffer->push(reinterpret_cast<const char*>(pData), nDataSize);
				// ���û����С���Ż��Ļ����������û�б�Ҫ�ģ���Ϊ�϶� != 0
				if ((this->m_nFlag&eNCF_DisableWrite) == 0)
					this->onSend();
			}
		}
		else
		{
			// ������һ�����û����������ֱ�ӷ���
			if (this->m_pSendBuffer->getTailDataSize() == 0 && nDataSize >= this->getSendBufferSize())
				return this->send(pData, nDataSize, false);
			
			this->m_pSendBuffer->push(reinterpret_cast<const char*>(pData), nDataSize);
			uint32_t nSendDataSize = this->m_pSendBuffer->getTotalReadableSize();

			if (nSendDataSize >= this->getSendBufferSize() && (this->m_nFlag&eNCF_DisableWrite) == 0)
				this->onSend(); // ����������Ͳ���ͻ�����һ��д�¼�������ʱ���������
			else
				this->m_pNetEventLoop->addSendConnecter(this);
		}

		return true;
	}

	/*	�������ӹر�:
	�����˳�ʱ, �Զ˷���RST�����RST���͵��㹻�죬ͬ�����£�����Ȼ�����յ�FIN������Recv�¼�����СΪ0
	����������ʱ���������ݻ�һֱ���ɴֱ����ʱ�������ʱ���ڣ����������ˣ����ڴ�ʱ�������ӵ���Ϣ��û�ˣ��Է��ڷ������ݺ���յ�RSTָ��
	����Ͽ�ʱ���������ݻ�һֱ���ɴֱ����ʱ�������ʱ����������������ô��ʱ���������ġ�
	ֱ��closesocketʱ����ʱ���͸����ܶ����У��Զ˻ᴥ��recv�¼������ݴ�СΪ0
	showdownʱ,��ʱ���Ͳ��У������ǿ��Եģ��Զ˻ᴥ��recv�¼������ݴ�СΪ0
	*/
	void CNetConnecter::shutdown(bool bForce, const char* szFormat, ...)
	{
		DebugAst(szFormat != nullptr);

		if (this->m_eConnecterState == eNCS_Disconnected)
			return;

		char szBuf[1024] = { 0 };
		va_list arg;
		va_start(arg, szFormat);
		base::crt::vsnprintf(szBuf, _countof(szBuf), szFormat, arg);
		va_end(arg);

		this->printInfo("request shutdown connection msg: %s", szBuf);

		this->m_eConnecterState = eNCS_Disconnecting;
		this->m_nFlag |= eNCF_CloseSend;
		
		if (bForce || this->m_pSendBuffer->getTailDataSize() == 0)
			this->close();
	}

	void CNetConnecter::setHandler(INetConnecterHandler* pHandler)
	{
		this->m_pHandler = pHandler;
		if (pHandler != nullptr)
			pHandler->setNetConnecter(this);
	}

	const SNetAddr& CNetConnecter::getLocalAddr() const
	{
		return this->m_sLocalAddr;
	}

	const SNetAddr& CNetConnecter::getRemoteAddr() const
	{
		return this->m_sRemoteAddr;
	}

	ENetConnecterType CNetConnecter::getConnecterType() const
	{
		return this->m_eConnecterType;
	}

	ENetConnecterState CNetConnecter::getConnecterState() const
	{
		return this->m_eConnecterState;
	}

	uint32_t CNetConnecter::getSendDataSize() const
	{
		return this->m_pSendBuffer->getTotalReadableSize();
	}

	uint32_t CNetConnecter::getRecvDataSize() const
	{
		return this->m_pRecvBuffer->getReadableSize();
	}

	bool CNetConnecter::setNoDelay(bool bEnable)
	{
		return CNetSocket::setNoDelay(bEnable);
	}

	void CNetConnecter::setConnecterType(ENetConnecterType eConnecterType)
	{
		this->m_eConnecterType = eConnecterType;
	}

	void CNetConnecter::setConnecterState(ENetConnecterState eConnecterState)
	{
		this->m_eConnecterState = eConnecterState;
	}

	bool CNetConnecter::isWriteEvent() const
	{
		return (this->m_nFlag&eNCF_DisableWrite) != 0;
	}

	bool CNetConnecter::isReadEvent() const
	{
		return (this->m_nFlag&eNCF_CloseRecv) == 0;
	}
}