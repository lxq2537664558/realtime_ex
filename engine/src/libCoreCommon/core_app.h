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

		bool					runAndServe(size_t argc, char** argv, const std::vector<CServiceBase*>& vecServiceBase);
		
		uint32_t				getNodeID() const;
		const SNodeBaseInfo&	getNodeBaseInfo() const;

		void					registerTicker(uint8_t nType, uint32_t nFromServiceID, uint64_t nFromActorID, CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		void					unregisterTicker(CTicker* pTicker);

		CNetRunnable*			getNetRunnable() const;
		CLogicRunnable*			getLogicRunnable() const;
		CTickerRunnable*		getTickerRunnable() const;

		uint32_t				getInvokeTimeout(uint32_t nServiceID) const;

		const std::string&		getConfigFileName() const;
		
		base::CWriteBuf&		getWriteBuf() const;

		std::vector<char>&		getWebsocketBuf();
		
		void					doQuit();
		
		uint32_t				getHeartbeatLimit() const;
		
		uint32_t				getHeartbeatTime() const;
		
		uint32_t				getSamplingTime() const;

		uint32_t				getLocalServiceInvokeTimeout() const;

		uint32_t				getCoroutineStackSize() const;
		
		uint32_t				getQPS() const;
		
		void					incQPS();

	private:
		bool					init(size_t argc, char** argv, const std::vector<CServiceBase*>& vecServiceBase);
		void					destroy();
		void					onQPS(uint64_t nContext);

	protected:
		SNodeBaseInfo		m_sNodeBaseInfo;
		std::string			m_szConfig;
		std::string			m_szPID;
		base::CWriteBuf		m_writeBuf;
		std::vector<char>	m_vecWebsocketBuf;
		uint32_t			m_nCycleCount;
		int64_t				m_nTotalSamplingTime;
		uint32_t			m_nSamplingTime;
		volatile uint32_t	m_nQuiting;
		uint32_t			m_nHeartbeatLimit;
		uint32_t			m_nHeartbeatTime;
		uint32_t			m_nLocalServiceInvokeTimeout;
		uint32_t			m_nCoroutineStackSize;
		uint32_t			m_nQPS;
		CTicker				m_tickerQPS;
		CNetRunnable*		m_pNetRunnable;
		CLogicRunnable*		m_pLogicRunnable;
		CTickerRunnable*	m_pTickerRunnable;

		CNetMessageQueue*	m_pNetMessageQueue;
		CLogicMessageQueue*	m_pLogicMessageQueue;
	};
}