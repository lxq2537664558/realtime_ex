#include "stdafx.h"
#include "service_base_mgr.h"

namespace core
{
	CServiceBaseMgr::CServiceBaseMgr()
	{

	}

	CServiceBaseMgr::~CServiceBaseMgr()
	{

	}

	bool CServiceBaseMgr::init(const std::vector<SServiceBaseInfo>& vecServiceBaseInfo)
	{
		for (size_t i = 0; i < vecServiceBaseInfo.size(); ++i)
		{
			const SServiceBaseInfo& sServiceBaseInfo = vecServiceBaseInfo[i];
			CServiceBase* pServiceBase = dynamic_cast<CServiceBase*>(CBaseObject::createObject(sServiceBaseInfo.szClassName));
			if (nullptr == pServiceBase)
			{
				PrintWarning("create service_base class_name: %s error", sServiceBaseInfo.szClassName.c_str());
				return false;
			}
			CServiceBaseImpl* pServiceBaseImpl = new CServiceBaseImpl();
			if (!pServiceBaseImpl->init(sServiceBaseInfo, pServiceBase))
			{
				PrintWarning("create service_base %s error", sServiceBaseInfo.szName.c_str());
				return false;
			}

			PrintInfo("create service %s ok", sServiceBaseInfo.szName.c_str());
			
			this->m_vecServiceBase.push_back(pServiceBaseImpl);
			this->m_mapServiceBase[pServiceBaseImpl->getServiceBaseInfo().nID] = pServiceBaseImpl;
		}

		return true;
	}

	CServiceBaseImpl* CServiceBaseMgr::getServiceBase(uint16_t nID) const
	{
		auto iter = this->m_mapServiceBase.find(nID);
		if (iter == this->m_mapServiceBase.end())
			return nullptr;

		return iter->second;
	}

	const std::vector<CServiceBaseImpl*>& CServiceBaseMgr::getServiceBase() const
	{
		return this->m_vecServiceBase;
	}
}