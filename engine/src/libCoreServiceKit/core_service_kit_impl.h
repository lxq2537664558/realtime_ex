#pragma once

#include "libBaseCommon/singleton.h"
#include "libBaseLua/lua_facade.h"
#include "libCoreCommon/core_common.h"

#include "core_service_kit_define.h"
#include "transporter.h"
#include "core_service_proxy.h"
#include "core_service_invoker.h"
#include "core_service_invoker.h"
#include "invoker_trace.h"

#include <map>

namespace core
{
	class CCoreServiceConnection;
	class CCoreConnectionFromService;
	class CCoreConnectionToMaster;
	class CServiceConnectionFactory;
	class CCoreServiceKitImpl :
		public base::CSingleton<CCoreServiceKitImpl>
	{
	public:
		CCoreServiceKitImpl();
		~CCoreServiceKitImpl();

		bool						init();
		void						release();

		CTransporter*				getTransporter() const;
		CCoreServiceProxy*			getCoreServiceProxy() const;
		CCoreServiceInvoker*		getCoreServiceInvoker() const;
		CInvokerTrace*				getInvokerTrace() const;

		CCoreConnectionToMaster*	getConnectionToMaster() const;

		const SServiceBaseInfo&		getServiceBaseInfo() const;

		void						addGlobalBeforeFilter(const ServiceGlobalFilter& callback);
		void						addGlobalAfterFilter(const ServiceGlobalFilter& callback);

		const std::vector<ServiceGlobalFilter>&	
									getGlobalBeforeFilter();
		const std::vector<ServiceGlobalFilter>&	
									getGlobalAfterFilter();

		base::CLuaFacade*			getLuaFacade() const;

		uint32_t					getInvokeTimeout() const;

	private:
		void						onConnectRefuse(const std::string& szContext);
		void						onCheckConnectMaster(uint64_t nContext);

	private:
		CTicker								m_tickCheckConnectMaster;
		CTransporter*						m_pTransporter;
		CCoreServiceInvoker*				m_pCoreServiceInvoker;
		CCoreServiceProxy*					m_pCoreServiceProxy;
		CInvokerTrace*						m_pInvokerTrace;
		CServiceConnectionFactory*			m_pServiceConnectionFactory;
		SServiceBaseInfo					m_sServiceBaseInfo;
		std::string							m_szMasterHost;
		uint16_t							m_nMasterPort;
		uint32_t							m_nInvokTimeout;

		base::CLuaFacade*					m_pLuaFacade;

		std::vector<ServiceGlobalFilter>	m_vecServiceGlobalBeforeFilter;
		std::vector<ServiceGlobalFilter>	m_vecServiceGlobalAfterFilter;
	};

}