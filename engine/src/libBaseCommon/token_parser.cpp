#include "stdafx.h"
#include "token_parser.h"
#include "debug_helper.h"

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

static std::string& ltrim(std::string &s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](const std::string::value_type& value) { return (value != 0x20); }));
	
	return s;
}

static std::string& rtrim(std::string &s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), [](const std::string::value_type& value) { return (value != 0x20); }).base(), s.end());

	return s;
}

namespace base
{
	std::string &trim(std::string s)
	{
		return ltrim(rtrim(s));
	}

	void splitString(const std::string& s, char delim, std::vector<std::string>& elements)
	{
		std::stringstream ss(s);
		std::string item;
		while (std::getline(ss, item, delim))
		{
			elements.push_back(item);
		}
	}

	CTokenParser::CTokenParser()
		: m_szElement(nullptr)
		, m_nCount(0)
	{
	}

	CTokenParser::~CTokenParser()
	{
		this->reset();
	}

	bool CTokenParser::parse(const char* szBuf, char nDelim)
	{
		DebugAstEx(szBuf != nullptr, false);

		std::vector<std::string> vecElement;
		splitString(szBuf, nDelim, vecElement);

		this->m_nCount = vecElement.size();
		this->m_szElement = new char*[this->m_nCount];
		for (size_t i = 0; i < vecElement.size(); ++i)
		{
			this->m_szElement[i] = new char[vecElement[i].size()+1];
			crt::strcpy(this->m_szElement[i], vecElement[i].size() + 1, vecElement[i].c_str());
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

		std::string str = trim(this->m_szElement[nIndex]);
		return base::crt::atoi(str.c_str(), nValue);
	}

	bool CTokenParser::getUint32Element(size_t nIndex, uint32_t& nValue)
	{
		if (nIndex >= this->m_nCount)
			return false;

		std::string str = trim(this->m_szElement[nIndex]);
		return base::crt::atoui(str.c_str(), nValue);
	}

	bool CTokenParser::getInt64Element(size_t nIndex, int64_t& nValue)
	{
		if (nIndex >= this->m_nCount)
			return false;

		std::string str = trim(this->m_szElement[nIndex]);
		return base::crt::atoi64(str.c_str(), nValue);
	}

	bool CTokenParser::getUint64Element(size_t nIndex, uint64_t& nValue)
	{
		if (nIndex >= this->m_nCount)
			return false;

		std::string str = trim(this->m_szElement[nIndex]);
		return base::crt::atoui64(str.c_str(), nValue);
	}

	bool CTokenParser::getStringElement(size_t nIndex, char* szBuf, size_t nBufSize)
	{
		if (nIndex >= this->m_nCount)
			return false;

		size_t nLen = crt::strnlen(this->m_szElement[nIndex], _TRUNCATE);
		if (nLen >= nBufSize)
			return false;

		crt::strcpy(szBuf, nBufSize, this->m_szElement[nIndex]);
		return true;
	}
}
