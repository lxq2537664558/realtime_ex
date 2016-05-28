#pragma once
#include "core_common.h"

namespace core
{
	class CCoreConnection;
	class CCoreConnectionMonitor
	{
	public:
		CCoreConnectionMonitor();
		~CCoreConnectionMonitor();

		void	onRecv(const message_header* pHeader);
		void	onSend(const message_header* pHeader);
		void	onConnect();
		void	onDisconnect();

	private:
		void	onMonitor(uint64_t nContext);

	private:
		uint32_t			m_nSendDataPerSecond;
		uint32_t			m_nRecvDataPerSecond;
		uint32_t			m_nMaxSendDataPerSecond;
		uint32_t			m_nMaxRecvDataPerSecond;
		CTicker				m_tickMonitor;
		CCoreConnection*	m_pCoreConnection;
	};

	void	initPacketMonitor();
	void	uninitPacketMonitor();
}