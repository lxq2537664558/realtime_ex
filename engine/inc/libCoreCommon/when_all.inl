namespace core
{
	template<class ALL>
	struct SWhenAllContext
	{
		CPromise<ALL>			promise;
		std::shared_ptr<ALL>	allFuture;
		int32_t					pendingCount;

		template<class T>
		void onFuture(const T* val, uint32_t nErrorCode)
		{
			if (--pendingCount == 0)
			{
				// when形式的future回调全部为eRRT_OK，业务自己去判断各个子项的错误码
				promise.setValue(allFuture, eRRT_OK);
			}
		}
	};

	template<int32_t N, class ...Args>
	struct SBindArgs
	{
		template<class T, class ...RemainArgs>
		static bool bind(std::shared_ptr<SWhenAllContext<std::tuple<Args...>>> pContext, T& sFuture, RemainArgs&... args)
		{
			std::get<N>(*pContext->allFuture) = sFuture;
			sFuture.then(std::bind(&SWhenAllContext<std::tuple<Args...>>::template onFuture<typename T::ValueType>, pContext, std::placeholders::_1, std::placeholders::_2));

			return SBindArgs<N + 1, Args...>::bind(pContext, args...);
		}

		static bool bind(std::shared_ptr<SWhenAllContext<std::tuple<Args...>>> pContext)
		{
			return true;
		}
	};

	template<class ...Args>
	CFuture<std::tuple<Args...>> whenAll(Args... args)
	{
		auto pContext = std::make_shared<SWhenAllContext<std::tuple<Args...>>>();
		pContext->allFuture = std::make_shared<std::tuple<Args...>>();
		pContext->pendingCount = sizeof...(Args);

		SBindArgs<0, Args...>::bind(pContext, args...);

		return pContext->promise.getFuture();
	}
}