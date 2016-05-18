#pragma once

#include "base_common.h"

namespace base
{
	template<class F>
	class CDeferWrapper
	{
	public:
		CDeferWrapper(F f) : m_f(f)
		{

		}

		~CDeferWrapper()
		{
			this->m_f();
		}

	private:
		F m_f;
	};

	template<class F>
	CDeferWrapper<F> makeDeferWrapper(F f)
	{
		return CDeferWrapper<F>(f);
	};
}

#define Defer(code) auto __##__LINE__##Defer = base::makeDeferWrapper( [=]() { code; } )