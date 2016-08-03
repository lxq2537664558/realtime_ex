#include "stdafx.h"
#include "message.h"

namespace core
{
	CMessage::CMessage()
		: m_pHeader(nullptr)
		, m_pRefCount(nullptr)
	{

	}

	CMessage::CMessage(message_header* pHeader)
	{
		if (pHeader != nullptr)
		{
			this->m_pHeader = pHeader;
			this->m_pRefCount = new int32_t(1);
		}
		else
		{
			this->m_pHeader = nullptr;
			this->m_pRefCount = nullptr;
		}
	}

	CMessage::CMessage(message_header* pHeader, std::function<void(void*)> del)
	{
		if (pHeader != nullptr)
		{
			this->m_pHeader = pHeader;
			this->m_pRefCount = new int32_t(1);
		}
		else
		{
			this->m_pHeader = nullptr;
			this->m_pRefCount = nullptr;
		}

		this->m_del = del;
	}

	CMessage::CMessage(CMessage& rhs)
	{
		this->m_pHeader = rhs.m_pHeader;
		this->m_pRefCount = rhs.m_pRefCount;
		this->m_del = rhs.m_del;

		this->incRef();
	}

	CMessage::CMessage(CMessage&& rhs)
	{
		this->m_pHeader = rhs.m_pHeader;
		this->m_pRefCount = rhs.m_pRefCount;
		this->m_del = rhs.m_del;

		rhs.m_pHeader = nullptr;
		rhs.m_pRefCount = nullptr;
		rhs.m_del = nullptr;
	}

	CMessage::~CMessage()
	{
		this->decRef();
	}

	CMessage& CMessage::operator = (CMessage& rhs)
	{
		this->decRef();

		this->m_pHeader = rhs.m_pHeader;
		this->m_pRefCount = rhs.m_pRefCount;
		this->m_del = rhs.m_del;

		this->incRef();

		return *this;
	}

	CMessage& CMessage::operator = (CMessage&& rhs)
	{
		this->decRef();

		this->m_pHeader = rhs.m_pHeader;
		this->m_pRefCount = rhs.m_pRefCount;
		this->m_del = rhs.m_del;

		rhs.m_pHeader = nullptr;
		rhs.m_pRefCount = nullptr;
		rhs.m_del = nullptr;

		return *this;
	}

	void CMessage::decRef()
	{
		if (this->m_pRefCount != nullptr)
		{
			--*this->m_pRefCount;
			if (*this->m_pRefCount <= 0)
			{
				if (this->m_del != nullptr)
					this->m_del(this->m_pHeader);
				SAFE_DELETE(this->m_pRefCount);
			}
		}
	}

	void CMessage::incRef()
	{
		if (this->m_pRefCount != nullptr)
			++*this->m_pRefCount;
	}

	message_header* CMessage::operator->() const
	{
		return this->m_pHeader;
	}

	CMessage::operator bool() const
	{
		return this->m_pHeader != nullptr;
	}

	void CMessage::clear(bool del)
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

	message_header* CMessage::get() const
	{
		return this->m_pHeader;
	}

	uint32_t CMessage::count() const
	{
		return this->m_pRefCount != nullptr ? *this->m_pRefCount : 0;
	}

	bool CMessage::unique() const
	{
		return this->count() == 1;
	}

	bool CMessage::operator != (const CMessage& rhs) const
	{
		return this->m_pHeader != rhs.m_pHeader;
	}

	bool CMessage::operator == (const CMessage& rhs) const
	{
		return this->m_pHeader == rhs.m_pHeader;
	}
}