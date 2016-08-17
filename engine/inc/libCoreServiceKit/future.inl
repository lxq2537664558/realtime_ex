namespace core
{
	template<class T>
	CFuture<T>::CFuture()
		: m_pContext(nullptr)
	{

	}

	template<class T>
	CFuture<T>::CFuture(std::shared_ptr<SFutureContext<T>> pContext)
		: m_pContext(pContext)
	{

	}

	template<class T>
	CFuture<T>::~CFuture()
	{

	}

	template<class T>
	bool core::CFuture<T>::isReady() const
	{
		if (!this->isVaild())
			return false;

		return this->m_pContext->bReady;
	}

	template<class T>
	bool core::CFuture<T>::isVaild() const
	{
		return this->m_pContext != nullptr;
	}

	template<class T>
	bool core::CFuture<T>::getValue(T& val) const
	{
		if (!this->isReady())
			return false;

		val = this->m_pContext->val;

		return true;
	}

	template<class T>
	uint32_t core::CFuture<T>::getError() const
	{
		if (!this->isReady())
			return 0;

		return this->m_pContext->nErrorCode;
	}

	template<class T>
	void CFuture<T>::then(const std::function<void(T, uint32_t)>& fn)
	{
		DebugAst(this->m_pContext != nullptr && fn != nullptr);

		this->m_pContext->callback = fn;

		if (this->m_pContext->bReady && this->m_pContext->nErrorCode == 0)
			this->m_pContext->callback(this->m_pContext->val, this->m_pContext->nErrorCode);
	}

	template<class T>
	template<class F, class R>
	R CFuture<T>::then_r(F& fn)
	{
		DebugAstEx(this->m_pContext != nullptr, R());

		std::shared_ptr<SFutureContext<R::ValueType>> pContext = std::make_shared<SFutureContext<R::ValueType>>();
		pContext->bReady = false;
		pContext->nErrorCode = 0;

		this->m_pContext->callback = [fn, pContext](T val, uint32_t nErrorCode)->void
		{
			CFuture<R::ValueType> sNewFuture = fn(val, nErrorCode);
			if (sNewFuture.m_pContext == nullptr)
				return;
			
			// 有可能返回的future已经是一个准备好的future，所以需要拷贝到pContext中
			pContext->bReady = sNewFuture.m_pContext->bReady;
			pContext->val = sNewFuture.m_pContext->val;
			pContext->nErrorCode = sNewFuture.m_pContext->nErrorCode;
			sNewFuture.m_pContext->callback = pContext->callback;
			// 在回调完成后如果返回的future已经准备好，并且回调函数也已经设置好了，就回调他，不然就没有机会回调了
			if (sNewFuture.m_pContext->bReady && sNewFuture.m_pContext->callback != nullptr)
				sNewFuture.m_pContext->callback(sNewFuture.m_pContext->val, sNewFuture.m_pContext->nErrorCode);
		};

		// 在调用then时future已经可用了
		if (this->m_pContext->bReady)
			this->m_pContext->callback(this->m_pContext->val, this->m_pContext->nErrorCode);
		
		return CFuture<R::ValueType>(pContext);
	}

	template<class T>
	void core::CFuture<T>::collect(std::vector<CFuture<T>>& vecFuture, const std::function<void(const std::vector<CFuture<T>>&)>& fn)
	{
		for (size_t i = 0; i < vecFuture.size(); ++i)
		{
			vecFuture[i].then([fn, vecFuture](T val, uint32_t)
			{
				bool bReady = true;
				for (size_t j = 0; j < vecFuture.size(); ++j)
				{
					if (!vecFuture[j].isReady())
					{
						bReady = false;
						break;
					}
				}

				if (bReady)
					fn(vecFuture);
			});
		}
	}

	template<class T>
	template<class F, class R>
	R CFuture<T>::collect_r(std::vector<CFuture<T>>& vecFuture, F& fn)
	{
		std::shared_ptr<SFutureContext<R::ValueType>> pContext = std::make_shared<SFutureContext<R::ValueType>>();
		pContext->bReady = false;
		pContext->nErrorCode = 0;

		for (size_t i = 0; i < vecFuture.size(); ++i)
		{
			vecFuture[i].then([fn, vecFuture, pContext](T val, uint32_t)
			{
				bool bReady = true;
				for (size_t j = 0; j < vecFuture.size(); ++j)
				{
					if (!vecFuture[j].isReady())
					{
						bReady = false;
						break;
					}
				}

				if (bReady)
				{
					CFuture<R::ValueType> sNewFuture = fn(vecFuture);
					if (sNewFuture.m_pContext == nullptr)
						return;

					// 有可能返回的future已经是一个准备好的future，所以需要拷贝到pContext中
					pContext->bReady = sNewFuture.m_pContext->bReady;
					pContext->val = sNewFuture.m_pContext->val;
					pContext->nErrorCode = sNewFuture.m_pContext->nErrorCode;
					sNewFuture.m_pContext->callback = pContext->callback;
					// 在回调完成后如果返回的future已经准备好，并且回调函数也已经设置好了，就回调他，不然就没有机会回调了
					if (sNewFuture.m_pContext->bReady && sNewFuture.m_pContext->callback != nullptr)
						sNewFuture.m_pContext->callback(sNewFuture.m_pContext->val, sNewFuture.m_pContext->nErrorCode);
				}
			});
		}

		return CFuture<R::ValueType>(pContext);
	}

	template<class T>
	CFuture<T> core::CFuture<T>::createFuture(T val)
	{
		std::shared_ptr<SFutureContext<T>> pContext = std::make_shared<SFutureContext<T>>();
		pContext->bReady = true;
		pContext->val = val;

		return CFuture<T>(pContext);
	}
}