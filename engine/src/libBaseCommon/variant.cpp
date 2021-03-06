#include "stdafx.h"
#include "variant.h"
#include "debug_helper.h"
#include "function_util.h"

#include <sstream>

namespace base
{
	CVariant::CVariant()
		: m_eType(eVVT_None)
	{
	}

	CVariant::CVariant(int8_t value)
	{
		this->m_eType = eVVT_Int32;
		this->m_nValue = value;
	}

	CVariant::CVariant(uint8_t value)
	{
		this->m_eType = eVVT_UInt32;
		this->m_nValue = value;
	}

	CVariant::CVariant(int16_t value)
	{
		this->m_eType = eVVT_Int32;
		this->m_nValue = value;
	}

	CVariant::CVariant(uint16_t value)
	{
		this->m_eType = eVVT_UInt32;
		this->m_nValue = value;
	}

	CVariant::CVariant(int32_t value)
	{
		this->m_eType = eVVT_Int32;
		this->m_nValue = value;
	}

	CVariant::CVariant(uint32_t value)
	{
		this->m_eType = eVVT_UInt32;
		this->m_nValue = value;
	}

	CVariant::CVariant(int64_t value)
	{
		this->m_eType = eVVT_Int64;
		this->m_nValue = value;
	}

	CVariant::CVariant(uint64_t value)
	{
		this->m_eType = eVVT_UInt64;
		this->m_nValue = value;
	}

	CVariant::CVariant(double value)
	{
		this->m_eType = eVVT_Double;
		this->m_fValue = value;
	}

	CVariant::CVariant(const char* value, uint32_t nLen)
	{
		// 必须设置成None，不然clear函数中会删除一个无效的指针
		this->m_eType = eVVT_None;
		this->setString(value, nLen);
	}

	CVariant::CVariant(const char* value)
	{
		// 必须设置成None，不然clear函数中会删除一个无效的指针
		this->m_eType = eVVT_None;
		this->setString(value, (uint32_t)base::function_util::strnlen(value, _TRUNCATE));
	}

	CVariant::CVariant(const CVariant& rhs)
	{
		this->m_eType = eVVT_None;
		*this = rhs;
	}

	CVariant::CVariant(CVariant&& rhs)
	{
		*this = rhs;
	}

	CVariant::~CVariant()
	{
		this->clear();
	}

	EVariantValueType CVariant::getType() const
	{
		return m_eType;
	}

	CVariant& CVariant::operator = (const CVariant& rhs)
	{
		if (this == &rhs)
			return *this;

		this->clear();
		this->m_eType = rhs.m_eType;
		switch (rhs.m_eType)
		{
		case eVVT_Int32:
		case eVVT_UInt32:
		case eVVT_Int64:
		case eVVT_UInt64:
			this->m_nValue = rhs.m_nValue;
			break;

		case eVVT_Double:
			this->m_fValue = rhs.m_fValue;
			break;

		case eVVT_String:
			this->setString(rhs.m_sText.szStr, rhs.m_sText.nLen);
			break;

		default:
			PrintWarning("CVariant::operator = & invalid type {}", (int32_t)rhs.m_eType);
		}

		return *this;
	}

	CVariant& CVariant::operator = (CVariant&& rhs)
	{
		if (this == &rhs)
			return *this;

		this->clear();
		this->m_eType = rhs.m_eType;
		switch (rhs.m_eType)
		{
		case eVVT_Int32:
		case eVVT_UInt32:
		case eVVT_Int64:
		case eVVT_UInt64:
		{
			this->m_nValue = rhs.m_nValue;
			rhs.m_nValue = 0;
		}
		break;

		case eVVT_Double:
		{
			this->m_fValue = rhs.m_fValue;
			rhs.m_fValue = 0.0;
		}
		break;

		case eVVT_String:
		{
			this->m_sText.szStr = rhs.m_sText.szStr;
			this->m_sText.nLen = rhs.m_sText.nLen;
			rhs.m_sText.szStr = nullptr;
			rhs.m_sText.nLen = 0;
		}
		break;

		default:
			PrintWarning("CVariant::operator = && invalid type {}", (int32_t)rhs.m_eType);
		}

		rhs.m_eType = eVVT_None;

		return *this;
	}

	void CVariant::clear()
	{
		switch (this->m_eType)
		{
		case eVVT_String:
			SAFE_DELETE_ARRAY(this->m_sText.szStr);
			this->m_sText.nLen = 0;
			break;

		default:
			break;
		}

		this->m_eType = eVVT_None;
		this->m_fValue = 0.0;
		this->m_nValue = 0;
		this->m_sText.szStr = nullptr;
	}

	CVariant::operator int8_t() const
	{
		return (int8_t)this->operator int64_t();
	}

	CVariant::operator uint8_t() const
	{
		return (uint8_t)this->operator int64_t();
	}

	CVariant::operator int16_t() const
	{
		return (int16_t)this->operator int64_t();
	}

	CVariant::operator uint16_t() const
	{
		return (uint16_t)this->operator int64_t();
	}

	CVariant::operator int32_t() const
	{
		return (int32_t)this->operator int64_t();
	}

	CVariant::operator uint32_t() const
	{
		return (uint32_t)this->operator int64_t();
	}

	CVariant::operator int64_t() const
	{
		switch (this->m_eType)
		{
		case eVVT_Int32:
		case eVVT_UInt32:
		case eVVT_Int64:
		case eVVT_UInt64:
			return this->m_nValue;

		case eVVT_Double:
			return (int64_t)this->m_fValue;

		default:
			PrintWarning("operator int64_t invalid type {}", (int32_t)this->m_eType);
		}
		return 0;
	}

	CVariant::operator uint64_t() const
	{
		return (uint64_t)this->operator int64_t();
	}

	CVariant::operator double() const
	{
		switch (this->m_eType)
		{
		case eVVT_Int32:
		case eVVT_UInt32:
		case eVVT_Int64:
		case eVVT_UInt64:
			return (double)this->m_nValue;

		case eVVT_Double:
			return this->m_fValue;

		default:
			PrintWarning("operator double invalid type {}", (int32_t)this->m_eType);
		}
		return 0.0;
	}

	CVariant::operator const char*() const
	{
		switch (this->m_eType)
		{
		case eVVT_String:
			return this->m_sText.szStr;

		default:
			PrintWarning("operator const char* invalid type {}", (int32_t)this->m_eType);
		}
		return nullptr;
	}

	size_t CVariant::getSize() const
	{
		switch (this->m_eType)
		{
		case eVVT_String:
			return this->m_sText.nLen;

		case eVVT_Int32:
		case eVVT_UInt32:
			return sizeof(uint32_t);

		case eVVT_Int64:
		case eVVT_UInt64:
			return sizeof(int64_t);

		case eVVT_Double:
			return sizeof(double);

		default:
			break;
		}
		return 0;
	}

	CVariant& CVariant::operator = (int8_t value)
	{
		this->clear();
		this->m_eType = eVVT_Int32;
		this->m_nValue = value;
		return *this;
	}

	CVariant& CVariant::operator = (uint8_t value)
	{
		this->clear();
		this->m_eType = eVVT_UInt32;
		this->m_nValue = value;
		return *this;
	}

	CVariant& CVariant::operator = (int16_t value)
	{
		this->clear();
		this->m_eType = eVVT_Int32;
		this->m_nValue = value;
		return *this;
	}

	CVariant& CVariant::operator = (uint16_t value)
	{
		this->clear();
		this->m_eType = eVVT_UInt32;
		this->m_nValue = value;
		return *this;
	}

	CVariant& CVariant::operator = (int32_t value)
	{
		this->clear();
		this->m_eType = eVVT_Int32;
		this->m_nValue = value;
		return *this;
	}

	CVariant& CVariant::operator = (uint32_t value)
	{
		this->clear();
		this->m_eType = eVVT_UInt32;
		this->m_nValue = value;
		return *this;
	}

	CVariant& CVariant::operator = (int64_t value)
	{
		this->clear();
		this->m_eType = eVVT_Int64;
		this->m_nValue = value;
		return *this;
	}

	CVariant& CVariant::operator = (uint64_t value)
	{
		this->clear();
		this->m_eType = eVVT_UInt64;
		this->m_nValue = value;
		return *this;
	}

	CVariant& CVariant::operator = (double value)
	{
		this->clear();
		this->m_eType = eVVT_Double;
		this->m_fValue = value;
		return *this;
	}

	void CVariant::setString(const char* szStr, uint32_t nLen)
	{
		this->clear();

		if (szStr == nullptr)
			return;

		this->m_eType = eVVT_String;

		this->m_sText.szStr = new char[nLen];
		memcpy(this->m_sText.szStr, szStr, nLen);
		this->m_sText.nLen = nLen;
	}

	bool CVariant::isVaild() const
	{
		return this->m_eType != eVVT_None;
	}
}