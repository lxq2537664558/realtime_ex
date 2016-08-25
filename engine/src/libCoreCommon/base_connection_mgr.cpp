#include "stdafx.h"
#include "base_connection_mgr.h"
#include "base_connection.h"
#include "base_app.h"
#include "core_app.h"
#include "core_connection_mgr.h"
#include "core_connection.h"
#include "base_connection_factory.h"

#include "libBaseCommon/base_time.h"

namespace core
{
	CBaseConnectionMgr::CBaseConnectionMgr()
	{
	}

	CBaseConnectionMgr::~CBaseConnectionMgr()
	{

	}

	void CBaseConnectionMgr::setConnectCallback(std::function<void(CBaseConnection*)> funConnect)
	{
		this->m_funConnect = funConnect;
	}

	void CBaseConnectionMgr::setDisconnectCallback(std::function<void(CBaseConnection*)> funDisconnect)
	{
		this->m_funDisconnect = funDisconnect;
	}

	void CBaseConnectionMgr::setConnectRefuseCallback(std::function<void(const std::string&)> funConnectRefuse)
	{
		this->m_funConnectRefuse = funConnectRefuse;
	}

	void CBaseConnectionMgr::onConnect(CBaseConnection* pBaseConnection)
	{
		DebugAst(pBaseConnection != nullptr);

		if (this->m_funConnect != nullptr)
			this->m_funConnect(pBaseConnection);
	}

	void CBaseConnectionMgr::onDisconnect(CBaseConnection* pBaseConnection)
	{
		DebugAst(pBaseConnection != nullptr);

		if (this->m_funDisconnect != nullptr)
			this->m_funDisconnect(pBaseConnection);
	}

	void CBaseConnectionMgr::onConnectRefuse(const std::string& szContext)
	{
		if (this->m_funConnectRefuse != nullptr)
			this->m_funConnectRefuse(szContext);
	}

	bool CBaseConnectionMgr::connect(const std::string& szHost, uint16_t nPort, uint32_t nType, const std::string& szContext, uint32_t nSendBufferSize, uint32_t nRecvBufferSize)
	{
		return this->m_pCoreConnectionMgr->connect(szHost, nPort, nType, szContext, nSendBufferSize, nRecvBufferSize);
	}

	bool CBaseConnectionMgr::listen(const std::string& szHost, uint16_t nPort, uint32_t nType, const std::string& szContext, uint32_t nSendBufferSize, uint32_t nRecvBufferSize)
	{
		return this->m_pCoreConnectionMgr->listen(szHost, nPort, nType, szContext, nSendBufferSize, nRecvBufferSize);
	}

	uint32_t CBaseConnectionMgr::getBaseConnectionCount(uint32_t nType) const
	{
		return this->m_pCoreConnectionMgr->getCoreConnectionCount(nType);
	}

	std::vector<CBaseConnection*> CBaseConnectionMgr::getBaseConnection(uint32_t nType) const
	{
		return this->m_pCoreConnectionMgr->getBaseConnection(nType);
	}

	CBaseConnection* CBaseConnectionMgr::getBaseConnectionByID(uint64_t nID) const
	{
		CCoreConnection* pCoreConnection = this->m_pCoreConnectionMgr->getCoreConnectionByID(nID);
		if (nullptr == pCoreConnection)
			return nullptr;

		return pCoreConnection->getBaseConnection();
	}

	void CBaseConnectionMgr::broadcast(uint32_t nType, uint8_t nMessageType, const void* pData, uint16_t nSize, const std::vector<uint64_t>* vecExcludeID)
	{
		this->m_pCoreConnectionMgr->broadcast(nType, nMessageType, pData, nSize, vecExcludeID);
	}

	void CBaseConnectionMgr::broadcast(std::vector<uint64_t>& vecSocketID, uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		for (size_t i = 0; i < vecSocketID.size(); ++i)
		{
			CCoreConnection* pCoreConnection = this->m_pCoreConnectionMgr->getCoreConnectionByID(vecSocketID[i]);
			if (nullptr == pCoreConnection)
				continue;

			pCoreConnection->send(nMessageType, pData, nSize);
		}
	}

	void CBaseConnectionMgr::setBaseConnectionFactory(uint32_t nType, CBaseConnectionFactory* pBaseConnectionFactory)
	{
		if (pBaseConnectionFactory == nullptr)
			this->m_mapBaseConnectionFactory.erase(nType);
		else
			this->m_mapBaseConnectionFactory[nType] = pBaseConnectionFactory;
	}

	CBaseConnectionFactory* CBaseConnectionMgr::getBaseConnectionFactory(uint32_t nType) const
	{
		auto iter = this->m_mapBaseConnectionFactory.find(nType);
		if (iter == this->m_mapBaseConnectionFactory.end())
			return nullptr;

		return iter->second;
	}

}