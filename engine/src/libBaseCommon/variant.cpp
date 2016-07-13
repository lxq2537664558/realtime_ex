#include "stdafx.h"
#include "variant.h"
#include "debug_helper.h"
#include "base_function.h"

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
		this->m_eType = eVVT_Int32;
		this->m_nValue = value;
	}

	CVariant::CVariant(int16_t value)
	{
		this->m_eType = eVVT_Int32;
		this->m_nValue = value;
	}

	CVariant::CVariant(uint16_t value)
	{
		this->m_eType = eVVT_Int32;
		this->m_nValue = value;
	}

	CVariant::CVariant(int32_t value)
	{
		this->m_eType = eVVT_Int32;
		this->m_nValue = value;
	}

	CVariant::CVariant(uint32_t value)
	{
		this->m_eType = eVVT_Int32;
		this->m_nValue = value;
	}

	CVariant::CVariant(int64_t value)
	{
		this->m_eType = eVVT_Int64;
		this->m_nValue = value;
	}

	CVariant::CVariant(uint64_t value)
	{
		this->m_eType = eVVT_Int64;
		this->m_nValue = value;
	}

	CVariant::CVariant(double value)
	{
		this->m_eType = eVVT_Double;
		this->m_fValue = value;
	}

	CVariant::CVariant(const char* value)
	{
		// 必须设置成None，不然Clear函数中会删除一个无效的指针
		this->m_eType = eVVT_None;
		this->setString(value);
	}

	CVariant::CVariant(char* value, size_t len)
	{
		if (value == nullptr || len == 0)
		{
			// 必须设置成None，不然Clear函数中会删除一个无效的指针
			this->m_eType = eVVT_None;
			this->clear();
		}
		else
		{
			this->m_eType = eVVT_Blob;
			this->m_pBlob = new char[len];
			this->m_nLen = len;
			memcpy(this->m_pBlob, value, len);
		}
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
		case eVVT_Int64:
			this->m_nValue = rhs.m_nValue;
			break;

		case eVVT_Double:
			this->m_fValue = rhs.m_fValue;
			break;

		case eVVT_String:
			this->setString(rhs.m_szStr);
			break;

		case eVVT_Blob:
			{
				this->m_nLen = rhs.m_nLen;
				SAFE_DELETE_ARRAY(this->m_pBlob);
				this->m_pBlob = new char[this->m_nLen];
				memcpy(this->m_pBlob, rhs.m_pBlob, this->m_nLen);
			}
			break;

		default:
			{
				PrintWarning("CVariant::operator = & invalid type %d", (int32_t)rhs.m_eType);
			}
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
		case eVVT_Int64:
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
				this->m_szStr = rhs.m_szStr;
				rhs.m_szStr = nullptr;
			}
			break;

		case eVVT_Blob:
			{
				this->m_nLen = rhs.m_nLen;
				this->m_pBlob = rhs.m_pBlob;
				rhs.m_pBlob = nullptr;
				rhs.m_nLen = 0;
			}
			break;

		default:
			{
				PrintWarning("CVariant::operator = && invalid type %d", (int32_t)rhs.m_eType);
			}
		}

		rhs.m_eType = eVVT_None;

		return *this;
	}

	void CVariant::clear()
	{
		switch (this->m_eType)
		{
		case eVVT_String:
			SAFE_DELETE_ARRAY(this->m_szStr);
			break;

		case eVVT_Blob:
			SAFE_DELETE_ARRAY(this->m_pBlob);
			break;

		default:
			break;
		}

		this->m_eType = eVVT_None;
		this->m_fValue = 0.0;
		this->m_nValue = 0;
		this->m_szStr = nullptr;
		this->m_pBlob = nullptr;
		this->m_nLen = 0;
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
		case eVVT_Int64:
			return this->m_nValue;

		case eVVT_Double:
			return (int64_t)this->m_fValue;

		default:
			{
				PrintWarning("operator int64_t invalid type %d", (int32_t)this->m_eType);
			}
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
		case eVVT_Int64:
			return (double)this->m_nValue;

		case eVVT_Double:
			return this->m_fValue;

		default:
			{
				PrintWarning("operator double invalid type %d", (int32_t)this->m_eType);
			}
		}
		return 0.0;
	}

	CVariant::operator const char*() const
	{
		switch (this->m_eType)
		{
		case eVVT_String:
			return this->m_szStr;

		default:
			{
				PrintWarning("operator const char* invalid type %d", (int32_t)this->m_eType);
			}
		}
		return nullptr;
	}

	size_t CVariant::getSize() const
	{
		switch (this->m_eType)
		{
		case eVVT_String:
			if (this->m_szStr == nullptr)
				return 0;
			return strlen(this->m_szStr);

		case eVVT_Blob:
			return this->m_nLen;

		case eVVT_Int32:
			return sizeof(uint32_t);

		case eVVT_Int64:
			return sizeof(int64_t);

		case eVVT_Double:
			return sizeof(double);

		default:
			break;
		}
		return 0;
	}

	const char* CVariant::getBlob() const
	{
		if (this->m_eType != eVVT_Blob)
			return nullptr;

		return this->m_pBlob;
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
		this->m_eType = eVVT_Int32;
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
		this->m_eType = eVVT_Int32;
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
		this->m_eType = eVVT_Int32;
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
		this->m_eType = eVVT_Int64;
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

	CVariant& CVariant::operator = (const char* szStr)
	{
		this->setString(szStr);

		return *this;
	}

	void CVariant::setString(const char* value)
	{
		this->clear();

		this->m_eType = eVVT_String;

		// 有可能默认是nullptr
		if (nullptr == value)
		{
			this->m_szStr = new char[1];
			this->m_szStr[0] = 0;
		}
		else
		{
			size_t nLen = base::crt::strnlen(value, -1) + 1;
			this->m_szStr = new char[nLen];
			base::crt::strcpy(this->m_szStr, nLen, value);
		}
	}
}