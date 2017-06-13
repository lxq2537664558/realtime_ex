#include "stdafx.h"
#include "base_connection.h"
#include "base_connection_mgr.h"
#include "core_connection.h"
#include "base_app.h"
#include "core_common.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/base_time.h"

#define _HEART_BEAT_TIME 5000

namespace core
{
	static SNetAddr s_DefaultNetAddr;

	CBaseConnection::CBaseConnection()
		: m_pCoreConnection(nullptr)
	{
	}

	CBaseConnection::~CBaseConnection()
	{
	}

	void CBaseConnection::send(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		DebugAst(pData != nullptr);
		DebugAst(this->m_pCoreConnection != nullptr);

		this->m_pCoreConnection->send(nMessageType, pData, nSize);
	}

	void CBaseConnection::send(uint8_t nMessageType, const void* pData, uint16_t nSize, const void* pExtraBuf, uint16_t nExtraSize)
	{
		DebugAst(pData != nullptr);
		DebugAst(this->m_pCoreConnection != nullptr);

		if (pExtraBuf == nullptr || nExtraSize == 0)
			return this->send(nMessageType, pData, nSize);

		return this->m_pCoreConnection->send(nMessageType, pData, nSize, pExtraBuf, nExtraSize);
	}

	void CBaseConnection::shutdown(base::ENetConnecterCloseType eType, const std::string& szMsg)
	{
		DebugAst(this->m_pCoreConnection != nullptr);

		this->m_pCoreConnection->shutdown(eType, szMsg);
	}

	void CBaseConnection::enableHeartbeat(bool bEnable)
	{
		DebugAst(this->m_pCoreConnection != nullptr);

		this->m_pCoreConnection->enableHeartbeat(bEnable);
	}

	uint64_t CBaseConnection::getID() const
	{
		DebugAstEx(this->m_pCoreConnection != nullptr, 0);

		return this->m_pCoreConnection->getID();
	}

	const SNetAddr& CBaseConnection::getLocalAddr() const
	{
		DebugAstEx(this->m_pCoreConnection != nullptr, s_DefaultNetAddr);

		return this->m_pCoreConnection->getLocalAddr();
	}

	const SNetAddr& CBaseConnection::getRemoteAddr() const
	{
		DebugAstEx(this->m_pCoreConnection != nullptr, s_DefaultNetAddr);

		return this->m_pCoreConnection->getRemoteAddr();
	}

	void CBaseConnection::setMessageParser(MessageParser parser)
	{
		DebugAst(this->m_pCoreConnection != nullptr);

		this->m_pCoreConnection->setMessageParser(parser);
	}
}