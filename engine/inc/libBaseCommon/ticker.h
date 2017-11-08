#pragma once

#include "noncopyable.h"
#include "noninheritable.h"

#include <functional>

namespace base
{
	class CTickerMgrImpl;
	
	// 定时器对象不支持直接拷贝，因为直接拷贝时如果有定时行为，这个行为是带过去呢还是拷贝一个空的定时器呢？
	// 这里定时器对象的拷贝只能通过用move的方式显式的拷贝
	// 当然你也可以以指针的形式持有CTicker对象，但是这样一来需要你自己管理这块内存，如果忘记删除，定时器还是在跑的，很可能就悲剧了
	// 这里建议以指针的形式持有CTicker对象用std::unique_ptr包裹
	class __BASE_COMMON_API__ CTicker :
		public virtual noninheritable<CTicker>,
		public noncopyable
	{
		friend class CTickerMgrImpl;
		
	public:
		CTicker();
		~CTicker();

		CTicker(CTicker&& rhs);
		CTicker& operator = (CTicker&& rhs);

		int64_t		getIntervalTime() const;
		bool		isRegister() const;
		uint64_t	getContext() const;
		void		setCallback(const std::function<void(uint64_t)>& callback, bool bCoroutine = false);
		std::function<void(uint64_t)>&
					getCallback();
		bool		isCoroutine() const;

	private:
		void*							m_pCoreContext;		// 这块数据只会有逻辑线程去读
		int64_t							m_nIntervalTime;	// 定时器运行的间隔时间
		uint64_t						m_nContext;
		std::function<void(uint64_t)>*	m_callback;
		bool							m_bCoroutine;
	};

	class __BASE_COMMON_API__ CTickerMgr :
		public virtual noninheritable<CTickerMgr>,
		public noncopyable
	{
	public:
		CTickerMgr(int64_t nTime, const std::function<void(CTicker*)>& callback);
		~CTickerMgr();

		int64_t	getLogicTime() const;
		void	update(int64_t nTime);

		bool	registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		void	unregisterTicker(CTicker* pTicker);

	private:
		CTickerMgrImpl*	m_pTickerMgrImpl;
	};
}