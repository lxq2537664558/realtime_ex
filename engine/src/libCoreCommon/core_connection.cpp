#include "stdafx.h"
#include "core_connection.h"
#include "core_connection_mgr.h"
#include "base_connection_mgr.h"
#include "core_app.h"
#include "core_common.h"
#include "base_connection.h"
#include "core_data.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/base_time.h"

#define CHECK_TIME 10000
#define MAX_HEARTBEAT_COUNT 60

namespace core
{
	
#pragma pack(push,1)
	struct remote_message_header
	{
		uint16_t	nMsgSize;	// 包括消息头的
		uint16_t	nMsgType;
	};

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

	bool CCoreConnection::init(CBaseConnection* pBaseConnection, uint64_t nID, funRawDataParser pfRawDataParser)
	{
		DebugAstEx(pBaseConnection != nullptr, false);

		this->m_pBaseConnection = pBaseConnection;
		this->m_pBaseConnection->m_pCoreConnection = this;
		this->m_nID = nID;
		this->m_funRawDataParser = pfRawDataParser;

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
				uint16_t nMsgSize = 0;

				if (this->m_funRawDataParser != nullptr)
				{
					int32_t nParserSize = this->m_funRawDataParser(pData, nDataSize);
					if (nParserSize == 0)
						break;

					if (nParserSize < 0 || nParserSize > UINT16_MAX)
					{
						char szBuf[256] = { 0 };
						base::crt::snprintf(szBuf, _countof(szBuf), "parser raw msg error %d", nParserSize);
						this->shutdown(true, szBuf);
						break;
					}

					this->onPacketMsg(eMT_CLIENT, pData, (uint16_t)nParserSize);

					nMsgSize = (uint16_t)nParserSize;
				}
				else
				{
					// 都不够消息头
					if (nDataSize < sizeof(remote_message_header))
						break;

					const remote_message_header* pHeader = reinterpret_cast<const remote_message_header*>(pData);
					if (pHeader->nMsgSize < sizeof(remote_message_header))
					{
						char szBuf[256] = { 0 };
						base::crt::snprintf(szBuf, _countof(szBuf), "msg type[%d] error", pHeader->nMsgType);
						this->shutdown(true, szBuf);
						break;
					}

					// 不是完整的消息
					if (nDataSize < pHeader->nMsgSize)
						break;

					nMsgSize = pHeader->nMsgSize;

					this->onPacketMsg(pHeader->nMsgType, pHeader + 1, pHeader->nMsgSize - sizeof(remote_message_header));
				}

				nRecvSize += nMsgSize;
				nDataSize -= nMsgSize;
				pData += nMsgSize;

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
		CCoreApp::Inst()->getBaseConnectionMgr()->onConnect(this->m_pBaseConnection);

		this->m_pHeartbeat = new CTicker();
		this->m_pHeartbeat->setCallback(&CCoreConnection::onHeartbeat, this);

		CCoreApp::Inst()->registTicker(this->m_pHeartbeat, CHECK_TIME, CHECK_TIME, 0);
	}

	uint64_t CCoreConnection::getID() const
	{
		return this->m_nID;
	}

	void CCoreConnection::onDisconnect()
	{
		DebugAst(this->m_pBaseConnection != nullptr);

		this->m_pBaseConnection->onDisconnect();

		CCoreApp::Inst()->getBaseConnectionMgr()->onDisconnect(this->m_pBaseConnection);

		SAFE_DELETE(this->m_pHeartbeat);
		this->m_pBaseConnection = nullptr;
		this->m_pNetConnecter = nullptr;
	}

	void CCoreConnection::onPacketMsg(uint16_t nMsgType, const void* pData, uint16_t nSize)
	{
		if (nMsgType == eMT_HEARTBEAT)
			return;

		this->m_pBaseConnection->onDispatch(nMsgType, pData, nSize);
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

	void CCoreConnection::send(uint16_t nMsgType, const void* pData, uint16_t nSize)
	{
		DebugAst(this->m_pNetConnecter != nullptr);
		DebugAst(pData != nullptr && nSize > 0);

		switch (nMsgType)
		{
		case eMT_CLIENT:
			this->m_pNetConnecter->send(pData, nSize);
			break;

		case eMT_HEARTBEAT:
		case eMT_REQUEST:
		case eMT_RESPONSE:
		case eMT_SYSTEM:
			{
				remote_message_header header;
				header.nMsgSize = sizeof(header) + nSize;
				header.nMsgType = nMsgType;
				this->m_pNetConnecter->send(&header, sizeof(header));
				this->m_pNetConnecter->send(pData, nSize);
			}
			break;
		}
	}

	void CCoreConnection::send(uint16_t nMsgType, const void* pData, uint16_t nSize, const void* pExtraBuf, uint16_t nExtraSize)
	{
		DebugAst(this->m_pNetConnecter != nullptr);
		DebugAst(pData != nullptr && nSize > 0 && pExtraBuf != nullptr && nExtraSize > 0);

		switch (nMsgType)
		{
		case eMT_CLIENT:
			{
				this->m_pNetConnecter->send(pData, nSize);
				this->m_pNetConnecter->send(pExtraBuf, nExtraSize);
			}
			break;

		case eMT_HEARTBEAT:
		case eMT_REQUEST:
		case eMT_RESPONSE:
		case eMT_SYSTEM:
			{
				remote_message_header header;
				header.nMsgSize = sizeof(header) + nSize;
				header.nMsgType = nMsgType;
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