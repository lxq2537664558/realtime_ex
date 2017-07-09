#pragma once

#include "libBaseCommon/singleton.h"

#include "base_app.h"
#include "core_connection_mgr.h"
#include "service_base.h"
#include "transporter.h"
#include "core_other_node_proxy.h"
#include "core_message_registry.h"
#include "actor_scheduler.h"
#include "message_dispatcher.h"
#include "actor_id_converter.h"
#include "base_connection_to_master.h"
#include "node_connection_factory.h"

namespace core
{
	/**
	@brief: 核心框架类，单例，主要管理游戏中各个管理器
	*/
	class CCoreApp :
		public base::CSingleton<CCoreApp>
	{
	public:
		CCoreApp();
		~CCoreApp();

		bool						run(int32_t argc, char** argv, const char* szConfig);
		
		void						registerTicker(uint32_t nType, uint64_t nFrom, CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		void						unregisterTicker(CTicker* pTicker);

		CBaseConnectionMgr*			getBaseConnectionMgr() const;

		void						setServiceFactory(uint16_t nServiceID, CServiceFactory* pServiceFactory);
		const std::vector<CServiceBase*>
									getServiceBase() const;

		CTransporter*				getTransporter() const;
		CCoreOtherNodeProxy*		getCoreOtherNodeProxy() const;
		CCoreMessageRegistry*		getCoreMessageRegistry() const;
		CActorScheduler*			getActorScheduler() const;
		CMessageDispatcher*			getMessageDispatcher() const;
		void						setActorIDConverter(CActorIDConverter* pActorIDConverter);
		CActorIDConverter*			getActorIDConverter() const;

		CBaseConnectionToMaster*	getConnectionToMaster() const;
		void						setCoreConnectionToMaster(CBaseConnectionToMaster* pCoreConnectionToMaster);

		const SNodeBaseInfo&		getNodeBaseInfo() const;
		const std::vector<SServiceBaseInfo>&
									getServiceBaseInfo() const;


		uint32_t					getInvokeTimeout() const;

		uint32_t					getThroughput() const;

		void						setServiceConnectCallback(const std::function<void(uint16_t)>& callback);
		void						setServiceDisconnectCallback(const std::function<void(uint16_t)>& callback);
		std::function<void(uint16_t)>&
									getServiceConnectCallback();
		std::function<void(uint16_t)>&
									getServiceDisconnectCallback();

		const std::string&			getConfigFileName() const;
		
		base::CWriteBuf&			getWriteBuf() const;
		
		void						doQuit();
		
		uint32_t					getHeartbeatLimit() const;
		
		uint32_t					getHeartbeatTime() const;
		
		uint32_t					getSamplingTime() const;
		
		uint32_t					getQPS() const;
		
		void						incQPS();

	private:
		bool						init();
		void						destroy();
		void						onQPS(uint64_t nContext);

	protected:
		std::string							m_szConfig;
		std::string							m_szPID;
		base::CWriteBuf						m_writeBuf;
		uint32_t							m_nCycleCount;
		int64_t								m_nTotalSamplingTime;
		uint32_t							m_nSamplingTime;
		volatile uint32_t					m_nQuiting;
		uint32_t							m_nHeartbeatLimit;
		uint32_t							m_nHeartbeatTime;
		uint32_t							m_nQPS;
		CTicker								m_tickerQPS;
		std::vector<CServiceBase*>			m_vecServiceBase;

		CTicker								m_tickCheckConnectMaster;
		CTransporter*						m_pTransporter;
		CCoreMessageRegistry*				m_pCoreMessageRegistry;
		CActorScheduler*					m_pActorScheduler;
		CCoreOtherNodeProxy*				m_pCoreOtherNodeProxy;
		CMessageDispatcher*					m_pMessageDispatcher;
		CNodeConnectionFactory*				m_pNodeConnectionFactory;
		CActorIDConverter*					m_pActorIDConverter;
		CBaseConnectionToMaster*			m_pServiceConnectionToMaster;
		SNodeBaseInfo						m_sNodeBaseInfo;
		std::vector<SServiceBaseInfo>		m_vecServiceBaseInfo;
		std::string							m_szMasterHost;
		uint16_t							m_nMasterPort;
		uint32_t							m_nInvokTimeout;
		uint32_t							m_nThroughput;

		std::function<void(uint16_t)>		m_fnServiceConnectCallback;
		std::function<void(uint16_t)>		m_fnServiceDisconnectCallback;
	};
}