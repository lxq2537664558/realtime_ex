#include "stdafx.h"
#include "base_connection_mgr.h"
#include "base_connection.h"
#include "base_app.h"
#include "core_app.h"
#include "base_connection_factory.h"
#include "message_command.h"
#include "net_runnable.h"

#include "libBaseCommon/base_time.h"

namespace core
{
	CBaseConnectionMgr::CBaseConnectionMgr()
	{
	}

	CBaseConnectionMgr::~CBaseConnectionMgr()
	{

	}

	void CBaseConnectionMgr::addConnectCallback(const std::string& szKey, std::function<void(CBaseConnection*)> callback)
	{
		if (this->m_mapConnectCalback.find(szKey) != this->m_mapConnectCalback.end())
		{
			PrintWarning("dup connect callback key: %s", szKey.c_str());
		}

		this->m_mapConnectCalback[szKey] = callback;
	}

	void CBaseConnectionMgr::delConnectCallback(const std::string& szKey)
	{
		this->m_mapConnectCalback.erase(szKey);
	}

	void CBaseConnectionMgr::addDisconnectCallback(const std::string& szKey, std::function<void(CBaseConnection*)> callback)
	{
		if (this->m_mapDisconnectCallback.find(szKey) != this->m_mapDisconnectCallback.end())
		{
			PrintWarning("dup disconnect callback key: %s", szKey.c_str());
		}

		this->m_mapDisconnectCallback[szKey] = callback;
	}

	void CBaseConnectionMgr::delDisconnectCallback(const std::string& szKey)
	{
		this->m_mapDisconnectCallback.erase(szKey);
	}

	void CBaseConnectionMgr::addConnectFailCallback(const std::string& szKey, std::function<void(const std::string&)> callback)
	{
		if (this->m_mapConnectFailCallback.find(szKey) != this->m_mapConnectFailCallback.end())
		{
			PrintWarning("dup connect fail callback key: %s", szKey.c_str());
		}

		this->m_mapConnectFailCallback[szKey] = callback;
	}

	void CBaseConnectionMgr::delConnectFailCallback(const std::string& szKey)
	{
		this->m_mapConnectFailCallback.erase(szKey);
	}

	void CBaseConnectionMgr::onConnect(uint64_t nSocketID, const std::string& szContext, uint32_t nType, const SNetAddr& sLocalAddr, const SNetAddr& sRemoteAddr)
	{
		CBaseConnectionFactory* pBaseConnectionFactory = this->getBaseConnectionFactory(nType);
		if (nullptr == pBaseConnectionFactory)
		{
			PrintWarning("can't find base connection factory type: %d context: %s", nType, szContext.c_str());
			return;
		}
		CBaseConnection* pBaseConnection = pBaseConnectionFactory->createBaseConnection(nType, szContext);
		if (nullptr == pBaseConnection)
		{
			PrintWarning("create base connection error type: %d context: %s", nType, szContext.c_str());
			return;
		}

		pBaseConnection->m_nID = nSocketID;
		pBaseConnection->m_sLocalAddr = sLocalAddr;
		pBaseConnection->m_sRemoteAddr = sRemoteAddr;

		this->m_mapBaseConnectionByID[nSocketID] = pBaseConnection;
		this->m_mapBaseConnectionByType[nType][nSocketID] = pBaseConnection;

		pBaseConnection->onConnect();

		for (auto iter = this->m_mapConnectCalback.begin(); iter != this->m_mapConnectCalback.end(); ++iter)
		{
			auto& callback = iter->second;
			if (callback != nullptr)
				callback(pBaseConnection);
		}

		SMCT_NOTIFY_SOCKET_CONNECT_ACK* pContext = new SMCT_NOTIFY_SOCKET_CONNECT_ACK();

		pContext->nSocketID = nSocketID;

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_NOTIFY_SOCKET_CONNECT_ACK;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_NOTIFY_SOCKET_CONNECT_ACK);

		CCoreApp::Inst()->getNetRunnable()->getMessageQueue()->send(sMessagePacket);
	}

	void CBaseConnectionMgr::onDisconnect(uint64_t nSocketID)
	{
		auto iter = this->m_mapBaseConnectionByID.find(nSocketID);
		if (iter == this->m_mapBaseConnectionByID.end())
			return;

		CBaseConnection* pBaseConnection = iter->second;
		if (pBaseConnection == nullptr)
		{
			PrintWarning("CBaseConnectionMgr::onDisconnect error socket_id: "UINT64FMT, nSocketID);
			this->m_mapBaseConnectionByID.erase(iter);
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

	void CBaseConnectionMgr::onConnectFail(const std::string& szContext)
	{
		for (auto iter = this->m_mapConnectFailCallback.begin(); iter != this->m_mapConnectFailCallback.end(); ++iter)
		{
			auto& callback = iter->second;
			if (callback != nullptr)
				callback(szContext);
		}
	}

	void CBaseConnectionMgr::connect(const std::string& szHost, uint16_t nPort, uint32_t nType, const std::string& szContext, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, const MessageParser& messageParser)
	{
		SMCT_REQUEST_SOCKET_CONNECT* pContext = new SMCT_REQUEST_SOCKET_CONNECT();
		pContext->szHost = szHost;
		pContext->nPort = nPort;
		pContext->szContext = szContext;
		pContext->nType = nType;
		pContext->nRecvBufferSize = nRecvBufferSize;
		pContext->nSendBufferSize = nSendBufferSize;
		pContext->messageParser = messageParser;

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_REQUEST_SOCKET_CONNECT;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_REQUEST_SOCKET_CONNECT);

		CCoreApp::Inst()->getNetRunnable()->getMessageQueue()->send(sMessagePacket);
	}

	void CBaseConnectionMgr::connect_s(const std::string& szHost, uint16_t nPort, uint32_t nSendBufferSize, uint32_t nRecvBufferSize)
	{
		this->connect(szHost, nPort, eBCT_ConnectionToOtherNode, "", nSendBufferSize, nRecvBufferSize, nullptr);
	}

	void CBaseConnectionMgr::listen(const std::string& szHost, uint16_t nPort, uint32_t nType, const std::string& szContext, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, MessageParser messageParser)
	{
		SMCT_REQUEST_SOCKET_LISTEN* pContext = new SMCT_REQUEST_SOCKET_LISTEN();
		pContext->szHost = szHost;
		pContext->nPort = nPort;
		pContext->szContext = szContext;
		pContext->nType = nType;
		pContext->nRecvBufferSize = nRecvBufferSize;
		pContext->nSendBufferSize = nSendBufferSize;
		pContext->messageParser = messageParser;

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_REQUEST_SOCKET_LISTEN;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_REQUEST_SOCKET_LISTEN);

		CCoreApp::Inst()->getNetRunnable()->getMessageQueue()->send(sMessagePacket);
	}

	uint32_t CBaseConnectionMgr::getBaseConnectionCount(uint32_t nType) const
	{
		auto iter = this->m_mapBaseConnectionByType.find(nType);
		if (iter == this->m_mapBaseConnectionByType.end())
			return 0;

		return (uint32_t)iter->second.size();
	}

	std::vector<CBaseConnection*> CBaseConnectionMgr::getBaseConnection(uint32_t nType) const
	{
		std::vector<CBaseConnection*> vecBaseConnection;
		auto iter = this->m_mapBaseConnectionByType.find(nType);
		if (iter == this->m_mapBaseConnectionByType.end())
			return vecBaseConnection;

		auto& mapBaseConnection = iter->second;
		vecBaseConnection.reserve(mapBaseConnection.size());

		for (auto iter = mapBaseConnection.begin(); iter != mapBaseConnection.end(); ++iter)
		{
			CBaseConnection* pBaseConnection = iter->second;
			IF_NOT(nullptr != pBaseConnection)
				continue;

			vecBaseConnection.push_back(pBaseConnection);
		}

		return vecBaseConnection;
	}

	CBaseConnection* CBaseConnectionMgr::getBaseConnectionByID(uint64_t nID) const
	{
		auto iter = this->m_mapBaseConnectionByID.find(nID);
		if (iter == this->m_mapBaseConnectionByID.end())
			return nullptr;

		return iter->second;
	}

	void CBaseConnectionMgr::broadcast(uint32_t nType, uint8_t nMessageType, const void* pData, uint16_t nSize, const std::vector<uint64_t>* vecExcludeID)
	{
		DebugAst(vecExcludeID == nullptr || vecExcludeID->size() <= UINT16_MAX);

		char* szBuf = nullptr;
		if (vecExcludeID != nullptr)
			szBuf = new char[sizeof(SMCT_BROADCAST_SOCKET_DATA2) + nSize + sizeof(uint64_t)*vecExcludeID->size()];
		else
			szBuf = new char[sizeof(SMCT_BROADCAST_SOCKET_DATA2) + nSize];

		SMCT_BROADCAST_SOCKET_DATA2* pContext = reinterpret_cast<SMCT_BROADCAST_SOCKET_DATA2*>(szBuf);
		pContext->nType = nType;
		pContext->nMessageType = nMessageType;
		pContext->nExcludeIDCount = 0;
		if (vecExcludeID != nullptr && !vecExcludeID->empty())
		{
			pContext->nExcludeIDCount = (uint16_t)vecExcludeID->size();
			memcpy(szBuf + sizeof(SMCT_BROADCAST_SOCKET_DATA2), &((*vecExcludeID)[0]), sizeof(uint64_t)*vecExcludeID->size());
		}
		memcpy(szBuf + sizeof(SMCT_BROADCAST_SOCKET_DATA2) + sizeof(uint64_t)*pContext->nExcludeIDCount, pData, nSize);

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_BROADCAST_SOCKET_DATA2;
		sMessagePacket.pData = pContext;
		if (vecExcludeID != nullptr)
			sMessagePacket.nDataSize = (uint32_t)(sizeof(SMCT_BROADCAST_SOCKET_DATA2)+nSize + sizeof(uint64_t)*vecExcludeID->size());
		else
			sMessagePacket.nDataSize = (uint32_t)(sizeof(SMCT_BROADCAST_SOCKET_DATA2)+nSize);
		CCoreApp::Inst()->getNetRunnable()->getMessageQueue()->send(sMessagePacket);
	}

	void CBaseConnectionMgr::broadcast(std::vector<uint64_t>& vecSocketID, uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		DebugAst(vecSocketID.size() <= UINT16_MAX);

		char* szBuf = new char[sizeof(SMCT_BROADCAST_SOCKET_DATA1) + nSize + sizeof(uint64_t)*vecSocketID.size()];
		SMCT_BROADCAST_SOCKET_DATA1* pContext = reinterpret_cast<SMCT_BROADCAST_SOCKET_DATA1*>(szBuf);
		pContext->nMessageType = nMessageType;
		pContext->nSocketIDCount = (uint16_t)vecSocketID.size();
		if (!vecSocketID.empty())
		{
			memcpy(szBuf + sizeof(SMCT_BROADCAST_SOCKET_DATA1), &vecSocketID[0], sizeof(uint64_t)*vecSocketID.size());
		}
		memcpy(szBuf + sizeof(SMCT_BROADCAST_SOCKET_DATA1) + sizeof(uint64_t)*pContext->nSocketIDCount, pData, nSize);
		
		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_BROADCAST_SOCKET_DATA1;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = (uint32_t)(sizeof(SMCT_BROADCAST_SOCKET_DATA1) + nSize + sizeof(uint64_t)*vecSocketID.size());

		CCoreApp::Inst()->getNetRunnable()->getMessageQueue()->send(sMessagePacket);
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