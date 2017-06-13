#pragma once

#include "network_base.h"

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

#include <list>
#include <vector>

#include "network.h"

namespace base
{

	class CNetSocket;
	class CNetConnecter;
	class CNetSendBufferBlock;
	class CNetWakeup;

	// ��������㣬��Windows�²���selectģ�ͣ���linux�²���epoll��ˮƽ����ģʽ
	class CNetEventLoop :
		public INetEventLoop
	{
	public:
		CNetEventLoop();
		virtual ~CNetEventLoop();

		virtual bool	init(uint32_t nMaxSocketCount);
		virtual bool	listen(const SNetAddr& netAddr, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, INetAccepterHandler* pHandler);
		virtual bool	connect(const SNetAddr& netAddr, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, INetConnecterHandler* pHandler);
		virtual void	update(int64_t nTime);
		virtual void	wakeup();
		virtual void	release();

		void			addCloseSocket(CNetSocket* pNetSocket);
		bool			addSocket(CNetSocket* pNetSocket);
		void			delSocket(CNetSocket* pNetSocket);
		int32_t			getSocketCount() const;
		int32_t			getMaxSocketCount() const;
		void			addSendConnecter(CNetConnecter* pNetConnecter);
		void			delSendConnecter(CNetConnecter* pNetConnecter);
		int32_t			getSendConnecterCount() const;

#ifndef _WIN32
		int32_t			getEpoll() const;
#endif

	private:
		std::vector<CNetSocket*>		m_vecSocket;
		std::list<CNetSocket*>			m_listCloseSocket;
		std::vector<CNetConnecter*>		m_vecSendConnecter;
		int32_t							m_nSocketCount;
		int32_t							m_nSendConnecterCount;

		uint32_t						m_nMaxSocketCount;

		CNetWakeup*						m_pWakeup;

#ifndef _WIN32
		int32_t							m_nEpoll;
		uint32_t						m_nExtraSocketCount;
		std::vector<struct epoll_event>	m_vecEpollEvent;
#endif
	};
}