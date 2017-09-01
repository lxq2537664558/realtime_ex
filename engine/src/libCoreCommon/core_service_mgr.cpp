#include "stdafx.h"
#include "core_service_mgr.h"

#include "libBaseCommon/process_util.h"

#ifndef _WIN32
#include<dlfcn.h>
#endif

namespace
{
	typedef core::CServiceBase*(*funcCreateServiceBase)();
}

namespace core
{
	CCoreServiceMgr::CCoreServiceMgr()
	{

	}

	CCoreServiceMgr::~CCoreServiceMgr()
	{

	}

	bool CCoreServiceMgr::init(tinyxml2::XMLElement* pNodeInfoXML)
	{
		DebugAstEx(pNodeInfoXML != nullptr, false);

		for (tinyxml2::XMLElement* pServiceInfoXML = pNodeInfoXML->FirstChildElement("service_info"); pServiceInfoXML != nullptr; pServiceInfoXML = pServiceInfoXML->NextSiblingElement("service_info"))
		{
			std::string szLibName = pServiceInfoXML->Attribute("lib_name");
#ifdef _WIN32
			szLibName += ".dll";
			HMODULE hModule = LoadLibraryA(szLibName.c_str());
			if (hModule == nullptr)
			{
				PrintWarning("hModule == nullptr lib_name: %s error_code: {}", szLibName, base::getLastError());
				return false;
			}

			funcCreateServiceBase pCreateServiceBase = reinterpret_cast<funcCreateServiceBase>(GetProcAddress(hModule, "createServiceBase"));
			if (nullptr == pCreateServiceBase)
			{
				PrintWarning("nullptr == pCreateServiceBase lib_name: {}", szLibName);
				return false;
			}
#else
			szLibName += ".so";
			void* hModule = dlopen(szLibName.c_str(), RTLD_NOW);
			if (hModule == nullptr)
			{
				const char* szErr = dlerror();
				if (szErr != nullptr)
				{
					PrintWarning("hModule == nullptr lib_name: %s error: {}", szLibName, szErr);
					return false;
				}
				
				PrintWarning("hModule == nullptr lib_name: {}", szLibName);
				return false;
			}

			//获取函数的地址
			funcCreateServiceBase pCreateServiceBase = reinterpret_cast<funcCreateServiceBase>(dlsym(hModule, "createServiceBase"));
			if (nullptr == pCreateServiceBase)
			{
				const char* szErr = dlerror();
				if (szErr != nullptr)
				{
					PrintWarning("nullptr == pCreateServiceBase lib_name: {} error: {}", szLibName, szErr);
					return false;
				}
				
				PrintWarning("nullptr == pCreateServiceBase lib_name: {}", szLibName);
				return false;
			}
#endif

			CServiceBase* pServiceBase = pCreateServiceBase();
			if (nullptr == pServiceBase)
			{
				PrintWarning("create service_base error: lib_name: {}", szLibName);
				return false;
			}

			SServiceBaseInfo sServiceBaseInfo;
			sServiceBaseInfo.nID = pServiceInfoXML->UnsignedAttribute("service_id");
			sServiceBaseInfo.szName = pServiceInfoXML->Attribute("service_name");
			sServiceBaseInfo.szType = pServiceInfoXML->Attribute("service_type");
			std::string szConfigFileName;
			if (pServiceInfoXML->Attribute("config_file_name") != nullptr)
			{
				szConfigFileName = base::process_util::getCurrentWorkPath();
				szConfigFileName += "/etc/";
				szConfigFileName += pServiceInfoXML->Attribute("config_file_name");
			}
			
			CCoreService* pCoreService = new CCoreService();
			if (!pCoreService->init(pServiceBase, sServiceBaseInfo, szConfigFileName))
			{
				PrintWarning("create service_base {} error", sServiceBaseInfo.szName);
				return false;
			}

			PrintInfo("create service service_name: {} service_id: {} ok", sServiceBaseInfo.szName, sServiceBaseInfo.nID);

			this->m_vecServiceBaseInfo.push_back(sServiceBaseInfo);
			this->m_vecCoreService.push_back(pCoreService);
			this->m_mapCoreService[pCoreService->getServiceID()] = pCoreService;
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

			for (size_t i = 0; i < this->m_vecServiceBaseInfo.size(); ++i)
			{
				if (this->m_vecServiceBaseInfo[i].nID == this->m_vecCoreService[k]->getServiceID())
					continue;

				callback(this->m_vecServiceBaseInfo[i].szType, this->m_vecServiceBaseInfo[i].nID);
			}
		}

		return true;
	}

	CCoreService* CCoreServiceMgr::getCoreServiceByID(uint32_t nID) const
	{
		auto iter = this->m_mapCoreService.find(nID);
		if (iter == this->m_mapCoreService.end())
			return nullptr;

		return iter->second;
	}

	CCoreService* CCoreServiceMgr::getCoreServiceByName(const std::string& szName) const
	{
		auto iter = this->m_mapServiceName.find(szName);
		if (iter == this->m_mapServiceName.end())
			return nullptr;

		return this->getCoreServiceByID(iter->second);
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
		for (size_t i = 0; i < this->m_vecServiceBaseInfo.size(); ++i)
		{
			if (this->m_vecServiceBaseInfo[i].nID == nServiceID)
				return true;
		}

		return false;
	}
}