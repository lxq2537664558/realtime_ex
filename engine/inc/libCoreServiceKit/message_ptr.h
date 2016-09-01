#pragma once
#include "libCoreCommon/core_common.h"

namespace core
{
	template<class T>
	class CMessagePtr
	{
		template<class U>
		friend class CMessagePtr;

	public:
		CMessagePtr();
		explicit CMessagePtr(T* pData, uint16_t nMessageID);
		CMessagePtr(nullptr_t);

		CMessagePtr(T* pData, uint16_t nMessageID, std::function<void(void*)> del);

		CMessagePtr(const CMessagePtr<T>& rhs);

		CMessagePtr(CMessagePtr<T>&& rhs);
		~CMessagePtr();

		CMessagePtr<T>& operator = (CMessagePtr<T>& rhs);

		CMessagePtr<T>& operator = (CMessagePtr<T>&& rhs);
		T* operator->() const;
		explicit operator bool() const;
		bool operator != (const CMessagePtr<T>& rhs) const;
		bool operator == (const CMessagePtr<T>& rhs) const;

		template<class U>
		U*			message() const
		{
			return reinterpret_cast<U*>(this->m_pData);
		}

		void		clear(bool del);
		T*			get() const;
		bool		unique() const;
		uint32_t	count() const;

		uint16_t	getMessageID() const;

		template<class U>
		static CMessagePtr<T> 
				static_cast_message(const CMessagePtr<U>& rhs);

		template<class U>
		static CMessagePtr<T>
				reinterpret_cast_message(const CMessagePtr<U>& rhs);

// 		static CMessagePtr<T>
// 				make_message(T* pData);
// 
// 		static CMessagePtr<T>
// 				make_message(T* pData, std::function<void(void*)> del);

	private:
		void		decRef();
		void		incRef();

		void		reset(T* pData, int32_t* pRefCount, uint16_t nMessageID, const std::function<void(void*)>& del, bool bDel);

	private:
		T*							m_pData;
		uint16_t					m_nMessageID;
		int32_t*					m_pRefCount;
		std::function<void(void*)>	m_del;
	};
}

#include "message_ptr.inl"