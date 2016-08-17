#pragma once

#include "libCoreCommon/core_common.h"

#include "future.h"

#include <memory>

namespace core
{
	template<class T>
	class CPromise :
		public base::noncopyable
	{
	public:
		CPromise();
		~CPromise();

		CPromise(CPromise<T>&& lhs);

		CPromise<T>& operator = (CPromise<T>&& lhs);

		CFuture<T>	getFuture();

		void		setValue(T val, uint32_t nErrorCode = 0);

	private:
		std::shared_ptr<SFutureContext<T>>	m_pFutureContext;
		bool								m_bFuture;
	};
}

#include "promise.inl"