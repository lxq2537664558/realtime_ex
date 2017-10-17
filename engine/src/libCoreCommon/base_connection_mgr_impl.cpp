#include "stdafx.h"
#include "base_connection_mgr_impl.h"
#include "base_connection.h"
#include "core_app.h"
#include "message_command.h"
#include "net_runnable.h"
#include "core_connection.h"

#include "libBaseCommon/time_util.h"

namespace core
{
	CBaseConnectionMgrImpl::CBaseConnectionMgrImpl(CLogicMessageQueue* pMessageQueue)
		: m_pMessageQueue(pMessageQueue)
	{
	}

	CBaseConnectionMgrImpl::~CBaseConnectionMgrImpl()
	{

	}

	CLogicMessageQueue* CBaseConnectionMgrImpl::getMessageQueue() const
	{
		return this->m_pMessageQueue;
	}

	void CBaseConnectionMgrImpl::addConnectCallback(const std::string& szKey, const std::function<void(CBaseConnection*)>& callback)
	{
		DebugAst(callback != nullptr);

		if (this->m_mapConnectCalback.find(szKey) != this->m_mapConnectCalback.end())
		{
			PrintWarning("dup connect callback key: {}", szKey);
		}

		this->m_mapConnectCalback[szKey] = callback;
	}

	void CBaseConnectionMgrImpl::delConnectCallback(const std::string& szKey)
	{
		this->m_mapConnectCalback.erase(szKey);
	}

	void CBaseConnectionMgrImpl::addDisconnectCallback(const std::string& szKey, const std::function<void(CBaseConnection*)>& callback)
	{
		DebugAst(callback != nullptr);

		if (this->m_mapDisconnectCallback.find(szKey) != this->m_mapDisconnectCallback.end())
		{
			PrintWarning("dup disconnect callback key: {}", szKey);
		}

		this->m_mapDisconnectCallback[szKey] = callback;
	}

	void CBaseConnectionMgrImpl::delDisconnectCallback(const std::string& szKey)
	{
		this->m_mapDisconnectCallback.erase(szKey);
	}

	void CBaseConnectionMgrImpl::addConnectFailCallback(const std::string& szKey, const std::function<void(const std::string&)>& callback)
	{
		DebugAst(callback != nullptr);
		
		if (this->m_mapConnectFailCallback.find(szKey) != this->m_mapConnectFailCallback.end())
		{
			PrintWarning("dup connect fail callback key: {}", szKey);
		}

		this->m_mapConnectFailCallback[szKey] = callback;
	}

	void CBaseConnectionMgrImpl::delConnectFailCallback(const std::string& szKey)
	{
		this->m_mapConnectFailCallback.erase(szKey);
	}

	bool CBaseConnectionMgrImpl::onConnect(CCoreConnection* pCoreConnection)
	{
		DebugAstEx(pCoreConnection != nullptr, false);
		CBaseConnectionFactory* pBaseConnectionFactory = this->getBaseConnectionFactory(pCoreConnection->getType());
		if (nullptr == pBaseConnectionFactory)
		{
			PrintWarning("can't find base connection factory type: {} context: {}", pCoreConnection->getType(), pCoreConnection->getContext());
			return false;
		}

		CBaseConnection* pBaseConnection = pBaseConnectionFactory->createBaseConnection(pCoreConnection->getType());
		if (nullptr == pBaseConnection)
		{
			PrintWarning("create base connection error type: {} context: {}", pCoreConnection->getType(), pCoreConnection->getContext());
			return false;
		}
		
		pBaseConnection->m_pCoreConnection = pCoreConnection;

		this->m_mapBaseConnectionByID[pCoreConnection->getID()] = pBaseConnection;
		this->m_mapBaseConnectionByType[pCoreConnection->getType()][pCoreConnection->getID()] = pBaseConnection;

		// 这里必须要在回调前发送连接应答，不然回调的时候如果有发送消息的话就会出错
		SMCT_NOTIFY_SOCKET_CONNECT_ACK* pContext = new SMCT_NOTIFY_SOCKET_CONNECT_ACK();
		pContext->pCoreConnection = pCoreConnection;
		pContext->bSuccess = true;

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_NOTIFY_SOCKET_CONNECT_ACK;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_NOTIFY_SOCKET_CONNECT_ACK);

		CNetRunnable::Inst()->getMessageQueue()->send(sMessagePacket);

		pBaseConnection->onConnect();

		for (auto iter = this->m_mapConnectCalback.begin(); iter != this->m_mapConnectCalback.end(); ++iter)
		{
			auto& callback = iter->second;
			if (callback != nullptr)
				callback(pBaseConnection);
		}

		return true;
	}

	void CBaseConnectionMgrImpl::onDisconnect(uint64_t nSocketID)
	{
		auto iter = this->m_mapBaseConnectionByID.find(nSocketID);
		if (iter == this->m_mapBaseConnectionByID.end())
			return;

		CBaseConnection* pBaseConnection = iter->second;
		this->m_mapBaseConnectionByID.erase(iter);
		if (pBaseConnection == nullptr)
		{
			PrintWarning("CBaseConnectionMgr::onDisconnect error socket_id: {}", nSocketID);
			return;
		}

		pBaseConnection->onDisconnect();

		this->m_mapBaseConnectionByType[pBaseConnection->getType()].erase(nSocketID);

		for (auto iter = this->m_mapDisconnectCallback.begin(); iter != this->m_mapDisconnectCallback.end(); ++iter)
		{
			auto& callback = iter->second;
			if (callback != nullptr)
				callback(pBaseConnection);
		}

		SAFE_RELEASE(pBaseConnection);
	}

	void CBaseConnectionMgrImpl::onConnectFail(const std::string& szContext)
	{
		for (auto iter = this->m_mapConnectFailCallback.begin(); iter != this->m_mapConnectFailCallback.end(); ++iter)
		{
			auto& callback = iter->second;
			if (callback != nullptr)
				callback(szContext.c_str());
		}
	}

	uint32_t CBaseConnectionMgrImpl::getBaseConnectionCount(const std::string& szType) const
	{
		auto iter = this->m_mapBaseConnectionByType.find(szType);
		if (iter == this->m_mapBaseConnectionByType.end())
			return 0;

		return (uint32_t)iter->second.size();
	}

	void CBaseConnectionMgrImpl::enumBaseConnection(const std::string& szType, const std::function<bool(CBaseConnection* pBaseConnection)>& callback) const
	{
		DebugAst(callback != nullptr);

		auto iter = this->m_mapBaseConnectionByType.find(szType);
		if (iter == this->m_mapBaseConnectionByType.end())
			return;

		auto& mapBaseConnection = iter->second;
		for (auto iter = mapBaseConnection.begin(); iter != mapBaseConnection.end(); ++iter)
		{
			CBaseConnection* pBaseConnection = iter->second;
			IF_NOT(nullptr != pBaseConnection)
				continue;

			if (!callback(pBaseConnection))
				break;
		}
	}

	CBaseConnection* CBaseConnectionMgrImpl::getBaseConnectionBySocketID(uint64_t nID) const
	{
		auto iter = this->m_mapBaseConnectionByID.find(nID);
		if (iter == this->m_mapBaseConnectionByID.end())
			return nullptr;

		return iter->second;
	}

	void CBaseConnectionMgrImpl::setBaseConnectionFactory(const std::string& szType, CBaseConnectionFactory* pBaseConnectionFactory)
	{
		if (pBaseConnectionFactory == nullptr)
			this->m_mapBaseConnectionFactory.erase(szType);
		else
			this->m_mapBaseConnectionFactory[szType] = pBaseConnectionFactory;
	}

	CBaseConnectionFactory* CBaseConnectionMgrImpl::getBaseConnectionFactory(const std::string& szType) const
	{
		auto iter = this->m_mapBaseConnectionFactory.find(szType);
		if (iter == this->m_mapBaseConnectionFactory.end())
			return nullptr;

		return iter->second;
	}
}