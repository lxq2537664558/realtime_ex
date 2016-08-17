namespace core
{
	template<class T>
	CPromise<T>::CPromise()
		: m_bFuture(false)
	{
		this->m_pFutureContext = std::make_shared<SFutureContext<T>>();
		this->m_pFutureContext->bReady = false;
		this->m_pFutureContext->nErrorCode = 0;
	}

	template<class T>
	CPromise<T>::~CPromise()
	{

	}

	template<class T>
	core::CPromise<T>::CPromise(CPromise<T>&& lhs)
	{
		this->m_bFuture = lhs.m_bFuture;
		this->m_pFutureContext = lhs.m_pFutureContext;

		lhs.m_bFuture = false;
		lhs.m_pFutureContext = nullptr;
	}

	template<class T>
	CPromise<T>& core::CPromise<T>::operator = (CPromise<T>&& lhs)
	{
		if (this == &lhs)
			return *this;

		this->m_bFuture = lhs.m_bFuture;
		this->m_pFutureContext = lhs.m_pFutureContext;

		lhs.m_bFuture = false;
		lhs.m_pFutureContext = nullptr;

		return *this;
	}

	template<class T>
	CFuture<T> CPromise<T>::getFuture()
	{
		CFuture<T> sFuture;
		if (this->m_bFuture)
			return sFuture;

		this->m_bFuture = true;
		sFuture.m_pContext = this->m_pFutureContext;
		return sFuture;
	}

	template<class T>
	void CPromise<T>::setValue(T val, uint32_t nErrorCode /* = 0 */)
	{
		DebugAst(this->m_pFutureContext != nullptr);

		if (this->m_pFutureContext->bReady)
		{
			PrintWarning("double set promise value");
			return;
		}

		this->m_pFutureContext->val = val;
		this->m_pFutureContext->nErrorCode = nErrorCode;
		this->m_pFutureContext->bReady = true;

		if (this->m_pFutureContext->callback == nullptr)
			return;

		this->m_pFutureContext->callback(val, nErrorCode);
	}
}