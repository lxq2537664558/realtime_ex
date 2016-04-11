#include "stdafx.h"
#include "core_connection.h"
#include "core_connection_mgr.h"
#include "base_connection_mgr.h"
#include "base_app.h"
#include "core_common.h"
#include "base_connection.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/base_time.h"

#define CHECK_TIME 10000
#define MAX_HEARTBEAT_COUNT 60

namespace core
{
	
#pragma pack(push,1)

	struct connection_heart_beat
	{
		uint8_t	nContext;
	};

#pragma pack(pop)

	CCoreConnection::CCoreConnection()
		: m_pBaseConnection(nullptr)
		, m_bHeartbeat(false)
		, m_pHeartbeat(nullptr)
		, m_nSendHeartbeatCount(0)
		, m_nID(0)
	{
	}
	
	CCoreConnection::~CCoreConnection()
	{
		DebugAst(this->m_pNetConnecter == nullptr);
		DebugAst(this->m_pHeartbeat == nullptr);
	}

	bool CCoreConnection::init(CBaseConnection* pBaseConnection, uint64_t nID, ClientDataCallback clientDataCallback)
	{
		DebugAstEx(pBaseConnection != nullptr, false);

		this->m_pBaseConnection = pBaseConnection;
		this->m_pBaseConnection->m_pCoreConnection = this;
		this->m_nID = nID;
		this->m_clientDataCallback = clientDataCallback;

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

				if (this->m_clientDataCallback != nullptr)
				{
					int32_t nParserSize = this->m_clientDataCallback(pData, nDataSize);
					if (nParserSize == 0)
						break;

					if (nParserSize <= 0 || nParserSize > UINT16_MAX)
					{
						char szBuf[256] = { 0 };
						base::crt::snprintf(szBuf, _countof(szBuf), "parser client msg error %d", nParserSize);
						this->shutdown(true, szBuf);
						break;
					}

					int16_t nClientMessageSize = (int16_t)nParserSize;
					// 消息长度的最高位决定了这个消息格式
					if (nClientMessageSize < 0)
					{
						nClientMessageSize = -nClientMessageSize;
						this->onPacketMsg(eMT_CLIENT|eMT_PROTOBUF, pData, (uint16_t)nClientMessageSize);
					}
					else
					{
						this->onPacketMsg(eMT_CLIENT, pData, (uint16_t)nClientMessageSize);
					}

					nMessageSize = (uint16_t)nClientMessageSize;
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
						base::crt::snprintf(szBuf, _countof(szBuf), "message size error message_type[%d]", pHeader->nMessageID);
						this->shutdown(true, szBuf);
						break;
					}

					// 不是完整的消息
					if (nDataSize < pHeader->nMessageSize)
						break;

					nMessageSize = pHeader->nMessageSize;

					this->onPacketMsg(pHeader->nMessageID, pHeader + 1, pHeader->nMessageSize - sizeof(message_header));
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
		DebugAst(this->m_pBaseConnection != nullptr);

		this->m_pBaseConnection->onConnect(this->m_szContext);
		CBaseApp::Inst()->getBaseConnectionMgr()->onConnect(this->m_pBaseConnection);

		this->m_pHeartbeat = new CTicker();
		this->m_pHeartbeat->setCallback(&CCoreConnection::onHeartbeat, this);

		CBaseApp::Inst()->registTicker(this->m_pHeartbeat, CHECK_TIME, CHECK_TIME, 0);
	}

	uint64_t CCoreConnection::getID() const
	{
		return this->m_nID;
	}

	void CCoreConnection::onDisconnect()
	{
		DebugAst(this->m_pBaseConnection != nullptr);

		this->m_pBaseConnection->onDisconnect();

		CBaseApp::Inst()->getBaseConnectionMgr()->onDisconnect(this->m_pBaseConnection);

		SAFE_DELETE(this->m_pHeartbeat);
		this->m_pBaseConnection = nullptr;
		this->m_pNetConnecter = nullptr;
	}

	void CCoreConnection::onPacketMsg(uint16_t nMessageType, const void* pData, uint16_t nSize)
	{
		if (nMessageType == eMT_HEARTBEAT)
			return;

		this->m_pBaseConnection->onDispatch(nMessageType, pData, nSize);
	}

	void CCoreConnection::onHeartbeat(uint64_t nContext)
	{
		if (this->m_bHeartbeat)
			return;
		
		if (this->m_nSendHeartbeatCount > MAX_HEARTBEAT_COUNT)
		{
			this->shutdown(true, "heart beat time out");
			return;
		}
		this->sendHeartbeat();
	}

	void CCoreConnection::sendHeartbeat()
	{
		++this->m_nSendHeartbeatCount;
		connection_heart_beat netMsg;
		this->send(eMT_HEARTBEAT, &netMsg, sizeof(netMsg));
	}

	void CCoreConnection::send(uint16_t nMessageType, const void* pData, uint16_t nSize)
	{
		DebugAst(this->m_pNetConnecter != nullptr);
		DebugAst(pData != nullptr && nSize > 0);

		switch (nMessageType)
		{
		case eMT_CLIENT:
		case eMT_HEARTBEAT:
			this->m_pNetConnecter->send(pData, nSize);
			break;

		case eMT_REQUEST:
		case eMT_RESPONSE:
		case eMT_SYSTEM:
		case eMT_FROM_GATE:
			{
				message_header header;
				header.nMessageSize = sizeof(header) + nSize;
				header.nMessageID = nMessageType;
				this->m_pNetConnecter->send(&header, sizeof(header));
				this->m_pNetConnecter->send(pData, nSize);
			}
			break;
		}
	}

	void CCoreConnection::send(uint16_t nMessageType, const void* pData, uint16_t nSize, const void* pExtraBuf, uint16_t nExtraSize)
	{
		DebugAst(this->m_pNetConnecter != nullptr);
		DebugAst(pData != nullptr && nSize > 0 && pExtraBuf != nullptr && nExtraSize > 0);

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
		case eMT_FROM_GATE:
			{
				message_header header;
				header.nMessageSize = sizeof(header) + nSize + nExtraSize;
				header.nMessageID = nMessageType;
				this->m_pNetConnecter->send(&header, sizeof(header));
				this->m_pNetConnecter->send(pData, nSize);
				this->m_pNetConnecter->send(pExtraBuf, nExtraSize);
			}
			break;
		}
	}

	void CCoreConnection::shutdown(bool bForce, const std::string& szMsg)
	{
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

	CBaseConnection* CCoreConnection::getBaseConnection() const
	{
		return this->m_pBaseConnection;
	}
}