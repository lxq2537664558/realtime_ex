#pragma once

#include "libBaseCommon/singleton.h"

#include "base_app.h"
#include "core_connection_mgr.h"
#include "service_base.h"
#include "transporter.h"
#include "core_other_node_proxy.h"
#include "actor_scheduler.h"
#include "message_dispatcher.h"
#include "actor_id_converter.h"
#include "base_connection_to_master.h"
#include "node_connection_factory.h"
#include "service_base_mgr.h"

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
		
		void						registerTicker(uint8_t nType, uint16_t nFromServiceID, uint64_t nFromActorID, CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		void						unregisterTicker(CTicker* pTicker);

		CBaseConnectionMgr*			getBaseConnectionMgr() const;

		CServiceBaseMgr*			getServiceBaseMgr() const;

		bool						isOwnerService(uint16_t nServiceID) const;

		CTransporter*				getTransporter() const;
		CCoreOtherNodeProxy*		getCoreOtherNodeProxy() const;
		
		const SNodeBaseInfo&		getNodeBaseInfo() const;
		uint16_t					getNodeID() const;
		const std::vector<SServiceBaseInfo>&
									getServiceBaseInfo() const;


		uint32_t					getInvokeTimeout() const;

		uint32_t					getThroughput() const;

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
		CServiceBaseMgr*					m_pServiceBaseMgr;

		CTransporter*						m_pTransporter;
		CCoreOtherNodeProxy*				m_pCoreOtherNodeProxy;
		CNodeConnectionFactory*				m_pNodeConnectionFactory;
		SNodeBaseInfo						m_sNodeBaseInfo;
		std::vector<SServiceBaseInfo>		m_vecServiceBaseInfo;
		std::string							m_szMasterHost;
		uint16_t							m_nMasterPort;
		uint32_t							m_nInvokTimeout;
		uint32_t							m_nThroughput;
	};
}