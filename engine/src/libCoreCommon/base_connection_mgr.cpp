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

	void CBaseConnectionMgr::addConnectCallback(const std::string& szKey, const std::function<void(CBaseConnection*)>& callback)
	{
		CCoreApp::Inst()->getLogicRunnable()->getBaseConnectionMgrImpl()->addConnectCallback(szKey, callback);
	}

	void CBaseConnectionMgr::delConnectCallback(const std::string& szKey)
	{
		CCoreApp::Inst()->getLogicRunnable()->getBaseConnectionMgrImpl()->delConnectCallback(szKey);
	}

	void CBaseConnectionMgr::addDisconnectCallback(const std::string& szKey, const std::function<void(CBaseConnection*)>& callback)
	{
		CCoreApp::Inst()->getLogicRunnable()->getBaseConnectionMgrImpl()->addDisconnectCallback(szKey, callback);
	}

	void CBaseConnectionMgr::delDisconnectCallback(const std::string& szKey)
	{
		CCoreApp::Inst()->getLogicRunnable()->getBaseConnectionMgrImpl()->delDisconnectCallback(szKey);
	}

	void CBaseConnectionMgr::addConnectFailCallback(const std::string& szKey, const std::function<void(const std::string&)>& callback)
	{
		CCoreApp::Inst()->getLogicRunnable()->getBaseConnectionMgrImpl()->addConnectFailCallback(szKey, callback);
	}

	void CBaseConnectionMgr::delConnectFailCallback(const std::string& szKey)
	{
		CCoreApp::Inst()->getLogicRunnable()->getBaseConnectionMgrImpl()->delConnectFailCallback(szKey);
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

	void CBaseConnectionMgr::connect_n(const std::string& szHost, uint16_t nPort, uint32_t nSendBufferSize, uint32_t nRecvBufferSize)
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
		return CCoreApp::Inst()->getLogicRunnable()->getBaseConnectionMgrImpl()->getBaseConnectionCount(nType);
	}

	void CBaseConnectionMgr::enumBaseConnection(uint32_t nType, const std::function<bool(CBaseConnection* pBaseConnection)>& callback) const
	{
		DebugAst(callback != nullptr);

		CCoreApp::Inst()->getLogicRunnable()->getBaseConnectionMgrImpl()->enumBaseConnection(nType, callback);
	}

	CBaseConnection* CBaseConnectionMgr::getBaseConnectionByID(uint64_t nID) const
	{
		return CCoreApp::Inst()->getLogicRunnable()->getBaseConnectionMgrImpl()->getBaseConnectionByID(nID);
	}

	void CBaseConnectionMgr::broadcast(uint32_t nType, uint8_t nMessageType, const void* pData, uint16_t nDataSize, const std::vector<uint64_t>* vecExcludeID)
	{
		char* szBuf = nullptr;
		if (vecExcludeID != nullptr)
			szBuf = new char[sizeof(SMCT_BROADCAST_SOCKET_DATA2) + nDataSize + sizeof(uint64_t)*vecExcludeID->size()];
		else
			szBuf = new char[sizeof(SMCT_BROADCAST_SOCKET_DATA2) + nDataSize];

		SMCT_BROADCAST_SOCKET_DATA2* pContext = reinterpret_cast<SMCT_BROADCAST_SOCKET_DATA2*>(szBuf);
		pContext->nType = nType;
		pContext->nMessageType = nMessageType;
		pContext->nExcludeIDCount = 0;
		if (vecExcludeID != nullptr && !vecExcludeID->empty())
		{
			pContext->nExcludeIDCount = (uint16_t)vecExcludeID->size();
			memcpy(szBuf + sizeof(SMCT_BROADCAST_SOCKET_DATA2), &((*vecExcludeID)[0]), sizeof(uint64_t)*vecExcludeID->size());
		}
		memcpy(szBuf + sizeof(SMCT_BROADCAST_SOCKET_DATA2) + sizeof(uint64_t)*pContext->nExcludeIDCount, pData, nDataSize);

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_BROADCAST_SOCKET_DATA2;
		sMessagePacket.pData = pContext;
		if (vecExcludeID != nullptr)
			sMessagePacket.nDataSize = (uint32_t)(sizeof(SMCT_BROADCAST_SOCKET_DATA2) + nDataSize + sizeof(uint64_t)*vecExcludeID->size());
		else
			sMessagePacket.nDataSize = (uint32_t)(sizeof(SMCT_BROADCAST_SOCKET_DATA2) + nDataSize);

		CCoreApp::Inst()->getNetRunnable()->getMessageQueue()->send(sMessagePacket);
	}

	void CBaseConnectionMgr::broadcast(const std::vector<uint64_t>& vecSocketID, uint8_t nMessageType, const void* pData, uint16_t nDataSize)
	{
		if (vecSocketID.empty())
			return;

		char* szBuf = new char[sizeof(SMCT_BROADCAST_SOCKET_DATA1) + nDataSize + sizeof(uint64_t)*vecSocketID.size()];
		SMCT_BROADCAST_SOCKET_DATA1* pContext = reinterpret_cast<SMCT_BROADCAST_SOCKET_DATA1*>(szBuf);
		pContext->nMessageType = nMessageType;
		pContext->nSocketIDCount = (uint16_t)vecSocketID.size();
		
		memcpy(szBuf + sizeof(SMCT_BROADCAST_SOCKET_DATA1), &vecSocketID[0], sizeof(uint64_t)*vecSocketID.size());
		memcpy(szBuf + sizeof(SMCT_BROADCAST_SOCKET_DATA1) + sizeof(uint64_t)*pContext->nSocketIDCount, pData, nDataSize);
		
		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_BROADCAST_SOCKET_DATA1;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = (uint32_t)(sizeof(SMCT_BROADCAST_SOCKET_DATA1) + nDataSize + sizeof(uint64_t)*vecSocketID.size());

		CCoreApp::Inst()->getNetRunnable()->getMessageQueue()->send(sMessagePacket);
	}

	void CBaseConnectionMgr::setBaseConnectionFactory(uint32_t nType, CBaseConnectionFactory* pBaseConnectionFactory)
	{
		CCoreApp::Inst()->getLogicRunnable()->getBaseConnectionMgrImpl()->setBaseConnectionFactory(nType, pBaseConnectionFactory);
	}

	CBaseConnectionFactory* CBaseConnectionMgr::getBaseConnectionFactory(uint32_t nType) const
	{
		return CCoreApp::Inst()->getLogicRunnable()->getBaseConnectionMgrImpl()->getBaseConnectionFactory(nType);
	}
}