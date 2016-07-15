#include "stdafx.h"
#include "net_event_loop.h"
#include "net_socket.h"
#include "net_connecter.h"
#include "net_accepter.h"
#include "net_buffer.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/profiling.h"

namespace base
{
	CNetEventLoop::CNetEventLoop()
		: m_nSendConnecterCount(0)
		, m_nSocketCount(0)
		, m_nMaxSocketCount(0)
#ifndef _WIN32
		, m_nEpoll(0)
#endif
	{

	}

	CNetEventLoop::~CNetEventLoop()
	{
		for (int32_t i = (int32_t)(this->m_vecSocket.size() - 1); i >= 0; --i)
		{
			CNetSocket* pSocket = this->m_vecSocket[i];
			if (nullptr == pSocket)
				continue;

			pSocket->forceClose();
		}
		this->m_vecSocket.clear();
		this->m_listCloseSocket.clear();
#ifndef _WIN32
		close(this->m_nEpoll);
#endif
	}

	bool CNetEventLoop::init(uint32_t nMaxSocketCount)
	{
#ifndef _WIN32
		this->m_nEpoll = epoll_create(nMaxSocketCount / 2);
#endif

		this->m_nSendConnecterCount = 0;
		this->m_nSocketCount = 0;
		this->m_nMaxSocketCount = nMaxSocketCount;
		return true;
	}

	bool CNetEventLoop::listen(const SNetAddr& netAddr, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, INetAccepterHandler* pHandler)
	{
		CNetAccepter* pNetAccepter = new CNetAccepter();
		if (!pNetAccepter->init(nSendBufferSize, nRecvBufferSize, this))
		{
			SAFE_DELETE(pNetAccepter);
			return false;
		}
		pNetAccepter->setHandler(pHandler);

		return pNetAccepter->listen(netAddr);
	}

	bool CNetEventLoop::connect(const SNetAddr& netAddr, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, INetConnecterHandler* pHandler)
	{
		CNetConnecter* pNetConnecter = new CNetConnecter();
		if (!pNetConnecter->init(nSendBufferSize, nRecvBufferSize, this))
		{
			SAFE_DELETE(pNetConnecter);
			return false;
		}
		pNetConnecter->setHandler(pHandler);
		if (!pNetConnecter->connect(netAddr))
			return false;

		return true;
	}

	void CNetEventLoop::release()
	{
		delete this;
	}

	void CNetEventLoop::update(int64_t nTime)
	{
		for (auto iter = this->m_listCloseSocket.begin(); iter != this->m_listCloseSocket.end(); ++iter)
		{
			CNetSocket* pSocket = *iter;
			if (nullptr == pSocket)
				continue;

			pSocket->forceClose();
		}
		this->m_listCloseSocket.clear();

		// 对写事件的处理主要采用先主动的写socket，如果写到写缓存都写满了，此时逻辑缓存还有数据，那么打开底层的写监听
		// 接下来的写事件就让底层来触发好了，这个写事件导致逻辑缓存的数据都发送完了，那么就移除该事件
		for (int32_t i = 0; i < this->m_nSendConnecterCount; ++i)
		{
			CNetConnecter* pNetConnecter = this->m_vecSendConnecter[i];
			if (nullptr == pNetConnecter)
				continue;
			pNetConnecter->activeSend();
			pNetConnecter->setSendConnecterIndex(_Invalid_SendConnecterIndex);
		}
		this->m_nSendConnecterCount = 0;
		// 如果在经过上面的循环后Socket中还存在数据没有发送出去，那么在下面的代码中，就会触发写事件继续发送
		// 下面的代码还是发送不完，下一帧中要不在上面的循环里面发送（应用层又发送了数据），要么在下面代码中发送

#ifdef _WIN32
		if (this->m_nSocketCount == 0)
		{
			// 为了让windows洗没有任何socket时不死跑，这里sleep下
			base::sleep((uint32_t)nTime);
			return;
		}

		fd_set readfds;
		fd_set writefds;
		fd_set exceptfds;
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_ZERO(&exceptfds);

		// 每次都监听写事件可能会导致busy loop，windows下随他吧，只保证正确性，不保证性能
		for (int32_t i = 0; i < this->m_nSocketCount; ++i)
		{
			CNetSocket* pNetSocket = this->m_vecSocket[i];
			if (nullptr == pNetSocket)
				continue;

			FD_SET(pNetSocket->GetSocketID(), &exceptfds);

			if (0 != (pNetSocket->getEvent()&eNET_Recv))
				FD_SET(pNetSocket->GetSocketID(), &readfds);
			if (0 != (pNetSocket->getEvent()&eNET_Send))
				FD_SET(pNetSocket->GetSocketID(), &writefds);
		}

		struct timeval timeout;
		timeout.tv_sec = (int32_t)((nTime * 1000) / 1000000);
		timeout.tv_usec = (int32_t)((nTime * 1000) % 1000000);
		PROFILING_BEGIN(select)
		int32_t nRet = select(0, &readfds, &writefds, &exceptfds, &timeout);
		PROFILING_END(select)
		if (SOCKET_ERROR == nRet)
		{
			PrintWarning("select error %d ", getLastError());
			return;
		}
		if (0 == nRet)
			return;

		// 绝对不会在下面这个循环中去删除Socket的
		for (int32_t i = 0; i < this->m_nSocketCount; ++i)
		{
			CNetSocket* pNetSocket = this->m_vecSocket[i];
			if (nullptr == pNetSocket)
				continue;

			uint32_t nEvent = 0;
			if (FD_ISSET(pNetSocket->GetSocketID(), &readfds))
				nEvent |= eNET_Recv;
			if (FD_ISSET(pNetSocket->GetSocketID(), &writefds))
				nEvent |= eNET_Send;
			if (FD_ISSET(pNetSocket->GetSocketID(), &exceptfds))
				nEvent |= eNET_Error;
			
			pNetSocket->onEvent(nEvent);
		}
#else
		do
		{
			PROFILING_BEGIN(epoll_wait)
			int32_t nActiveCount = epoll_wait(this->m_nEpoll, &this->m_vecEpollEvent[0], this->m_vecEpollEvent.size(), nTime);
			PROFILING_END(epoll_wait)
			if (nActiveCount >= 0)
			{
				for (int32_t i = 0; i < nActiveCount; ++i)
				{
					CNetSocket* pNetSocket = static_cast<CNetSocket*>(this->m_vecEpollEvent[i].data.ptr);
					if (pNetSocket != nullptr)
						pNetSocket->onEvent(this->m_vecEpollEvent[i].events);
				}
				break;
			}
			else if (nActiveCount < 0)
			{
				if (getLastError() == NW_EINTR)
					continue;

				PrintWarning("epoll_wait error %d", getLastError());
				break;
			}
		} while(true);
#endif
		return;
	}

#ifndef _WIN32
	void CNetEventLoop::updateEpollOperator(CNetSocket* pNetSocket, int32_t nOperator)
	{
		struct epoll_event event;
		memset(&event, 0, sizeof(event));
		event.data.ptr = pNetSocket;
		event.events = pNetSocket->getEvent();
		if (epoll_ctl(this->m_nEpoll, nOperator, pNetSocket->GetSocketID(), &event) < 0)
			PrintWarning("epoll_ctl error operator = %d error %d", nOperator, getLastError());
	}
#endif

	void CNetEventLoop::addCloseSocket(CNetSocket* pNetSocket)
	{
		DebugAst(pNetSocket != nullptr);

		this->m_listCloseSocket.push_back(pNetSocket);
	}

	bool CNetEventLoop::addSocket(CNetSocket* pNetSocket)
	{
		DebugAstEx(pNetSocket != nullptr, false);
		DebugAstEx(pNetSocket->getSocketIndex() == _Invalid_SocketIndex, false);

		if (pNetSocket->GetSocketID() == _Invalid_SocketID)
			return false;

		if (this->m_nSocketCount >= (int32_t)this->m_vecSocket.size())
			this->m_vecSocket.resize(this->m_nSocketCount + 1);

		pNetSocket->setSocketIndex(this->m_nSocketCount);
		this->m_vecSocket[this->m_nSocketCount++] = pNetSocket;

		PrintNW("addSocket socket_id %d socket_index %d", pNetSocket->GetSocketID(), pNetSocket->getSocketIndex());

#ifndef _WIN32
		this->m_vecEpollEvent.resize(this->m_nSocketCount);
		this->updateEpollOperator(pNetSocket, EPOLL_CTL_ADD);
#endif
		return true;
	}

	void CNetEventLoop::delSocket(CNetSocket* pNetSocket)
	{
		DebugAst(pNetSocket != nullptr);
		DebugAst(pNetSocket->getSocketIndex() != _Invalid_SocketIndex);
		if (pNetSocket->GetSocketID() == _Invalid_SocketID)
			return;

		if (pNetSocket->getSocketIndex() >= this->m_nSocketCount)
			return;

		this->m_vecSocket[pNetSocket->getSocketIndex()] = this->m_vecSocket[this->m_nSocketCount - 1];
		if (this->m_vecSocket[this->m_nSocketCount - 1] != nullptr)
			this->m_vecSocket[this->m_nSocketCount - 1]->setSocketIndex(pNetSocket->getSocketIndex());
		this->m_vecSocket[this->m_nSocketCount - 1] = nullptr;
		--this->m_nSocketCount;
		pNetSocket->setSocketIndex(_Invalid_SocketIndex);

#ifndef _WIN32
		this->m_vecEpollEvent.resize(this->m_nSocketCount);
		this->updateEpollOperator(pNetSocket, EPOLL_CTL_DEL);
#endif
	}

	int32_t CNetEventLoop::getSocketCount() const
	{
		return this->m_nSocketCount;
	}

	int32_t CNetEventLoop::getMaxSocketCount() const
	{
		return this->m_nMaxSocketCount;
	}

	void CNetEventLoop::addSendConnecter(CNetConnecter* pNetConnecter)
	{
		DebugAst(pNetConnecter != nullptr);
		if (_Invalid_SendConnecterIndex != pNetConnecter->getSendConnecterIndex())
			return;

		if (this->m_nSendConnecterCount >= (int32_t)this->m_vecSendConnecter.size())
			this->m_vecSendConnecter.resize(this->m_nSendConnecterCount + 1);
		
		pNetConnecter->setSendConnecterIndex(this->m_nSendConnecterCount);
		this->m_vecSendConnecter[this->m_nSendConnecterCount++] = pNetConnecter;
	}

	void CNetEventLoop::delSendConnecter(CNetConnecter* pNetConnecter)
	{
		DebugAst(pNetConnecter != nullptr);
		if (_Invalid_SendConnecterIndex == pNetConnecter->getSendConnecterIndex())
			return;

		if (pNetConnecter->getSendConnecterIndex() >= this->m_nSendConnecterCount)
			return;

		this->m_vecSendConnecter[pNetConnecter->getSendConnecterIndex()] = this->m_vecSendConnecter[this->m_nSendConnecterCount - 1];
		if (this->m_vecSendConnecter[this->m_nSendConnecterCount - 1] != nullptr)
			this->m_vecSendConnecter[this->m_nSendConnecterCount - 1]->setSendConnecterIndex(pNetConnecter->getSendConnecterIndex());
		this->m_vecSendConnecter[this->m_nSendConnecterCount - 1] = nullptr;
		--this->m_nSendConnecterCount;
	}

	int32_t CNetEventLoop::getSendConnecterCount() const
	{
		return this->m_nSendConnecterCount;
	}


	//=======================================================================

	INetEventLoop* createNetEventLoop()
	{
		CNetEventLoop* pNetFacade = new CNetEventLoop();

		return pNetFacade;
	}

	bool startupNetwork()
	{
#ifdef _WIN32
		WSADATA wsaData;
		uint16_t nVersion = MAKEWORD(2, 0);
		int32_t nRet = WSAStartup(nVersion, &wsaData);
		if (nRet != 0)
		{
			PrintWarning("WSAStartup error %d", getLastError());
			return false;
		}

#endif

		return true;
	}

	void cleanupNetwork()
	{
#ifdef _WIN32
		WSACleanup();
#endif
	}
}