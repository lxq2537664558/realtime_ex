#pragma once

#include "libBaseCommon/singleton.h"

#include "base_app.h"
#include "service_base.h"
#include "logic_runnable.h"
#include "net_runnable.h"
#include "ticker_runnable.h"

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

		bool						run(const std::string& szInstanceName, const std::string& szConfig);
		
		uint32_t					getNodeID() const;
		const SNodeBaseInfo&		getNodeBaseInfo() const;

		void						registerTicker(uint8_t nType, uint32_t nFromServiceID, uint64_t nFromActorID, CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		void						unregisterTicker(CTicker* pTicker);

		void						addGlobalBeforeFilter(const std::string& szKey, NodeGlobalFilter callback);
		void						delGlobalBeforeFilter(const std::string& szKey);
		void						addGlobalAfterFilter(const std::string& szKey, NodeGlobalFilter callback);
		void						delGlobalAfterFilter(const std::string& szKey);

		CNetRunnable*				getNetRunnable() const;
		CLogicRunnable*				getLogicRunnable() const;
		CTickerRunnable*			getTickerRunnable() const;

		uint32_t					getInvokeTimeout() const;

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
		SNodeBaseInfo						m_sNodeBaseInfo;
		std::string							m_szConfig;
		std::string							m_szPID;
		base::CWriteBuf						m_writeBuf;
		uint32_t							m_nCycleCount;
		int64_t								m_nTotalSamplingTime;
		uint32_t							m_nSamplingTime;
		volatile uint32_t					m_nQuiting;
		uint32_t							m_nHeartbeatLimit;
		uint32_t							m_nHeartbeatTime;
		uint32_t							m_nInvokeTimeout;
		uint32_t							m_nQPS;
		CTicker								m_tickerQPS;
		CNetRunnable*						m_pNetRunnable;
		CLogicRunnable*						m_pLogicRunnable;
		CTickerRunnable*					m_pTickerRunnable;
	};
}