#include "stdafx.h"
#include "core_connection_monitor.h"
#include "core_app.h"

#include <map>

#define _CONNECTION_MONITOR_LOG "ConnectionMonitor"

struct SPacketMonitor
{
	uint32_t	nSendPacketCountPerSecond;
	uint32_t	nRecvPacketCountPerSecond;
	uint32_t	nSendPacketSizePerSecond;
	uint32_t	nRecvPacketSizePerSecond;
	uint32_t	nMaxSendPacketSizePerSecond;
	uint32_t	nMaxRecvPacketSizePerSecond;
};

class CPacketMonitorMgr
{
public:
	CPacketMonitorMgr();
	~CPacketMonitorMgr();

	void			init();
	void			uninit();
	SPacketMonitor&	getPacketMonitor(uint32_t nMessageID);

private:
	void			onMonitor(uint64_t nContext);
	
private:
	std::map<uint32_t, SPacketMonitor>	m_mapPacketMonitor;
	core::CTicker						m_tickMonitor;
};

CPacketMonitorMgr::CPacketMonitorMgr()
{
	this->m_tickMonitor.setCallback(std::bind(&CPacketMonitorMgr::onMonitor, this, std::placeholders::_1));
}

CPacketMonitorMgr::~CPacketMonitorMgr()
{

}

void CPacketMonitorMgr::init()
{
	if (!this->m_tickMonitor.isRegister())
		core::CCoreApp::Inst()->registerTicker(&this->m_tickMonitor, 1000, 1000, 0);
}

void CPacketMonitorMgr::uninit()
{
	if (this->m_tickMonitor.isRegister())
		core::CCoreApp::Inst()->unregisterTicker(&this->m_tickMonitor);
}

void CPacketMonitorMgr::onMonitor(uint64_t nContext)
{
	for (auto iter = this->m_mapPacketMonitor.begin(); iter != this->m_mapPacketMonitor.end(); ++iter)
	{
		uint32_t nMessageID = iter->first;
		SPacketMonitor& sPacketMonitor = iter->second;
		base::saveLogEx(_CONNECTION_MONITOR_LOG, false, "message_id: %d send_packet_count_ps: %d recv_packet_count_ps: %d send_packet_size_ps: %d recv_packet_size_ps: %d max_send_packet_size_ps: %d max_recv_packet_size_ps: %d",
			nMessageID, sPacketMonitor.nSendPacketCountPerSecond, sPacketMonitor.nRecvPacketCountPerSecond, sPacketMonitor.nSendPacketSizePerSecond, sPacketMonitor.nRecvPacketSizePerSecond, sPacketMonitor.nMaxSendPacketSizePerSecond, sPacketMonitor.nMaxRecvPacketSizePerSecond);
		
		sPacketMonitor.nSendPacketCountPerSecond = 0;
		sPacketMonitor.nRecvPacketCountPerSecond = 0;
		sPacketMonitor.nSendPacketSizePerSecond = 0;
		sPacketMonitor.nRecvPacketSizePerSecond = 0;
	}
}

SPacketMonitor& CPacketMonitorMgr::getPacketMonitor(uint32_t nMessageID)
{
	return this->m_mapPacketMonitor[nMessageID];
}

static CPacketMonitorMgr	s_packetMonitorMgr;

namespace core
{
	CCoreConnectionMonitor::CCoreConnectionMonitor()
		: m_pCoreConnection(nullptr)
		, m_nSendDataPerSecond(0)
		, m_nRecvDataPerSecond(0)
		, m_nMaxSendDataPerSecond(0)
		, m_nMaxRecvDataPerSecond(0)
	{
		this->m_tickMonitor.setCallback(std::bind(&CCoreConnectionMonitor::onMonitor, this, std::placeholders::_1));
	}

	CCoreConnectionMonitor::~CCoreConnectionMonitor()
	{

	}

	void CCoreConnectionMonitor::onMonitor(uint64_t nContext)
	{
		DebugAst(this->m_pCoreConnection != nullptr);

		const SNetAddr& localAddr = this->m_pCoreConnection->getLocalAddr();
		const SNetAddr& remoteAddr = this->m_pCoreConnection->getRemoteAddr();
		base::saveLogEx(_CONNECTION_MONITOR_LOG, false, "local addr %s %d remote addr %s %d send_data_ps: %d recv_data_ps: %d max_send_data_ps: %d max_recv_data_ps: %d",
			localAddr.szHost, localAddr.nPort, remoteAddr.szHost, remoteAddr.nPort, this->m_nSendDataPerSecond, this->m_nRecvDataPerSecond, this->m_nMaxSendDataPerSecond, this->m_nMaxRecvDataPerSecond);
	
		this->m_nRecvDataPerSecond = 0;
		this->m_nSendDataPerSecond = 0;
	}

	void CCoreConnectionMonitor::onRecv(const message_header* pHeader)
	{
		DebugAst(pHeader != nullptr);

		this->m_nRecvDataPerSecond += pHeader->nMessageSize;
		if (this->m_nRecvDataPerSecond > (this->m_nMaxRecvDataPerSecond + pHeader->nMessageSize))
			this->m_nMaxRecvDataPerSecond = this->m_nRecvDataPerSecond;

		SPacketMonitor& sPacketMonitor = s_packetMonitorMgr.getPacketMonitor(pHeader->nMessageID);
		++sPacketMonitor.nRecvPacketCountPerSecond;
		sPacketMonitor.nRecvPacketSizePerSecond += pHeader->nMessageSize;
		if (sPacketMonitor.nRecvPacketSizePerSecond > (sPacketMonitor.nMaxRecvPacketSizePerSecond + pHeader->nMessageSize))
			sPacketMonitor.nMaxRecvPacketSizePerSecond = sPacketMonitor.nRecvPacketSizePerSecond;
	}

	void CCoreConnectionMonitor::onSend(const message_header* pHeader)
	{
		DebugAst(pHeader != nullptr);

		this->m_nSendDataPerSecond += pHeader->nMessageSize;
		if (this->m_nSendDataPerSecond > (this->m_nMaxSendDataPerSecond + pHeader->nMessageSize))
			this->m_nMaxSendDataPerSecond = this->m_nSendDataPerSecond;

		SPacketMonitor& sPacketMonitor = s_packetMonitorMgr.getPacketMonitor(pHeader->nMessageID);
		++sPacketMonitor.nSendPacketCountPerSecond;
		sPacketMonitor.nSendPacketSizePerSecond += pHeader->nMessageSize;
		if (sPacketMonitor.nSendPacketSizePerSecond > (sPacketMonitor.nMaxSendPacketSizePerSecond + pHeader->nMessageSize))
			sPacketMonitor.nMaxSendPacketSizePerSecond = sPacketMonitor.nSendPacketSizePerSecond;
	}

	void CCoreConnectionMonitor::onConnect()
	{
		if (!this->m_tickMonitor.isRegister())
			CCoreApp::Inst()->registerTicker(&this->m_tickMonitor, 1000, 1000, 0);
	}

	void CCoreConnectionMonitor::onDisconnect()
	{
		if (this->m_tickMonitor.isRegister())
			CCoreApp::Inst()->unregisterTicker(&this->m_tickMonitor);
	}
}