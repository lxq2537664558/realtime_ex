#include "stdafx.h"
#include "base_connection_mgr.h"
#include "base_connection.h"
#include "core_connection_mgr.h"
#include "base_connection_factory.h"
#include "message_command.h"
#include "net_runnable.h"
#include "core_app.h"

#include "libBaseCommon/time_util.h"

namespace core
{
	CBaseConnectionMgr::CBaseConnectionMgr(CLogicMessageQueue* pMessageQueue)
		: m_pBaseConnectionMgrImpl(nullptr)
	{
		this->m_pBaseConnectionMgrImpl = new CBaseConnectionMgrImpl(pMessageQueue);
	}

	CBaseConnectionMgr::~CBaseConnectionMgr()
	{
		SAFE_DELETE(this->m_pBaseConnectionMgrImpl);
	}

	void CBaseConnectionMgr::addConnectCallback(const std::string& szKey, const std::function<void(CBaseConnection*)>& callback)
	{
		this->m_pBaseConnectionMgrImpl->addConnectCallback(szKey, callback);
	}

	void CBaseConnectionMgr::delConnectCallback(const std::string& szKey)
	{
		this->m_pBaseConnectionMgrImpl->delConnectCallback(szKey);
	}

	void CBaseConnectionMgr::addDisconnectCallback(const std::string& szKey, const std::function<void(CBaseConnection*)>& callback)
	{
		this->m_pBaseConnectionMgrImpl->addDisconnectCallback(szKey, callback);
	}

	void CBaseConnectionMgr::delDisconnectCallback(const std::string& szKey)
	{
		this->m_pBaseConnectionMgrImpl->delDisconnectCallback(szKey);
	}

	void CBaseConnectionMgr::addConnectFailCallback(const std::string& szKey, const std::function<void(const std::string&)>& callback)
	{
		this->m_pBaseConnectionMgrImpl->addConnectFailCallback(szKey, callback);
	}

	void CBaseConnectionMgr::delConnectFailCallback(const std::string& szKey)
	{
		this->m_pBaseConnectionMgrImpl->delConnectFailCallback(szKey);
	}

	void CBaseConnectionMgr::connect(const std::string& szHost, uint16_t nPort, const std::string& szType, const std::string& szContext, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, const MessageParser& messageParser)
	{
		SMCT_REQUEST_SOCKET_CONNECT* pContext = new SMCT_REQUEST_SOCKET_CONNECT();
		pContext->szHost = szHost;
		pContext->nPort = nPort;
		pContext->szContext = szContext;
		pContext->szType = szType;
		pContext->pMessageQueue = this->m_pBaseConnectionMgrImpl->getMessageQueue();
		pContext->nRecvBufferSize = nRecvBufferSize;
		pContext->nSendBufferSize = nSendBufferSize;
		pContext->messageParser = messageParser;

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_REQUEST_SOCKET_CONNECT;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_REQUEST_SOCKET_CONNECT);

		CNetRunnable::Inst()->getMessageQueue()->send(sMessagePacket);
	}

	bool CBaseConnectionMgr::listen(const std::string& szHost, uint16_t nPort, bool bReusePort, const std::string& szType, const std::string& szContext, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, MessageParser messageParser, ECoreConnectionType eCoreConnectionType /* = eCCT_Normal */)
	{
		CNetRunnable::Inst()->pause();
		bool bRet = CNetRunnable::Inst()->getCoreConnectionMgr()->listen(this->m_pBaseConnectionMgrImpl->getMessageQueue(), szHost, nPort, bReusePort, szType, szContext, nSendBufferSize, nRecvBufferSize, messageParser, (uint8_t)eCoreConnectionType);
		CNetRunnable::Inst()->resume();

		return bRet;
	}

	void CBaseConnectionMgr::send(uint64_t nSocketID, uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		DebugAst(pData != nullptr);

		char* szBuf = new char[sizeof(SMCT_SEND_SOCKET_DATA2) + nSize];
		SMCT_SEND_SOCKET_DATA2* pContext = reinterpret_cast<SMCT_SEND_SOCKET_DATA2*>(szBuf);
		pContext->nMessageType = nMessageType;
		pContext->nSocketID = nSocketID;
		memcpy(szBuf + sizeof(SMCT_SEND_SOCKET_DATA2), pData, nSize);

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_SEND_SOCKET_DATA2;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_SEND_SOCKET_DATA1) + nSize;

		CNetRunnable::Inst()->getMessageQueue()->send(sMessagePacket);
	}

	void CBaseConnectionMgr::send(uint64_t nSocketID, uint8_t nMessageType, const void* pData, uint16_t nSize, const void* pExtraBuf, uint16_t nExtraSize)
	{
		DebugAst(pData != nullptr);

		if (pExtraBuf == nullptr || nExtraSize == 0)
			return this->send(nSocketID, nMessageType, pData, nSize);

		char* szBuf = new char[sizeof(SMCT_SEND_SOCKET_DATA2) + nSize + nExtraSize];
		SMCT_SEND_SOCKET_DATA2* pContext = reinterpret_cast<SMCT_SEND_SOCKET_DATA2*>(szBuf);
		pContext->nMessageType = nMessageType;
		pContext->nSocketID = nSocketID;
		memcpy(szBuf + sizeof(SMCT_SEND_SOCKET_DATA2), pData, nSize);
		memcpy(szBuf + sizeof(SMCT_SEND_SOCKET_DATA2) + nSize, pExtraBuf, nExtraSize);

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_SEND_SOCKET_DATA2;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_SEND_SOCKET_DATA2) + nSize + nExtraSize;

		CNetRunnable::Inst()->getMessageQueue()->send(sMessagePacket);
	}

	uint32_t CBaseConnectionMgr::getBaseConnectionCount(const std::string& szType) const
	{
		return this->m_pBaseConnectionMgrImpl->getBaseConnectionCount(szType);
	}

	void CBaseConnectionMgr::enumBaseConnection(const std::string& szType, const std::function<bool(CBaseConnection* pBaseConnection)>& callback) const
	{
		DebugAst(callback != nullptr);

		this->m_pBaseConnectionMgrImpl->enumBaseConnection(szType, callback);
	}

	CBaseConnection* CBaseConnectionMgr::getBaseConnectionBySocketID(uint64_t nID) const
	{
		return this->m_pBaseConnectionMgrImpl->getBaseConnectionBySocketID(nID);
	}

	void CBaseConnectionMgr::broadcast(const std::string& szType, uint8_t nMessageType, const void* pData, uint16_t nDataSize, const std::vector<uint64_t>* vecExcludeID)
	{
		char* szBuf = nullptr;
		if (vecExcludeID != nullptr)
			szBuf = new char[sizeof(SMCT_BROADCAST_SOCKET_DATA2) + nDataSize + sizeof(uint64_t)*vecExcludeID->size() + szType.size()];
		else
			szBuf = new char[sizeof(SMCT_BROADCAST_SOCKET_DATA2) + nDataSize + szType.size()];

		SMCT_BROADCAST_SOCKET_DATA2* pContext = reinterpret_cast<SMCT_BROADCAST_SOCKET_DATA2*>(szBuf);
		pContext->nTypeLen = (uint16_t)szType.size();
		base::function_util::strcpy(pContext->szType, szType.size() + 1, szType.c_str());
		pContext->nMessageType = nMessageType;
		pContext->nExcludeIDCount = 0;
		if (vecExcludeID != nullptr && !vecExcludeID->empty())
		{
			pContext->nExcludeIDCount = (uint16_t)vecExcludeID->size();
			memcpy(szBuf + sizeof(SMCT_BROADCAST_SOCKET_DATA2) + szType.size(), &((*vecExcludeID)[0]), sizeof(uint64_t)*vecExcludeID->size());
		}
		memcpy(szBuf + sizeof(SMCT_BROADCAST_SOCKET_DATA2) + szType.size() + sizeof(uint64_t)*pContext->nExcludeIDCount, pData, nDataSize);

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_BROADCAST_SOCKET_DATA2;
		sMessagePacket.pData = pContext;
		if (vecExcludeID != nullptr)
			sMessagePacket.nDataSize = (uint32_t)(sizeof(SMCT_BROADCAST_SOCKET_DATA2) + szType.size() + nDataSize + sizeof(uint64_t)*vecExcludeID->size());
		else
			sMessagePacket.nDataSize = (uint32_t)(sizeof(SMCT_BROADCAST_SOCKET_DATA2) + szType.size() + nDataSize);

		CNetRunnable::Inst()->getMessageQueue()->send(sMessagePacket);
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

		CNetRunnable::Inst()->getMessageQueue()->send(sMessagePacket);
	}

	void CBaseConnectionMgr::setBaseConnectionFactory(const std::string& szType, CBaseConnectionFactory* pBaseConnectionFactory)
	{
		this->m_pBaseConnectionMgrImpl->setBaseConnectionFactory(szType, pBaseConnectionFactory);
	}

	CBaseConnectionFactory* CBaseConnectionMgr::getBaseConnectionFactory(const std::string& szType) const
	{
		return this->m_pBaseConnectionMgrImpl->getBaseConnectionFactory(szType);
	}

	CBaseConnectionMgrImpl* CBaseConnectionMgr::getBaseConnectionMgrImpl() const
	{
		return this->m_pBaseConnectionMgrImpl;
	}
}