#include "stdafx.h"
#include "base_connection.h"
#include "base_connection_mgr.h"
#include "core_connection.h"
#include "base_app.h"
#include "core_common.h"
#include "message_command.h"
#include "net_message_queue.h"
#include "net_runnable.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/base_time.h"

#define _HEART_BEAT_TIME 5000

namespace core
{
	CBaseConnection::CBaseConnection()
		: m_nID(0)
		, m_bHeartbeat(false)
	{
		this->m_heartbeat.setCallback(std::bind(&CBaseConnection::onHeartbeat, this, std::placeholders::_1));
	}

	CBaseConnection::~CBaseConnection()
	{
	}

	void CBaseConnection::send(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		DebugAst(pData != nullptr);

		SMCT_SEND_SOCKET_DATA* pContext = reinterpret_cast<SMCT_SEND_SOCKET_DATA*>(new char[sizeof(SMCT_SEND_SOCKET_DATA)+nSize]);
		pContext->nMessageType = nMessageType;
		pContext->nSocketID = this->getID();
		memcpy(pContext + 1, pData, nSize);

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_SEND_SOCKET_DATA;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_SEND_SOCKET_DATA)+nSize;

		CNetRunnable::Inst()->getMessageQueue()->send(sMessagePacket);
	}

	void CBaseConnection::send(uint8_t nMessageType, const void* pData, uint16_t nSize, const void* pExtraBuf, uint16_t nExtraSize)
	{
		DebugAst(pData != nullptr);

		if (pExtraBuf == nullptr || nExtraSize == 0)
			return this->send(nMessageType, pData, nSize);

		SMCT_SEND_SOCKET_DATA* pContext = reinterpret_cast<SMCT_SEND_SOCKET_DATA*>(new char[sizeof(SMCT_SEND_SOCKET_DATA) + nSize + nExtraSize]);
		pContext->nMessageType = nMessageType;
		pContext->nSocketID = this->getID();
		memcpy(pContext + 1, pData, nSize);
		memcpy(reinterpret_cast<char*>(pContext + 1) + nSize, pExtraBuf, nExtraSize);

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_SEND_SOCKET_DATA;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_SEND_SOCKET_DATA)+nSize + nExtraSize;

		CNetRunnable::Inst()->getMessageQueue()->send(sMessagePacket);
	}

	void CBaseConnection::shutdown(bool bForce, const std::string& szMsg)
	{
		SMCT_REQUEST_SOCKET_SHUTDOWN* pContext = new SMCT_REQUEST_SOCKET_SHUTDOWN();
		pContext->nSocketID = this->getID();
		pContext->bForce = bForce;
		pContext->szMsg = szMsg;

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_REQUEST_SOCKET_SHUTDOWN;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_REQUEST_SOCKET_SHUTDOWN);

		CNetRunnable::Inst()->getMessageQueue()->send(sMessagePacket);
	}

	void CBaseConnection::enableHeartbeat(bool bEnable)
	{
		this->m_bHeartbeat = bEnable;

		if (this->m_heartbeat.isRegister())
			CBaseApp::Inst()->unregisterTicker(&this->m_heartbeat);

		if (this->m_bHeartbeat)
			CBaseApp::Inst()->registerTicker(&this->m_heartbeat, _HEART_BEAT_TIME, _HEART_BEAT_TIME, 0);

		SMCT_ENABLE_HEARTBEAT* pContext = new SMCT_ENABLE_HEARTBEAT();
		pContext->nSocketID = this->getID();
		pContext->nEnable = bEnable;

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_ENABLE_HEARTBEAT;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_ENABLE_HEARTBEAT);

		CNetRunnable::Inst()->getMessageQueue()->send(sMessagePacket);
	}

	uint64_t CBaseConnection::getID() const
	{
		return this->m_nID;
	}

	const SNetAddr& CBaseConnection::getLocalAddr() const
	{
		return this->m_sLocalAddr;
	}

	const SNetAddr& CBaseConnection::getRemoteAddr() const
	{
		return this->m_sRemoteAddr;
	}

	void CBaseConnection::onHeartbeat(uint64_t nContext)
	{
		if (this->m_bHeartbeat)
			return;

		SMCT_SEND_HEARTBEAT* pContext = new SMCT_SEND_HEARTBEAT();
		pContext->nSocketID = this->getID();
		
		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_SEND_HEARTBEAT;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_SEND_HEARTBEAT);

		CNetRunnable::Inst()->getMessageQueue()->send(sMessagePacket);
	}
}