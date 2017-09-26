#include "stdafx.h"
#include "core_connection_monitor.h"
#include "core_connection.h"
#include "core_app.h"

#define _CONNECTION_MONITOR_LOG "ConnectionMonitor"

namespace core
{
	CCoreConnectionMonitor::CCoreConnectionMonitor()
		: m_pCoreConnection(nullptr)
		, m_nSendDataPerSecond(0)
		, m_nRecvDataPerSecond(0)
		, m_nMaxSendDataPerSecond(0)
		, m_nMaxRecvDataPerSecond(0)
	{
		this->m_tickerMonitor.setCallback(std::bind(&CCoreConnectionMonitor::onMonitor, this, std::placeholders::_1));
	}

	CCoreConnectionMonitor::~CCoreConnectionMonitor()
	{

	}

	void CCoreConnectionMonitor::onMonitor(uint64_t nContext)
	{
		DebugAst(this->m_pCoreConnection != nullptr);

		const SNetAddr& localAddr = this->m_pCoreConnection->getLocalAddr();
		const SNetAddr& remoteAddr = this->m_pCoreConnection->getRemoteAddr();
		base::log::saveEx(_CONNECTION_MONITOR_LOG, false, "local addr %s %d remote addr %s %d send_data_ps: %d recv_data_ps: %d max_send_data_ps: %d max_recv_data_ps: %d",
			localAddr.szHost, localAddr.nPort, remoteAddr.szHost, remoteAddr.nPort, this->m_nSendDataPerSecond, this->m_nRecvDataPerSecond, this->m_nMaxSendDataPerSecond, this->m_nMaxRecvDataPerSecond);
	
		this->m_nRecvDataPerSecond = 0;
		this->m_nSendDataPerSecond = 0;
	}

	void CCoreConnectionMonitor::onRecv(uint32_t nSize)
	{
		this->m_nRecvDataPerSecond += nSize;
		if (this->m_nRecvDataPerSecond > (this->m_nMaxRecvDataPerSecond + nSize))
			this->m_nMaxRecvDataPerSecond = this->m_nRecvDataPerSecond;
	}

	void CCoreConnectionMonitor::onSend(uint32_t nSize)
	{
		this->m_nSendDataPerSecond += nSize;
		if (this->m_nSendDataPerSecond > (this->m_nMaxSendDataPerSecond + nSize))
			this->m_nMaxSendDataPerSecond = this->m_nSendDataPerSecond;
	}

	void CCoreConnectionMonitor::onConnect()
	{
		if (this->m_tickerMonitor.isRegister())
			CCoreApp::Inst()->unregisterTicker(&this->m_tickerMonitor);

		CCoreApp::Inst()->registerTicker(CTicker::eTT_Service, 0, 0, &this->m_tickerMonitor, 60000, 60000, 0);
	}

	void CCoreConnectionMonitor::onDisconnect()
	{
		if (this->m_tickerMonitor.isRegister())
			CCoreApp::Inst()->unregisterTicker(&this->m_tickerMonitor);
	}
}