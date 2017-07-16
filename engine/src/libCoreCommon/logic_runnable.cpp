#include "stdafx.h"
#include "logic_runnable.h"
#include "core_common.h"
#include "message_command.h"
#include "core_app.h"
#include "base_connection_mgr.h"
#include "base_connection.h"
#include "ticker_runnable.h"
#include "net_runnable.h"
#include "service_base.h"
#include "service_base_impl.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/base_time.h"
#include "libBaseCommon/profiling.h"

#include <algorithm>

// 放这里为了调试或者看dump的时候方便
core::CLogicRunnable*	g_pLogicRunnable;

#define _DEFAULT_MESSAGE_QUEUE 1024

namespace core
{
	CLogicRunnable::CLogicRunnable()
		: m_pThreadBase(nullptr)
		, m_pMessageQueue(nullptr)
		, m_insideQueue(_DEFAULT_MESSAGE_QUEUE)
		, m_pBaseConnectionMgr(nullptr)
	{
	}

	CLogicRunnable::~CLogicRunnable()
	{
		SAFE_DELETE(this->m_pMessageQueue);
		SAFE_DELETE(this->m_pBaseConnectionMgr);
		SAFE_RELEASE(this->m_pThreadBase);
	}

	CLogicRunnable* CLogicRunnable::Inst()
	{
		if (g_pLogicRunnable == nullptr)
			g_pLogicRunnable = new CLogicRunnable();

		return g_pLogicRunnable;
	}

	bool CLogicRunnable::init()
	{
		this->m_pMessageQueue = new CLogicMessageQueue();
		if (!this->m_pMessageQueue->init())
			return false;

		this->m_pBaseConnectionMgr = new CBaseConnectionMgr();
		if (!this->m_pBaseConnectionMgr->init())
			return false;

		this->m_pThreadBase = base::CThreadBase::createNew(this);
		return nullptr != this->m_pThreadBase;
	}

	void CLogicRunnable::release()
	{
		delete g_pLogicRunnable;
		g_pLogicRunnable = nullptr;
	}

	CLogicMessageQueue* CLogicRunnable::getMessageQueue() const
	{
		return this->m_pMessageQueue;
	}

	CBaseConnectionMgr* CLogicRunnable::getBaseConnectionMgr() const
	{
		return this->m_pBaseConnectionMgr;
	}

	void CLogicRunnable::sendInsideMessage(const SMessagePacket& sMessagePacket)
	{
		PROFILING_GUARD(CLogicRunnable::sendInsideMessage);

		this->m_insideQueue.send(sMessagePacket);
	}

	void CLogicRunnable::recvInsideMessage(std::vector<SMessagePacket>& vecMessagePacket)
	{
		PROFILING_GUARD(CLogicRunnable::recvInsideMessage);
		vecMessagePacket.clear();

		SMessagePacket sMessagePacket;
		while (this->m_insideQueue.recv(sMessagePacket))
		{
			vecMessagePacket.push_back(sMessagePacket);
		}
	}

	bool CLogicRunnable::onInit()
	{
		return true;
	}

	void CLogicRunnable::onDestroy()
	{

	}

	bool CLogicRunnable::onProcess()
	{
		int64_t nBeginTime = base::getProcessPassTime();

		static std::vector<SMessagePacket> vecMessagePacket;

		this->recvInsideMessage(vecMessagePacket);

		for (auto iter = vecMessagePacket.begin(); iter != vecMessagePacket.end(); ++iter)
		{
			const SMessagePacket& sMessagePacket = *iter;

			this->dispatch(sMessagePacket);
		}

		this->m_pMessageQueue->recv(vecMessagePacket);

		for (auto iter = vecMessagePacket.begin(); iter != vecMessagePacket.end(); ++iter)
		{
			const SMessagePacket& sMessagePacket = *iter;

			if (!this->dispatch(sMessagePacket))
				return false;
		}

		return true;
	}

	bool CLogicRunnable::dispatch(const SMessagePacket& sMessagePacket)
	{
		switch (sMessagePacket.nType)
		{
		case eMCT_QUIT:
			{
				const std::vector<CServiceBaseImpl*>& vecServiceBase = CCoreApp::Inst()->getServiceBaseMgr()->getServiceBase();
				if (vecServiceBase.empty())
					return false;

				for (size_t i = 0; i < vecServiceBase.size(); ++i)
				{
					vecServiceBase[i]->quit();
				}
			}
			break;

		case eMCT_FRAME:
			{
				const std::vector<CServiceBaseImpl*>& vecServiceBase = CCoreApp::Inst()->getServiceBaseMgr()->getServiceBase();

				for (size_t i = 0; i < vecServiceBase.size(); ++i)
				{
					vecServiceBase[i]->run();
				}

				bool bQuit = true;
				for (size_t i = 0; i < vecServiceBase.size(); ++i)
				{
					if (vecServiceBase[i]->getRunState() != eSRS_Quit)
					{
						bQuit = false;
						break;
					}
				}
				if (bQuit)
				{
					CNetRunnable::Inst()->quit();
					CTickerRunnable::Inst()->quit();

					return false;
				}
			}
			break;

		case eMCT_NOTIFY_SOCKET_CONNECT:
			{
				SMCT_NOTIFY_SOCKET_CONNECT* pContext = reinterpret_cast<SMCT_NOTIFY_SOCKET_CONNECT*>(sMessagePacket.pData);
				if (pContext == nullptr)
				{
					PrintWarning("context == nullptr type: eMCT_NOTIFY_SOCKET_CONNECT");
					return true;
				}
				this->m_pBaseConnectionMgr->onConnect(pContext->nSocketID, pContext->szContext, pContext->nType, pContext->sLocalAddr, pContext->sRemoteAddr);
				SAFE_DELETE(pContext);
			}
			break;

		case eMCT_NOTIFY_SOCKET_DISCONNECT:
			{
				SMCT_NOTIFY_SOCKET_DISCONNECT* pContext = reinterpret_cast<SMCT_NOTIFY_SOCKET_DISCONNECT*>(sMessagePacket.pData);
				if (pContext == nullptr)
				{
					PrintWarning("context == nullptr type: eMPT_NOTIFY_SOCKET_DISCONNECT");
					return true;
				}
				this->m_pBaseConnectionMgr->onDisconnect(pContext->nSocketID);
				SAFE_DELETE(pContext);
			}
			break;

		case eMCT_NOTIFY_SOCKET_CONNECT_FAIL:
			{
				SMCT_NOTIFY_SOCKET_CONNECT_FAIL* pContext = reinterpret_cast<SMCT_NOTIFY_SOCKET_CONNECT_FAIL*>(sMessagePacket.pData);
				if (pContext == nullptr)
				{
					PrintWarning("context == nullptr type: SMCT_NOTIFY_SOCKET_CONNECT_FAIL");
					return true;
				}
				this->m_pBaseConnectionMgr->onConnectFail(pContext->szContext);
				SAFE_DELETE(pContext);
			}
			break;

		case eMCT_RECV_SOCKET_DATA:
			{
				PROFILING_GUARD(eMCT_RECV_SOCKET_DATA)
				SMCT_RECV_SOCKET_DATA* pContext = reinterpret_cast<SMCT_RECV_SOCKET_DATA*>(sMessagePacket.pData);
				if (pContext == nullptr)
				{
					PrintWarning("context == nullptr type: eMCT_RECV_SOCKET_DATA");
					return true;
				}
				CBaseConnection* pBaseConnection = this->m_pBaseConnectionMgr->getBaseConnectionByID(pContext->nSocketID);
				if (pBaseConnection == nullptr)
				{
					PrintWarning("pContext->pBaseConnection == nullptr type: eMCT_RECV_SOCKET_DATA socket_id: %d", pContext->nSocketID);
					return true;
				}

				if (pContext->nMessageType != eMT_SYSTEM)
				{
					if (pBaseConnection->getType() == eBCT_ConnectionOtherNode)
					{
						CBaseConnectionOtherNode* pBaseConnectionOtherNode = dynamic_cast<CBaseConnectionOtherNode*>(pBaseConnection);
						if (nullptr == pBaseConnectionOtherNode)
						{
							pBaseConnection->shutdown(base::eNCCT_Force, "invalid connection");
							return true;
						}
						if (pBaseConnectionOtherNode->getNodeID() == 0)
						{
							pBaseConnection->shutdown(base::eNCCT_Force, "invalid connection");
							return true;
						}
					}

					CServiceBaseImpl* pServiceBaseImpl = CCoreApp::Inst()->getServiceBaseMgr()->getServiceBase(pContext->nToServiceID);
					if (pServiceBaseImpl == nullptr)
					{
						PrintWarning("pServiceBaseImpl == nullptr type: eMCT_RECV_SOCKET_DATA");
						return true;
					}

					pServiceBaseImpl->getMessageDispatcher()->dispatch(0, CCoreApp::Inst()->getNodeID(), pContext);
				}
				else
				{
					pBaseConnection->onDispatch(pContext->nMessageType, pContext->pData, pContext->nDataSize, pContext);
				}
				char* pBuf = reinterpret_cast<char*>(sMessagePacket.pData);
				SAFE_DELETE_ARRAY(pBuf);
			}
			break;

		case eMCT_TICKER:
			{
				PROFILING_GUARD(eMCT_TICKER)
				CCoreTickerNode* pCoreTickerNode = reinterpret_cast<CCoreTickerNode*>(sMessagePacket.pData);
				if (pCoreTickerNode == nullptr)
				{
					PrintWarning("pCoreTickerNode == nullptr type: eMCT_TICKER");
					return true;
				}

				if (pCoreTickerNode->Value.m_pTicker == nullptr)
				{
					pCoreTickerNode->Value.release();
					return true;
				}

				if (pCoreTickerNode->Value.m_nType == CTicker::eTT_Service)
				{
					CTicker* pTicker = pCoreTickerNode->Value.m_pTicker;
					pTicker->getCallback()(pTicker->getContext());
					pCoreTickerNode->Value.release();
				}
				else
				{
					CTicker* pTicker = pCoreTickerNode->Value.m_pTicker;
					CServiceBaseImpl* pServiceBaseImpl = CCoreApp::Inst()->getServiceBaseMgr()->getServiceBase(pTicker->getServiceID());
					if (pServiceBaseImpl == nullptr)
					{
						PrintWarning("pServiceBaseImpl == nullptr type: eMCT_TICKER");
						pCoreTickerNode->Value.release();
						return true;
					}

					CActorBaseImpl* pActorBaseImpl = pServiceBaseImpl->getActorScheduler()->getActorBase(pTicker->getActorID());
					if (pActorBaseImpl == nullptr)
					{
						PrintWarning("pActorBaseImpl == nullptr type: eMCT_TICKER");
						pCoreTickerNode->Value.release();
						return true;
					}

					SActorMessagePacket sActorMessagePacket;
					sActorMessagePacket.nData = 0;
					sActorMessagePacket.nSessionID = uint64_t(pCoreTickerNode);
					sActorMessagePacket.nType = eMT_TICKER;
					sActorMessagePacket.pMessage = nullptr;
					pActorBaseImpl->getChannel()->send(sActorMessagePacket);

					pServiceBaseImpl->getActorScheduler()->addWorkActorBase(pActorBaseImpl);
				}
			}
			break;

		default:
			{
				PrintWarning("invalid type: %d", sMessagePacket.nType);
			}
		}

		return true;
	}

}