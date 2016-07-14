#include "stdafx.h"
#include "core_connection.h"
#include "core_connection_mgr.h"
#include "base_connection_mgr.h"
#include "base_app_impl.h"
#include "base_app.h"
#include "core_common.h"
#include "base_connection.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/base_time.h"
#include "message_queue.h"
#include "message_command.h"

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
						this->shutdown(true, szBuf);
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
						this->shutdown(true, szBuf);
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
		catch (...)
		{
			this->shutdown(true, "dispatch data error");
		}

		return nRecvSize;
	}

	void CCoreConnection::onConnect()
	{
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

		CBaseAppImpl::Inst()->getMessageQueue()->pushMessagePacket(sMessagePacket);

		this->m_nState = eCCS_Connectting;
	}

	uint64_t CCoreConnection::getID() const
	{
		return this->m_nID;
	}

	uint32_t CCoreConnection::getType() const
	{
		return this->m_nType;
	}

	void CCoreConnection::onDisconnect()
	{
		SMCT_NOTIFY_SOCKET_DISCONNECT* pContext = new SMCT_NOTIFY_SOCKET_DISCONNECT();
		pContext->nSocketID = this->getID();

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_NOTIFY_SOCKET_DISCONNECT;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_NOTIFY_SOCKET_DISCONNECT);

		CBaseAppImpl::Inst()->getMessageQueue()->pushMessagePacket(sMessagePacket);

		this->m_nState = eCCS_Disconnectting;
		this->m_pNetConnecter = nullptr;
	}

	void CCoreConnection::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		if (nMessageType == eMT_HEARTBEAT)
			return;

		CBaseAppImpl::Inst()->incQPS();

		this->m_monitor.onRecv(nSize);

		SMessagePacket sMessagePacket;
		SMCT_RECV_SOCKET_DATA* pContext = new SMCT_RECV_SOCKET_DATA();
		pContext->nSocketID = this->getID();
		pContext->nMessageType = nMessageType;
		pContext->nDataSize = nSize;
		pContext->pData = new char[nSize];
		memcpy(pContext->pData, pData, nSize);

		sMessagePacket.nType = eMCT_RECV_SOCKET_DATA;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_RECV_SOCKET_DATA);

		CBaseAppImpl::Inst()->getMessageQueue()->pushMessagePacket(sMessagePacket);
	}

	void CCoreConnection::onHeartbeat()
	{
		if (this->m_bHeartbeat)
			return;
		
		if (this->m_nSendHeartbeatCount > CBaseAppImpl::Inst()->getHeartbeatLimit())
		{
			this->shutdown(true, "heart beat time out");
			return;
		}
		
		++this->m_nSendHeartbeatCount;
		connection_heart_beat netMsg;
		this->send(eMT_HEARTBEAT, &netMsg, sizeof(netMsg));
	}

	void CCoreConnection::send(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		if (this->m_nState == eCCS_Disconnectting)
			return;

		DebugAst(this->m_pNetConnecter != nullptr);
		DebugAst(pData != nullptr && nSize > 0);

		this->m_monitor.onSend(nSize);

		switch (nMessageType)
		{
		case eMT_CLIENT:
		case eMT_HEARTBEAT:
			this->m_pNetConnecter->send(pData, nSize);
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
				this->m_pNetConnecter->send(&header, sizeof(header));
				this->m_pNetConnecter->send(pData, nSize);
			}
			break;
		}
	}

	void CCoreConnection::send(uint8_t nMessageType, const void* pData, uint16_t nSize, const void* pExtraBuf, uint16_t nExtraSize)
	{
		if (this->m_nState == eCCS_Disconnectting)
			return;

		DebugAst(this->m_pNetConnecter != nullptr);
		DebugAst(pData != nullptr && nSize > 0 && pExtraBuf != nullptr && nExtraSize > 0);

		this->m_monitor.onSend(nSize + nExtraSize);

		switch (nMessageType)
		{
		case eMT_CLIENT:
		case eMT_HEARTBEAT:
			{
				this->m_pNetConnecter->send(pData, nSize);
				this->m_pNetConnecter->send(pExtraBuf, nExtraSize);
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
				this->m_pNetConnecter->send(&header, sizeof(header));
				this->m_pNetConnecter->send(pData, nSize);
				this->m_pNetConnecter->send(pExtraBuf, nExtraSize);
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