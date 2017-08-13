#include "stdafx.h"
#include "net_event_loop.h"
#include "net_socket.h"
#include "net_connecter.h"
#include "net_accepter.h"
#include "net_buffer.h"
#include "net_wakeup.h"

#include <thread>

#include "libBaseCommon/logger.h"
#include "libBaseCommon/debug_helper.h"

namespace base
{
	CNetEventLoop::CNetEventLoop()
		: m_nSendConnecterCount(0)
		, m_nSocketCount(0)
		, m_nMaxSocketCount(0)
		, m_pWakeup(nullptr)
#ifndef _WIN32
		, m_nEpoll(0)
		, m_nExtraSocketCount(0)
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

			pSocket->release();
		}
		delete this->m_pWakeup;
		this->m_vecSocket.clear();
		this->m_listCloseSocket.clear();
#ifndef _WIN32
		::close(this->m_nEpoll);
#endif
	}

	bool CNetEventLoop::init(uint32_t nMaxSocketCount)
	{	
		this->m_nSendConnecterCount = 0;
		this->m_nSocketCount = 0;
		this->m_nMaxSocketCount = nMaxSocketCount;
#ifndef _WIN32
		this->m_nEpoll = epoll_create(nMaxSocketCount / 2);
		if (this->m_nEpoll < 0)
			return false;

		// ��fork+execl��رո����̼̳�����fd
		int32_t val = fcntl(this->m_nEpoll, F_GETFD);
		val |= FD_CLOEXEC;
		fcntl(this->m_nEpoll, F_SETFD, val);
		
		this->m_nExtraSocketCount = 1;
#endif

		this->m_pWakeup = new CNetWakeup();
		if (!this->m_pWakeup->init(this))
		{
			delete this->m_pWakeup;
			return false;
		}
		return true;
	}

	bool CNetEventLoop::listen(const SNetAddr& netAddr, bool bReusePort, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, INetAccepterHandler* pHandler)
	{
		CNetAccepter* pNetAccepter = new CNetAccepter();
		if (!pNetAccepter->init(nSendBufferSize, nRecvBufferSize, this))
		{
			delete pNetAccepter;
			return false;
		}
		pNetAccepter->setHandler(pHandler);

		if (!pNetAccepter->listen(netAddr, bReusePort))
		{
			delete pNetAccepter;
			return false;
		}

		return true;
	}

	bool CNetEventLoop::connect(const SNetAddr& netAddr, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, INetConnecterHandler* pHandler)
	{
		CNetConnecter* pNetConnecter = new CNetConnecter();
		if (!pNetConnecter->init(nSendBufferSize, nRecvBufferSize, this))
		{
			delete pNetConnecter;
			return false;
		}
		pNetConnecter->setHandler(pHandler);
		if (!pNetConnecter->connect(netAddr))
		{
			delete pNetConnecter;
			return false;
		}
		return true;
	}

	void CNetEventLoop::release()
	{
		delete this;
	}

	void CNetEventLoop::update(uint32_t nTime)
	{
		for (auto iter = this->m_listCloseSocket.begin(); iter != this->m_listCloseSocket.end(); ++iter)
		{
			CNetSocket* pSocket = *iter;
			if (nullptr == pSocket)
				continue;

			pSocket->release();
		}
		this->m_listCloseSocket.clear();

		// ��д�¼��Ĵ�����Ҫ������������дsocket�����д��д���涼д���ˣ���ʱ�߼����滹�����ݣ���ô�򿪵ײ��д����
		// ��������д�¼����õײ����������ˣ����д�¼������߼���������ݶ��������ˣ���ô���Ƴ����¼�
		for (int32_t i = 0; i < this->m_nSendConnecterCount; ++i)
		{
			CNetConnecter* pNetConnecter = this->m_vecSendConnecter[i];
			if (nullptr == pNetConnecter)
				continue;

			pNetConnecter->flushSend();
			pNetConnecter->setSendConnecterIndex(_Invalid_SendConnecterIndex);
		}
		this->m_nSendConnecterCount = 0;
		// ����ھ��������ѭ����Socket�л���������û�з��ͳ�ȥ����ô������Ĵ����У��ͻᴥ��д�¼���������
		// ����Ĵ��뻹�Ƿ��Ͳ��꣬��ô�ͻ�����һ֡��д�¼�����ʱ����

		if (this->m_nSocketCount == 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(nTime));
			return;
		}

#ifdef _WIN32

		fd_set readfds;
		fd_set writefds;
		fd_set exceptfds;
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_ZERO(&exceptfds);

		// ÿ�ζ�����д�¼����ܻᵼ��busy loop��windows�������ɣ�ֻ��֤��ȷ�ԣ�����֤����
		for (int32_t i = 0; i < this->m_nSocketCount; ++i)
		{
			CNetSocket* pNetSocket = this->m_vecSocket[i];
			if (nullptr == pNetSocket)
				continue;

			FD_SET(pNetSocket->getSocketID(), &writefds);
			FD_SET(pNetSocket->getSocketID(), &readfds);
			FD_SET(pNetSocket->getSocketID(), &exceptfds);
		}

		struct timeval timeout;
		timeout.tv_sec = (int32_t)(((int64_t)nTime * 1000) / 1000000);
		timeout.tv_usec = (int32_t)(((int64_t)nTime * 1000) % 1000000);
		int32_t nRet = ::select(0, &readfds, &writefds, &exceptfds, &timeout);
		if (SOCKET_ERROR == nRet)
		{
			PrintWarning("select error %d ", getLastError());
			return;
		}
		if (0 == nRet)
			return;

		// ���Բ������������ѭ����ȥɾ��Socket�ģ������п��ܻ�����
		for (int32_t i = 0; i < this->m_nSocketCount; ++i)
		{
			CNetSocket* pNetSocket = this->m_vecSocket[i];
			if (nullptr == pNetSocket)
				continue;

			uint32_t nEvent = 0;
			if (FD_ISSET(pNetSocket->getSocketID(), &readfds))
				nEvent |= eNET_Recv;
			if (FD_ISSET(pNetSocket->getSocketID(), &writefds))
				nEvent |= eNET_Send;
			if (FD_ISSET(pNetSocket->getSocketID(), &exceptfds))
				nEvent |= eNET_Error;
			
			if (nEvent != 0)
				pNetSocket->onEvent(nEvent);
		}
#else
		do
		{
			this->m_pWakeup->wait(true);
			int32_t nActiveCount = ::epoll_wait(this->m_nEpoll, &this->m_vecEpollEvent[0], this->m_vecEpollEvent.size(), nTime);
			this->m_pWakeup->wait(false);
			if (nActiveCount >= 0)
			{
				for (int32_t i = 0; i < nActiveCount; ++i)
				{
					INetBase* pNetBase = reinterpret_cast<INetBase*>(this->m_vecEpollEvent[i].data.ptr);
					if (pNetBase != nullptr)
					{
						uint32_t nRawEvent = this->m_vecEpollEvent[i].events;
						uint32_t nEvent = 0;
						if (nRawEvent & (EPOLLHUP | EPOLLERR))
							nEvent = eNET_Error;
						if (nRawEvent & (EPOLLIN | POLLPRI))
							nEvent |= eNET_Recv;
						if (nRawEvent & EPOLLOUT)
							nEvent |= eNET_Send;
						// ���ﲻ����EPOLLRDHUP�¼�
						pNetBase->onEvent(nEvent);
					}
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
	int32_t CNetEventLoop::getEpoll() const
	{
		return this->m_nEpoll;
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

		if (pNetSocket->getSocketID() == _Invalid_SocketID)
			return false;

		if (this->m_nSocketCount >= (int32_t)this->m_vecSocket.size())
			this->m_vecSocket.resize(this->m_nSocketCount + 1);

		pNetSocket->setSocketIndex(this->m_nSocketCount);
		this->m_vecSocket[this->m_nSocketCount++] = pNetSocket;

		PrintInfo("addSocket socket_id %d socket_index %d", pNetSocket->getSocketID(), pNetSocket->getSocketIndex());

#ifndef _WIN32
		this->m_vecEpollEvent.resize(this->m_nSocketCount + this->m_nExtraSocketCount);
		struct epoll_event event;
		memset(&event, 0, sizeof(event));
		event.data.ptr = pNetSocket;
		if (pNetSocket->getSocketType() == CNetSocket::eNST_Connector)
			event.events = EPOLLIN|POLLPRI|EPOLLOUT|EPOLLET;
		else
			event.events = EPOLLIN|POLLPRI|EPOLLOUT;
		if (::epoll_ctl(this->m_nEpoll, EPOLL_CTL_ADD, pNetSocket->getSocketID(), &event) < 0)
			PrintWarning("epoll_ctl error EPOLL_CTL_ADD error %d", getLastError());
#endif
		return true;
	}

	void CNetEventLoop::delSocket(CNetSocket* pNetSocket)
	{
		DebugAst(pNetSocket != nullptr);
		DebugAst(pNetSocket->getSocketIndex() != _Invalid_SocketIndex);
		if (pNetSocket->getSocketID() == _Invalid_SocketID)
			return;

		if (pNetSocket->getSocketIndex() >= this->m_nSocketCount)
			return;

		PrintInfo("delSocket socket_id %d socket_index %d", pNetSocket->getSocketID(), pNetSocket->getSocketIndex());

		this->m_vecSocket[pNetSocket->getSocketIndex()] = this->m_vecSocket[this->m_nSocketCount - 1];
		if (this->m_vecSocket[this->m_nSocketCount - 1] != nullptr)
			this->m_vecSocket[this->m_nSocketCount - 1]->setSocketIndex(pNetSocket->getSocketIndex());
		this->m_vecSocket[this->m_nSocketCount - 1] = nullptr;
		--this->m_nSocketCount;
		pNetSocket->setSocketIndex(_Invalid_SocketIndex);

#ifndef _WIN32
		struct epoll_event event;
		memset(&event, 0, sizeof(event));
		event.data.ptr = pNetSocket;
		event.events = 0;
		if (::epoll_ctl(this->m_nEpoll, EPOLL_CTL_DEL, pNetSocket->getSocketID(), &event) < 0)
			PrintWarning("epoll_ctl error EPOLL_CTL_DEL error %d", getLastError());
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

	void CNetEventLoop::wakeup()
	{
		this->m_pWakeup->wakeup();
	}

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