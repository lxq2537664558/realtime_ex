#include "stdafx.h"
#include "base_connection.h"
#include "base_connection_mgr.h"
#include "core_connection.h"
#include "base_app.h"
#include "core_common.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/base_time.h"


namespace core
{
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

	void CBaseConnection::shutdown(bool bForce, const std::string& szMsg)
	{
		DebugAst(this->m_pCoreConnection != nullptr);

		this->m_pCoreConnection->shutdown(bForce, szMsg);
	}

	uint64_t CBaseConnection::getID() const
	{
		if (nullptr == this->m_pCoreConnection)
			return 0;

		return this->m_pCoreConnection->getID();
	}

	const SNetAddr& CBaseConnection::getLocalAddr() const
	{
		if (nullptr == this->m_pCoreConnection)
		{
			static SNetAddr s_Default;
			return s_Default;
		}

		return this->m_pCoreConnection->getLocalAddr();
	}

	const SNetAddr& CBaseConnection::getRemoteAddr() const
	{
		if (nullptr == this->m_pCoreConnection)
		{
			static SNetAddr s_Default;
			return s_Default;
		}

		return this->m_pCoreConnection->getRemoteAddr();
	}
}