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
			
			if (this->m_mapCoreService.find(pCoreService->getServiceID()) != this->m_mapCoreService.end())
			{
				PrintWarning("CCoreServiceMgr::init error dup service_id: {}", pCoreService->getServiceID());
				return false;
			}

			this->m_mapCoreService[pCoreService->getServiceID()] = pCoreService;

			this->m_vecCoreService.push_back(pCoreService);
			this->m_vecServiceBaseInfo.push_back(pCoreService->getServiceBaseInfo());

			PrintInfo("create service service_name: {} service_id: {} successful", pServiceBase->getServiceBaseInfo().szName, pServiceBase->getServiceBaseInfo().nID);
		}

		return true;
	}

	bool CCoreServiceMgr::onInit()
	{
		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_INIT;
		sMessagePacket.pData = nullptr;
		sMessagePacket.nDataSize = 0;

		for (size_t i = 0; i < this->m_vecCoreService.size(); ++i)
		{
			this->m_vecCoreService[i]->getMessageQueue()->send(sMessagePacket);
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

	const std::vector<SServiceBaseInfo>& CCoreServiceMgr::getServiceBaseInfo() const
	{
		return this->m_vecServiceBaseInfo;
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