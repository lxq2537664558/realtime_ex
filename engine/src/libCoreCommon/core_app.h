#pragma once

#include "libBaseCommon/singleton.h"

#include "base_app.h"
#include "service_base.h"
#include "logic_message_queue_mgr.h"
#include "transporter.h"
#include "core_service_mgr.h"
#include "global_service_registry_proxy.h"
#include "node_connection_factory.h"
#include "logic_message_queue.h"
#include "base_connection_mgr_impl.h"
#include "base_connection_mgr.h"

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

		bool				runAndServe(size_t argc, char** argv, const std::vector<CServiceBase*>& vecServiceBase);
		void				doQuit();

		void				onFrame();

		uint32_t			getNodeID() const;
		const SNodeBaseInfo&
							getNodeBaseInfo() const;

		void				registerTicker(base::CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		void				unregisterTicker(base::CTicker* pTicker);

		CCoreServiceMgr*	getCoreServiceMgr() const;
		CLogicMessageQueueMgr*	
							getLogicMessageQueueMgr() const;
		CLogicMessageQueue*	getGlobalLogicMessageQueue() const;
		CBaseConnectionMgr*	getGlobalBaseConnectionMgr() const;
		CGlobalServiceRegistryProxy*
							getGlobalServiceRegistryProxy() const;

		uint32_t			getQPS() const;

		uint32_t			getDefaultServiceInvokeTimeout() const;
		const std::string&	getConfigFileName() const;
		uint32_t			getHeartbeatLimit() const;
		uint32_t			getHeartbeatTime() const;
		uint32_t			getSamplingTime() const;
		uint32_t			getLogicThreadCount() const;
		uint32_t			getCoroutineStackSize() const;

		base::CWriteBuf&	getWriteBuf() const;

	private:
		bool				init(size_t argc, char** argv, const std::vector<CServiceBase*>& vecServiceBase);
		void				destroy();

		void				printNodeInfo();

	private:
		SNodeBaseInfo					m_sNodeBaseInfo;
		std::string						m_szConfig;
		std::string						m_szPID;
		base::CWriteBuf					m_writeBuf;
		volatile uint32_t				m_nQuiting;
		
		uint32_t						m_nSamplingTime;
		uint32_t						m_nHeartbeatLimit;
		uint32_t						m_nHeartbeatTime;
		uint32_t						m_nLogicThreadCount;
		uint32_t						m_nDefaultServiceInvokeTimeout;
		uint32_t						m_nCoroutineStackSize;

		std::vector<CLogicRunnable*>	m_vecLogicRunnable;
		
		base::CTickerMgr*				m_pGlobalTickerMgr;
		CBaseConnectionMgr*				m_pGlobalBaseConnectionMgr;
		CLogicMessageQueue*				m_pGlobalLogicMessageQueue;
		CLogicMessageQueueMgr*			m_pLogicMessageQueueMgr;
		CCoreServiceMgr*				m_pCoreServiceMgr;
		CGlobalServiceRegistryProxy*	m_pGlobalServiceRegistryProxy;
		CNodeConnectionFactory*			m_pNodeConnectionFactory;
	};
}