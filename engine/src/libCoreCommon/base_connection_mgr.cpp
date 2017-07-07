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

	void CBaseConnectionMgr::setConnectCallback(std::function<void(CBaseConnection*)> funConnect)
	{
		this->m_funConnect = funConnect;
	}

	void CBaseConnectionMgr::setDisconnectCallback(std::function<void(CBaseConnection*)> funDisconnect)
	{
		this->m_funDisconnect = funDisconnect;
	}

	void CBaseConnectionMgr::setConnectFailCallback(std::function<void(const std::string&)> funConnectFail)
	{
		this->m_funConnectFail = funConnectFail;
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

		if (this->m_funConnect != nullptr)
			this->m_funConnect(pBaseConnection);

		SMCT_NOTIFY_SOCKET_CONNECT_ACK* pContext = new SMCT_NOTIFY_SOCKET_CONNECT_ACK();

		pContext->nSocketID = nSocketID;

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_NOTIFY_SOCKET_CONNECT_ACK;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_NOTIFY_SOCKET_CONNECT_ACK);

		CNetRunnable::Inst()->getMessageQueue()->send(sMessagePacket);
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

		if (this->m_funDisconnect != nullptr)
			this->m_funDisconnect(pBaseConnection);

		SAFE_RELEASE(pBaseConnection);
	}

	void CBaseConnectionMgr::onConnectFail(const std::string& szContext)
	{
		if (this->m_funConnectFail != nullptr)
			this->m_funConnectFail(szContext);
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
		pContext->mssageParser = messageParser;

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_REQUEST_SOCKET_CONNECT;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_REQUEST_SOCKET_CONNECT);

		CNetRunnable::Inst()->getMessageQueue()->send(sMessagePacket);
	}

	void CBaseConnectionMgr::listen(const std::string& szHost, uint16_t nPort, uint32_t nType, const std::string& szContext, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, MessageParser messageParser)
	{
		SMCT_REQUEST_SOCKET_CONNECT* pContext = new SMCT_REQUEST_SOCKET_CONNECT();
		pContext->szHost = szHost;
		pContext->nPort = nPort;
		pContext->szContext = szContext;
		pContext->nType = nType;
		pContext->nRecvBufferSize = nRecvBufferSize;
		pContext->nSendBufferSize = nSendBufferSize;
		pContext->mssageParser = messageParser;

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_REQUEST_SOCKET_CONNECT;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_REQUEST_SOCKET_CONNECT);

		CNetRunnable::Inst()->getMessageQueue()->send(sMessagePacket);
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
		SMCT_BROADCAST_SOCKET_DATA2* pContext = reinterpret_cast<SMCT_BROADCAST_SOCKET_DATA2*>(new char[sizeof(SMCT_BROADCAST_SOCKET_DATA2) + nSize]);
		pContext->nType = nType;
		pContext->nMessageType = nMessageType;
		pContext->vecExcludeID = nullptr;
		memcpy(pContext + 1, pData, nSize);
		if (vecExcludeID != nullptr)
		{
			pContext->vecExcludeID = new std::vector<uint64_t>();
			*pContext->vecExcludeID = *vecExcludeID;
		}

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_BROADCAST_SOCKET_DATA2;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_BROADCAST_SOCKET_DATA2) + nSize;

		CNetRunnable::Inst()->getMessageQueue()->send(sMessagePacket);
	}

	void CBaseConnectionMgr::broadcast(std::vector<uint64_t>& vecSocketID, uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		if (vecSocketID.empty())
			return;

		SMCT_BROADCAST_SOCKET_DATA1* pContext = reinterpret_cast<SMCT_BROADCAST_SOCKET_DATA1*>(new char[sizeof(SMCT_BROADCAST_SOCKET_DATA1) + nSize]);
		pContext->nMessageType = nMessageType;
		pContext->vecSocketID = new std::vector<uint64_t>();
		*pContext->vecSocketID = vecSocketID;
		memcpy(pContext + 1, pData, nSize);

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_BROADCAST_SOCKET_DATA1;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_BROADCAST_SOCKET_DATA1) + nSize;

		CNetRunnable::Inst()->getMessageQueue()->send(sMessagePacket);
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