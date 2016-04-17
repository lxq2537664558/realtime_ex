#include "stdafx.h"
#include "base_connection_mgr.h"
#include "base_connection.h"
#include "base_app.h"
#include "core_connection_mgr.h"
#include "core_connection.h"

#include "libBaseCommon/base_time.h"

namespace core
{
	CBaseConnectionMgr::CBaseConnectionMgr()
		: m_pCoreConnectionMgr(nullptr)
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

	bool CBaseConnectionMgr::connect(const std::string& szHost, uint16_t nPort, const std::string& szContext, const std::string& szClassName, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, ClientDataCallback clientDataCallback)
	{
		uint32_t nClassID = CBaseObject::getClassID(szClassName);

		return this->m_pCoreConnectionMgr->connect(szHost, nPort, szContext, nClassID, nSendBufferSize, nRecvBufferSize, clientDataCallback);
	}

	bool CBaseConnectionMgr::listen(const std::string& szHost, uint16_t nPort, const std::string& szContext, const std::string& szClassName, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, ClientDataCallback clientDataCallback)
	{
		uint32_t nClassID = CBaseObject::getClassID(szClassName);

		return this->m_pCoreConnectionMgr->listen(szHost, nPort, szContext, nClassID, nSendBufferSize, nRecvBufferSize, clientDataCallback);
	}

	uint32_t CBaseConnectionMgr::getBaseConnectionCount(const std::string& szClassName) const
	{
		uint32_t nClassID = CBaseObject::getClassID(szClassName);
		
		return this->m_pCoreConnectionMgr->getCoreConnectionCount(nClassID);
	}

	CBaseConnection* CBaseConnectionMgr::getBaseConnection(uint64_t nID) const
	{
		CCoreConnection* pCoreConnection = this->m_pCoreConnectionMgr->getCoreConnection(nID);
		if (nullptr == pCoreConnection)
			return nullptr;

		return pCoreConnection->getBaseConnection();
	}

	void CBaseConnectionMgr::getBaseConnection(const std::string& szClassName, std::vector<CBaseConnection*> vecBaseConnection) const
	{
		uint32_t nClassID = CBaseObject::getClassID(szClassName);

		this->m_pCoreConnectionMgr->getBaseConnection(nClassID, vecBaseConnection);
	}

	void CBaseConnectionMgr::broadcast(const std::string& szClassName, uint16_t nMsgType, const void* pData, uint16_t nSize)
	{
		uint32_t nClassID = CBaseObject::getClassID(szClassName);

		this->m_pCoreConnectionMgr->broadcast(nClassID, nMsgType, pData, nSize);
	}
}