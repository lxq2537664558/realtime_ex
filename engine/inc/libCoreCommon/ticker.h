#pragma once

#include "libBaseCommon/noncopyable.h"
#include "libBaseCommon/noninheritable.h"

#include "core_common.h"

#include <functional>


namespace core
{
	class CTickerRunnable;
	class CLogicRunnable;
	class CNetRunnable;
	
	// 定时器对象不支持直接拷贝，因为直接拷贝时如果有定时行为，这个行为是带过去呢还是拷贝一个空的定时器呢？
	// 这里定时器对象的拷贝只能通过用move的方式
	// 当然你也可以以指针的形式持有CTicker对象，但是这样一来需要你自己管理这块内存，如果忘记删除，定时器还是在跑的，很可能就悲剧了
	// 这里建议以指针的形式持有CTicker对象用std::unique_ptr包裹
	class __CORE_COMMON_API__ CTicker :
		public virtual base::noninheritable<CTicker>,
		public base::noncopyable
	{
		friend class CTickerRunnable;
		friend class CLogicRunnable;
		friend class CNetRunnable;
		
	public:
		CTicker();
		~CTicker();

		CTicker(CTicker&& rhs);
		CTicker& operator = (CTicker&& rhs);

		int64_t		getIntervalTime() const;
		bool		isRegister() const;
		uint64_t	getContext() const;
		void		setCallback(const std::function<void(uint64_t)>& callback);
		std::function<void(uint64_t)>&
					getCallback();

	private:
		void*							m_pCoreContext;		// 这块数据只会有逻辑线程去读
		int64_t							m_nIntervalTime;	// 定时器运行的间隔时间
		uint64_t						m_nContext;
		std::function<void(uint64_t)>*	m_callback;
	};
}