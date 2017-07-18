#include "stdafx.h"
#include "core_connection.h"
#include "core_connection_mgr.h"
#include "base_connection_mgr.h"
#include "core_common.h"
#include "message_command.h"
#include "logic_runnable.h"
#include "protobuf_helper.h"
#include "core_app.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/base_time.h"
#include "libBaseCommon/profiling.h"


namespace core
{
	
#pragma pack(push,1)

	struct connection_heart_beat
	{
		uint8_t	nContext;
	};

	// 消息头
	struct core_message_header
	{
		uint16_t	nMessageSize;	// 包括消息头的
		uint8_t		nMessageType;
	};

#pragma pack(pop)

	CCoreConnection::CCoreConnection()
		: m_bHeartbeat(false)
		, m_nSendHeartbeatCount(0)
		, m_nID(0)
		, m_nType(0)
		, m_nState(eCCS_None)
	{
	}
	
	CCoreConnection::~CCoreConnection()
	{
		DebugAst(this->m_pNetConnecter == nullptr);
	}

	bool CCoreConnection::init(uint32_t nType, uint64_t nID, const std::string& szContext, const MessageParser& messageParser)
	{
		this->m_nType = nType;
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
						this->shutdown(base::eNCCT_Force, szBuf);
						break;
					}

					nMessageSize = (uint16_t)nParserSize;
					this->onDispatch(nMessageType, pData, nMessageSize);
				}
				else
				{
					// 都不够消息头
					if (nDataSize < sizeof(core_message_header))
						break;

					const core_message_header* pHeader = reinterpret_cast<const core_message_header*>(pData);
					if (pHeader->nMessageSize < sizeof(message_header))
					{
						char szBuf[256] = { 0 };
						base::crt::snprintf(szBuf, _countof(szBuf), "message size error message_type[%d]", pHeader->nMessageSize);
						this->shutdown(base::eNCCT_Force, szBuf);
						break;
					}

					// 不是完整的消息
					if (nDataSize < pHeader->nMessageSize)
						break;

					nMessageSize = pHeader->nMessageSize;

					this->onDispatch(pHeader->nMessageType, pHeader + 1, pHeader->nMessageSize - sizeof(core_message_header));
				}

				nRecvSize += nMessageSize;
				nDataSize -= nMessageSize;
				pData += nMessageSize;

			} while (true);
		}
		catch (base::CBaseException& exp)
		{
			char szBuf[1024] = { 0 };
			base::crt::snprintf(szBuf, _countof(szBuf), "dispatch data error %s", exp.getInfo());
			this->shutdown(base::eNCCT_Force, szBuf);
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
		pContext->szContext = this->m_szContext;
		pContext->nType = this->getType();
		pContext->sLocalAddr = this->m_pNetConnecter->getLocalAddr();
		pContext->sRemoteAddr = this->m_pNetConnecter->getRemoteAddr();
		pContext->nSocketID = this->getID();

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

		this->m_nState = eCCS_Disconnectting;
		this->m_pNetConnecter = nullptr;
	}

	void CCoreConnection::onConnectFail()
	{

	}

	uint64_t CCoreConnection::getID() const
	{
		return this->m_nID;
	}

	uint32_t CCoreConnection::getType() const
	{
		return this->m_nType;
	}

	void CCoreConnection::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		if (nMessageType == eMT_HEARTBEAT)
			return;

		CCoreApp::Inst()->incQPS();

		this->m_monitor.onRecv(nSize);

		SMCT_RECV_SOCKET_DATA* pContext = nullptr;
		if (nMessageType == eMT_REQUEST)
		{
			const request_cookice* pCookice = reinterpret_cast<const request_cookice*>(pData);

			DebugAst(nSize > sizeof(request_cookice));
			DebugAst(nSize > sizeof(request_cookice) + pCookice->nMessageNameLen);
			DebugAst(pCookice->szMessageName[pCookice->nMessageNameLen] == 0);

			const char* pMessageData = reinterpret_cast<const char*>(pCookice + 1) + pCookice->nMessageNameLen;
			const std::string szMessageName = pCookice->szMessageName;

			google::protobuf::Message* pMessage = create_protobuf_message(szMessageName);
			if (nullptr == pMessage || !pMessage->ParseFromArray(pMessageData, nSize - sizeof(request_cookice) - pCookice->nMessageNameLen))
			{
				SAFE_DELETE(pMessage);
				return;
			}

			pContext = new SMCT_RECV_SOCKET_DATA();
			pContext->nSocketID = this->getID();
			pContext->nSessionID = pCookice->nSessionID;
			pContext->nData = pCookice->nFromID;
			pContext->nToActorID = pCookice->nToActorID;
			pContext->nToServiceID = pCookice->nToServiceID;
			pContext->nTargetType = pCookice->nTargetType;
			pContext->nMessageType = eMT_REQUEST;
			pContext->nDataSize = nSize;
			pContext->pData = pMessage;
		}
		else if (nMessageType == eMT_RESPONSE)
		{
			const response_cookice* pCookice = reinterpret_cast<const response_cookice*>(pData);

			DebugAst(nSize > sizeof(response_cookice));
			DebugAst(nSize > sizeof(response_cookice) + pCookice->nMessageNameLen);
			DebugAst(pCookice->szMessageName[pCookice->nMessageNameLen] == 0);

			const char* pMessageData = reinterpret_cast<const char*>(pCookice + 1) + pCookice->nMessageNameLen;
			const std::string szMessageName = pCookice->szMessageName;

			google::protobuf::Message* pMessage = create_protobuf_message(szMessageName);
			if (nullptr == pMessage || !pMessage->ParseFromArray(pMessageData, nSize - sizeof(request_cookice) - pCookice->nMessageNameLen))
			{
				SAFE_DELETE(pMessage);
				return;
			}

			pContext = new SMCT_RECV_SOCKET_DATA();
			pContext->nSocketID = this->getID();
			pContext->nSessionID = pCookice->nSessionID;
			pContext->nData = pCookice->nResult;
			pContext->nToActorID = pCookice->nToActorID;
			pContext->nToServiceID = pCookice->nToServiceID;
			pContext->nTargetType = pCookice->nTargetType;
			pContext->nMessageType = eMT_RESPONSE;
			pContext->nDataSize = nSize;
			pContext->pData = pMessage;
		}
		else if (nMessageType == eMT_GATE_FORWARD)
		{
			const gate_forward_cookice* pCookice = reinterpret_cast<const gate_forward_cookice*>(pData);

			DebugAst(nSize > sizeof(gate_forward_cookice));
			DebugAst(nSize > sizeof(gate_forward_cookice) + pCookice->nMessageNameLen);
			DebugAst(pCookice->szMessageName[pCookice->nMessageNameLen] == 0);

			const char* pMessageData = reinterpret_cast<const char*>(pCookice + 1) + pCookice->nMessageNameLen;
			const std::string szMessageName = pCookice->szMessageName;

			google::protobuf::Message* pMessage = create_protobuf_message(szMessageName);
			if (nullptr == pMessage || !pMessage->ParseFromArray(pMessageData, nSize - sizeof(request_cookice) - pCookice->nMessageNameLen))
			{
				SAFE_DELETE(pMessage);
				return;
			}

			pContext = new SMCT_RECV_SOCKET_DATA();
			pContext->nSocketID = this->getID();
			pContext->nSessionID = pCookice->nSessionID;
			pContext->nData = pCookice->nFromID;
			pContext->nToActorID = pCookice->nToActorID;
			pContext->nToServiceID = pCookice->nToServiceID;
			pContext->nTargetType = pCookice->nTargetType;
			pContext->nMessageType = eMT_GATE_FORWARD;
			pContext->nDataSize = nSize;
			pContext->pData = pMessage;
		}
		else
		{
			char* pBuf = new char[sizeof(SMCT_RECV_SOCKET_DATA) + nSize];
			pContext = reinterpret_cast<SMCT_RECV_SOCKET_DATA*>(pBuf);
			pContext->nSocketID = this->getID();
			pContext->nSessionID = 0;
			pContext->nData = 0;
			pContext->nToActorID = 0;
			pContext->nToServiceID = 0;
			pContext->nTargetType = 0;
			pContext->nMessageType = nMessageType;
			pContext->nDataSize = nSize;
			pContext->pData = pBuf + sizeof(SMCT_RECV_SOCKET_DATA);
			memcpy(pContext->pData, pData, nSize);
		}

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_RECV_SOCKET_DATA;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_RECV_SOCKET_DATA);

		CCoreApp::Inst()->getLogicRunnable()->getMessageQueue()->send(sMessagePacket);
	}

	void CCoreConnection::onHeartbeat(uint64_t nContext)
	{
		if (this->m_bHeartbeat)
			return;
		
		if (this->m_nSendHeartbeatCount > CCoreApp::Inst()->getHeartbeatLimit())
		{
			this->shutdown(base::eNCCT_Force, "heart beat time out");
			return;
		}
		
		++this->m_nSendHeartbeatCount;
		connection_heart_beat netMsg;
		this->send(eMT_HEARTBEAT, &netMsg, sizeof(netMsg));
	}

	void CCoreConnection::send(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		PROFILING_GUARD(CCoreConnection::send)

		DebugAst(this->m_pNetConnecter != nullptr);
		DebugAst(pData != nullptr && nSize > 0);

		this->m_monitor.onSend(nSize);

		switch (nMessageType)
		{
		case eMT_CLIENT:
		case eMT_HEARTBEAT:
			this->m_pNetConnecter->send(pData, nSize, true);
			break;

		case eMT_REQUEST:
		case eMT_RESPONSE:
		case eMT_SYSTEM:
		case eMT_GATE_FORWARD:
		case eMT_TO_GATE:
			{
				core_message_header header;
				header.nMessageSize = sizeof(header) + nSize;
				header.nMessageType = nMessageType;
				this->m_pNetConnecter->send(&header, sizeof(header), true);
				this->m_pNetConnecter->send(pData, nSize, true);
			}
			break;
		}
	}

	void CCoreConnection::send(uint8_t nMessageType, const void* pData, uint16_t nSize, const void* pExtraBuf, uint16_t nExtraSize)
	{
		PROFILING_GUARD(CCoreConnection::send)

		DebugAst(this->m_pNetConnecter != nullptr);
		DebugAst(pData != nullptr && pExtraBuf != nullptr && nSize + nExtraSize < UINT16_MAX);

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
				core_message_header header;
				header.nMessageSize = sizeof(header) + nSize + nExtraSize;
				header.nMessageType = nMessageType;
				this->m_pNetConnecter->send(&header, sizeof(header), true);
				this->m_pNetConnecter->send(pData, nSize, true);
				this->m_pNetConnecter->send(pExtraBuf, nExtraSize, true);
			}
			break;
		}
	}

	void CCoreConnection::shutdown(base::ENetConnecterCloseType eType, const std::string& szMsg)
	{
		if (this->m_pNetConnecter)
			return;

		if (this->m_pNetConnecter != nullptr)
			this->m_pNetConnecter->shutdown(eType, szMsg.c_str());
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
		{
			static SNetAddr s_Default;
			return s_Default;
		}
		return this->m_pNetConnecter->getLocalAddr();
	}

	const SNetAddr& CCoreConnection::getRemoteAddr() const
	{
		if (this->m_pNetConnecter == nullptr)
		{
			static SNetAddr s_Default;
			return s_Default;
		}

		return this->m_pNetConnecter->getRemoteAddr();
	}

	void CCoreConnection::setMessageParser(MessageParser& parser)
	{
		this->m_messageParser = parser;
	}

	void CCoreConnection::enableHeartbeat(bool bEnable)
	{
		this->m_bHeartbeat = bEnable;
	}

	void CCoreConnection::setState(uint32_t nState)
	{
		this->m_nState = nState;
	}

	uint32_t CCoreConnection::getState() const
	{
		return this->m_nState;
	}
}