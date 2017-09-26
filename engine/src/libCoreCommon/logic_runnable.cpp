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
#include "core_service.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/time_util.h"
#include "libBaseCommon/profiling.h"
#include "libBaseCommon/process_util.h"

#include <algorithm>

#define _DEFAULT_MESSAGE_QUEUE 1024
#define _MAIN_STACK_SIZE 1024*1024

namespace core
{
	CLogicRunnable::CLogicRunnable()
		: m_pThreadBase(nullptr)
		, m_pBaseConnectionMgr(nullptr)
		, m_pBaseConnectionMgrImpl(nullptr)
		, m_pMessageQueue(nullptr)
		, m_pCoreServiceMgr(nullptr)
		, m_pTransporter(nullptr)
		, m_pServiceRegistryProxy(nullptr)
		, m_pNodeConnectionFactory(nullptr)
		, m_insideMessageQueue(_DEFAULT_MESSAGE_QUEUE)
		, m_nTotalSamplingTime(0)
	{
	}

	CLogicRunnable::~CLogicRunnable()
	{
		SAFE_RELEASE(this->m_pThreadBase);
	}

	bool CLogicRunnable::init(CLogicMessageQueue* pMessageQueue, const std::vector<CServiceBase*>& vecServiceBase, tinyxml2::XMLElement* pRootXML)
	{
		DebugAstEx(pRootXML != nullptr && pMessageQueue != nullptr, false);

		this->m_pMessageQueue = pMessageQueue;
		
		this->m_pBaseConnectionMgr = new CBaseConnectionMgr();
		this->m_pBaseConnectionMgrImpl = new CBaseConnectionMgrImpl();
		
		this->m_pNodeConnectionFactory = new CNodeConnectionFactory();
		this->getBaseConnectionMgr()->setBaseConnectionFactory("CBaseConnectionToMaster", this->m_pNodeConnectionFactory);
		this->getBaseConnectionMgr()->setBaseConnectionFactory("CBaseConnectionOtherNode", this->m_pNodeConnectionFactory);

		this->m_pTransporter = new CTransporter();

		this->m_pCoreServiceMgr = new CCoreServiceMgr();
		if (!this->m_pCoreServiceMgr->init(vecServiceBase))
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
		{
			if (!this->m_pBaseConnectionMgr->listen(sNodeBaseInfo.szHost, sNodeBaseInfo.nPort, false, "CBaseConnectionOtherNode", "", sNodeBaseInfo.nSendBufSize, sNodeBaseInfo.nRecvBufSize, nullptr))
			{
				PrintWarning("node listen error");
				return false;
			}
		}

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

	CCoreServiceMgr* CLogicRunnable::getCoreServiceMgr() const
	{
		return this->m_pCoreServiceMgr;
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

		this->m_insideMessageQueue.send(sMessagePacket);
	}

	void CLogicRunnable::recvInsideMessage(std::vector<SMessagePacket>& vecMessagePacket)
	{
		PROFILING_GUARD(CLogicRunnable::recvInsideMessage);
		vecMessagePacket.clear();

		SMessagePacket sMessagePacket;
		while (this->m_insideMessageQueue.recv(sMessagePacket))
		{
			vecMessagePacket.push_back(sMessagePacket);
		}
	}

	bool CLogicRunnable::onInit()
	{
		coroutine::init(_MAIN_STACK_SIZE);

		if (!this->m_pCoreServiceMgr->onInit())
			return false;

		if (!CBaseApp::Inst()->onInit())
			return false;

		this->printNodeInfo();

		return true;
	}

	void CLogicRunnable::onDestroy()
	{
		CBaseApp::Inst()->onDestroy();

		SAFE_DELETE(this->m_pBaseConnectionMgr);
		SAFE_DELETE(this->m_pBaseConnectionMgrImpl);
		SAFE_DELETE(this->m_pCoreServiceMgr);
		SAFE_DELETE(this->m_pTransporter);
		SAFE_DELETE(this->m_pServiceRegistryProxy);
		SAFE_DELETE(this->m_pNodeConnectionFactory);
	}

	bool CLogicRunnable::onProcess()
	{
		int64_t nBeginSamplingTime = base::time_util::getProcessPassTime();

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

		// ÷¥––actorœ˚œ¢
		const std::vector<CCoreService*>& vecCoreService = CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->getCoreService();
		for (size_t i = 0; i < vecCoreService.size(); ++i)
		{
			vecCoreService[i]->getActorScheduler()->dispatch();
		}

		int64_t nEndSamplingTime = base::time_util::getProcessPassTime();
		this->m_nTotalSamplingTime = this->m_nTotalSamplingTime + (uint32_t)(nEndSamplingTime - nBeginSamplingTime);

		if (this->m_nTotalSamplingTime / 1000 >= CCoreApp::Inst()->getSamplingTime())
		{
			base::profiling::update(this->m_nTotalSamplingTime);
			this->m_nTotalSamplingTime = 0;
		}

		return true;
	}

	bool CLogicRunnable::dispatch(const SMessagePacket& sMessagePacket)
	{
		switch (sMessagePacket.nType)
		{
		case eMCT_QUIT:
		{
			const std::vector<CCoreService*>& vecCoreService = CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->getCoreService();
			for (size_t i = 0; i < vecCoreService.size(); ++i)
			{
				vecCoreService[i]->quit();
			}
		}
		break;

		case eMCT_FRAME:
		{
			const std::vector<CCoreService*>& vecCoreService = CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->getCoreService();

			for (size_t i = 0; i < vecCoreService.size(); ++i)
			{
				if (vecCoreService[i]->getRunState() == eSRS_Normal)
					vecCoreService[i]->onFrame();
			}

			CBaseApp::Inst()->onProcess();

			bool bQuit = true;

			for (size_t i = 0; i < vecCoreService.size(); ++i)
			{
				if (vecCoreService[i]->getRunState() != eSRS_Quit)
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

			CBaseConnection* pBaseConnection = this->m_pBaseConnectionMgrImpl->getBaseConnectionBySocketID(pContext->nSocketID);
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
			else if (sMessagePacket.nType == eMCT_TO_GATE)
			{
				nToServiceID = reinterpret_cast<SMCT_TO_GATE*>(sMessagePacket.pData)->nToServiceID;
				nMessageType = eMT_TO_GATE;
			}
			else if (sMessagePacket.nType == eMCT_TO_GATE_BROADCAST)
			{
				nToServiceID = reinterpret_cast<SMCT_TO_GATE_BROADCAST*>(sMessagePacket.pData)->nToServiceID;
				nMessageType = eMT_TO_GATE_BROADCAST;
			}
			else if (sMessagePacket.nType == eMCT_GATE_FORWARD)
			{
				nToServiceID = reinterpret_cast<SMCT_GATE_FORWARD*>(sMessagePacket.pData)->nToServiceID;
				nMessageType = eMT_GATE_FORWARD;
			}
			CCoreService* pCoreService = CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->getCoreService(nToServiceID);
			if (pCoreService == nullptr)
			{
				PrintWarning("pCoreService == nullptr type: eMCT_RECV_SOCKET_DATA");
				return true;
			}

			pCoreService->getMessageDispatcher()->dispatch(CCoreApp::Inst()->getNodeID(), nMessageType, sMessagePacket.pData);
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
				if (pCoreTickerNode->Value.getRef() == 1)
				{
					CCoreApp::Inst()->unregisterTicker(pTicker);
				}
				auto& callback = pTicker->getCallback();
				if (callback != nullptr)
					callback(pTicker->getContext());
				pCoreTickerNode->Value.release();
			}
			else
			{
				CTicker* pTicker = pCoreTickerNode->Value.m_pTicker;
				CCoreService* pCoreService = CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->getCoreService(pTicker->getServiceID());
				if (pCoreService == nullptr)
				{
					PrintWarning("pCoreService == nullptr type: eMCT_TICKER");
					pCoreTickerNode->Value.release();
					return true;
				}

				CCoreActor* pCoreActor = pCoreService->getActorScheduler()->getCoreActor(pTicker->getActorID());
				if (pCoreActor == nullptr)
				{
					PrintWarning("pCoreActor == nullptr type: eMCT_TICKER");
					pCoreTickerNode->Value.release();
					return true;
				}

				pCoreActor->addTicker(pCoreTickerNode);

				pCoreService->getActorScheduler()->addWorkCoreActor(pCoreActor);
			}
		}
		break;

		default:
		{
			PrintWarning("invalid type: {}", sMessagePacket.nType);
		}
		}

		return true;
	}

	void CLogicRunnable::printNodeInfo()
	{
		const SNodeBaseInfo& sNodeBaseInfo = CCoreApp::Inst()->getNodeBaseInfo();
		const std::vector<SServiceBaseInfo>& vecServiceBaseInfo = this->m_pCoreServiceMgr->getServiceBaseInfo();

		std::stringstream ss;
		ss << std::endl;
		ss << "############################################################" << std::endl;
		ss << "\tnode_name: " << sNodeBaseInfo.szName << " node_id: " << sNodeBaseInfo.nID << std::endl;
		ss << std::endl;
		for (size_t i = 0; i < vecServiceBaseInfo.size(); ++i)
		{
			const SServiceBaseInfo& sServiceBaseInfo = vecServiceBaseInfo[i];
			ss << "\tservice_name: " << sServiceBaseInfo.szName << "\tservice_type: " << sServiceBaseInfo.szType << " \tservice_id: " << sServiceBaseInfo.nID << std::endl;
		}
		ss << std::endl;
		ss << "\twork_path: " << base::process_util::getCurrentWorkPath() << std::endl;
		ss << std::endl;
		ss << "\tlog_path: " << base::log::getPath() << std::endl;
		ss << std::endl;
		ss << "\tlog_async: " << base::log::isAsync() << std::endl;
		ss << std::endl;
		ss << "\tlog_debug: " << base::log::isDebug() << std::endl;
		ss << std::endl;
		ss << "\tprofiling: " << CCoreApp::Inst()->getSamplingTime() << std::endl;
		ss << "############################################################";

		base::log::save("INFO", true, "%s", ss.str().c_str());
	}
}