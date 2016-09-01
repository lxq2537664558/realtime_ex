namespace core
{

	template<class T>
	void CMessagePtr<T>::reset(T* pData, int32_t* pRefCount, uint16_t nMessageID, const std::function<void(void*)>& del, bool bDel)
	{
		if (this->m_pRefCount != nullptr && bDel)
			this->decRef();

		this->m_del = del;
		this->m_pData = pData;
		this->m_nMessageID = nMessageID;
		this->m_pRefCount = pRefCount;
		if (pRefCount != nullptr)
			this->incRef();
	}

	template<class T>
	CMessagePtr<T>::CMessagePtr()
		: m_pData(nullptr)
		, m_pRefCount(nullptr)
		, m_nMessageID(0)
	{

	}

	template<class T>
	CMessagePtr<T>::CMessagePtr(nullptr_t)
		: m_pData(nullptr)
		, m_pRefCount(nullptr)
		, m_nMessageID(0)
	{

	}

	template<class T>
	CMessagePtr<T>::CMessagePtr(T* pData, uint16_t nMessageID)
	{
		int32_t* pRefCount = nullptr;
		if (pData != nullptr)
			pRefCount = new int32_t(0);

		this->reset(pData, pRefCount, nMessageID, nullptr, false);
	}

	template<class T>
	CMessagePtr<T>::CMessagePtr(T* pData, uint16_t nMessageID, std::function<void(void*)> del)
		: m_nMessageID(nMessageID)
	{
		int32_t* pRefCount = nullptr;
		if (pData != nullptr)
			pRefCount = new int32_t(0);

		this->reset(pData, pRefCount, nMessageID, del, false);
	}

	template<class T>
	CMessagePtr<T>::CMessagePtr(const CMessagePtr<T>& rhs)
	{
		this->reset(rhs.m_pData, rhs.m_pRefCount, rhs.m_nMessageID, rhs.m_del, false);
	}

	template<class T>
	CMessagePtr<T>::CMessagePtr(CMessagePtr<T>&& rhs)
	{
		this->m_pData = rhs.m_pData;
		this->m_pRefCount = rhs.m_pRefCount;
		this->m_del = rhs.m_del;
		this->m_nMessageID = rhs.m_nMessageID;

		rhs.m_pData = nullptr;
		rhs.m_pRefCount = nullptr;
		rhs.m_del = nullptr;
		rhs.m_nMessageID = 0;
	}

	template<class T>
	CMessagePtr<T>::~CMessagePtr()
	{
		this->decRef();
	}

	template<class T>
	CMessagePtr<T>& CMessagePtr<T>::operator = (CMessagePtr<T>& rhs)
	{
		if (&rhs == this)
			return *this;

		this->reset(rhs.m_pData, rhs.m_pRefCount, rhs.m_nMessageID, rhs.m_del, true);

		return *this;
	}

	template<class T>
	CMessagePtr<T>& CMessagePtr<T>::operator = (CMessagePtr<T>&& rhs)
	{
		if (&rhs == this)
			return *this;

		this->decRef();

		this->m_pData = rhs.m_pData;
		this->m_pRefCount = rhs.m_pRefCount;
		this->m_del = rhs.m_del;
		this->m_nMessageID = rhs.m_nMessageID;

		rhs.m_pData = nullptr;
		rhs.m_pRefCount = nullptr;
		rhs.m_del = nullptr;

		return *this;
	}

	template<class T>
	void CMessagePtr<T>::decRef()
	{
		if (this->m_pRefCount != nullptr)
		{
			--*this->m_pRefCount;
			if (*this->m_pRefCount <= 0)
			{
				if (this->m_del != nullptr)
					this->m_del(this->m_pData);
				SAFE_DELETE(this->m_pRefCount);
			}
		}
	}

	template<class T>
	void CMessagePtr<T>::incRef()
	{
		if (this->m_pRefCount != nullptr)
			++*this->m_pRefCount;
	}

	template<class T>
	T* CMessagePtr<T>::operator->() const
	{
		return this->m_pData;
	}

	template<class T>
	CMessagePtr<T>::operator bool() const
	{
		return this->m_pData != nullptr;
	}

	template<class T>
	void CMessagePtr<T>::clear(bool del)
	{
		if (del)
		{
			this->decRef();
			return;
		}

		if (this->m_pRefCount != nullptr)
		{
			--*this->m_pRefCount;
			if (*this->m_pRefCount <= 0)
			{
				SAFE_DELETE(this->m_pRefCount);
			}
		}
	}

	template<class T>
	T* CMessagePtr<T>::get() const
	{
		return this->m_pData;
	}

	template<class T>
	uint32_t CMessagePtr<T>::count() const
	{
		return this->m_pRefCount != nullptr ? *this->m_pRefCount : 0;
	}

	template<class T>
	bool CMessagePtr<T>::unique() const
	{
		return this->count() == 1;
	}


	template<class T>
	uint16_t core::CMessagePtr<T>::getMessageID() const
	{
		return this->m_nMessageID;
	}

	template<class T>
	bool CMessagePtr<T>::operator != (const CMessagePtr<T>& rhs) const
	{
		return this->m_pData != rhs.m_pData;
	}

	template<class T>
	bool CMessagePtr<T>::operator == (const CMessagePtr<T>& rhs) const
	{
		return this->m_pData == rhs.m_pData;
	}

	template<class T>
	template<class U>
	CMessagePtr<T> CMessagePtr<T>::static_cast_message(const CMessagePtr<U>& rhs)
	{
		CMessagePtr<T> sMessagePtr;
		sMessagePtr.reset(static_cast<T*>(rhs.m_pData), rhs.m_pRefCount, rhs.m_nMessageID, rhs.m_del, false);
		return sMessagePtr;
	}

	template<class T>
	template<class U>
	CMessagePtr<T> CMessagePtr<T>::reinterpret_cast_message(const CMessagePtr<U>& rhs)
	{
		CMessagePtr<T> sMessagePtr;
		sMessagePtr.reset(reinterpret_cast<T*>(rhs.m_pData), rhs.m_pRefCount, rhs.m_nMessageID, rhs.m_del, false);
		return sMessagePtr;
	}

// 	template<class T>
// 	CMessagePtr<T> core::CMessagePtr<T>::make_message(T* pData, std::function<void(void*)> del)
// 	{
// 		return CMessagePtr<T>(pData, del);
// 	}
// 
// 	template<class T>
// 	CMessagePtr<T> core::CMessagePtr<T>::make_message(T* pData)
// 	{
// 		return CMessagePtr<T>(pData);
// 	}
}