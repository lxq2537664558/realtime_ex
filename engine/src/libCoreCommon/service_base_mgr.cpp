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

	void CServiceBaseMgr::sendMessage(uint16_t nToServiceID, const SMessagePacket& sMessagePacket)
	{
		CServiceBaseImpl* pServiceBaseImpl = this->getServiceBase(nToServiceID);
		if (nullptr == pServiceBaseImpl)
			return;

		pServiceBaseImpl->getMessageQueue()->send(sMessagePacket);
		if (pServiceBaseImpl->isWorking())
			return;

		std::unique_lock<std::mutex> guard(this->m_lock);
		if (this->m_setWorkServiceBaseID.find(nToServiceID) != this->m_setWorkServiceBaseID.end())
			return;

		bool bEmpty = this->m_listWorkServiceBase.empty();
		this->m_setWorkServiceBaseID.insert(nToServiceID);
		this->m_listWorkServiceBase.push_back(pServiceBaseImpl);

		if (bEmpty)
			this->m_cond.notify_all();
	}

	CServiceBaseImpl* CServiceBaseMgr::getWorkServiceBase()
	{
		std::unique_lock<std::mutex> guard(this->m_lock);

		while (this->m_listWorkServiceBase.empty())
		{
			this->m_cond.wait(guard);
		}

		CServiceBaseImpl* pServiceBaseImpl = this->m_listWorkServiceBase.front();
		if (pServiceBaseImpl != nullptr)
			this->m_setWorkServiceBaseID.erase(pServiceBaseImpl->getServiceBaseInfo().nID);

		return pServiceBaseImpl;
	}

	void CServiceBaseMgr::addWorkServiceBase(CServiceBaseImpl* pServiceBaseImpl)
	{
		DebugAst(pServiceBaseImpl != nullptr);

		if (pServiceBaseImpl->getMessageQueue()->empty())
			return;

		std::unique_lock<std::mutex> guard(this->m_lock);
		if (this->m_setWorkServiceBaseID.find(pServiceBaseImpl->getServiceBaseInfo().nID) != this->m_setWorkServiceBaseID.end())
			return;

		bool bEmpty = this->m_listWorkServiceBase.empty();
		this->m_setWorkServiceBaseID.insert(pServiceBaseImpl->getServiceBaseInfo().nID);
		this->m_listWorkServiceBase.push_back(pServiceBaseImpl);

		if (bEmpty)
			this->m_cond.notify_all();
	}
}