#pragma once

#include "base_common.h"

#include <functional>

#define defer _DEFER_ACTION_MAKE /* ([&](){ ... }); */

// auto _defer_action_line???_ = _DeferredActionCtor([&](){ ... })
#define _DEFER_ACTION_MAKE auto _DEFER_ACTION_VAR(_defer_action_line, __LINE__, _) = base::DeferredActionCtor
#define _DEFER_ACTION_VAR(a, b, c) _DEFER_TOKEN_CONNECT(a, b, c)
#define _DEFER_TOKEN_CONNECT(a, b, c) a##b##c

namespace base
{
	// 持有闭包函数
	class CDeferredAction
	{
	public:
		CDeferredAction(CDeferredAction&& other)
			: m_func(std::forward<std::function<void()>>(other.m_func))
		{
			other.m_func = nullptr;
		}
		~CDeferredAction()
		{
			if (this->m_func != nullptr) 
				this->m_func();
		}

	private:
		template<class T>
		friend CDeferredAction DeferredActionCtor(T&& p);

		template<class T>
		CDeferredAction(T&& p) : m_func(std::bind(std::forward<T>(p))) { }

		CDeferredAction();
		CDeferredAction(CDeferredAction const&);
		CDeferredAction& operator=(CDeferredAction const&);
		CDeferredAction& operator=(CDeferredAction&&);

	private:
		std::function<void()> m_func;
	};

	template<class T>
	CDeferredAction DeferredActionCtor(T&& p)
	{
		return CDeferredAction(std::forward<T>(p));
	}
}