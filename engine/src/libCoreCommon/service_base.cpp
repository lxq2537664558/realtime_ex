#include "stdafx.h"
#include "service_base.h"
#include "core_app.h"
#include "core_service.h"
#include "actor_base.h"
#include "service_invoker.h"

namespace core
{
	CServiceBase::CServiceBase(const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName)
	{
		this->m_pCoreService = new CCoreService(this, sServiceBaseInfo, szConfigFileName);
	}

	uint32_t CServiceBase::getServiceID() const
	{
		return this->m_pCoreService->getServiceID();
	}

	const SServiceBaseInfo& CServiceBase::getServiceBaseInfo() const
	{
		return this->m_pCoreService->getServiceBaseInfo();
	}

	CServiceBase::~CServiceBase()
	{
		SAFE_DELETE(this->m_pCoreService);
	}

	void CServiceBase::registerServiceMessageHandler(const std::string& szMessageName, const std::function<void(CServiceBase*, SSessionInfo, const void*)>& callback)
	{
		this->m_pCoreService->registerServiceMessageHandler(szMessageName, callback);
	}

	void CServiceBase::registerServiceForwardHandler(const std::string& szMessageName, const std::function<void(CServiceBase*, SClientSessionInfo, const void*)>& callback)
	{
		this->m_pCoreService->registerServiceForwardHandler(szMessageName, callback);
	}

	void CServiceBase::registerActorMessageHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SSessionInfo, const void*)>& callback)
	{
		this->m_pCoreService->registerActorMessageHandler(szMessageName, callback);
	}

	void CServiceBase::registerActorForwardHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SClientSessionInfo, const void*)>& callback)
	{
		this->m_pCoreService->registerActorForwardHandler(szMessageName, callback);
	}

	const std::string& CServiceBase::getForwardMessageName(uint32_t nMessageID)
	{
		return this->m_pCoreService->getForwardMessageName(nMessageID);
	}

	void CServiceBase::setServiceConnectCallback(const std::function<void(const std::string&, uint32_t)>& callback)
	{
		this->m_pCoreService->setServiceConnectCallback(callback);
	}

	void CServiceBase::setServiceDisconnectCallback(const std::function<void(const std::string&, uint32_t)>& callback)
	{
		this->m_pCoreService->setServiceDisconnectCallback(callback);
	}

	void CServiceBase::registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext)
	{
		CCoreApp::Inst()->registerTicker(CTicker::eTT_Service, this->getServiceID(), 0, pTicker, nStartTime, nIntervalTime, nContext);
	}

	void CServiceBase::unregisterTicker(CTicker* pTicker)
	{
		CCoreApp::Inst()->unregisterTicker(pTicker);
	}

	void CServiceBase::doQuit()
	{
		this->m_pCoreService->doQuit();
	}

	CServiceInvoker* CServiceBase::getServiceInvoker() const
	{
		return this->m_pCoreService->getServiceInvoker();
	}

	CActorBase* CServiceBase::createActor(const std::string& szType, uint64_t nActorID, const void* pContext)
	{
		DebugAstEx(nActorID != 0, nullptr);

		if (this->m_pCoreService->getActorScheduler()->getCoreActor(nActorID) != nullptr)
		{
			PrintWarning("CServiceBase::createActor error actor id exist actor_id: {} type: {}", nActorID, szType);
			return nullptr;
		}

		CActorFactory* pActorFactory = this->getActorFactory(szType);
		if (nullptr == pActorFactory)
		{
			PrintWarning("CServiceBase::createActor error not find actor factory actor_id: {} type: {}", nActorID, szType);
			return nullptr;
		}

		CActorBase* pActorBase = pActorFactory->createActor(szType);
		if (nullptr == pActorBase)
		{
			PrintWarning("CServiceBase::createActor error create actor error actor_id: {} type: {}", nActorID, szType);
			return nullptr;
		}

		pActorBase->m_pCoreActor = this->m_pCoreService->getActorScheduler()->createCoreActor(nActorID, pActorBase);
		if (pActorBase->m_pCoreActor == nullptr)
		{
			SAFE_RELEASE(pActorBase);
			return nullptr;
		}

		this->m_pCoreService->getActorScheduler()->setCurWorkActorID(nActorID);

		pActorBase->onInit(pContext);
// 		uint64_t nCoroutineID = coroutine::create(0, [pActorBase, szContext](uint64_t){ pActorBase->onInit(szContext); });
// 		coroutine::resume(nCoroutineID, 0);

		this->m_pCoreService->getActorScheduler()->setCurWorkActorID(0);

		return pActorBase;
	}

	void CServiceBase::destroyActor(CActorBase* pActorBase)
	{
		DebugAst(pActorBase != nullptr);

		pActorBase->onDestroy();
		this->m_pCoreService->getActorScheduler()->destroyCoreActor(pActorBase->m_pCoreActor);

		pActorBase->release();
	}

	CActorBase* CServiceBase::getActorBase(uint64_t nID) const
	{
		CCoreActor* pCoreActor = this->m_pCoreService->getActorScheduler()->getCoreActor(nID);
		if (nullptr == pCoreActor)
			return nullptr;

		return pCoreActor->getActorBase();
	}

	EServiceRunState CServiceBase::getRunState() const
	{
		return this->m_pCoreService->getRunState();
	}

	const std::string& CServiceBase::getConfigFileName() const
	{
		return this->m_pCoreService->getConfigFileName();
	}

	void CServiceBase::setServiceSelector(uint32_t nType, CServiceSelector* pServiceSelector)
	{
		this->m_pCoreService->setServiceSelector(nType, pServiceSelector);
	}

	CServiceSelector* CServiceBase::getServiceSelector(uint32_t nType) const
	{
		return this->m_pCoreService->getServiceSelector(nType);
	}

	void CServiceBase::setToGateMessageCallback(const std::function<void(uint64_t, const void*, uint16_t)>& callback)
	{
		this->m_pCoreService->setToGateMessageCallback(callback);
	}

	void CServiceBase::setToGateBroadcastMessageCallback(const std::function<void(const uint64_t*, uint16_t, const void*, uint16_t)>& callback)
	{
		this->m_pCoreService->setToGateBroadcastMessageCallback(callback);
	}

	void CServiceBase::addServiceMessageSerializer(CMessageSerializer* pMessageSerializer)
	{
		this->m_pCoreService->addServiceMessageSerializer(pMessageSerializer);
	}

	void CServiceBase::setServiceMessageSerializer(uint32_t nServiceID, uint32_t nType)
	{
		this->m_pCoreService->setServiceMessageSerializer(nServiceID, nType);
	}

	void CServiceBase::setForwardMessageSerializer(CMessageSerializer* pMessageSerializer)
	{
		this->m_pCoreService->setForwardMessageSerializer(pMessageSerializer);
	}

	CMessageSerializer* CServiceBase::getServiceMessageSerializer(uint32_t nServiceID) const
	{
		return this->m_pCoreService->getServiceMessageSerializer(nServiceID);
	}

	CMessageSerializer* CServiceBase::getForwardMessageSerializer() const
	{
		return this->m_pCoreService->getForwardMessageSerializer();
	}

	uint32_t CServiceBase::getServiceMessageSerializerType(uint32_t nServiceID) const
	{
		return this->m_pCoreService->getServiceMessageSerializerType(nServiceID);
	}

}