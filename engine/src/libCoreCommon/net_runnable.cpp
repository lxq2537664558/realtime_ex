#include "stdafx.h"
#include "net_runnable.h"
#include "core_common.h"
#include "core_connection.h"
#include "core_connection_mgr.h"
#include "core_app.h"
#include "message_command.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/base_time.h"
#include "libBaseCommon/profiling.h"

#include <algorithm>

// 放这里为了调试或者看dump的时候方便
core::CNetRunnable*	g_pNetRunnable;

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
		SAFE_DELETE(this->m_pMessageQueue);
		SAFE_DELETE(this->m_pCoreConnectionMgr);
		SAFE_RELEASE(this->m_pThreadBase);
	}

	CNetRunnable* CNetRunnable::Inst()
	{
		if (g_pNetRunnable == nullptr)
			g_pNetRunnable = new CNetRunnable();

		return g_pNetRunnable;
	}

	bool CNetRunnable::init(uint32_t nMaxSocketCount)
	{
		this->m_pMessageQueue = new CNetMessageQueue();
		if (!this->m_pMessageQueue->init())
			return false;

		this->m_pCoreConnectionMgr = new CCoreConnectionMgr();
		if (!this->m_pCoreConnectionMgr->init(nMaxSocketCount))
			return false;

		this->m_nLastCheckTime = base::getGmtTime();

		this->m_pThreadBase = base::CThreadBase::createNew(this);
		return nullptr != this->m_pThreadBase;
	}

	void CNetRunnable::join()
	{
		this->m_pThreadBase->join();
	}

	void CNetRunnable::release()
	{
		delete g_pNetRunnable;
		g_pNetRunnable = nullptr;
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

	}

	bool CNetRunnable::onProcess()
	{
		int64_t nBeginSamplingTime = base::getProcessPassTime();

		this->m_pCoreConnectionMgr->update(-1);
		
		static std::vector<SMessagePacket> vecMessagePacket;
		this->m_pMessageQueue->recv(vecMessagePacket);

		for (auto iter = vecMessagePacket.begin(); iter != vecMessagePacket.end(); ++iter)
		{
			const SMessagePacket& sMessagePacket = *iter;

			switch (sMessagePacket.nType)
			{
			case eMCT_EXIT:
				{
					return false;
				}
				break;

			case eMCT_REQUEST_SOCKET_LISTEN:
				{
					PROFILING_GUARD(eMCT_REQUEST_SOCKET_LISTEN)
					SMCT_REQUEST_SOCKET_LISTEN* pContext = reinterpret_cast<SMCT_REQUEST_SOCKET_LISTEN*>(sMessagePacket.pData);
					if (pContext == nullptr)
					{
						PrintWarning("context == nullptr type: eMCT_REQUEST_SOCKET_LISTEN");
						continue;
					}
					this->m_pCoreConnectionMgr->listen(pContext->szHost, pContext->nPort, pContext->nType, pContext->szContext, pContext->nSendBufferSize, pContext->nSendBufferSize, pContext->messageParser);
					SAFE_DELETE(pContext);
				}
				break;

			case eMCT_REQUEST_SOCKET_CONNECT:
				{
					PROFILING_GUARD(eMCT_REQUEST_SOCKET_CONNECT)
					SMCT_REQUEST_SOCKET_CONNECT* pContext = reinterpret_cast<SMCT_REQUEST_SOCKET_CONNECT*>(sMessagePacket.pData);
					if (pContext == nullptr)
					{
						PrintWarning("context == nullptr type: eMCT_REQUEST_SOCKET_CONNECT");
						continue;
					}
					this->m_pCoreConnectionMgr->connect(pContext->szHost, pContext->nPort, pContext->nType, pContext->szContext, pContext->nSendBufferSize, pContext->nSendBufferSize, pContext->mssageParser);
					SAFE_DELETE(pContext);
				}
				break;

			case eMCT_REQUEST_SOCKET_SHUTDOWN:
				{
					PROFILING_GUARD(eMCT_REQUEST_SOCKET_SHUTDOWN)
					SMCT_REQUEST_SOCKET_SHUTDOWN* pContext = reinterpret_cast<SMCT_REQUEST_SOCKET_SHUTDOWN*>(sMessagePacket.pData);
					if (pContext == nullptr)
					{
						PrintWarning("context == nullptr type: eMCT_REQUEST_SOCKET_SHUTDOWN");
						continue;
					}
					CCoreConnection* pCoreConnection = this->m_pCoreConnectionMgr->getCoreConnectionByID(pContext->nSocketID);
					if (nullptr != pCoreConnection)
						pCoreConnection->shutdown((base::ENetConnecterCloseType)pContext->nType, pContext->szMsg);

					SAFE_DELETE(pContext);
				}
				break;

			case eMCT_NOTIFY_SOCKET_DISCONNECT_ACK:
				{
					PROFILING_GUARD(eMCT_NOTIFY_SOCKET_DISCONNECT_ACK)
					SMCT_NOTIFY_SOCKET_DISCONNECT_ACK* pContext = reinterpret_cast<SMCT_NOTIFY_SOCKET_DISCONNECT_ACK*>(sMessagePacket.pData);
					if (pContext == nullptr)
					{
						PrintWarning("context == nullptr type: eMCT_NOTIFY_CONNECTION_DESTROY");
						continue;
					}
					this->m_pCoreConnectionMgr->destroyCoreConnection(pContext->nSocketID);

					SAFE_DELETE(pContext);
				}
				break;

			case eMCT_NOTIFY_SOCKET_CONNECT_ACK:
				{
					PROFILING_GUARD(eMCT_NOTIFY_SOCKET_CONNECT_ACK)
					SMCT_NOTIFY_SOCKET_CONNECT_ACK* pContext = reinterpret_cast<SMCT_NOTIFY_SOCKET_CONNECT_ACK*>(sMessagePacket.pData);
					if (pContext == nullptr)
					{
						PrintWarning("context == nullptr type: SMCT_NOTIFY_SOCKET_CONNECT_ACK");
						continue;
					}

					CCoreConnection* pCoreConnection = this->m_pCoreConnectionMgr->getCoreConnectionByID(pContext->nSocketID);
					if (nullptr != pCoreConnection)
						pCoreConnection->setState(CCoreConnection::eCCS_Connected);

					SAFE_DELETE(pContext);
				}
				break;

			case eMCT_SEND_SOCKET_DATA:
				{
					PROFILING_GUARD(eMCT_SEND_SOCKET_DATA)
					SMCT_SEND_SOCKET_DATA* pContext = reinterpret_cast<SMCT_SEND_SOCKET_DATA*>(sMessagePacket.pData);
					if (pContext == nullptr)
					{
						PrintWarning("context == nullptr type: eMCT_SEND_SOCKET_DATA");
						continue;
					}
					CCoreConnection* pCoreConnection = this->m_pCoreConnectionMgr->getCoreConnectionByID(pContext->nSocketID);
					if (nullptr != pCoreConnection)
						pCoreConnection->send(pContext->nMessageType, pContext + 1, (uint16_t)(sMessagePacket.nDataSize - sizeof(SMCT_SEND_SOCKET_DATA)));
				
					char* szBuf = reinterpret_cast<char*>(sMessagePacket.pData);
					SAFE_DELETE_ARRAY(szBuf);
				}
				break;

			case eMCT_BROADCAST_SOCKET_DATA1:
				{
					PROFILING_GUARD(eMCT_BROADCAST_SOCKET_DATA1)
					SMCT_BROADCAST_SOCKET_DATA1* pContext = reinterpret_cast<SMCT_BROADCAST_SOCKET_DATA1*>(sMessagePacket.pData);
					if (pContext == nullptr)
					{
						PrintWarning("context == nullptr type: eMCT_BROADCAST_SOCKET_DATA1");
						continue;
					}
					void* pData = pContext + 1;
					for (size_t i = 0; i < pContext->vecSocketID->size(); ++i)
					{
						CCoreConnection* pCoreConnection = this->m_pCoreConnectionMgr->getCoreConnectionByID(pContext->vecSocketID->at(i));
						if (nullptr != pCoreConnection)
						{
							pCoreConnection->send(pContext->nMessageType, pContext + 1, (uint16_t)(sMessagePacket.nDataSize - sizeof(SMCT_BROADCAST_SOCKET_DATA1)));
						}
					}
					SAFE_DELETE(pContext->vecSocketID);
					char* szBuf = reinterpret_cast<char*>(sMessagePacket.pData);
					SAFE_DELETE_ARRAY(szBuf);
				}
				break;

			case eMCT_BROADCAST_SOCKET_DATA2:
				{
					PROFILING_GUARD(eMCT_BROADCAST_SOCKET_DATA2)
					SMCT_BROADCAST_SOCKET_DATA2* pContext = reinterpret_cast<SMCT_BROADCAST_SOCKET_DATA2*>(sMessagePacket.pData);
					if (pContext == nullptr)
					{
						PrintWarning("context == nullptr type: eMCT_BROADCAST_SOCKET_DATA2");
						continue;
					}
					this->m_pCoreConnectionMgr->broadcast(pContext->nType, pContext->nMessageType, pContext + 1, (uint16_t)(sMessagePacket.nDataSize - sizeof(SMCT_BROADCAST_SOCKET_DATA2)), pContext->vecExcludeID);
					char* szBuf = reinterpret_cast<char*>(sMessagePacket.pData);
					SAFE_DELETE_ARRAY(szBuf);
				}
				break;

			case eMCT_ENABLE_HEARTBEAT:
				{
					PROFILING_GUARD(eMCT_ENABLE_HEARTBEAT)
					SMCT_ENABLE_HEARTBEAT* pContext = reinterpret_cast<SMCT_ENABLE_HEARTBEAT*>(sMessagePacket.pData);
					if (pContext == nullptr)
					{
						PrintWarning("context == nullptr type: SMCT_ENABLE_HEARTBEAT");
						continue;
					}

					CCoreConnection* pCoreConnection = this->m_pCoreConnectionMgr->getCoreConnectionByID(pContext->nSocketID);
					if (nullptr != pCoreConnection)
						pCoreConnection->enableHeartbeat(!!pContext->nEnable);
					char* szBuf = reinterpret_cast<char*>(sMessagePacket.pData);
					SAFE_DELETE_ARRAY(szBuf);
				}
				break;

			default:
				PrintWarning("invalid cmd type: %d", sMessagePacket.nType);
			}
		}

		int64_t nEndSamplingTime = base::getProcessPassTime();
		this->m_nTotalSamplingTime = this->m_nTotalSamplingTime + (uint32_t)(nEndSamplingTime - nBeginSamplingTime);

		if (this->m_nTotalSamplingTime / 1000 >= CCoreApp::Inst()->getSamplingTime())
		{
			base::profiling(this->m_nTotalSamplingTime);
			this->m_nTotalSamplingTime = 0;
		}

		return true;
	}
}