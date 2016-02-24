#pragma once

#include <vector>

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/tiny_list.h"
#include "libBaseCommon/noncopyable.h"
#include "libBaseCommon/noninheritable.h"

namespace core
{
	class CTicker;
	struct STickerNodeInfo
	{
		CTicker*	pTicker;
	};
	typedef base::STinyListNode<STickerNodeInfo> TickerNode_t;

	// 定时器对象不支持拷贝，因为拷贝时如果有定时行为，这个行为是带过去呢还是拷贝一个空的定时器呢？所以这里直接不允许拷贝
	// 如果你得类有拷贝需求，那么就需要以指针的形式持有CTicker对象，但是这样一来需要你自己管理这块内存，如果忘记删除，定时器还是在跑的，很可能就悲剧了
	// 这里建议以指针的形式持有CTicker对象用std::unique_ptr包裹
	class CTicker :
		public virtual base::noninheritable<CTicker>,
		public base::noncopyable
	{
		friend class CTickerMgr;
		
	public:
		CTicker();
		~CTicker();

		int64_t		getIntervalTime() const;
		int64_t		getNextTickTime() const;
		int64_t		getRemainTime() const;
		bool		isRegist() const;
		uint64_t	getContext() const;

		// C函数回调
		void		setCallback(void(*callback)(uint64_t))
		{
			DebugAst(callback != nullptr);
			DebugAst(sizeof(this->m_pRawCallback) == sizeof(callback));

			memcpy(&this->m_pRawCallback, &callback, sizeof(callback));
			this->setCallback(std::bind(callback, std::placeholders::_1));
		}

		// C++成员函数回调
		template<typename T>
		void		setCallback(void(T::*callback)(uint64_t), T* pObject)
		{
			DebugAst(callback != nullptr && pObject != nullptr);
			
			// 有些情况下callback大小是两个void*大小，因为涉及到this指针的切割
			void* pAddr[2] = { nullptr };
			memcpy(pAddr, &callback, sizeof(callback));
			this->m_pRawCallback = pAddr[0];

			this->setCallback(std::bind(callback, pObject, std::placeholders::_1));
		}

	private:
		void		setCallback(const std::function<void(uint64_t)>& callback);

	private:
		TickerNode_t*					m_pTickerNode;
		int64_t							m_nIntervalTime;	// 定时器运行的间隔时间
		int64_t							m_nNextTickTime;	// 下一次定时器运行时间
		uint64_t						m_nContext;
		std::function<void(uint64_t)>	m_callback;
		void*							m_pRawCallback;
	};
}