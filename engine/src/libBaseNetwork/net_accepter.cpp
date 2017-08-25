#include "stdafx.h"
#include "net_event_loop.h"
#include "net_accepter.h"
#include "net_connecter.h"

#include "libBaseCommon/logger.h"
#include "libBaseCommon/debug_helper.h"

namespace base
{
	void CNetAccepter::onEvent(uint32_t nEvent)
	{
		if ((eNET_Recv&nEvent) == 0)
		{
			PrintInfo("event type error socket_id: {} error code: {}", this->getSocketID(), getLastError());
			return;
		}

		for (uint32_t i = 0; i < 10; ++i)
		{
			int32_t nSocketID = (int32_t)::accept(this->getSocketID(), nullptr, nullptr);

			if (_Invalid_SocketID == nSocketID)
			{
				if (getLastError() == NW_EINTR)
					continue;
#ifndef _WIN32
				if (getLastError() == NW_EMFILE)
				{
					// 这么做主要是为了解决在fd数量不足时，有丢弃这个新的连接机会
					::close(this->m_nIdleID);
					nSocketID = (int32_t)::accept(this->getSocketID(), nullptr, nullptr);
					::close(nSocketID);
					nSocketID = _Invalid_SocketID;
					this->m_nIdleID = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
				}
#endif
				if (getLastError() != NW_EWOULDBLOCK
					&& getLastError() != NW_ECONNABORTED
					&& getLastError() != NW_EPROTO)
				{
					PrintWarning("CNetAccepter::onEvent Error: {}", getLastError());
				}
				break;
			}

			if (this->m_pNetEventLoop->getSocketCount() >= this->m_pNetEventLoop->getMaxSocketCount())
			{
				PrintWarning("out of max connection: {}", this->m_pNetEventLoop->getMaxSocketCount());
				::closesocket(nSocketID);
				break;
			}

			PrintInfo("new connection accept listen addr: {} {} cur connection: {}", this->getListenAddr().szHost, this->getListenAddr().nPort, this->m_pNetEventLoop->getSocketCount());

			CNetConnecter* pNetConnecter = new CNetConnecter();
			if (!pNetConnecter->init(this->getSendBufferSize(), this->getRecvBufferSize(), this->m_pNetEventLoop))
			{
				delete pNetConnecter;
				::closesocket(nSocketID);
				continue;
			}
			pNetConnecter->setSocketID(nSocketID);
			pNetConnecter->setLocalAddr();
			pNetConnecter->setRemoteAddr();
			pNetConnecter->setConnecterMode(eNCM_Passive);
			pNetConnecter->setConnecterState(eNCS_Connecting);
			if (!pNetConnecter->nonblock())
			{
				delete pNetConnecter;
				::closesocket(nSocketID);
				continue;
			}
			INetConnecterHandler* pHandler = this->m_pHandler->onAccept(pNetConnecter);
			if (nullptr == pHandler)
			{
				delete pNetConnecter;
				::closesocket(nSocketID);
				continue;
			}
			if (!this->m_pNetEventLoop->addSocket(pNetConnecter))
			{
				delete pNetConnecter;
				delete pHandler;
				::closesocket(nSocketID);
				continue;
			}
			pNetConnecter->setHandler(pHandler);
		}
	}

	void CNetAccepter::release()
	{
		CNetSocket::release();
		delete this;
	}

	bool CNetAccepter::init(uint32_t nSendBufferSize, uint32_t nRecvBufferSize, CNetEventLoop* pNetEventLoop)
	{
		if (!CNetSocket::init(nSendBufferSize, nRecvBufferSize, pNetEventLoop))
			return false;

#ifndef _WIN32
		this->m_nIdleID = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
		if (this->m_nIdleID == 0)
			return false;
#endif

		return true;
	}

	CNetAccepter::CNetAccepter()
		: m_pHandler(nullptr)
#ifndef _WIN32
		, m_nIdleID(0)
#endif
	{
	}

	CNetAccepter::~CNetAccepter()
	{
		DebugAst(this->getSocketID() == _Invalid_SocketID);
#ifndef _WIN32
		::close(this->m_nIdleID);
#endif
	}

	bool CNetAccepter::listen(const SNetAddr& netAddr, bool bReusePort)
	{
		if (!this->open())
			return false;
		
		if (!this->reuseAddr())
		{
			::closesocket(this->getSocketID());
			return false;
		}
		
		if (bReusePort)
			this->reusePort();
		
		if (!this->nonblock())
		{
			::closesocket(this->getSocketID());
			return false;
		}
		// 监听到的连接会继承缓冲区大小
		if (!this->setBufferSize())
		{
			::closesocket(this->getSocketID());
			return false;
		}

		this->m_sLocalAddr = netAddr;
		struct sockaddr_in listenAddr;
		listenAddr.sin_family = AF_INET;
		listenAddr.sin_port = base::function_util::hton16(this->m_sLocalAddr.nPort);
		listenAddr.sin_addr.s_addr = this->m_sLocalAddr.isAnyIP() ? INADDR_ANY : inet_addr(this->m_sLocalAddr.szHost);
		memset(listenAddr.sin_zero, 0, sizeof(listenAddr.sin_zero));
		if (0 != ::bind(this->getSocketID(), (sockaddr*)&listenAddr, sizeof(listenAddr)))
		{
			PrintWarning("bind socket to {} {} error {}", this->m_sLocalAddr.szHost, this->m_sLocalAddr.nPort, getLastError());
			::closesocket(this->getSocketID());
			return false;
		}

		if (0 != ::listen(this->getSocketID(), SOMAXCONN))
		{
			PrintWarning("listen socket to {} {} error {}", this->m_sLocalAddr.szHost, this->m_sLocalAddr.nPort, getLastError());
			::closesocket(this->getSocketID());
			return false;
		}
		PrintInfo("start listen {} {} socket_id: {} ", this->m_sLocalAddr.szHost, this->m_sLocalAddr.nPort, this->getSocketID());

		this->m_pNetEventLoop->addSocket(this);

		return true;
	}

	void CNetAccepter::setHandler(INetAccepterHandler* pHandler)
	{
		DebugAst(pHandler != nullptr);

		this->m_pHandler = pHandler;
		pHandler->setNetAccepter(this);
	}

	const SNetAddr& CNetAccepter::getListenAddr() const
	{
		return this->m_sLocalAddr;
	}

	void CNetAccepter::shutdown()
	{
		this->close();
	}
}