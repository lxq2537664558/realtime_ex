#pragma once

#include "libBaseCommon\noncopyable.h"

#include "future.h"

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

		uint64_t	getWaitCoroutineID() const;
		void		setValue(std::shared_ptr<T>& val, uint32_t nErrorCode = 0);

	private:
		std::shared_ptr<SFutureContext<T>>	m_pFutureContext;
		bool								m_bFuture;
	};
}

#include "promise.inl"