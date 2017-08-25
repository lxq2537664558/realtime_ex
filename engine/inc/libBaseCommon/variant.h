#pragma once
#include "base_common.h"

#include <string>

namespace base
{
	enum EVariantValueType
	{
		eVVT_None,
		eVVT_Int32,
		eVVT_UInt32,
		eVVT_Int64,
		eVVT_UInt64,
		eVVT_Double,
		eVVT_String,
	};

	class __BASE_COMMON_API__ CVariant
	{
	public:
		CVariant();
		CVariant(int8_t value);
		CVariant(uint8_t value);
		CVariant(int16_t value);
		CVariant(uint16_t value);
		CVariant(int32_t value);
		CVariant(uint32_t value);
		CVariant(int64_t value);
		CVariant(uint64_t value);
		CVariant(double value);
		CVariant(const char* value);
		CVariant(const char* value, uint32_t nLen);

		CVariant(const CVariant& rhs);

		CVariant(CVariant&& rhs);

		~CVariant();

		operator int8_t() const;
		operator uint8_t() const;
		operator int16_t() const;
		operator uint16_t() const;
		operator int32_t() const;
		operator uint32_t() const;
		operator int64_t() const;
		operator uint64_t() const;
		operator double() const;
		operator const char*() const;
		
		CVariant& operator = (const CVariant& rhs);
		CVariant& operator = (CVariant&& rhs);
		CVariant& operator = (int8_t value);
		CVariant& operator = (uint8_t value);
		CVariant& operator = (int16_t value);
		CVariant& operator = (uint16_t value);
		CVariant& operator = (int32_t value);
		CVariant& operator = (uint32_t value);
		CVariant& operator = (int64_t value);
		CVariant& operator = (uint64_t value);
		CVariant& operator = (double value);
		
		size_t				getSize() const;
		EVariantValueType	getType(void) const;
		bool				isVaild() const;
		
	private:
		void				setString(const char* szStr, uint32_t nLen);
		void				clear();

	private:
		EVariantValueType m_eType;
		union
		{
			struct
			{
				char*		szStr;
				uint32_t	nLen;
			} m_sText;
			int64_t	m_nValue;
			double	m_fValue;
		};
	};
}