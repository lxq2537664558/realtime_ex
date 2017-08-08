
namespace base
{
	template<class T, bool FIX>
	CCircleQueue<T, FIX>::CCircleQueue(uint32_t nCap)
		: m_nQueueBegin(0)
		, m_nQueueEnd(0)
		, m_nQueueCapacity(nCap)
		, m_pQueue(new T[nCap])
#ifdef _DEBUG
		, m_nSize(0)
#endif
	{
	}

	template<class T, bool FIX>
	CCircleQueue<T, FIX>::~CCircleQueue()
	{
		if (this->m_pQueue != nullptr)
			delete[] this->m_pQueue;
	}

	template<class T, bool FIX>
	bool CCircleQueue<T, FIX>::send(const T& sValue)
	{
		if (FIX && (int32_t)this->size() == this->m_nQueueCapacity)
			return false;

		this->m_pQueue[this->m_nQueueEnd] = sValue;
		if (++this->m_nQueueEnd >= this->m_nQueueCapacity)
			this->m_nQueueEnd = 0;

		if (this->m_nQueueBegin == this->m_nQueueEnd)
		{
			T* pNewMessageQueue = new T[this->m_nQueueCapacity * 2];
			for (int32_t i = 0; i < this->m_nQueueCapacity; ++i)
				pNewMessageQueue[i] = this->m_pQueue[(this->m_nQueueBegin + i) % this->m_nQueueCapacity];
			this->m_nQueueBegin = 0;
			this->m_nQueueEnd = this->m_nQueueCapacity;
			this->m_nQueueCapacity *= 2;
			delete [] this->m_pQueue;
			this->m_pQueue = pNewMessageQueue;
		}

#ifdef _DEBUG
		++this->m_nSize;
#endif
		return true;
	}

	template<class T, bool FIX>
	bool CCircleQueue<T, FIX>::recv(T& sValue)
	{
		if (this->empty())
			return false;

		sValue = this->m_pQueue[this->m_nQueueBegin];
			
		if (++this->m_nQueueBegin >= this->m_nQueueCapacity)
			this->m_nQueueBegin = 0;

#ifdef _DEBUG
		--this->m_nSize;
#endif
		return true;
	}

	template<class T, bool FIX>
	uint32_t CCircleQueue<T, FIX>::size() const
	{
		if (this->m_nQueueEnd >= this->m_nQueueBegin)
			return this->m_nQueueEnd - this->m_nQueueBegin;
		else
			return this->m_nQueueCapacity - (this->m_nQueueBegin - this->m_nQueueEnd);
	}

	template<class T, bool FIX>
	bool CCircleQueue<T, FIX>::empty() const
	{
		return this->m_nQueueEnd - this->m_nQueueBegin == 0;
	}
}