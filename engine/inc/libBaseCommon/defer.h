#pragma once

#include "base_common.h"
#include "noncopyable.h"

namespace base
{
	template<class FUN>
	class CDeferWrapper :
		public noncopyable
	{
	public:
		CDeferWrapper(FUN fun) : m_fun(fun)
		{

		}

		~CDeferWrapper()
		{
			this->m_fun();
		}

	private:
		FUN m_fun;
	};

	template<class FUN>
	CDeferWrapper<FUN> makeDeferWrapper(FUN fun)
	{
		return CDeferWrapper<FUN>(fun);
	};
}

#define Defer(code) auto __##__LINE__##Defer = base::makeDeferWrapper( [=]() { code; } )