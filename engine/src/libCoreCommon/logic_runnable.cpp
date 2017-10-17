#include "stdafx.h"
#include "logic_runnable.h"
#include "core_common.h"
#include "message_command.h"
#include "base_connection_mgr.h"
#include "base_connection.h"
#include "ticker_runnable.h"
#include "net_runnable.h"
#include "core_app.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/time_util.h"
#include "libBaseCommon/profiling.h"
#include "libBaseCommon/process_util.h"

#include <algorithm>

#define _MAIN_STACK_SIZE 1024*1024

namespace
{
	core::CBaseConnectionMgrImpl* getBaseConnectionMgrImpl(core::CCoreService* pCoreService)
	{
		core::CBaseConnectionMgrImpl* pBaseConnectionMgrImpl = nullptr;
		if (pCoreService != nullptr)
			pBaseConnectionMgrImpl = pCoreService->getBaseConnectionMgr()->getBaseConnectionMgrImpl();
		else
			pBaseConnectionMgrImpl = core::CCoreApp::Inst()->getBaseConnectionMgr()->getBaseConnectionMgrImpl();

		return pBaseConnectionMgrImpl;
	}
}

namespace core
{
	CLogicRunnable::CLogicRunnable()
		: m_pThreadBase(nullptr)
		, m_nTotalSamplingTime(0)
	{
	}

	CLogicRunnable::~CLogicRunnable()
	{
		SAFE_RELEASE(this->m_pThreadBase);
	}

	bool CLogicRunnable::init()
	{
		this->m_pThreadBase = base::CThreadBase::createNew(this);
		return nullptr != this->m_pThreadBase;
	}

	bool CLogicRunnable::onInit()
	{
		coroutine::init(_MAIN_STACK_SIZE);
		
		return true;
	}

	void CLogicRunnable::onDestroy()
	{
	}

	bool CLogicRunnable::onProcess()
	{
		int64_t nBeginSamplingTime = base::time_util::getProcessPassTime();

		CLogicMessageQueue* pMessageQueue = CCoreApp::Inst()->getLogicMessageQueueMgr()->getMessageQueue();
		DebugAstEx(pMessageQueue != nullptr, true);

		pMessageQueue->recv(this->m_vecMessagePacket);

		CCoreService* pCoreService = pMessageQueue->getCoreService();
		
		PROFILING_BEGIN(Message_Process)
		for (auto iter = this->m_vecMessagePacket.begin(); iter != this->m_vecMessagePacket.end(); ++iter)
		{
			const SMessagePacket& sMessagePacket = *iter;

			this->dispatch(pCoreService, sMessagePacket);
		}
		PROFILING_END(Message_Process)

		pMessageQueue->dispatchEnd();
		coroutine::update();

		int64_t nEndSamplingTime = base::time_util::getProcessPassTime();
		this->m_nTotalSamplingTime = this->m_nTotalSamplingTime + (uint32_t)(nEndSamplingTime - nBeginSamplingTime);

		if (this->m_nTotalSamplingTime / 1000 >= CCoreApp::Inst()->getSamplingTime())
		{
			PROFILING_UPDATE(this->m_nTotalSamplingTime);
			this->m_nTotalSamplingTime = 0;
		}
		return true;
	}

	bool CLogicRunnable::dispatch(CCoreService* pCoreService, const SMessagePacket& sMessagePacket)
	{
		switch (sMessagePacket.nType)
		{
		case eMCT_QUIT:
		{
			DebugAstEx(pCoreService != nullptr, false);

			pCoreService->quit();
		}
		break;

		case eMCT_FRAME:
		{
			PROFILING_GUARD(eMCT_FRAME)

			DebugAstEx(pCoreService != nullptr, false);
			
			if (pCoreService->getRunState() == eSRS_Normal)
				pCoreService->onFrame();
		}
		break;

		case eMCT_NOTIFY_SOCKET_CONNECT:
		{
			PROFILING_GUARD(eMCT_NOTIFY_SOCKET_CONNECT)

			SMCT_NOTIFY_SOCKET_CONNECT* pContext = reinterpret_cast<SMCT_NOTIFY_SOCKET_CONNECT*>(sMessagePacket.pData);
			defer([&]()
			{
				SAFE_DELETE(pContext);
			});

			CBaseConnectionMgrImpl* pBaseConnectionMgrImpl = ::getBaseConnectionMgrImpl(pCoreService);
			DebugAstEx(pBaseConnectionMgrImpl != nullptr, true);

			if (!pBaseConnectionMgrImpl->onConnect(pContext->pCoreConnection))
			{
				SMCT_NOTIFY_SOCKET_CONNECT_ACK* pContextAck = new SMCT_NOTIFY_SOCKET_CONNECT_ACK();

				pContextAck->pCoreConnection = pContext->pCoreConnection;
				pContextAck->bSuccess = false;

				SMessagePacket sMessagePacket;
				sMessagePacket.nType = eMCT_NOTIFY_SOCKET_CONNECT_ACK;
				sMessagePacket.pData = pContextAck;
				sMessagePacket.nDataSize = sizeof(SMCT_NOTIFY_SOCKET_CONNECT_ACK);

				CNetRunnable::Inst()->getMessageQueue()->send(sMessagePacket);
			}
		}
		break;

		case eMCT_NOTIFY_SOCKET_DISCONNECT:
		{
			PROFILING_GUARD(eMCT_NOTIFY_SOCKET_DISCONNECT)

			SMCT_NOTIFY_SOCKET_DISCONNECT* pContext = reinterpret_cast<SMCT_NOTIFY_SOCKET_DISCONNECT*>(sMessagePacket.pData);
			defer([&]()
			{
				SAFE_DELETE(pContext);
			});

			CBaseConnectionMgrImpl* pBaseConnectionMgrImpl = ::getBaseConnectionMgrImpl(pCoreService);
			DebugAstEx(pBaseConnectionMgrImpl != nullptr, true);

			uint64_t nSocketID = pContext->nSocketID;
			pBaseConnectionMgrImpl->onDisconnect(nSocketID);
			
			SMCT_NOTIFY_SOCKET_DISCONNECT_ACK* pContextAck = new SMCT_NOTIFY_SOCKET_DISCONNECT_ACK();

			pContextAck->nSocketID = nSocketID;

			SMessagePacket sMessagePacket;
			sMessagePacket.nType = eMCT_NOTIFY_SOCKET_DISCONNECT_ACK;
			sMessagePacket.pData = pContextAck;
			sMessagePacket.nDataSize = sizeof(SMCT_NOTIFY_SOCKET_DISCONNECT_ACK);

			CNetRunnable::Inst()->getMessageQueue()->send(sMessagePacket);
		}
		break;

		case eMCT_NOTIFY_SOCKET_CONNECT_FAIL:
		{
			PROFILING_GUARD(eMCT_NOTIFY_SOCKET_CONNECT_FAIL)

			SMCT_NOTIFY_SOCKET_CONNECT_FAIL* pContext = reinterpret_cast<SMCT_NOTIFY_SOCKET_CONNECT_FAIL*>(sMessagePacket.pData);
			defer([&]()
			{
				SAFE_DELETE(pContext);
			});

			CBaseConnectionMgrImpl* pBaseConnectionMgrImpl = ::getBaseConnectionMgrImpl(pCoreService);
			DebugAstEx(pBaseConnectionMgrImpl != nullptr, true);

			pBaseConnectionMgrImpl->onConnectFail(pContext->szContext);
		}
		break;

		case eMCT_RECV_SOCKET_DATA:
		{
			PROFILING_GUARD(eMCT_RECV_SOCKET_DATA)
				
			SMCT_RECV_SOCKET_DATA* pContext = reinterpret_cast<SMCT_RECV_SOCKET_DATA*>(sMessagePacket.pData);

			defer([&]()
			{
				char* pBuf = reinterpret_cast<char*>(sMessagePacket.pData);
				SAFE_DELETE_ARRAY(pBuf);
			});

			CBaseConnectionMgrImpl* pBaseConnectionMgrImpl = ::getBaseConnectionMgrImpl(pCoreService);
			DebugAstEx(pBaseConnectionMgrImpl != nullptr, true);

			CBaseConnection* pBaseConnection = pBaseConnectionMgrImpl->getBaseConnectionBySocketID(pContext->nSocketID);
			if (pBaseConnection == nullptr)
			{
				PrintWarning("pBaseConnection == nullptr type: eMCT_RECV_SOCKET_DATA socket_id: {}", pContext->nSocketID);
				return true;
			}

			pBaseConnection->onDispatch(pContext->nMessageType, pContext->pData, pContext->nDataSize);
		}
		break;

		case eMCT_REQUEST:
		case eMCT_RESPONSE:
		case eMCT_GATE_FORWARD:
		case eMCT_TO_GATE:
		case eMCT_TO_GATE_BROADCAST:
		{
			defer([&]()
			{
				char* pBuf = reinterpret_cast<char*>(sMessagePacket.pData);
				SAFE_DELETE_ARRAY(pBuf);
			});

			DebugAstEx(pCoreService != nullptr, true);

			uint8_t nMessageType = 0;
			if (sMessagePacket.nType == eMCT_REQUEST)
				nMessageType = eMT_REQUEST;
			else if (sMessagePacket.nType == eMCT_RESPONSE)
				nMessageType = eMT_RESPONSE;
			else if (sMessagePacket.nType == eMCT_TO_GATE)
				nMessageType = eMT_TO_GATE;
			else if (sMessagePacket.nType == eMCT_TO_GATE_BROADCAST)
				nMessageType = eMT_TO_GATE_BROADCAST;
			else if (sMessagePacket.nType == eMCT_GATE_FORWARD)
				nMessageType = eMT_GATE_FORWARD;

			pCoreService->getMessageDispatcher()->dispatch(CCoreApp::Inst()->getNodeID(), nMessageType, sMessagePacket.pData);
		}
		break;

		case eMCT_TICKER:
		{
			PROFILING_GUARD(eMCT_TICKER_LOGIC)
				
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

			CTicker* pTicker = pCoreTickerNode->Value.m_pTicker;
			if (pCoreTickerNode->Value.getRef() == 1)
				CCoreApp::Inst()->unregisterTicker(pTicker);

			auto& callback = pTicker->getCallback();
			if (callback != nullptr)
			{
				uint64_t nContext = pTicker->getContext();
				uint64_t nCoroutineID = coroutine::create(CCoreApp::Inst()->getCoroutineStackSize(), [&callback, nContext](uint64_t) { callback(nContext); });
				coroutine::resume(nCoroutineID, 0);
			}

			pCoreTickerNode->Value.release();
		}
		break;

		default:
		{
			PrintWarning("invalid type: {}", sMessagePacket.nType);
		}
		}

		return true;
	}
}