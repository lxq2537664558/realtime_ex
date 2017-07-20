#include "stdafx.h"
#include "base_connection.h"
#include "base_connection_mgr.h"
#include "core_common.h"
#include "message_command.h"
#include "net_runnable.h"
#include "core_app.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/base_time.h"



#define _HEART_BEAT_TIME 5000

namespace core
{
	static SNetAddr s_DefaultNetAddr;

	CBaseConnection::CBaseConnection()
		: m_nID(0)
		, m_nType(0)
	{
	}

	CBaseConnection::~CBaseConnection()
	{
	}

	bool CBaseConnection::init(uint32_t nType, const std::string& szContext)
	{
		this->m_nType = nType;

		return true;
	}

	uint32_t CBaseConnection::getType() const
	{
		return this->m_nType;
	}

	void CBaseConnection::send(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		DebugAst(pData != nullptr);

		char* szBuf = new char[sizeof(SMCT_SEND_SOCKET_DATA) + nSize];
		SMCT_SEND_SOCKET_DATA* pContext = reinterpret_cast<SMCT_SEND_SOCKET_DATA*>(szBuf);
		pContext->nMessageType = nMessageType;
		pContext->nSocketID = this->getID();
		memcpy(szBuf + sizeof(SMCT_SEND_SOCKET_DATA), pData, nSize);

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_SEND_SOCKET_DATA;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_SEND_SOCKET_DATA) + nSize;

		CCoreApp::Inst()->getNetRunnable()->getMessageQueue()->send(sMessagePacket);
	}

	void CBaseConnection::send(uint8_t nMessageType, const void* pData, uint16_t nSize, const void* pExtraBuf, uint16_t nExtraSize)
	{
		DebugAst(pData != nullptr);
		
		if (pExtraBuf == nullptr || nExtraSize == 0)
			return this->send(nMessageType, pData, nSize);

		char* szBuf = new char[sizeof(SMCT_SEND_SOCKET_DATA) + nSize];
		SMCT_SEND_SOCKET_DATA* pContext = reinterpret_cast<SMCT_SEND_SOCKET_DATA*>(szBuf);
		pContext->nMessageType = nMessageType;
		pContext->nSocketID = this->getID();
		memcpy(szBuf + sizeof(SMCT_SEND_SOCKET_DATA), pData, nSize);
		memcpy(szBuf + sizeof(SMCT_SEND_SOCKET_DATA) + nSize, pExtraBuf, nExtraSize);

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_SEND_SOCKET_DATA;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_SEND_SOCKET_DATA) + nSize + nExtraSize;

		CCoreApp::Inst()->getNetRunnable()->getMessageQueue()->send(sMessagePacket);
	}

	void CBaseConnection::shutdown(base::ENetConnecterCloseType eType, const std::string& szMsg)
	{
		SMCT_REQUEST_SOCKET_SHUTDOWN* pContext = new SMCT_REQUEST_SOCKET_SHUTDOWN();
		pContext->nSocketID = this->getID();
		pContext->nType = (uint32_t)eType;
		pContext->szMsg = szMsg;

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_REQUEST_SOCKET_SHUTDOWN;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_REQUEST_SOCKET_SHUTDOWN);

		CCoreApp::Inst()->getNetRunnable()->getMessageQueue()->send(sMessagePacket);
	}

	void CBaseConnection::enableHeartbeat(bool bEnable)
	{
		SMCT_ENABLE_HEARTBEAT* pContext = new SMCT_ENABLE_HEARTBEAT();
		pContext->nSocketID = this->getID();
		pContext->nEnable = bEnable;

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_ENABLE_HEARTBEAT;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_ENABLE_HEARTBEAT);

		CCoreApp::Inst()->getNetRunnable()->getMessageQueue()->send(sMessagePacket);
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

	void CBaseConnection::setMessageParser(MessageParser parser)
	{
		
	}
}