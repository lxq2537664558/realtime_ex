#pragma once

#include "libBaseCommon/singleton.h"

#include "base_app.h"
#include "service_base.h"
#include "logic_message_queue_mgr.h"
#include "transporter.h"
#include "core_service_mgr.h"
#include "service_registry_proxy.h"
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

		bool					runAndServe(size_t argc, char** argv, const std::vector<CServiceBase*>& vecServiceBase);
		
		uint32_t				getNodeID() const;
		const SNodeBaseInfo&	getNodeBaseInfo() const;

		void					registerTicker(CMessageQueue* pMessageQueue, CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		void					unregisterTicker(CTicker* pTicker);

		CLogicMessageQueueMgr*	getLogicMessageQueueMgr() const;
		CLogicMessageQueue*		getGlobalLogicMessageQueue() const;
		CCoreServiceMgr*		getCoreServiceMgr() const;
		CTransporter*			getTransporter() const;
		CServiceRegistryProxy*	getServiceRegistryProxy() const;
		CBaseConnectionMgr*		getBaseConnectionMgr() const;
		
		uint32_t				getInvokeTimeout(uint32_t nServiceID) const;

		const std::string&		getConfigFileName() const;
		
		base::CWriteBuf&		getWriteBuf() const;

		std::vector<char>&		getWebsocketBuf();
		
		void					doQuit();
		
		uint32_t				getHeartbeatLimit() const;
		
		uint32_t				getHeartbeatTime() const;
		
		uint32_t				getSamplingTime() const;

		uint32_t				getLogicThreadCount() const;

		uint32_t				getLocalServiceInvokeTimeout() const;

		uint32_t				getCoroutineStackSize() const;
		
		uint32_t				getQPS() const;
		
		void					incQPS();

	private:
		bool					init(size_t argc, char** argv, const std::vector<CServiceBase*>& vecServiceBase);
		void					destroy();
		void					onQPS(uint64_t nContext);
		void					printNodeInfo();

	protected:
		SNodeBaseInfo			m_sNodeBaseInfo;
		std::string				m_szConfig;
		std::string				m_szPID;
		base::CWriteBuf			m_writeBuf;
		std::vector<char>		m_vecWebsocketBuf;
		uint32_t				m_nCycleCount;
		int64_t					m_nTotalSamplingTime;
		uint32_t				m_nSamplingTime;
		volatile uint32_t		m_nQuiting;
		uint32_t				m_nHeartbeatLimit;
		uint32_t				m_nHeartbeatTime;
		uint32_t				m_nLogicThreadCount;
		uint32_t				m_nLocalServiceInvokeTimeout;
		uint32_t				m_nCoroutineStackSize;
		uint32_t				m_nQPS;
		CTicker					m_tickerQPS;

		std::vector<CLogicRunnable*>	m_vecLogicRunnable;
		CBaseConnectionMgr*				m_pBaseConnectionMgr;
		CLogicMessageQueueMgr*			m_pLogicMessageQueueMgr;
		CLogicMessageQueue*				m_pGlobalLogicMessageQueue;
		CCoreServiceMgr*				m_pCoreServiceMgr;
		CTransporter*					m_pTransporter;
		CServiceRegistryProxy*			m_pServiceRegistryProxy;
		CNodeConnectionFactory*			m_pNodeConnectionFactory;
	};
}