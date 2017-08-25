#include "stdafx.h"
#include "token_parser.h"
#include "debug_helper.h"
#include "string_util.h"

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

namespace base
{
	CTokenParser::CTokenParser()
		: m_szElement(nullptr)
		, m_nCount(0)
	{
	}

	CTokenParser::~CTokenParser()
	{
		this->reset();
	}

	bool CTokenParser::parse(const char* szBuf, const char* szDelim)
	{
		DebugAstEx(szBuf != nullptr && szDelim != nullptr, false);

		std::vector<std::string> vecElement;
		base::string_util::split(szBuf, vecElement, szDelim);

		this->m_nCount = vecElement.size();
		this->m_szElement = new char*[this->m_nCount];
		for (size_t i = 0; i < vecElement.size(); ++i)
		{
			this->m_szElement[i] = new char[vecElement[i].size()+1];
			base::function_util::strcpy(this->m_szElement[i], vecElement[i].size() + 1, vecElement[i].c_str());
		}

		return true;
	}

	void CTokenParser::reset()
	{
		for (size_t i = 0; i < this->m_nCount; ++i)
		{
			SAFE_DELETE_ARRAY(this->m_szElement[i]);
		}

		SAFE_DELETE_ARRAY(this->m_szElement);
		this->m_nCount = 0;
	}

	size_t CTokenParser::getElementCount() const
	{
		return this->m_nCount;
	}

	bool CTokenParser::getInt32Element(size_t nIndex, int32_t& nValue)
	{
		if (nIndex >= this->m_nCount)
			return false;

		std::string str = this->m_szElement[nIndex];
		str = base::string_util::trim(str);
		return base::string_util::convert_to_value(str, nValue);
	}

	bool CTokenParser::getUint32Element(size_t nIndex, uint32_t& nValue)
	{
		if (nIndex >= this->m_nCount)
			return false;

		std::string str = this->m_szElement[nIndex];
		str = base::string_util::trim(str);
		return base::string_util::convert_to_value(str, nValue);
	}

	bool CTokenParser::getInt64Element(size_t nIndex, int64_t& nValue)
	{
		if (nIndex >= this->m_nCount)
			return false;

		std::string str = this->m_szElement[nIndex];
		str = base::string_util::trim(str);
		return base::string_util::convert_to_value(str, nValue);
	}

	bool CTokenParser::getUint64Element(size_t nIndex, uint64_t& nValue)
	{
		if (nIndex >= this->m_nCount)
			return false;

		std::string str = this->m_szElement[nIndex];
		str = base::string_util::trim(str);
		return base::string_util::convert_to_value(str, nValue);
	}

	bool CTokenParser::getStringElement(size_t nIndex, char* szBuf, size_t nBufSize)
	{
		if (nIndex >= this->m_nCount)
			return false;

		size_t nLen = base::function_util::strnlen(this->m_szElement[nIndex], _TRUNCATE);
		if (nLen >= nBufSize)
			return false;

		base::function_util::strcpy(szBuf, nBufSize, this->m_szElement[nIndex]);
		return true;
	}
}
