#include "stdafx.h"
#include "core_service_mgr.h"
#include "core_app.h"

namespace core
{
	CCoreServiceMgr::CCoreServiceMgr()
	{

	}

	CCoreServiceMgr::~CCoreServiceMgr()
	{
		for (size_t i = 0; i < this->m_vecCoreService.size(); ++i)
		{
			CCoreService* pCoreService = this->m_vecCoreService[i];
			pCoreService->getServiceBase()->release();
		}
	}

	bool CCoreServiceMgr::init(const std::vector<CServiceBase*>& vecServiceBase)
	{
		for (size_t i = 0; i < vecServiceBase.size(); ++i)
		{
			CServiceBase* pServiceBase = vecServiceBase[i];
			DebugAstEx(pServiceBase != nullptr, false);

			CCoreService* pCoreService = vecServiceBase[i]->m_pCoreService;
			DebugAstEx(pCoreService != nullptr, false);
			
			this->m_vecCoreService.push_back(pCoreService);
			this->m_mapCoreService[pCoreService->getServiceID()] = pCoreService;

			PrintInfo("create service service_name: {} service_id: {} successful", pServiceBase->getServiceBaseInfo().szName, pServiceBase->getServiceBaseInfo().nID);
		}

		return true;
	}

	bool CCoreServiceMgr::onInit()
	{
		for (size_t i = 0; i < this->m_vecCoreService.size(); ++i)
		{
			CCoreService* pCoreService = this->m_vecCoreService[i];
			if (!pCoreService->onInit())
			{
				PrintWarning("CCoreServiceMgr::onInit error service_name: {}", pCoreService->getServiceBaseInfo().szName);
				return false;
			}
		}

		for (size_t k = 0; k < this->m_vecCoreService.size(); ++k)
		{
			auto& callback = this->m_vecCoreService[k]->getServiceConnectCallback();
			if (callback == nullptr)
				continue;

			for (size_t i = 0; i < this->m_vecCoreService.size(); ++i)
			{
				if (this->m_vecCoreService[i]->getServiceID() == this->m_vecCoreService[k]->getServiceID())
					continue;

				callback(this->m_vecCoreService[i]->getServiceBaseInfo().szType, this->m_vecCoreService[i]->getServiceBaseInfo().nID);
			}
		}

		return true;
	}

	CCoreService* CCoreServiceMgr::getCoreService(uint32_t nID) const
	{
		auto iter = this->m_mapCoreService.find(nID);
		if (iter == this->m_mapCoreService.end())
			return nullptr;

		return iter->second;
	}

	const std::vector<CCoreService*>& CCoreServiceMgr::getCoreService() const
	{
		return this->m_vecCoreService;
	}

	std::vector<SServiceBaseInfo> CCoreServiceMgr::getServiceBaseInfo() const
	{
		std::vector<SServiceBaseInfo> vecServiceBaseInfo;
		vecServiceBaseInfo.reserve(this->m_vecCoreService.size());
		for (size_t i = 0; i < this->m_vecCoreService.size(); ++i)
		{
			vecServiceBaseInfo.push_back(this->m_vecCoreService[i]->getServiceBaseInfo());
		}

		return vecServiceBaseInfo;
	}

	bool CCoreServiceMgr::isLocalService(uint32_t nServiceID) const
	{
		for (size_t i = 0; i < this->m_vecCoreService.size(); ++i)
		{
			if (this->m_vecCoreService[i]->getServiceID() == nServiceID)
				return true;
		}

		return false;
	}
}