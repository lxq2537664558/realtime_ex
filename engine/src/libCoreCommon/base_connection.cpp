#include "stdafx.h"
#include "base_connection.h"
#include "core_connection.h"
#include "base_connection_mgr.h"
#include "message_command.h"
#include "net_runnable.h"
#include "core_app.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/time_util.h"

#define _HEART_BEAT_TIME 5000

namespace
{
	SNetAddr s_DefaultNetAddr;
}

namespace core
{
	CBaseConnection::CBaseConnection()
		: m_pCoreConnection(nullptr)
		, m_bClosing(false)
	{
	}

	CBaseConnection::~CBaseConnection()
	{
	}

	const std::string& CBaseConnection::getType() const
	{
		return this->m_pCoreConnection->getType();
	}

	uint64_t CBaseConnection::getID() const
	{
		return this->m_pCoreConnection->getID();
	}

	const SNetAddr& CBaseConnection::getLocalAddr() const
	{
		return this->m_pCoreConnection->getLocalAddr();
	}

	const SNetAddr& CBaseConnection::getRemoteAddr() const
	{
		return this->m_pCoreConnection->getRemoteAddr();
	}

	void CBaseConnection::setMessageParser(MessageParser parser)
	{

	}

	void CBaseConnection::send(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		DebugAst(pData != nullptr);

		char* szBuf = new char[sizeof(SMCT_SEND_SOCKET_DATA1) + nSize];
		SMCT_SEND_SOCKET_DATA1* pContext = reinterpret_cast<SMCT_SEND_SOCKET_DATA1*>(szBuf);
		pContext->nMessageType = nMessageType;
		pContext->pCoreConnection = this->m_pCoreConnection;
		memcpy(szBuf + sizeof(SMCT_SEND_SOCKET_DATA1), pData, nSize);

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_SEND_SOCKET_DATA1;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_SEND_SOCKET_DATA1) + nSize;

		CNetRunnable::Inst()->getMessageQueue()->send(sMessagePacket);
	}

	void CBaseConnection::send(uint8_t nMessageType, const void* pData, uint16_t nSize, const void* pExtraBuf, uint16_t nExtraSize)
	{
		DebugAst(pData != nullptr);
		
		if (pExtraBuf == nullptr || nExtraSize == 0)
			return this->send(nMessageType, pData, nSize);

		char* szBuf = new char[sizeof(SMCT_SEND_SOCKET_DATA1) + nSize + nExtraSize];
		SMCT_SEND_SOCKET_DATA1* pContext = reinterpret_cast<SMCT_SEND_SOCKET_DATA1*>(szBuf);
		pContext->nMessageType = nMessageType;
		pContext->pCoreConnection = this->m_pCoreConnection;
		memcpy(szBuf + sizeof(SMCT_SEND_SOCKET_DATA1), pData, nSize);
		memcpy(szBuf + sizeof(SMCT_SEND_SOCKET_DATA1) + nSize, pExtraBuf, nExtraSize);

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_SEND_SOCKET_DATA1;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_SEND_SOCKET_DATA1) + nSize + nExtraSize;

		CNetRunnable::Inst()->getMessageQueue()->send(sMessagePacket);
	}

	void CBaseConnection::shutdown(bool bForce, const std::string& szMsg)
	{
		if (this->m_bClosing)
			return;

		this->m_bClosing = true;

		SMCT_REQUEST_SOCKET_SHUTDOWN* pContext = new SMCT_REQUEST_SOCKET_SHUTDOWN();
		pContext->nSocketID = this->getID();
		pContext->nForce = bForce;
		pContext->szMsg = szMsg;

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_REQUEST_SOCKET_SHUTDOWN;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_REQUEST_SOCKET_SHUTDOWN);

		CNetRunnable::Inst()->getMessageQueue()->send(sMessagePacket);
	}

	void CBaseConnection::enableHeartbeat(bool bEnable)
	{
		this->m_pCoreConnection->enableHeartbeat(bEnable);
	}

	const std::string& CBaseConnection::getContext() const
	{
		return this->m_pCoreConnection->getContext();
	}

	base::net::ENetConnecterMode CBaseConnection::getMode() const
	{
		return this->m_pCoreConnection->getMode();
	}

	bool CBaseConnection::isClosing() const
	{
		return this->m_bClosing;
	}
}