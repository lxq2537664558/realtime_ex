#include "stdafx.h"
#include "net_runnable.h"
#include "core_common.h"
#include "core_connection.h"
#include "core_connection_mgr.h"
#include "core_app.h"
#include "message_command.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/time_util.h"
#include "libBaseCommon/profiling.h"

#include <algorithm>

#define _CYCLE_TIME 10

namespace core
{
	CNetRunnable::CNetRunnable()
		: m_pCoreConnectionMgr(nullptr)
		, m_pThreadBase(nullptr)
		, m_pMessageQueue(nullptr)
		, m_nLastCheckTime(0)
		, m_nTotalSamplingTime(0)
	{
	}

	CNetRunnable::~CNetRunnable()
	{
		SAFE_RELEASE(this->m_pThreadBase);
	}

	bool CNetRunnable::init(uint32_t nMaxSocketCount)
	{
		this->m_pCoreConnectionMgr = new CCoreConnectionMgr();
		if (!this->m_pCoreConnectionMgr->init(nMaxSocketCount))
			return false;

		this->m_pMessageQueue = new CNetMessageQueue();

		this->m_nLastCheckTime = base::time_util::getGmtTime();

		this->m_pThreadBase = base::CThreadBase::createNew(this);
		return nullptr != this->m_pThreadBase;
	}

	CCoreConnectionMgr* CNetRunnable::getCoreConnectionMgr() const
	{
		return this->m_pCoreConnectionMgr;
	}

	CNetMessageQueue* CNetRunnable::getMessageQueue() const
	{
		return this->m_pMessageQueue;
	}

	bool CNetRunnable::onInit()
	{
		return true;
	}

	void CNetRunnable::onDestroy()
	{
		SAFE_DELETE(this->m_pCoreConnectionMgr);
	}

	bool CNetRunnable::onProcess()
	{
		int64_t nBeginSamplingTime = base::time_util::getProcessPassTime();

		PROFILING_BEGIN(this->m_pCoreConnectionMgr->update)
		this->m_pCoreConnectionMgr->update(_CYCLE_TIME);
		PROFILING_END(this->m_pCoreConnectionMgr->update)

		int64_t nCurTime = base::time_util::getGmtTime();
		if (nCurTime - this->m_nLastCheckTime >= _CYCLE_TIME)
		{
			this->m_nLastCheckTime = nCurTime;

			SMessagePacket sMessagePacket;
			sMessagePacket.nType = eMCT_FRAME;
			sMessagePacket.pData = nullptr;
			sMessagePacket.nDataSize = 0;

			const std::vector<CCoreService*>& vecCoreService = CCoreApp::Inst()->getCoreServiceMgr()->getCoreService();
			for (size_t i = 0; i < vecCoreService.size(); ++i)
			{
				vecCoreService[i]->getMessageQueue()->send(sMessagePacket);
			}

			sMessagePacket.nType = eMCT_GLOBAL_FRAME;
			
			CCoreApp::Inst()->getGlobalLogicMessageQueue()->send(sMessagePacket);
		}

		this->m_pMessageQueue->recv(this->m_vecMessagePacket);

		for (size_t i = 0; i < this->m_vecMessagePacket.size(); ++i)
		{
			const SMessagePacket& sMessagePacket = this->m_vecMessagePacket[i];

			switch (sMessagePacket.nType)
			{
			case eMCT_REQUEST_SOCKET_LISTEN:
			{
				PROFILING_GUARD(eMCT_REQUEST_SOCKET_LISTEN)
				SMCT_REQUEST_SOCKET_LISTEN* pContext = reinterpret_cast<SMCT_REQUEST_SOCKET_LISTEN*>(sMessagePacket.pData);

				if (!this->m_pCoreConnectionMgr->listen(pContext->pMessageQueue, pContext->szHost, pContext->nPort, pContext->nReusePort != 0, pContext->szType, pContext->szContext, pContext->nSendBufferSize, pContext->nSendBufferSize, pContext->messageParser, pContext->nCoreConnectionType))
				{
					SMCT_NOTIFY_SOCKET_LISTEN_FAIL* pFailContext = new SMCT_NOTIFY_SOCKET_LISTEN_FAIL();
					pFailContext->szContext = pContext->szContext;
					pFailContext->szHost = pContext->szHost;
					pFailContext->nPort = pContext->nPort;

					SMessagePacket sMessagePacket;
					sMessagePacket.nType = eMCT_NOTIFY_SOCKET_LISTEN_FAIL;
					sMessagePacket.pData = pFailContext;
					sMessagePacket.nDataSize = sizeof(SMCT_NOTIFY_SOCKET_LISTEN_FAIL);

					pContext->pMessageQueue->send(sMessagePacket);
				}

				SAFE_DELETE(pContext);
			}
			break;
			case eMCT_REQUEST_SOCKET_CONNECT:
			{
				PROFILING_GUARD(eMCT_REQUEST_SOCKET_CONNECT)
				SMCT_REQUEST_SOCKET_CONNECT* pContext = reinterpret_cast<SMCT_REQUEST_SOCKET_CONNECT*>(sMessagePacket.pData);
				DebugAstEx(pContext->pMessageQueue != nullptr, true);

				if (!this->m_pCoreConnectionMgr->connect(pContext->pMessageQueue, pContext->szHost, pContext->nPort, pContext->szType, pContext->szContext, pContext->nSendBufferSize, pContext->nSendBufferSize, pContext->messageParser))
				{
					SMCT_NOTIFY_SOCKET_CONNECT_FAIL* pFailContext = new SMCT_NOTIFY_SOCKET_CONNECT_FAIL();
					pFailContext->szContext = pContext->szContext;

					SMessagePacket sMessagePacket;
					sMessagePacket.nType = eMCT_NOTIFY_SOCKET_CONNECT_FAIL;
					sMessagePacket.pData = pFailContext;
					sMessagePacket.nDataSize = sizeof(SMCT_NOTIFY_SOCKET_CONNECT_FAIL);

					pContext->pMessageQueue->send(sMessagePacket);
				}

				SAFE_DELETE(pContext);
			}
			break;

			case eMCT_REQUEST_SOCKET_SHUTDOWN:
			{
				PROFILING_GUARD(eMCT_REQUEST_SOCKET_SHUTDOWN)
				SMCT_REQUEST_SOCKET_SHUTDOWN* pContext = reinterpret_cast<SMCT_REQUEST_SOCKET_SHUTDOWN*>(sMessagePacket.pData);

				CCoreConnection* pCoreConnection = this->m_pCoreConnectionMgr->getCoreConnectionBySocketID(pContext->nSocketID);
				if (nullptr != pCoreConnection)
					pCoreConnection->shutdown(pContext->nForce != 0, pContext->szMsg);

				SAFE_DELETE(pContext);
			}
			break;

			case eMCT_NOTIFY_SOCKET_CONNECT_ACK:
			{
				PROFILING_GUARD(eMCT_NOTIFY_SOCKET_CONNECT_ACK)
				SMCT_NOTIFY_SOCKET_CONNECT_ACK* pContext = reinterpret_cast<SMCT_NOTIFY_SOCKET_CONNECT_ACK*>(sMessagePacket.pData);

				if (pContext->bSuccess)
					pContext->pCoreConnection->onConnectAck();
				else
					pContext->pCoreConnection->shutdown(true, "create base connection error");

				SAFE_DELETE(pContext);
			}
			break;

			case eMCT_NOTIFY_SOCKET_DISCONNECT_ACK:
			{
				SMCT_NOTIFY_SOCKET_DISCONNECT_ACK* pContext = reinterpret_cast<SMCT_NOTIFY_SOCKET_DISCONNECT_ACK*>(sMessagePacket.pData);

				CNetRunnable::Inst()->getCoreConnectionMgr()->destroyCoreConnection(pContext->nSocketID);

				SAFE_DELETE(pContext);
			}
			break;

			case eMCT_SEND_SOCKET_DATA1:
			{
				PROFILING_GUARD(eMCT_SEND_SOCKET_DATA1)
				SMCT_SEND_SOCKET_DATA1* pContext = reinterpret_cast<SMCT_SEND_SOCKET_DATA1*>(sMessagePacket.pData);

				pContext->pCoreConnection->send(pContext->nMessageType, pContext + 1, (uint16_t)(sMessagePacket.nDataSize - sizeof(SMCT_SEND_SOCKET_DATA1)));

				char* szBuf = reinterpret_cast<char*>(sMessagePacket.pData);
				SAFE_DELETE_ARRAY(szBuf);
			}
			break;

			case eMCT_SEND_SOCKET_DATA2:
			{
				PROFILING_GUARD(eMCT_SEND_SOCKET_DATA2)
				SMCT_SEND_SOCKET_DATA2* pContext = reinterpret_cast<SMCT_SEND_SOCKET_DATA2*>(sMessagePacket.pData);
				defer([&] 
				{
					char* szBuf = reinterpret_cast<char*>(sMessagePacket.pData);
					SAFE_DELETE_ARRAY(szBuf);
				});

				CCoreConnection* pCoreConnection = this->m_pCoreConnectionMgr->getCoreConnectionBySocketID(pContext->nSocketID);
				if (nullptr == pCoreConnection)
					return true;

				pCoreConnection->send(pContext->nMessageType, pContext + 1, (uint16_t)(sMessagePacket.nDataSize - sizeof(SMCT_SEND_SOCKET_DATA2)));
			}
			break;

			case eMCT_BROADCAST_SOCKET_DATA1:
			{
				PROFILING_GUARD(eMCT_BROADCAST_SOCKET_DATA1)
				char* szBuf = reinterpret_cast<char*>(sMessagePacket.pData);
				SMCT_BROADCAST_SOCKET_DATA1* pContext = reinterpret_cast<SMCT_BROADCAST_SOCKET_DATA1*>(sMessagePacket.pData);

				void* pData = szBuf + sizeof(SMCT_BROADCAST_SOCKET_DATA1) + sizeof(uint64_t)*pContext->nSocketIDCount;
				uint64_t* pSocketID = reinterpret_cast<uint64_t*>(szBuf + sizeof(SMCT_BROADCAST_SOCKET_DATA1));
				for (size_t i = 0; i < pContext->nSocketIDCount; ++i)
				{
					CCoreConnection* pCoreConnection = this->m_pCoreConnectionMgr->getCoreConnectionBySocketID(pSocketID[i]);
					if (nullptr != pCoreConnection)
					{
						pCoreConnection->send(pContext->nMessageType, pData, (uint16_t)(sMessagePacket.nDataSize - sizeof(SMCT_BROADCAST_SOCKET_DATA1) - sizeof(uint64_t)*pContext->nSocketIDCount));
					}
				}

				SAFE_DELETE_ARRAY(szBuf);
			}
			break;

			case eMCT_BROADCAST_SOCKET_DATA2:
			{
				PROFILING_GUARD(eMCT_BROADCAST_SOCKET_DATA2)
				char* szBuf = reinterpret_cast<char*>(sMessagePacket.pData);
				SMCT_BROADCAST_SOCKET_DATA2* pContext = reinterpret_cast<SMCT_BROADCAST_SOCKET_DATA2*>(sMessagePacket.pData);

				void* pData = szBuf + sizeof(SMCT_BROADCAST_SOCKET_DATA2) + sizeof(uint64_t)*pContext->nExcludeIDCount + pContext->nTypeLen;
				uint64_t* pExcludeID = reinterpret_cast<uint64_t*>(szBuf + sizeof(SMCT_BROADCAST_SOCKET_DATA2) + pContext->nTypeLen);

				this->m_pCoreConnectionMgr->broadcast(pContext->szType, pContext->nMessageType, pData, (uint16_t)(sMessagePacket.nDataSize - sizeof(SMCT_BROADCAST_SOCKET_DATA2) - pContext->nTypeLen - sizeof(uint64_t)*pContext->nExcludeIDCount), pExcludeID, pContext->nExcludeIDCount);

				SAFE_DELETE_ARRAY(szBuf);
			}
			break;

			default:
				PrintWarning("invalid cmd type: {}", sMessagePacket.nType);
			}
		}

		int64_t nEndSamplingTime = base::time_util::getProcessPassTime();
		this->m_nTotalSamplingTime = this->m_nTotalSamplingTime + (uint32_t)(nEndSamplingTime - nBeginSamplingTime);

		if (this->m_nTotalSamplingTime / 1000 >= CCoreApp::Inst()->getSamplingTime())
		{
			PROFILING_UPDATE(this->m_nTotalSamplingTime);
			this->m_nTotalSamplingTime = 0;
		}

		return true;
	}
}