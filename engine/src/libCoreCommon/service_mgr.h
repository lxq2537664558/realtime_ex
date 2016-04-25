#pragma once

#include "libBaseCommon/noncopyable.h"

#include "core_common.h"

#include <map>

namespace core
{
	class CCoreConnectionToService;
	class CCoreConnectionFromService;
	class CCoreConnectionToMaster;
	class CServiceMgr :
		public base::noncopyable
	{
	public:
		CServiceMgr();
		~CServiceMgr();

		bool	init(bool bNormalService, const std::string& szMasterHost, uint16_t nMasterPort);

		CCoreConnectionToService*
				getConnectionToService(const std::string& szName) const;
		bool	addConnectionToService(CCoreConnectionToService* pCoreConnectionToService);
		void	delConnectionToService(const std::string& szName);
		CCoreConnectionFromService*
				getConnectionFromService(const std::string& szName) const;
		bool	addConnectionFromService(CCoreConnectionFromService* pCoreConnectionFromService);
		void	delConnectionFromService(const std::string& szName);

		CCoreConnectionToMaster*
				getConnectionToMaster() const;

		void	addOtherService(const SServiceBaseInfo& sServiceBaseInfo);
		void	delOtherService(const std::string& szServiceName);
		const SServiceBaseInfo*
				getServiceBaseInfo(const std::string& szName) const;
		void	getServiceName(const std::string& szType, std::vector<std::string>& vecServiceName) const;

	private:
		void	onConnectRefuse(const std::string& szContext);
		void	onCheckConnectMaster(uint64_t nContext);

	private:
		CTicker			m_tickCheckConnectMaster;

		std::map<std::string, CCoreConnectionToService*>
						m_mapConnectionToService;
		std::map<std::string, CCoreConnectionFromService*>	
						m_mapConnectionFromService;
		std::map<std::string, SServiceBaseInfo>
						m_mapServiceBaseInfo;

		std::string		m_szMasterHost;
		uint16_t		m_nMasterPort;
	};

}