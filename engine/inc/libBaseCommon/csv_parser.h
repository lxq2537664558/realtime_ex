#pragma once

#include "base_common.h"
#include "string_util.h"

#include <string>
#include <type_traits>

namespace base
{
	struct SCSVParserInfo;
	class __BASE_COMMON_API__ CCSVParser
	{
	public:
		CCSVParser();
		~CCSVParser();

		bool				load(const std::string& szName, char cDelim = ',');
		void				clear();

        const char*			getValue(uint32_t nRow, uint32_t nColumn) const;
		const char*			getValue(uint32_t nRow, const char* szColumn) const;

		template<class T>
		bool				getValue(uint32_t nRow, uint32_t nColumn, T& val) const;
		template<class T>
		bool				getValue(uint32_t nRow, const char* szColumn, T& val) const;

		uint32_t			getRowCount() const;
		uint32_t			getColumnCount() const;
        const char*			getHeaderElement(uint32_t nRow) const;
        const char*			getFileName() const;

	private:
		int32_t				findColumn(const char* szColumn) const;

    private:
		SCSVParserInfo*	m_pParserInfo;
    };

	template<class T>
	bool CCSVParser::getValue(uint32_t nRow, uint32_t nColumn, T& val) const
	{
		const char* szValue = this->getValue(nRow, nColumn);
		if (nullptr == szValue)
			return false;

		if (szValue[0] == '\0')
		{
			if (std::is_integral<T>::value)
			{
				szValue = "0";
			}
		}
		return base::string_util::convert_to_value(szValue, val);
	}

	template<class T>
	bool CCSVParser::getValue(uint32_t nRow, const char* szColumn, T& val) const
	{
		int32_t nColumn = this->findColumn(szColumn);
		if (nColumn < 0)
			return false;

		return this->getValue(nRow, nColumn, val);
	}
}