#include "stdafx.h"
#include "net_event_loop.h"
#include "net_accepter.h"
#include "net_connecter.h"

#include "libBaseCommon\logger.h"

namespace base
{
	void CNetAccepter::onEvent(uint32_t nEvent)
	{
		if ((eNET_Recv&nEvent) == 0)
		{
			PrintInfo("event type error socket_id: %d error code[%d]", this->GetSocketID(), getLastError());
			return;
		}

		for (uint32_t i = 0; i < 10; ++i)
		{
			int32_t nSocketID = (int32_t)::accept(this->GetSocketID(), nullptr, nullptr);

			if (_Invalid_SocketID == nSocketID)
			{
				if (getLastError() == NW_EINTR)
					continue;
#ifndef _WIN32
				if (getLastError() == NW_EMFILE)
				{
					// ��ô����Ҫ��Ϊ�˽����fd��������ʱ���ж�������µ����ӻ���
					::close(this->m_nIdleID);
					nSocketID = (int32_t)::accept(this->GetSocketID(), nullptr, nullptr);
					::close(nSocketID);
					nSocketID = _Invalid_SocketID;
					this->m_nIdleID = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
				}
#endif
				if (getLastError() != NW_EWOULDBLOCK
					&& getLastError() != NW_ECONNABORTED
					&& getLastError() != NW_EPROTO)
				{
					PrintWarning("CNetAccepter::onEvent Error: %d", getLastError());
				}
				break;
			}

			if (this->m_pNetEventLoop->getSocketCount() >= this->m_pNetEventLoop->getMaxSocketCount())
			{
				PrintWarning("out of max connection[%d]", this->m_pNetEventLoop->getMaxSocketCount());
				::closesocket(nSocketID);
				break;
			}

			PrintInfo("new connection accept listen addr: %s %d cur connection[%d]", this->getListenAddr().szHost, this->getListenAddr().nPort, this->m_pNetEventLoop->getSocketCount());

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
			pNetConnecter->setConnecterType(eNCT_Passive);
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
		DebugAst(this->GetSocketID() == _Invalid_SocketID);
#ifndef _WIN32
		::close(this->m_nIdleID);
#endif
	}

	bool CNetAccepter::listen(const SNetAddr& netAddr)
	{
		if (!this->open())
			return false;
		
		if (!this->reuseAddr())
		{
			::closesocket(this->GetSocketID());
			return false;
		}
		
		this->reusePort();
		
		if (!this->nonblock())
		{
			::closesocket(this->GetSocketID());
			return false;
		}
		// �����������ӻ�̳л�������С
		if (!this->setBufferSize())
		{
			::closesocket(this->GetSocketID());
			return false;
		}

		this->m_sLocalAddr = netAddr;
		struct sockaddr_in listenAddr;
		listenAddr.sin_family = AF_INET;
		// ������::htons https://bbs.archlinux.org/viewtopic.php?id=53751
		listenAddr.sin_port = htons(this->m_sLocalAddr.nPort);
		listenAddr.sin_addr.s_addr = this->m_sLocalAddr.isAnyIP() ? htonl(INADDR_ANY) : inet_addr(this->m_sLocalAddr.szHost);
		memset(listenAddr.sin_zero, 0, sizeof(listenAddr.sin_zero));
		if (0 != ::bind(this->GetSocketID(), (sockaddr*)&listenAddr, sizeof(listenAddr)))
		{
			PrintWarning("bind socket to %s %d error %d", this->m_sLocalAddr.szHost, this->m_sLocalAddr.nPort, getLastError());
			::closesocket(this->GetSocketID());
			return false;
		}

		if (0 != ::listen(this->GetSocketID(), SOMAXCONN))
		{
			PrintWarning("listen socket to %s %d error %d", this->m_sLocalAddr.szHost, this->m_sLocalAddr.nPort, getLastError());
			::closesocket(this->GetSocketID());
			return false;
		}
		PrintInfo("start listen %s %d socket_id: %d ", this->m_sLocalAddr.szHost, this->m_sLocalAddr.nPort, this->GetSocketID());

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
		this->close(true, false);
	}
}