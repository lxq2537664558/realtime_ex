#pragma once

#include "libBaseCommon/noncopyable.h"

#include "core_common.h"

#include <map>

namespace core
{
	class CConnectionToService;
	class CConnectionFromService;
	class CConnectionToMaster;
	class CServiceMgr :
		public base::noncopyable
	{
	public:
		CServiceMgr();
		~CServiceMgr();

		bool					init(bool bNormalService, const std::string& szMasterHost, uint16_t nMasterPort);

		CConnectionToService*	getConnectionToService(const std::string& szName) const;
		void					addConnectionToService(CConnectionToService* pConnectionToService);
		void					delConnectionToService(const std::string& szName);
		CConnectionFromService*	getConnectionFromService(const std::string& szName) const;
		void					addConnectionFromService(CConnectionFromService* pConnectionFromService);
		void					delConnectionFromService(const std::string& szName);

		CConnectionToMaster*	getConnectionToMaster() const;

		void					addOtherService(const SServiceBaseInfo& sServiceBaseInfo);
		void					delOtherService(const std::string& szServiceName);
		const SServiceBaseInfo*	getServiceBaseInfo(const std::string& szName) const;
		void					getServiceName(const std::string& szType, std::vector<std::string>& vecServiceName) const;

	private:
		void					onConnectRefuse(const std::string& szContext);
		void					onCheckConnect(uint64_t nContext);

	private:
		CTicker											m_tickCheckConnect;

		std::map<std::string, CConnectionToService*>	m_mapConnectionToService;
		std::map<std::string, CConnectionFromService*>	m_mapConnectionFromService;
		std::map<std::string, SServiceBaseInfo>			m_mapServiceBaseInfo;

		std::string										m_szMasterHost;
		uint16_t										m_nMasterPort;
		bool											m_bMasterRefuse;
	};

}