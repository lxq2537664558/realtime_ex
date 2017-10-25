#include "stdafx.h"
#include "core_service.h"
#include "service_base.h"
#include "core_app.h"
#include "hash_service_selector.h"
#include "random_service_selector.h"
#include "round_robin_service_selector.h"
#include "native_proto_system.h"
#include "native_serializer.h"

#include "libBaseCommon/rand_gen.h"
#include "libBaseCommon/time_util.h"
#include "libBaseCommon/defer.h"

#define _MAX_SERVICE_HEALTH 100
#define _HIGH_SERVICE_HEALTH 80
#define _LOW_SERVICE_HEALTH 20
#define _SUCCESS_HEALTH		1
#define _TIMEOUT_HEALTH		2
#define _CHECK_SERVICE_HEALTH_TIME 100

namespace core
{
	CCoreService::CCoreService(CServiceBase* pServiceBase, const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName)
		: m_eRunState(eSRS_Start)
		, m_szConfigFileName(szConfigFileName)
		, m_sServiceBaseInfo(sServiceBaseInfo)
		, m_pServiceBase(pServiceBase)
		, m_pForwardMessageSerializer(nullptr)
		, m_pBaseConnectionMgr(nullptr)
		, m_pMessageQueue(nullptr)
		, m_nNextSessionID(0)
		, m_nQPS(0)
		, m_nCurQPS(0)
		, m_nDefaultServiceMessageSerializerType(0)
	{
		this->m_pMessageQueue = new CLogicMessageQueue(this, CCoreApp::Inst()->getLogicMessageQueueMgr());
	
		this->m_pBaseConnectionMgr = new CBaseConnectionMgr(this->m_pMessageQueue);

		this->m_pLocalServiceRegistryProxy = new CLocalServiceRegistryProxy();
		this->m_pTransporter = new CTransporter(this);
		this->m_pServiceInvoker = new CServiceInvoker(pServiceBase);
		this->m_pMessageDispatcher = new CMessageDispatcher(this);

		this->m_mapServiceSelector[eSST_Random] = new CRandomServiceSelector(this->m_pServiceBase);
		this->m_mapServiceSelector[eSST_Hash] = new CHashServiceSelector(this->m_pServiceBase);
		this->m_mapServiceSelector[eSST_RoundRobin] = new CRoundRobinServiceSelector(this->m_pServiceBase);

		this->m_tickerCheckHealth.setCallback(std::bind(&CCoreService::onCheckServiceHealth, this, std::placeholders::_1));
		this->m_tickerQPS.setCallback(std::bind(&CCoreService::onQPS, this, std::placeholders::_1));
	}

	CCoreService::~CCoreService()
	{
		SAFE_DELETE(this->m_pServiceInvoker);
		SAFE_DELETE(this->m_pMessageDispatcher);
		SAFE_DELETE(this->m_pTransporter);
		SAFE_DELETE(this->m_pLocalServiceRegistryProxy);

		SAFE_DELETE(this->m_mapServiceSelector[eSST_Random]);
		SAFE_DELETE(this->m_mapServiceSelector[eSST_Hash]);
		SAFE_DELETE(this->m_mapServiceSelector[eSST_RoundRobin]);

		SAFE_DELETE(this->m_pBaseConnectionMgr);
	}

	void CCoreService::quit()
	{
		DebugAst(this->m_eRunState == eSRS_Normal);

		this->m_eRunState = eSRS_Quitting;
		this->m_pServiceBase->onQuit();
	}

	bool CCoreService::onInit()
	{
		if (!this->m_pServiceBase->onInit())
			return false;

		this->m_eRunState = eSRS_Normal;

		this->addServiceMessageSerializer(new CNativeSerializer());
		
		this->registerServiceMessageHandler("service_health_request", [this](CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const void*)
		{
			service_health_response response_msg;
			pServiceBase->getServiceInvoker()->response(sSessionInfo, &response_msg, eRRT_OK, eMST_Native);
		});
		this->m_pServiceBase->registerTicker(&this->m_tickerCheckHealth, _CHECK_SERVICE_HEALTH_TIME, _CHECK_SERVICE_HEALTH_TIME, 0);
		
		this->m_pServiceBase->registerTicker(&this->m_tickerQPS, 1000, 1000, 0);
		return true;
	}

	CServiceBase* CCoreService::getServiceBase() const
	{
		return this->m_pServiceBase;
	}

	uint32_t CCoreService::getServiceID() const
	{
		return this->m_sServiceBaseInfo.nID;
	}

	const SServiceBaseInfo& CCoreService::getServiceBaseInfo() const
	{
		return this->m_sServiceBaseInfo;
	}

	void CCoreService::registerServiceMessageHandler(const std::string& szMessageName, const std::function<void(CServiceBase*, SSessionInfo, const void*)>& callback)
	{
		this->m_mapServiceMessageHandler[szMessageName] = callback;
	}

	void CCoreService::registerServiceForwardHandler(const std::string& szMessageName, const std::function<void(CServiceBase*, SClientSessionInfo, const void*)>& callback)
	{
		uint32_t nMessageID = _GET_MESSAGE_ID(szMessageName);

		std::unique_lock<base::spin_lock> guard(this->m_lockForwardMessage);

		auto iter = this->m_mapForwardMessageName.find(nMessageID);
		if (iter != this->m_mapForwardMessageName.end() && szMessageName != iter->second)
		{
			PrintWarning("dup forward message id message_id: {} old_message_name: {} new_message_name: {}", nMessageID, iter->second, szMessageName);
			return;
		}

		this->m_mapForwardMessageName[nMessageID] = szMessageName;
		this->m_mapServiceForwardHandler[szMessageName] = callback;
	}

	std::function<void(CServiceBase*, SSessionInfo, const void*)>& CCoreService::getServiceMessageHandler(const std::string& szMessageName)
	{
		auto iter = this->m_mapServiceMessageHandler.find(szMessageName);
		if (iter == this->m_mapServiceMessageHandler.end())
		{
			static std::function<void(CServiceBase*, SSessionInfo, const void*)> callback;
			return callback;
		}

		return iter->second;
	}

	std::function<void(CServiceBase*, SClientSessionInfo, const void*)>& CCoreService::getServiceForwardHandler(const std::string& szMessageName)
	{
		auto iter = this->m_mapServiceForwardHandler.find(szMessageName);
		if (iter == this->m_mapServiceForwardHandler.end())
		{
			static std::function<void(CServiceBase*, SClientSessionInfo, const void*)> callback;
			return callback;
		}

		return iter->second;
	}

	void CCoreService::doQuit()
	{
		DebugAst(this->m_eRunState == eSRS_Quitting);

		PrintInfo("CCoreService::doQuit service_id: {}", this->getServiceID());
		
		this->m_eRunState = eSRS_Quit;
	}

	void CCoreService::onFrame()
	{
		this->m_pServiceBase->onFrame();
	}

	void CCoreService::setServiceConnectCallback(const std::function<void(const std::string&, uint32_t)>& callback)
	{
		this->m_fnServiceConnectCallback = callback;
	}

	void CCoreService::setServiceDisconnectCallback(const std::function<void(const std::string&, uint32_t)>& callback)
	{
		this->m_fnServiceDisconnectCallback = callback;
	}

	std::function<void(const std::string&, uint32_t)>& CCoreService::getServiceConnectCallback()
	{
		return this->m_fnServiceConnectCallback;
	}

	std::function<void(const std::string&, uint32_t)>& CCoreService::getServiceDisconnectCallback()
	{
		return this->m_fnServiceDisconnectCallback;
	}

	CServiceInvoker* CCoreService::getServiceInvoker() const
	{
		return this->m_pServiceInvoker;
	}

	CMessageDispatcher* CCoreService::getMessageDispatcher() const
	{
		return this->m_pMessageDispatcher;
	}
	
	CLogicMessageQueue* CCoreService::getMessageQueue() const
	{
		return this->m_pMessageQueue;
	}

	CTransporter* CCoreService::getTransporter() const
	{
		return this->m_pTransporter;
	}

	EServiceRunState CCoreService::getRunState() const
	{
		return this->m_eRunState;
	}

	const std::string& CCoreService::getConfigFileName() const
	{
		return this->m_szConfigFileName;
	}

	void CCoreService::setServiceSelector(uint32_t nType, CServiceSelector* pServiceSelector)
	{
		if (pServiceSelector == nullptr)
			this->m_mapServiceSelector.erase(nType);
		else
			this->m_mapServiceSelector[nType] = pServiceSelector;
	}

	CServiceSelector* CCoreService::getServiceSelector(uint32_t nType) const
	{
		auto iter = this->m_mapServiceSelector.find(nType);
		if (iter == this->m_mapServiceSelector.end())
			return nullptr;

		return iter->second;
	}

	CBaseConnectionMgr* CCoreService::getBaseConnectionMgr() const
	{
		return this->m_pBaseConnectionMgr;
	}

	const std::string& CCoreService::getForwardMessageName(uint32_t nMessageID)
	{
		std::unique_lock<base::spin_lock> guard(this->m_lockForwardMessage);

		auto iter = this->m_mapForwardMessageName.find(nMessageID);
		if (iter == this->m_mapForwardMessageName.end())
		{
			static std::string s_Default;
			return s_Default;
		}

		return iter->second;
	}

	void CCoreService::setToGateMessageCallback(const std::function<void(uint64_t, const void*, uint16_t)>& callback)
	{
		this->m_fnToGateMessageCallback = callback;
	}

	std::function<void(uint64_t, const void*, uint16_t)>& CCoreService::getToGateMessageCallback()
	{
		return this->m_fnToGateMessageCallback;
	}

	void CCoreService::setToGateBroadcastMessageCallback(const std::function<void(const uint64_t*, uint16_t, const void*, uint16_t)>& callback)
	{
		this->m_fnToGateBroadcastMessageCallback = callback;
	}

	std::function<void(const uint64_t*, uint16_t, const void*, uint16_t)>& CCoreService::getToGateBroadcastMessageCallback()
	{
		return this->m_fnToGateBroadcastMessageCallback;
	}

	bool CCoreService::isServiceHealth(uint32_t nServiceID) const
	{
		if (!this->m_pLocalServiceRegistryProxy->isValidService(nServiceID))
			return false;

		auto iter = this->m_mapServiceHealth.find(nServiceID);
		if (iter == this->m_mapServiceHealth.end())
			return true;

		if (iter->second >= _HIGH_SERVICE_HEALTH)
			return true;
		
		uint32_t nRand = base::CRandGen::getGlobalRand(_MAX_SERVICE_HEALTH);
		return nRand <= (uint32_t)iter->second;
	}

	void CCoreService::updateServiceHealth(uint32_t nServiceID, bool bTimeout)
	{
		auto iter = this->m_mapServiceHealth.find(nServiceID);
		if (iter == this->m_mapServiceHealth.end())
		{
			this->m_mapServiceHealth[nServiceID] = _MAX_SERVICE_HEALTH;
			iter = this->m_mapServiceHealth.find(nServiceID);
			if (iter == this->m_mapServiceHealth.end())
				return;
		}

		int32_t& nHealth = iter->second;
		if (bTimeout)
		{
			nHealth = std::max<int32_t>(nHealth - _TIMEOUT_HEALTH, 0);
		}
		else
		{
			nHealth = std::min<int32_t>(nHealth + _SUCCESS_HEALTH, _MAX_SERVICE_HEALTH);
		}

		//PrintInfo("CCoreService::updateServiceHealth: {} health {}", bTimeout ? "timeout" : "normal", nHealth);
	}

	uint64_t CCoreService::genSessionID()
	{
		++this->m_nNextSessionID;
		if (this->m_nNextSessionID == 0)
			this->m_nNextSessionID = 1;

		return this->m_nNextSessionID;
	}

	void CCoreService::onRequestMessageTimeout(uint64_t nContext)
	{
		auto iter = this->m_mapPendingResponseInfo.find(nContext);
		if (iter == this->m_mapPendingResponseInfo.end())
		{
			PrintWarning("iter == this->m_mapProtoBufResponseInfo.end() session_id: {}", nContext);
			return;
		}

		SPendingResponseInfo* pPendingResponseInfo = iter->second;
		if (nullptr == pPendingResponseInfo)
		{
			PrintWarning("nullptr == pPendingResponseInfo session_id: {}", nContext);
			return;
		}

		char* szBuf = new char[sizeof(SMCT_RESPONSE)];
		SMCT_RESPONSE* pContext = reinterpret_cast<SMCT_RESPONSE*>(szBuf);
		pContext->nSessionID = pPendingResponseInfo->nSessionID;
		pContext->nFromServiceID = pPendingResponseInfo->nToServiceID;
		pContext->nMessageSerializerType = 0;
		pContext->nResult = eRRT_TIME_OUT;
		pContext->nMessageDataLen = 0;
		pContext->nMessageNameLen = 0;
		pContext->szMessageName[0] = 0;
		
		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_RESPONSE;
		sMessagePacket.nDataSize = sizeof(SMCT_RESPONSE);
		sMessagePacket.pData = pContext;

		this->m_pMessageQueue->send(sMessagePacket);
	}

	SPendingResponseInfo* CCoreService::getPendingResponseInfo(uint64_t nSessionID)
	{
		auto iter = this->m_mapPendingResponseInfo.find(nSessionID);
		if (iter == this->m_mapPendingResponseInfo.end())
			return nullptr;

		SPendingResponseInfo* pPendingResponseInfo = iter->second;
		this->m_mapPendingResponseInfo.erase(iter);
		if (pPendingResponseInfo->nHolderID != 0)
		{
			auto iter = this->m_mapHolderSessionIDList.find(pPendingResponseInfo->nHolderID);
			if (iter != this->m_mapHolderSessionIDList.end())
			{
				std::list<uint64_t>& listSessionID = iter->second;
				listSessionID.erase(pPendingResponseInfo->iterHolder);
			}
		}

		return pPendingResponseInfo;
	}

	SPendingResponseInfo* CCoreService::addPendingResponseInfo(uint32_t nToServiceID, uint64_t nSessionID, uint64_t nCoroutineID, const std::function<void(std::shared_ptr<void>, uint32_t)>& callback, uint32_t nTimeout, uint64_t nHolderID)
	{
		auto iter = this->m_mapPendingResponseInfo.find(nSessionID);
		DebugAstEx(iter == this->m_mapPendingResponseInfo.end(), nullptr);

		SPendingResponseInfo* pPendingResponseInfo = new SPendingResponseInfo();
		pPendingResponseInfo->callback = callback;
		pPendingResponseInfo->nSessionID = nSessionID;
		pPendingResponseInfo->nCoroutineID = nCoroutineID;
		pPendingResponseInfo->nHolderID = nHolderID;
		pPendingResponseInfo->nBeginTime = base::time_util::getGmtTime();
		pPendingResponseInfo->nToServiceID = nToServiceID;
		pPendingResponseInfo->tickTimeout.setCallback(std::bind(&CCoreService::onRequestMessageTimeout, this, std::placeholders::_1));
		
		if (nTimeout == 0)
			nTimeout = CCoreApp::Inst()->getDefaultServiceInvokeTimeout();

		this->getServiceBase()->registerTicker(&pPendingResponseInfo->tickTimeout, nTimeout, 0, nSessionID);

		this->m_mapPendingResponseInfo[pPendingResponseInfo->nSessionID] = pPendingResponseInfo;

		if (nHolderID != 0)
		{
			std::list<uint64_t>& listSessionID = this->m_mapHolderSessionIDList[nHolderID];
			listSessionID.push_back(nSessionID);
			pPendingResponseInfo->iterHolder = (--listSessionID.end());
			pPendingResponseInfo->nHolderID = nHolderID;
		}

		return pPendingResponseInfo;
	}

	void CCoreService::delPendingResponseInfo(uint64_t nHolderID)
	{
		auto iter = this->m_mapHolderSessionIDList.find(nHolderID);
		if (iter == this->m_mapHolderSessionIDList.end())
			return;

		std::list<uint64_t>& listSessionID = iter->second;
		for (auto iter = listSessionID.begin(); iter != listSessionID.end(); ++iter)
		{
			auto iterPendingResponseInfo = this->m_mapPendingResponseInfo.find(*iter);
			if (iterPendingResponseInfo == this->m_mapPendingResponseInfo.end())
				continue;

			SPendingResponseInfo* pPendingResponseInfo = iterPendingResponseInfo->second;
			this->m_mapPendingResponseInfo.erase(iterPendingResponseInfo);

			SAFE_DELETE(pPendingResponseInfo);
		}

		this->m_mapHolderSessionIDList.erase(iter);
	}

	void CCoreService::onCheckServiceHealth(uint64_t nContext)
	{
		for (auto iter = this->m_mapServiceHealth.begin(); iter != this->m_mapServiceHealth.end(); ++iter)
		{
			uint32_t nToServiceID = iter->first;
			int32_t nHealth = iter->second;

			if (nHealth > _LOW_SERVICE_HEALTH)
				continue;

			service_health_request request_msg;
			uint64_t nSessionID = this->genSessionID();
			SInvokeOption sInvokeOption;
			sInvokeOption.nSerializerType = eMST_Native;
			sInvokeOption.nTimeout = 0;
			if (!this->m_pTransporter->invoke(nToServiceID, nSessionID, &request_msg, &sInvokeOption))
				continue;

			auto callback = [this, nToServiceID](std::shared_ptr<void>, uint32_t nErrorCode)
			{
			};

			this->addPendingResponseInfo(nToServiceID, nSessionID, 0, callback, 0, 0);
		}
	}

	void CCoreService::addServiceMessageSerializer(CMessageSerializer* pMessageSerializer)
	{
		DebugAst(pMessageSerializer != nullptr);

		this->m_mapMessageSerializer[pMessageSerializer->getType()] = pMessageSerializer;
	}

	void CCoreService::setServiceMessageSerializer(uint32_t nServiceID, uint32_t nType)
	{
		if (nServiceID == 0)
		{
			this->m_nDefaultServiceMessageSerializerType = nType;
			return;
		}

		this->m_mapServiceMessageSerializerType[nServiceID] = nType;
	}

	void CCoreService::setForwardMessageSerializer(CMessageSerializer* pMessageSerializer)
	{
		DebugAst(pMessageSerializer != nullptr);

		this->m_pForwardMessageSerializer = pMessageSerializer;
	}

	CMessageSerializer* CCoreService::getServiceMessageSerializer(uint32_t nServiceID) const
	{
		uint32_t nMessageSerializerType = this->m_nDefaultServiceMessageSerializerType;
		auto iter = this->m_mapServiceMessageSerializerType.find(nServiceID);
		if (iter != this->m_mapServiceMessageSerializerType.end())
			nMessageSerializerType = iter->second;

		auto iterSerializer = this->m_mapMessageSerializer.find(nMessageSerializerType);
		if (iterSerializer == this->m_mapMessageSerializer.end())
			return nullptr;

		return iterSerializer->second;
	}

	CMessageSerializer* CCoreService::getForwardMessageSerializer() const
	{
		return this->m_pForwardMessageSerializer;
	}

	CMessageSerializer* CCoreService::getServiceMessageSerializerByType(uint8_t nType) const
	{
		auto iter = this->m_mapMessageSerializer.find(nType);
		if (iter == this->m_mapMessageSerializer.end())
			return nullptr;

		return iter->second;
	}

	uint32_t CCoreService::getServiceMessageSerializerType(uint32_t nServiceID) const
	{
		uint32_t nMessageSerializerType = this->m_nDefaultServiceMessageSerializerType;
		auto iter = this->m_mapServiceMessageSerializerType.find(nServiceID);
		if (iter != this->m_mapServiceMessageSerializerType.end())
			nMessageSerializerType = iter->second;

		return nMessageSerializerType;
	}

	uint32_t CCoreService::getQPS() const
	{
		return this->m_nQPS.load(std::memory_order_acquire);
	}

	void CCoreService::incQPS()
	{
		++this->m_nCurQPS;
	}

	void CCoreService::onQPS(uint64_t nContext)
	{
		this->m_nQPS.store(this->m_nCurQPS, std::memory_order_release);

		this->m_nCurQPS = 0;
	}

	CLocalServiceRegistryProxy* CCoreService::getLocalServiceRegistryProxy() const
	{
		return this->m_pLocalServiceRegistryProxy;
	}
}