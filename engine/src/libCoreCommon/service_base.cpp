#include "stdafx.h"
#include "service_base.h"
#include "core_app.h"
#include "core_service.h"
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

	uint32_t CServiceBase::getServiceID(const std::string& szName) const
	{
		return this->m_pCoreService->getLocalServiceRegistryProxy()->getServiceID(szName);
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

	void CServiceBase::setServiceConnectCallback(const std::function<void(const std::string&, uint32_t)>& callback)
	{
		this->m_pCoreService->setServiceConnectCallback(callback);
	}

	void CServiceBase::setServiceDisconnectCallback(const std::function<void(const std::string&, uint32_t)>& callback)
	{
		this->m_pCoreService->setServiceDisconnectCallback(callback);
	}

	int64_t CServiceBase::getLogicTime() const
	{
		return this->m_pCoreService->getLogicTime();
	}

	void CServiceBase::registerTicker(base::CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext)
	{
		this->m_pCoreService->registerTicker(pTicker, nStartTime, nIntervalTime, nContext);
	}

	void CServiceBase::unregisterTicker(base::CTicker* pTicker)
	{
		this->m_pCoreService->unregisterTicker(pTicker);
	}

	void CServiceBase::doQuit()
	{
		this->m_pCoreService->doQuit();
	}

	CServiceInvoker* CServiceBase::getServiceInvoker() const
	{
		return this->m_pCoreService->getServiceInvoker();
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

	bool CServiceBase::isServiceHealth(uint32_t nServiceID) const
	{
		return this->m_pCoreService->isServiceHealth(nServiceID);
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

	void CServiceBase::setServiceMessageSerializer(const std::string& szServiceType, uint32_t nType)
	{
		this->m_pCoreService->setServiceMessageSerializer(szServiceType, nType);
	}

	void CServiceBase::setForwardMessageSerializer(uint32_t nType)
	{
		this->m_pCoreService->setForwardMessageSerializer(nType);
	}

	CMessageSerializer* CServiceBase::getServiceMessageSerializer(const std::string& szServiceType) const
	{
		return this->m_pCoreService->getServiceMessageSerializer(szServiceType);
	}

	CMessageSerializer* CServiceBase::getForwardMessageSerializer() const
	{
		return this->m_pCoreService->getForwardMessageSerializer();
	}

	CBaseConnectionMgr* CServiceBase::getBaseConnectionMgr() const
	{
		return this->m_pCoreService->getBaseConnectionMgr();
	}

	uint32_t CServiceBase::getQPS() const
	{
		return this->m_pCoreService->getQPS();
	}

	const std::set<uint32_t>& CServiceBase::getServiceIDByType(const std::string& szName) const
	{
		return this->m_pCoreService->getLocalServiceRegistryProxy()->getServiceIDByType(szName);
	}

	const std::vector<uint32_t>& CServiceBase::getActiveServiceIDByType(const std::string& szName) const
	{
		return this->m_pCoreService->getLocalServiceRegistryProxy()->getActiveServiceIDByType(szName);
	}
}