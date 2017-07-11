#include "stdafx.h"
#include "logic_runnable.h"
#include "core_common.h"
#include "message_command.h"
#include "core_app.h"
#include "base_connection_mgr.h"
#include "base_connection.h"
#include "ticker_runnable.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/base_time.h"
#include "libBaseCommon/profiling.h"

#include <algorithm>
#include "net_runnable.h"

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

			switch (sMessagePacket.nType)
			{
			case eMCT_INSIDE_MESSAGE:
				{
					SMCT_INSIDE_MESSAGE* pContext = reinterpret_cast<SMCT_INSIDE_MESSAGE*>(sMessagePacket.pData);
					if (pContext == nullptr)
					{
						PrintWarning("context == nullptr type: eMCT_INSIDE_MESSAGE");
						continue;
					}
					if (pContext->pData == nullptr)
					{
						PrintWarning("pContext->pData == nullptr type: eMCT_INSIDE_MESSAGE");
						continue;
					}
					
					CCoreApp::Inst()->getMessageDispatcher()->dispatch(0, CCoreApp::Inst()->getNodeBaseInfo().nID, pContext->nMessageType, pContext->pData, pContext->nDataSize);
					char* pBuf = reinterpret_cast<char*>(sMessagePacket.pData);
					SAFE_DELETE_ARRAY(pBuf);
				}
				break;

			default:
				{
					PrintWarning("invalid type: %d", sMessagePacket.nType);
				}
			}
		}

		this->m_pMessageQueue->recv(vecMessagePacket,  true);

		for (auto iter = vecMessagePacket.begin(); iter != vecMessagePacket.end(); ++iter)
		{
			const SMessagePacket& sMessagePacket = *iter;

			switch (sMessagePacket.nType)
			{
			case eMCT_QUIT:
				{
					const std::vector<CServiceBase*>& vecServiceBase = CCoreApp::Inst()->getServiceBase();
					if (vecServiceBase.empty())
						return false;

					for (size_t i = 0; i < vecServiceBase.size(); ++i)
					{
						if (vecServiceBase[i]->m_eState != eSRS_Normal)
							continue;

						vecServiceBase[i]->m_eState = eSRS_Quitting;
						vecServiceBase[i]->onQuit();
					}
				}
				break;

			case eMCT_FRAME:
				{
					const std::vector<CServiceBase*>& vecServiceBase = CCoreApp::Inst()->getServiceBase();
					
					for (size_t i = 0; i < vecServiceBase.size(); ++i)
					{
						  vecServiceBase[i]->onFrame();
					}

					bool bQuit = true;
					for (size_t i = 0; i < vecServiceBase.size(); ++i)
					{
						if (vecServiceBase[i]->getState() != eSRS_Quit)
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
						continue;
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
						continue;
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
						continue;
					}
					this->m_pBaseConnectionMgr->onConnectFail(pContext->szContext);
					SAFE_DELETE(pContext);
				}
				break;

			case eMCT_RECV_SOCKET_DATA:
				{
					SMCT_RECV_SOCKET_DATA* pContext = reinterpret_cast<SMCT_RECV_SOCKET_DATA*>(sMessagePacket.pData);
					if (pContext == nullptr)
					{
						PrintWarning("context == nullptr type: eMCT_RECV_SOCKET_DATA");
						continue;
					}
					CBaseConnection* pBaseConnection = this->m_pBaseConnectionMgr->getBaseConnectionByID(pContext->nSocketID);
					if (pBaseConnection == nullptr)
					{
						PrintWarning("pContext->pBaseConnection == nullptr type: eMCT_RECV_SOCKET_DATA socket_id: %d", pContext->nSocketID);
						continue;
					}
					pBaseConnection->onDispatch(pContext->nMessageType, pContext->pData, pContext->nDataSize);
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
						continue;
					}

					if (pCoreTickerNode->Value.m_pTicker == nullptr)
					{
						pCoreTickerNode->Value.release();
						continue;
					}

					CTicker* pTicker = pCoreTickerNode->Value.m_pTicker;
					pTicker->getCallback()(pTicker->getContext());
					pCoreTickerNode->Value.release();
				}
				break;

			default:
				{
					PrintWarning("invalid type: %d", sMessagePacket.nType);
				}
			}
		}

// 		PROFILING_BEGIN(CBaseApp::Inst()->onProcess)
// 			CBaseApp::Inst()->onProcess();
// 		PROFILING_END(CBaseApp::Inst()->onProcess)
// 
// 			if (this->m_nRunState == eARS_Quitting && !this->m_bMarkQuit)
// 			{
// 				this->m_bMarkQuit = true;
// 
// 				PrintInfo("CCoreApp::onQuit");
// 
// 				base::flushLog();
// 				CBaseApp::Inst()->onQuit();
// 			}
// 		if (this->m_nRunState == eARS_Quit)
// 			return false;
// 
// 		int64_t nEndTime = base::getProcessPassTime();
// 		this->m_nTotalSamplingTime = this->m_nTotalSamplingTime + (uint32_t)(nEndTime - nBeginTime);
// 
// 		if (this->m_nTotalSamplingTime / 1000 >= this->m_nSamplingTime)
// 		{
// 			base::profiling(this->m_nTotalSamplingTime);
// 			this->m_nTotalSamplingTime = 0;
// 		}

		return true;
	}
}