#pragma once
#include "core_common.h"

namespace core
{
	class CMessage
	{
	public:
		CMessage();
		CMessage(message_header* pHeader);
		CMessage(message_header* pHeader, std::function<void(void*)> del);
		CMessage(CMessage& rhs);
		CMessage(CMessage&& rhs);
		~CMessage();

		CMessage& operator = (CMessage& rhs);
		CMessage& operator = (CMessage&& rhs);
		message_header* operator->() const;
		explicit operator bool() const;
		bool operator != (const CMessage& rhs) const;
		bool operator == (const CMessage& rhs) const;

		template<class T>
		T*				message() const
		{
			return reinterpret_cast<T*>(this->m_pHeader);
		}

		void			clear(bool del);
		message_header* get() const;
		bool			unique() const;
		uint32_t		count() const;

	private:
		void			decRef();
		void			incRef();

	private:
		message_header*				m_pHeader;
		std::function<void(void*)>	m_del;
		int32_t*					m_pRefCount;
	};
}