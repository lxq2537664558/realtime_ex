#pragma once

#include "libCoreCommon/core_common.h"

#include "message_ptr.h"

#include <memory>

namespace core
{
	template<class T>
	struct SFutureContext
	{
		std::function<void(T, uint32_t)>
					callback;
		bool		bReady;
		uint32_t	nErrorCode;
		T			val;
	};

	// 支持串联并联的future
	template<class T>
	class CFuture
	{
		template<class U>
		friend class CPromise;
		template<class U>
		friend class CFuture;

	public:
		typedef T ValueType;

	public:
		CFuture();
		CFuture(std::shared_ptr<SFutureContext<T>> pContext);
		~CFuture();

		bool		isReady() const;
		bool		isVaild() const;
		bool		getValue(T& val) const;
		uint32_t	getErrorCode() const;

		void		then(const std::function<void(T, uint32_t)>& fn);
		
		template<class F, class R = typename std::result_of<F(T, uint32_t)>::type>
		R			then_r(F& fn);

		static void	collect(std::vector<CFuture<T>>& vecFuture, const std::function<void(const std::vector<CFuture<T>>&)>& fn);

		template<class F, class R = typename std::result_of<F(const std::vector<CFuture<T>>&)>::type>
		static R	collect_r(std::vector<CFuture<T>>& vecFuture, F& fn);

		static CFuture<T>
					createFuture(T val);

	private:
		std::shared_ptr<SFutureContext<T>>	m_pContext;
	};
}

#include "future.inl"