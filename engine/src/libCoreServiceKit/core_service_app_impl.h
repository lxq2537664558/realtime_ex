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
#include "scheduler.h"

#include <map>

namespace core
{
	class CCoreServiceConnection;
	class CCoreConnectionFromService;
	class CCoreConnectionToMaster;
	class CServiceConnectionFactory;
	class CCoreServiceAppImpl :
		public base::CSingleton<CCoreServiceAppImpl>
	{
	public:
		CCoreServiceAppImpl();
		~CCoreServiceAppImpl();

		bool						init();
		void						release();
		void						run();

		CTransporter*				getTransporter() const;
		CCoreServiceProxy*			getCoreServiceProxy() const;
		CCoreServiceInvoker*		getCoreServiceInvoker() const;
		CInvokerTrace*				getInvokerTrace() const;
		CScheduler*					getScheduler() const;

		CCoreConnectionToMaster*	getConnectionToMaster() const;
		void						setCoreConnectionToMaster(CCoreConnectionToMaster* pCoreConnectionToMaster);

		const SServiceBaseInfo&		getServiceBaseInfo() const;

		void						addGlobalBeforeFilter(const ServiceGlobalFilter& callback);
		void						addGlobalAfterFilter(const ServiceGlobalFilter& callback);

		const std::vector<ServiceGlobalFilter>&
									getGlobalBeforeFilter();
		const std::vector<ServiceGlobalFilter>&
									getGlobalAfterFilter();

		base::CLuaFacade*			getLuaFacade() const;

		uint32_t					getInvokeTimeout() const;

		uint32_t					getThroughput() const;

		void						setServiceConnectCallback(std::function<void(uint16_t)> funConnect);
		void						setServiceDisconnectCallback(std::function<void(uint16_t)> funDisconnect);
		std::function<void(uint16_t)>&
									getServiceConnectCallback();
		std::function<void(uint16_t)>&
									getServiceDisconnectCallback();

	private:
		void						onConnectRefuse(const std::string& szContext);
		void						onCheckConnectMaster(uint64_t nContext);

	private:
		CTicker								m_tickCheckConnectMaster;
		CTransporter*						m_pTransporter;
		CCoreServiceInvoker*				m_pCoreServiceInvoker;
		CScheduler*							m_pScheduler;
		CCoreServiceProxy*					m_pCoreServiceProxy;
		CInvokerTrace*						m_pInvokerTrace;
		CServiceConnectionFactory*			m_pServiceConnectionFactory;
		CCoreConnectionToMaster*			m_pCoreConnectionToMaster;
		SServiceBaseInfo					m_sServiceBaseInfo;
		std::string							m_szMasterHost;
		uint16_t							m_nMasterPort;
		uint32_t							m_nInvokTimeout;
		uint32_t							m_nThroughput;

		base::CLuaFacade*					m_pLuaFacade;

		std::vector<ServiceGlobalFilter>	m_vecGlobalBeforeFilter;
		std::vector<ServiceGlobalFilter>	m_vecGlobalAfterFilter;

		std::function<void(uint16_t)>		m_serviceConnectCallback;
		std::function<void(uint16_t)>		m_serviceDisconnectCallback;
	};

}