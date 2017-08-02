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

	bool CServiceBaseMgr::init(tinyxml2::XMLElement* pNodeInfoXML)
	{
		DebugAstEx(pNodeInfoXML != nullptr, false);

		for (tinyxml2::XMLElement* pServiceInfoXML = pNodeInfoXML->FirstChildElement("service_info"); pServiceInfoXML != nullptr; pServiceInfoXML = pServiceInfoXML->NextSiblingElement("service_info"))
		{
			std::string szLibName = pServiceInfoXML->Attribute("lib_name");

#ifdef _WIN32
			szLibName += ".dll";
			HINSTANCE hInstance = LoadLibraryA(szLibName.c_str());
			if (hInstance == nullptr)
			{
				PrintWarning("hInstance == nullptr lib_name: %s", szLibName.c_str());
				return false;
			}
#endif

			SServiceBaseInfo sServiceBaseInfo;
			sServiceBaseInfo.nID = pServiceInfoXML->UnsignedAttribute("service_id");
			sServiceBaseInfo.szName = pServiceInfoXML->Attribute("service_name");
			sServiceBaseInfo.szType = pServiceInfoXML->Attribute("service_type");
			sServiceBaseInfo.szClassName = pServiceInfoXML->Attribute("class_name");
			std::string szConfigFileName;
			if (pServiceInfoXML->Attribute("config_file_name") != nullptr)
			{
				szConfigFileName = base::getCurrentWorkPath();
				szConfigFileName += "/etc/";
				szConfigFileName += pServiceInfoXML->Attribute("config_file_name");
			}
			CServiceBase* pServiceBase = dynamic_cast<CServiceBase*>(CBaseObject::createObject(sServiceBaseInfo.szClassName));
			if (nullptr == pServiceBase)
			{
				PrintWarning("create service_base class_name: %s error", sServiceBaseInfo.szClassName.c_str());
				return false;
			}
			CServiceBaseImpl* pServiceBaseImpl = new CServiceBaseImpl();
			if (!pServiceBaseImpl->init(pServiceBase, sServiceBaseInfo, szConfigFileName))
			{
				PrintWarning("create service_base %s error", sServiceBaseInfo.szName.c_str());
				return false;
			}

			PrintInfo("create service %s ok", sServiceBaseInfo.szName.c_str());

			this->m_vecServiceBaseInfo.push_back(sServiceBaseInfo);
			this->m_vecServiceBase.push_back(pServiceBaseImpl);
			this->m_mapServiceBase[pServiceBaseImpl->getServiceID()] = pServiceBaseImpl;
		}

		return true;
	}

	bool CServiceBaseMgr::onInit()
	{
		for (size_t i = 0; i < this->m_vecServiceBase.size(); ++i)
		{
			CServiceBaseImpl* pServiceBaseImpl = this->m_vecServiceBase[i];
			if (!pServiceBaseImpl->onInit())
			{
				PrintWarning("CServiceBaseMgr::onInit error service_name: %s", pServiceBaseImpl->getServiceBaseInfo().szName.c_str());
				return false;
			}
		}

		for (size_t k = 0; k < this->m_vecServiceBase.size(); ++k)
		{
			auto& callback = this->m_vecServiceBase[k]->getServiceConnectCallback();
			if (callback == nullptr)
				continue;

			for (size_t i = 0; i < this->m_vecServiceBaseInfo.size(); ++i)
			{
				if (this->m_vecServiceBaseInfo[i].nID == this->m_vecServiceBase[k]->getServiceID())
					continue;

				callback(this->m_vecServiceBaseInfo[i].szType, this->m_vecServiceBaseInfo[i].nID);
			}
		}

		return true;
	}

	CServiceBaseImpl* CServiceBaseMgr::getServiceBaseByID(uint32_t nID) const
	{
		auto iter = this->m_mapServiceBase.find(nID);
		if (iter == this->m_mapServiceBase.end())
			return nullptr;

		return iter->second;
	}

	CServiceBaseImpl* CServiceBaseMgr::getServiceBaseByName(const std::string& szName) const
	{
		auto iter = this->m_mapServiceName.find(szName);
		if (iter == this->m_mapServiceName.end())
			return nullptr;

		return this->getServiceBaseByID(iter->second);
	}

	const std::vector<CServiceBaseImpl*>& CServiceBaseMgr::getServiceBase() const
	{
		return this->m_vecServiceBase;
	}

	const std::vector<SServiceBaseInfo>& CServiceBaseMgr::getServiceBaseInfo() const
	{
		return this->m_vecServiceBaseInfo;
	}

	bool CServiceBaseMgr::isLocalService(uint32_t nServiceID) const
	{
		for (size_t i = 0; i < this->m_vecServiceBaseInfo.size(); ++i)
		{
			if (this->m_vecServiceBaseInfo[i].nID == nServiceID)
				return true;
		}

		return false;
	}
}