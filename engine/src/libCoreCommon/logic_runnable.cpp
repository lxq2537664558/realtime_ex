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
#define _MAIN_STACK_SIZE 1024*1024

namespace core
{
	CLogicRunnable::CLogicRunnable()
		: m_pThreadBase(nullptr)
		, m_pMessageQueue(nullptr)
		, m_insideQueue(_DEFAULT_MESSAGE_QUEUE)
		, m_pBaseConnectionMgr(nullptr)
		, m_pBaseConnectionMgrImpl(nullptr)
		, m_pServiceBaseMgr(nullptr)
		, m_pTransporter(nullptr)
		, m_pServiceRegistryProxy(nullptr)
		, m_pNodeConnectionFactory(nullptr)
	{
		this->m_pMessageQueue = new CLogicMessageQueue();

		this->m_pBaseConnectionMgr = new CBaseConnectionMgr();
		this->m_pBaseConnectionMgrImpl = new CBaseConnectionMgrImpl();
	}

	CLogicRunnable::~CLogicRunnable()
	{
		SAFE_DELETE(this->m_pMessageQueue);
		SAFE_DELETE(this->m_pNodeConnectionFactory);
		SAFE_DELETE(this->m_pServiceBaseMgr);
		SAFE_DELETE(this->m_pServiceRegistryProxy);
		SAFE_DELETE(this->m_pBaseConnectionMgr);
		SAFE_DELETE(this->m_pBaseConnectionMgrImpl);
		SAFE_RELEASE(this->m_pThreadBase);
	}

	bool CLogicRunnable::init(tinyxml2::XMLElement* pRootXML)
	{
		DebugAstEx(pRootXML != nullptr, false);

		tinyxml2::XMLElement* pNodeInfoXML = pRootXML->FirstChildElement("node_info");
		DebugAstEx(pNodeInfoXML != nullptr, false);

		this->m_pNodeConnectionFactory = new CNodeConnectionFactory();
		this->getBaseConnectionMgr()->setBaseConnectionFactory(eBCT_ConnectionToMaster, this->m_pNodeConnectionFactory);
		this->getBaseConnectionMgr()->setBaseConnectionFactory(eBCT_ConnectionFromOtherNode, this->m_pNodeConnectionFactory);
		this->getBaseConnectionMgr()->setBaseConnectionFactory(eBCT_ConnectionToOtherNode, this->m_pNodeConnectionFactory);

		this->m_pTransporter = new CTransporter();

		this->m_pServiceBaseMgr = new CServiceBaseMgr();
		if (!this->m_pServiceBaseMgr->init(pNodeInfoXML))
		{
			PrintWarning("this->m_pServiceBaseMgr->init(pNodeInfoXML)");
			return false;
		}

		this->m_pServiceRegistryProxy = new CServiceRegistryProxy();
		if (!this->m_pServiceRegistryProxy->init(pRootXML))
		{
			PrintWarning("this->m_pCoreOtherNodeProxy->init(pRootXML)");
			return false;
		}

		const SNodeBaseInfo& sNodeBaseInfo = CCoreApp::Inst()->getNodeBaseInfo();

		if (!sNodeBaseInfo.szHost.empty())
			this->m_pBaseConnectionMgr->listen(sNodeBaseInfo.szHost, sNodeBaseInfo.nPort, eBCT_ConnectionFromOtherNode, "", sNodeBaseInfo.nSendBufSize, sNodeBaseInfo.nRecvBufSize, nullptr);

		this->m_pThreadBase = base::CThreadBase::createNew(this);
		return nullptr != this->m_pThreadBase;
	}

	CLogicMessageQueue* CLogicRunnable::getMessageQueue() const
	{
		return this->m_pMessageQueue;
	}

	CBaseConnectionMgr* CLogicRunnable::getBaseConnectionMgr() const
	{
		return this->m_pBaseConnectionMgr;
	}

	CBaseConnectionMgrImpl* CLogicRunnable::getBaseConnectionMgrImpl() const
	{
		return this->m_pBaseConnectionMgrImpl;
	}

	CServiceBaseMgr* CLogicRunnable::getServiceBaseMgr() const
	{
		return this->m_pServiceBaseMgr;
	}

	CTransporter* CLogicRunnable::getTransporter() const
	{
		return this->m_pTransporter;
	}

	CServiceRegistryProxy* CLogicRunnable::getServiceRegistryProxy() const
	{
		return this->m_pServiceRegistryProxy;
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
		coroutine::init(_MAIN_STACK_SIZE);
		if (!this->m_pServiceBaseMgr->onInit())
			return false;

		return CBaseApp::Inst()->onInit();
	}

	void CLogicRunnable::onDestroy()
	{
		CBaseApp::Inst()->onDestroy();
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
				const std::vector<CServiceBaseImpl*>& vecServiceBase = CCoreApp::Inst()->getLogicRunnable()->getServiceBaseMgr()->getServiceBase();
				for (size_t i = 0; i < vecServiceBase.size(); ++i)
				{
					vecServiceBase[i]->quit();
				}
			}
			break;

		case eMCT_FRAME:
			{
				const std::vector<CServiceBaseImpl*>& vecServiceBase = CCoreApp::Inst()->getLogicRunnable()->getServiceBaseMgr()->getServiceBase();

				for (size_t i = 0; i < vecServiceBase.size(); ++i)
				{
					vecServiceBase[i]->run();
				}
				
				bool bQuit = true;
				
				if (CBaseApp::Inst()->onProcess())
					bQuit = false;

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
					CCoreApp::Inst()->getNetRunnable()->quit();
					CCoreApp::Inst()->getTickerRunnable()->quit();

					return false;
				}
			}
			break;

		case eMCT_NOTIFY_SOCKET_CONNECT:
			{
				SMCT_NOTIFY_SOCKET_CONNECT* pContext = reinterpret_cast<SMCT_NOTIFY_SOCKET_CONNECT*>(sMessagePacket.pData);

				if (!this->m_pBaseConnectionMgrImpl->onConnect(pContext->pCoreConnection))
				{
					SMCT_NOTIFY_SOCKET_CONNECT_ACK* pContextAck = new SMCT_NOTIFY_SOCKET_CONNECT_ACK();

					pContextAck->pCoreConnection = pContext->pCoreConnection;
					pContextAck->bSuccess = false;

					SMessagePacket sMessagePacket;
					sMessagePacket.nType = eMCT_NOTIFY_SOCKET_CONNECT_ACK;
					sMessagePacket.pData = pContextAck;
					sMessagePacket.nDataSize = sizeof(SMCT_NOTIFY_SOCKET_CONNECT_ACK);

					CCoreApp::Inst()->getNetRunnable()->getMessageQueue()->send(sMessagePacket);
				}
				SAFE_DELETE(pContext);
			}
			break;

		case eMCT_NOTIFY_SOCKET_DISCONNECT:
			{
				SMCT_NOTIFY_SOCKET_DISCONNECT* pContext = reinterpret_cast<SMCT_NOTIFY_SOCKET_DISCONNECT*>(sMessagePacket.pData);
				
				uint64_t nSocketID = pContext->nSocketID;
				this->m_pBaseConnectionMgrImpl->onDisconnect(nSocketID);
				SAFE_DELETE(pContext);

				SMCT_NOTIFY_SOCKET_DISCONNECT_ACK* pContextAck = new SMCT_NOTIFY_SOCKET_DISCONNECT_ACK();

				pContextAck->nSocketID = nSocketID;
				
				SMessagePacket sMessagePacket;
				sMessagePacket.nType = eMCT_NOTIFY_SOCKET_DISCONNECT_ACK;
				sMessagePacket.pData = pContextAck;
				sMessagePacket.nDataSize = sizeof(SMCT_NOTIFY_SOCKET_DISCONNECT_ACK);

				CCoreApp::Inst()->getNetRunnable()->getMessageQueue()->send(sMessagePacket);
			}
			break;

		case eMCT_NOTIFY_SOCKET_CONNECT_FAIL:
			{
				SMCT_NOTIFY_SOCKET_CONNECT_FAIL* pContext = reinterpret_cast<SMCT_NOTIFY_SOCKET_CONNECT_FAIL*>(sMessagePacket.pData);
				this->m_pBaseConnectionMgrImpl->onConnectFail(pContext->szContext);
				SAFE_DELETE(pContext);
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

				const auto& mapGlobalBeforeFilter = this->m_pBaseConnectionMgrImpl->getGlobalBeforeFilter();
				for (auto iter = mapGlobalBeforeFilter.begin(); iter != mapGlobalBeforeFilter.end(); ++iter)
				{
					const NodeGlobalFilter& callback = iter->second;
					if (!callback(pContext->nSocketID, pContext->pData, pContext->nDataSize))
						return true;
				}

				CBaseConnection* pBaseConnection = this->m_pBaseConnectionMgrImpl->getBaseConnectionByID(pContext->nSocketID);
				if (pBaseConnection == nullptr)
				{
					PrintWarning("pBaseConnection == nullptr type: eMCT_RECV_SOCKET_DATA socket_id: %d", pContext->nSocketID);
					return true;
				}

				pBaseConnection->onDispatch(pContext->nMessageType, pContext->pData, pContext->nDataSize);
				const auto& mapGlobalAfterFilter = this->m_pBaseConnectionMgrImpl->getGlobalAfterFilter();
				for (auto iter = mapGlobalAfterFilter.begin(); iter != mapGlobalAfterFilter.end(); ++iter)
				{
					const NodeGlobalFilter& callback = iter->second;
					if (!callback(pContext->nSocketID, pContext->pData, pContext->nDataSize))
						return true;
				}
			}
			break;

		case eMCT_REQUEST:
		case eMCT_RESPONSE:
		case eMCT_GATE_FORWARD:
			{
				defer([&]()
				{
					char* pBuf = reinterpret_cast<char*>(sMessagePacket.pData);
					SAFE_DELETE_ARRAY(pBuf);
				});
				uint32_t nToServiceID = 0;
				uint8_t nMessageType = 0;
				if (sMessagePacket.nType == eMCT_REQUEST)
				{
					nToServiceID = reinterpret_cast<SMCT_REQUEST*>(sMessagePacket.pData)->nToServiceID;
					nMessageType = eMT_REQUEST;
				}
				else if (sMessagePacket.nType == eMCT_RESPONSE)
				{
					nToServiceID = reinterpret_cast<SMCT_RESPONSE*>(sMessagePacket.pData)->nToServiceID;
					nMessageType = eMT_RESPONSE;
				}
				else if (sMessagePacket.nType == eMCT_GATE_FORWARD)
				{
					nToServiceID = reinterpret_cast<SMCT_GATE_FORWARD*>(sMessagePacket.pData)->nToServiceID;
					nMessageType = eMT_GATE_FORWARD;
				}
				CServiceBaseImpl* pServiceBaseImpl = CCoreApp::Inst()->getLogicRunnable()->getServiceBaseMgr()->getServiceBaseByID(nToServiceID);
				if (pServiceBaseImpl == nullptr)
				{
					PrintWarning("pServiceBaseImpl == nullptr type: eMCT_RECV_SOCKET_DATA");
					return true;
				}

				pServiceBaseImpl->getMessageDispatcher()->dispatch(CCoreApp::Inst()->getNodeID(), nMessageType, sMessagePacket.pData);
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
					CServiceBaseImpl* pServiceBaseImpl = CCoreApp::Inst()->getLogicRunnable()->getServiceBaseMgr()->getServiceBaseByID(pTicker->getServiceID());
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