#pragma once

#include "libBaseCommon/singleton.h"
#include "libCoreCommon/core_common.h"

#include "core_service_kit_define.h"
#include "transporter.h"
#include "core_service_proxy.h"
#include "core_service_invoker.h"
#include "core_service_invoker.h"
#include "load_balance_mgr.h"

#include <map>

namespace core
{
	class CCoreConnectionToService;
	class CCoreConnectionFromService;
	class CCoreConnectionToMaster;
	class CCoreServiceKitImpl :
		public base::CSingleton<CCoreServiceKitImpl>
	{
	public:
		CCoreServiceKitImpl();
		~CCoreServiceKitImpl();

		bool						init();
		void						release();

		CTransporter*				getTransporter() const;
		CLoadBalanceMgr*			getLoadBalanceMgr() const;
		CCoreServiceProxy*			getCoreServiceProxy() const;
		CCoreServiceInvoker*		getCoreServiceInvoker() const;

		CCoreConnectionToMaster*	getConnectionToMaster() const;

		const SServiceBaseInfo&		getServiceBaseInfo() const;
		void						getServiceName(const std::string& szType, std::vector<std::string>& vecServiceName) const;

		void						addGlobalBeforeFilter(const ServiceGlobalFilter& callback);
		void						addGlobalAfterFilter(const ServiceGlobalFilter& callback);

		const std::vector<ServiceGlobalFilter>&	
									getGlobalBeforeFilter();
		const std::vector<ServiceGlobalFilter>&	
									getGlobalAfterFilter();

	private:
		void						onConnectRefuse(const std::string& szContext);
		void						onCheckConnectMaster(uint64_t nContext);

	private:
		CTicker								m_tickCheckConnectMaster;
		CTransporter*						m_pTransporter;
		CCoreServiceInvoker*				m_pCoreServiceInvoker;
		CCoreServiceProxy*					m_pCoreServiceProxy;
		CLoadBalanceMgr*					m_pLoadBalanceMgr;
		SServiceBaseInfo					m_sServiceBaseInfo;
		std::string							m_szMasterHost;
		uint16_t							m_nMasterPort;

		std::vector<ServiceGlobalFilter>	m_vecServiceGlobalBeforeFilter;
		std::vector<ServiceGlobalFilter>	m_vecServiceGlobalAfterFilter;
	};

}