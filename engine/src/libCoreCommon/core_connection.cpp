#include "stdafx.h"
#include "core_connection.h"
#include "core_connection_mgr.h"
#include "base_connection_mgr.h"
#include "core_common.h"
#include "message_command.h"
#include "logic_runnable.h"
#include "core_app.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/base_time.h"
#include "libBaseCommon/profiling.h"
#include "libBaseCommon/singleton.h"

namespace core
{
	static SNetAddr s_DefaultAddr;
	
#pragma pack(push,1)

	struct connection_heart_beat
	{
		uint8_t	nContext;
	};

#pragma pack(pop)

	CCoreConnection::CCoreConnection()
		: m_bHeartbeat(true)
		, m_nSendHeartbeatCount(0)
		, m_nID(0)
		, m_nState(eCCS_None)
		, m_nSessionID(0)
	{
		this->m_heartbeat.setCallback(std::bind(&CCoreConnection::onHeartbeat, this, std::placeholders::_1));
	}
	
	CCoreConnection::~CCoreConnection()
	{
		DebugAst(this->m_pNetConnecter == nullptr);
	}

	bool CCoreConnection::init(const std::string& szType, uint64_t nID, const std::string& szContext, const MessageParser& messageParser)
	{
		this->m_szType = szType;
		this->m_nID = nID;
		this->m_szContext = szContext;
		this->m_messageParser = messageParser;

		return true;
	}

	uint32_t CCoreConnection::onRecv(const char* pData, uint32_t nDataSize)
	{
		this->m_nSendHeartbeatCount = 0;
		uint32_t nRecvSize = 0;
		try
		{
			do
			{
				uint16_t nMessageSize = 0;

				if (this->m_messageParser != nullptr)
				{
					uint8_t nMessageType = 0;
					int32_t nParserSize = this->m_messageParser(pData, nDataSize, nMessageType);
					if (nParserSize == 0)
						break;

					if (nParserSize <= 0 || nParserSize > UINT16_MAX)
					{
						char szBuf[256] = { 0 };
						base::crt::snprintf(szBuf, _countof(szBuf), "parser message error %d", nParserSize);
						this->shutdown(true, szBuf);
						break;
					}

					nMessageSize = (uint16_t)nParserSize;
					this->onDispatch(nMessageType, pData, nMessageSize);
				}
				else
				{
					// 都不够消息头
					if (nDataSize < sizeof(message_header))
						break;

					const message_header* pHeader = reinterpret_cast<const message_header*>(pData);
					if (pHeader->nMessageSize < sizeof(message_header))
					{
						char szBuf[256] = { 0 };
						base::crt::snprintf(szBuf, _countof(szBuf), "message size error message_type[%d]", pHeader->nMessageSize);
						this->shutdown(true, szBuf);
						break;
					}

					// 不是完整的消息
					if (nDataSize < pHeader->nMessageSize)
						break;

					nMessageSize = pHeader->nMessageSize;

					this->onDispatch((uint8_t)pHeader->nMessageID, pHeader + 1, pHeader->nMessageSize - sizeof(message_header));
				}

				nRecvSize += nMessageSize;
				nDataSize -= nMessageSize;
				pData += nMessageSize;

			} while (true);
		}
		catch (...)
		{
			this->shutdown(true, "cache");
		}
		// 这里不能catch ... 因为一旦这里捕获的如果逻辑代码中有存在不是异常安全的代码 轻则资源泄露，重则服务器没有在第一现场崩溃，等下跑了一段时间崩溃了

		return nRecvSize;
	}

	void CCoreConnection::onSendComplete(uint32_t nSize)
	{

	}

	void CCoreConnection::onConnect()
	{
		PROFILING_GUARD(CCoreConnection::onConnect)

		DebugAst(this->m_pNetConnecter != nullptr);

		SMCT_NOTIFY_SOCKET_CONNECT* pContext = new SMCT_NOTIFY_SOCKET_CONNECT();
		pContext->pCoreConnection = this;
		
		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_NOTIFY_SOCKET_CONNECT;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_NOTIFY_SOCKET_CONNECT);

		CCoreApp::Inst()->getLogicRunnable()->getMessageQueue()->send(sMessagePacket);

		this->m_nState = eCCS_Connectting;
	}

	void CCoreConnection::onDisconnect()
	{
		PROFILING_GUARD(CCoreConnection::onDisconnect)

		SMCT_NOTIFY_SOCKET_DISCONNECT* pContext = new SMCT_NOTIFY_SOCKET_DISCONNECT();
		pContext->nSocketID = this->getID();

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_NOTIFY_SOCKET_DISCONNECT;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_NOTIFY_SOCKET_DISCONNECT);

		CCoreApp::Inst()->getLogicRunnable()->getMessageQueue()->send(sMessagePacket);

		this->m_nState = eCCS_Disconnecting;
		this->m_pNetConnecter = nullptr;

		if (this->m_heartbeat.isRegister())
			CCoreApp::Inst()->unregisterTicker(&this->m_heartbeat);
	}

	void CCoreConnection::onConnectFail()
	{

	}

	void CCoreConnection::onConnectAck()
	{
		if (this->m_nState != eCCS_Connectting)
		{
			PrintWarning("CCoreConnection::onConnectAck error socket_id: "UINT64FMT, this->getID());
			this->shutdown(true, "CCoreConnection::onConnectAck error");
			return;
		}

		this->m_nState = eCCS_Connected;

		CCoreApp::Inst()->registerTicker(CTicker::eTT_Net, 0, 0, &this->m_heartbeat, CCoreApp::Inst()->getHeartbeatTime(), CCoreApp::Inst()->getHeartbeatTime(), 0);
	}

	uint64_t CCoreConnection::getID() const
	{
		return this->m_nID;
	}

	const std::string& CCoreConnection::getType() const
	{
		return this->m_szType;
	}

	const std::string& CCoreConnection::getContext() const
	{
		return this->m_szContext;
	}

	void CCoreConnection::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		if (nMessageType == eMT_HEARTBEAT)
			return;

		CCoreApp::Inst()->incQPS();

		this->m_monitor.onRecv(nSize);

		if (nMessageType == eMT_REQUEST)
		{
			const request_cookice* pCookice = reinterpret_cast<const request_cookice*>(pData);

			DebugAst(nSize > sizeof(request_cookice));
			DebugAst(nSize > sizeof(request_cookice) + pCookice->nMessageNameLen);
			DebugAst(pCookice->szMessageName[pCookice->nMessageNameLen] == 0);

			CCoreService* pCoreService = CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->getCoreServiceByID(pCookice->nToServiceID);
			if (pCoreService == nullptr)
			{
				PrintWarning("CCoreConnection::onDispatch eMT_REQUEST error pCoreService == nullptr service_id: %d", pCookice->nToServiceID);
				return;
			}

			CProtobufFactory* pProtobufFactory = pCoreService->getServiceBase()->getProtobufFactory();
			if (pProtobufFactory == nullptr)
			{
				PrintWarning("CCoreConnection::onDispatch eMT_REQUEST error pProtobufFactory == nullptr service_id: %d", pCookice->nToServiceID);
				return;
			}

			const char* pMessageData = reinterpret_cast<const char*>(pCookice + 1) + pCookice->nMessageNameLen;
			const std::string szMessageName = pCookice->szMessageName;

			google::protobuf::Message* pMessage = pProtobufFactory->unserialize_protobuf_message_from_buf(szMessageName, pMessageData, nSize - sizeof(request_cookice) - pCookice->nMessageNameLen);
			if (nullptr == pMessage)
			{
				PrintWarning("CCoreConnection::onDispatch eMT_REQUEST error pMessage == nullptr service_id: %d message_name: %s", pCookice->nToServiceID, szMessageName.c_str());
				return;
			}

			char* szBuf = new char[sizeof(SMCT_REQUEST)];
			SMCT_REQUEST* pContext = reinterpret_cast<SMCT_REQUEST*>(szBuf);
			pContext->nSessionID = pCookice->nSessionID;
			pContext->nFromActorID = pCookice->nFromActorID;
			pContext->nFromServiceID = pCookice->nFromServiceID;
			pContext->nToActorID = pCookice->nToActorID;
			pContext->nToServiceID = pCookice->nToServiceID;
			pContext->pMessage = pMessage;

			SMessagePacket sMessagePacket;
			sMessagePacket.nType = eMCT_REQUEST;
			sMessagePacket.pData = pContext;
			sMessagePacket.nDataSize = sizeof(SMCT_REQUEST);

			CCoreApp::Inst()->getLogicRunnable()->getMessageQueue()->send(sMessagePacket);
		}
		else if (nMessageType == eMT_RESPONSE)
		{
			const response_cookice* pCookice = reinterpret_cast<const response_cookice*>(pData);

			DebugAst(nSize > sizeof(response_cookice));
			DebugAst(nSize > sizeof(response_cookice) + pCookice->nMessageNameLen);
			DebugAst(pCookice->szMessageName[pCookice->nMessageNameLen] == 0);

			CCoreService* pCoreService = CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->getCoreServiceByID(pCookice->nToServiceID);
			if (pCoreService == nullptr)
			{
				PrintWarning("CCoreConnection::onDispatch eMT_RESPONSE error pCoreService == nullptr service_id: %d", pCookice->nToServiceID);
				return;
			}

			CProtobufFactory* pProtobufFactory = pCoreService->getServiceBase()->getProtobufFactory();
			if (pProtobufFactory == nullptr)
			{
				PrintWarning("CCoreConnection::onDispatch eMT_RESPONSE error pProtobufFactory == nullptr service_id: %d", pCookice->nToServiceID);
				return;
			}

			const char* pMessageData = reinterpret_cast<const char*>(pCookice + 1) + pCookice->nMessageNameLen;
			const std::string szMessageName = pCookice->szMessageName;

			google::protobuf::Message* pMessage = pProtobufFactory->unserialize_protobuf_message_from_buf(szMessageName, pMessageData, nSize - sizeof(response_cookice) - pCookice->nMessageNameLen);
			if (nullptr == pMessage)
			{
				PrintWarning("CCoreConnection::onDispatch eMT_RESPONSE error pMessage == nullptr service_id: %d message_name: %s", pCookice->nToServiceID, szMessageName.c_str());
				return;
			}

			char* szBuf = new char[sizeof(SMCT_RESPONSE)];
			SMCT_RESPONSE* pContext = reinterpret_cast<SMCT_RESPONSE*>(szBuf);
			pContext->nSessionID = pCookice->nSessionID;
			pContext->nToActorID = pCookice->nToActorID;
			pContext->nToServiceID = pCookice->nToServiceID;
			pContext->nResult = pCookice->nResult;
			pContext->pMessage = pMessage;

			SMessagePacket sMessagePacket;
			sMessagePacket.nType = eMCT_RESPONSE;
			sMessagePacket.pData = pContext;
			sMessagePacket.nDataSize = sizeof(SMCT_RESPONSE);

			CCoreApp::Inst()->getLogicRunnable()->getMessageQueue()->send(sMessagePacket);
		}
		else if (nMessageType == eMT_GATE_FORWARD)
		{
			const gate_forward_cookice* pCookice = reinterpret_cast<const gate_forward_cookice*>(pData);

			DebugAst(nSize > sizeof(gate_forward_cookice));

			CCoreService* pCoreService = CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->getCoreServiceByID(pCookice->nToServiceID);
			if (pCoreService == nullptr)
			{
				PrintWarning("CCoreConnection::onDispatch eMT_GATE_FORWARD error pCoreService == nullptr service_id: %d", pCookice->nToServiceID);
				return;
			}

			CProtobufFactory* pProtobufFactory = pCoreService->getServiceBase()->getProtobufFactory();
			if (pProtobufFactory == nullptr)
			{
				PrintWarning("CCoreConnection::onDispatch eMT_GATE_FORWARD error pProtobufFactory == nullptr service_id: %d", pCookice->nToServiceID);
				return;
			}

			const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);
			DebugAst(sizeof(gate_forward_cookice) + pHeader->nMessageSize == nSize);
			const std::string& szMessageName = pCoreService->getForwardMessageName(pHeader->nMessageID);
			if (szMessageName.empty())
			{
				PrintWarning("CCoreConnection::onDispatch eMT_GATE_FORWARD error szMessageName.empty() service_id: %d message_id: %d", pCookice->nToServiceID, pHeader->nMessageID);
				return;
			}

			const char* pMessageData = reinterpret_cast<const char*>(pHeader + 1);
			
			google::protobuf::Message* pMessage = pProtobufFactory->unserialize_protobuf_message_from_buf(szMessageName, pMessageData, nSize - sizeof(gate_forward_cookice)- sizeof(message_header));
			if (nullptr == pMessage)
			{
				PrintWarning("CCoreConnection::onDispatch eMT_GATE_FORWARD error pMessage == nullptr service_id: %d message_name: %s", pCookice->nToServiceID, szMessageName.c_str());
				return;
			}

			char* szBuf = new char[sizeof(SMCT_GATE_FORWARD)];
			SMCT_GATE_FORWARD* pContext = reinterpret_cast<SMCT_GATE_FORWARD*>(szBuf);
			pContext->nSessionID = pCookice->nSessionID;
			pContext->nSocketID = pCookice->nSocketID;
			pContext->nFromServiceID = pCookice->nFromServiceID;
			pContext->nToActorID = pCookice->nToActorID;
			pContext->nToServiceID = pCookice->nToServiceID;
			pContext->pMessage = pMessage;

			SMessagePacket sMessagePacket;
			sMessagePacket.nType = eMCT_GATE_FORWARD;
			sMessagePacket.pData = pContext;
			sMessagePacket.nDataSize = sizeof(SMCT_GATE_FORWARD);

			CCoreApp::Inst()->getLogicRunnable()->getMessageQueue()->send(sMessagePacket);
		}
		else if (nMessageType == eMT_TO_GATE)
		{
			const gate_send_cookice* pCookice = reinterpret_cast<const gate_send_cookice*>(pData);
			CCoreConnection* pCoreConnection = CCoreApp::Inst()->getNetRunnable()->getCoreConnectionMgr()->getCoreConnectionBySocketID(pCookice->nSocketID);
			if (nullptr == pCoreConnection)
				return;

			if (pCoreConnection->getSessionID() != pCookice->nSessionID)
				return;

			const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);
			pCoreConnection->send(eMT_CLIENT, pHeader, pHeader->nMessageSize);
		}
		else if (nMessageType == eMT_TO_GATE_BROADCAST)
		{
			const gate_broadcast_cookice* pCookice = reinterpret_cast<const gate_broadcast_cookice*>(pData);
			
			const message_header* pHeader = reinterpret_cast<const message_header*>(reinterpret_cast<const char*>(pData) + sizeof(gate_broadcast_cookice) + pCookice->nSessionCount * sizeof(std::pair<uint64_t, uint64_t>));

			const std::pair<uint64_t, uint64_t>* pSession = reinterpret_cast<const std::pair<uint64_t, uint64_t>*>(pCookice + 1);

			for (uint16_t i = 0; i < pCookice->nSessionCount; ++i)
			{
				CCoreConnection* pCoreConnection = CCoreApp::Inst()->getNetRunnable()->getCoreConnectionMgr()->getCoreConnectionBySocketID(pSession[i].second);
				if (nullptr == pCoreConnection)
					continue;

				if (pCoreConnection->getSessionID() != pSession[i].first)
					return;

				pCoreConnection->send(eMT_CLIENT, pHeader, pHeader->nMessageSize);
			}
		}
		else
		{
			char* pBuf = new char[sizeof(SMCT_RECV_SOCKET_DATA) + nSize];
			SMCT_RECV_SOCKET_DATA* pContext = reinterpret_cast<SMCT_RECV_SOCKET_DATA*>(pBuf);
			pContext->nSocketID = this->getID();
			pContext->nMessageType = nMessageType;
			pContext->nDataSize = nSize;
			pContext->pData = pBuf + sizeof(SMCT_RECV_SOCKET_DATA);
			memcpy(pContext->pData, pData, nSize);

			SMessagePacket sMessagePacket;
			sMessagePacket.nType = eMCT_RECV_SOCKET_DATA;
			sMessagePacket.pData = pContext;
			sMessagePacket.nDataSize = sizeof(SMCT_RECV_SOCKET_DATA);

			CCoreApp::Inst()->getLogicRunnable()->getMessageQueue()->send(sMessagePacket);
		}
	}

	void CCoreConnection::onHeartbeat(uint64_t nContext)
	{
// 		if (this->m_bHeartbeat.load(std::memory_order_acquire) == 0)
// 			return;
// 		
// 		if (this->m_nSendHeartbeatCount > CCoreApp::Inst()->getHeartbeatLimit())
// 		{
// 			this->shutdown(true, "heart beat time out");
// 			return;
// 		}
// 		
// 		++this->m_nSendHeartbeatCount;
// 		connection_heart_beat netMsg;
// 		this->send(eMT_HEARTBEAT, &netMsg, sizeof(netMsg));
	}

	void CCoreConnection::send(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		PROFILING_GUARD(CCoreConnection::send)

		DebugAst(pData != nullptr && nSize > 0);
		
		if (this->m_pNetConnecter == nullptr)
			return;

		this->m_monitor.onSend(nSize);

		switch (nMessageType)
		{
		case eMT_CLIENT:
			this->m_pNetConnecter->send(pData, nSize, true);
			break;

		case eMT_HEARTBEAT:
		case eMT_REQUEST:
		case eMT_RESPONSE:
		case eMT_SYSTEM:
		case eMT_GATE_FORWARD:
		case eMT_TO_GATE:
			{
				message_header header;
				header.nMessageSize = sizeof(header) + nSize;
				header.nMessageID = nMessageType;
				this->m_pNetConnecter->send(&header, sizeof(header), true);
				this->m_pNetConnecter->send(pData, nSize, true);
			}
			break;
		}
	}

	void CCoreConnection::send(uint8_t nMessageType, const void* pData, uint16_t nSize, const void* pExtraBuf, uint16_t nExtraSize)
	{
		PROFILING_GUARD(CCoreConnection::send)

		DebugAst(pData != nullptr && pExtraBuf != nullptr && nSize + nExtraSize < UINT16_MAX);
		
		if (this->m_pNetConnecter == nullptr)
			return;

		this->m_monitor.onSend(nSize + nExtraSize);

		switch (nMessageType)
		{
		case eMT_CLIENT:
		case eMT_HEARTBEAT:
			{
				this->m_pNetConnecter->send(pData, nSize, true);
				this->m_pNetConnecter->send(pExtraBuf, nExtraSize, true);
			}
			break;

		case eMT_REQUEST:
		case eMT_RESPONSE:
		case eMT_SYSTEM:
		case eMT_GATE_FORWARD:
		case eMT_TO_GATE:
			{
				message_header header;
				header.nMessageSize = sizeof(header) + nSize + nExtraSize;
				header.nMessageID = nMessageType;
				this->m_pNetConnecter->send(&header, sizeof(header), true);
				this->m_pNetConnecter->send(pData, nSize, true);
				this->m_pNetConnecter->send(pExtraBuf, nExtraSize, true);
			}
			break;
		}
	}

	void CCoreConnection::shutdown(bool bForce, const std::string& szMsg)
	{
		if (this->m_pNetConnecter)
			return;

		if (this->m_pNetConnecter != nullptr)
			this->m_pNetConnecter->shutdown(bForce, szMsg.c_str());
	}

	uint32_t CCoreConnection::getSendDataSize() const
	{
		if (nullptr == this->m_pNetConnecter)
			return 0;

		return this->m_pNetConnecter->getSendDataSize();
	}

	uint32_t CCoreConnection::getRecvDataSize() const
	{
		if (nullptr == this->m_pNetConnecter)
			return 0;

		return this->m_pNetConnecter->getRecvDataSize();
	}

	const SNetAddr& CCoreConnection::getLocalAddr() const
	{
		if (this->m_pNetConnecter == nullptr)
			return s_DefaultAddr;
		
		return this->m_pNetConnecter->getLocalAddr();
	}

	const SNetAddr& CCoreConnection::getRemoteAddr() const
	{
		if (this->m_pNetConnecter == nullptr)
			return s_DefaultAddr;

		return this->m_pNetConnecter->getRemoteAddr();
	}

	void CCoreConnection::setMessageParser(MessageParser& parser)
	{
		this->m_messageParser = parser;
	}

	void CCoreConnection::enableHeartbeat(bool bEnable)
	{
		this->m_bHeartbeat.store(bEnable, std::memory_order_release);
	}

	uint32_t CCoreConnection::getState() const
	{
		return this->m_nState;
	}

	void CCoreConnection::setSessionID(uint64_t nSessionID)
	{
		this->m_nSessionID = nSessionID;
	}

	uint64_t CCoreConnection::getSessionID() const
	{
		return this->m_nSessionID;
	}

	base::ENetConnecterMode CCoreConnection::getMode() const
	{
		if (this->m_pNetConnecter == nullptr)
			return base::eNCM_Unknown;

		return this->m_pNetConnecter->getConnecterMode();
	}

}