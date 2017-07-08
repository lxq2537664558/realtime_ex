#pragma once

#include "libBaseCommon/thread_base.h"
#include "libBaseCommon/spin_lock.h"
#include "libBaseCommon/link.h"

#include "ticker.h"

#include <memory>
#include <vector>

namespace core
{
	class CTickerRunnable;
	class CTicker;
	class CCoreTickerInfo
	{
		friend class CTickerRunnable;
		friend class CTicker;

	public:
		CCoreTickerInfo();
		~CCoreTickerInfo();

		CCoreTickerInfo(const CCoreTickerInfo& rhs) = delete;
		const CCoreTickerInfo& operator = (const CCoreTickerInfo& rhs) = delete;

		void	addRef();
		void	release();
		int32_t	getRef() const;

	private:
		CTicker*				m_pTicker;
		void*					m_pMemory;
		int64_t					m_nNextTime;		// 下一次定时器运行时间
		int64_t					m_nIntervalTime;	// 定时器运行的间隔时间
		std::atomic<int32_t>	m_nRef;
	};

	typedef base::TLinkNode<CCoreTickerInfo> CCoreTickerNode;

	class CTickerRunnable :
		public base::IRunnable
	{
	private:
		enum
		{
			__TIME_NEAR_BITS = 16,
			__TIME_CASCADE_BITS = 8,
			__TIME_CASCADE_COUNT = 3,
			__TIME_NEAR_SIZE = 1 << __TIME_NEAR_BITS,
			__TIME_NEAR_MASK = __TIME_NEAR_SIZE - 1,
			__TIME_CASCADE_SIZE = 1 << __TIME_CASCADE_BITS,
			__TIME_CASCADE_MASK = __TIME_CASCADE_SIZE - 1,
		};

	public:
		CTickerRunnable();
		virtual ~CTickerRunnable();

		static CTickerRunnable*	Inst();

		bool			init();

		bool			registerTicker(uint32_t nType, uint64_t nFrom, CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		void			unregisterTicker(CTicker* pTicker);
		
		void			join();
		void			release();

	private:
		virtual bool	onInit();
		virtual bool	onProcess();
		virtual void	onDestroy();

		void			update(int64_t nTime);

		void			insertTicker(CCoreTickerNode* pCoreTickerNode);
		void			cascadeTicker();
		void			onTicker(CCoreTickerNode* pCoreTickerNode);

	private:
		base::CThreadBase*				m_pThreadBase;
		base::TLink<CCoreTickerNode>	m_listNearTicker[__TIME_NEAR_SIZE];								// 最近运行到的时间刻度
		base::TLink<CCoreTickerNode>	m_listCascadeTicker[__TIME_CASCADE_COUNT][__TIME_CASCADE_SIZE];	// 联级时间刻度
		base::TLink<CCoreTickerNode>	m_listFarTicker;												// 最远的定时器链表

		std::vector<CCoreTickerNode*>	m_vecTempTickerNode;
		int64_t							m_nLogicTime;													// 当前刻度时间
		base::spin_lock					m_lock;
	};
}