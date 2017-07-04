#pragma once

#include "libBaseCommon/singleton.h"
#include "libBaseLua/lua_facade.h"
#include "libCoreCommon/core_common.h"

#include "scheduler.h"
#include "transporter.h"
#include "core_other_node_proxy.h"
#include "core_message_registry.h"
#include "message_dispatcher.h"
#include "core_service_app.h"

#include <map>

namespace core
{
	class CCoreServiceConnection;
	class CCoreConnectionOtherNode;
	class CCoreConnectionToMaster;
	class CNodeConnectionFactory;
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
		CCoreOtherNodeProxy*		getCoreOtherNodeProxy() const;
		CCoreMessageRegistry*		getCoreMessageRegistry() const;
		CScheduler*					getScheduler() const;
		CMessageDispatcher*			getMessageDispatcher() const;
		void						setActorIDConverter(CActorIDConverter* pActorIDConverter);
		CActorIDConverter*			getActorIDConverter() const;

		CCoreConnectionToMaster*	getConnectionToMaster() const;
		void						setCoreConnectionToMaster(CCoreConnectionToMaster* pCoreConnectionToMaster);

		const SNodeBaseInfo&		getNodeBaseInfo() const;
		const std::vector<SServiceBaseInfo>&
									getServiceBaseInfo() const;

		base::CLuaFacade*			getLuaFacade() const;

		uint32_t					getInvokeTimeout() const;

		uint32_t					getThroughput() const;

		void						setNodeConnectCallback(const std::function<void(uint16_t)>& callback);
		void						setNodeDisconnectCallback(const std::function<void(uint16_t)>& callback);
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
		CCoreMessageRegistry*				m_pCoreMessageRegistry;
		CScheduler*							m_pScheduler;
		CCoreOtherNodeProxy*				m_pCoreOtherNodeProxy;
		CMessageDispatcher*					m_pMessageDispatcher;
		CNodeConnectionFactory*				m_pNodeConnectionFactory;
		CActorIDConverter*					m_pActorIDConverter;
		CCoreConnectionToMaster*			m_pCoreConnectionToMaster;
		SNodeBaseInfo						m_sNodeBaseInfo;
		std::vector<SServiceBaseInfo>		m_vecServiceBaseInfo;
		std::string							m_szMasterHost;
		uint16_t							m_nMasterPort;
		uint32_t							m_nInvokTimeout;
		uint32_t							m_nThroughput;

		base::CLuaFacade*					m_pLuaFacade;

		std::function<void(uint16_t)>		m_fnNodeConnectCallback;
		std::function<void(uint16_t)>		m_fnNodeDisconnectCallback;
	};

}