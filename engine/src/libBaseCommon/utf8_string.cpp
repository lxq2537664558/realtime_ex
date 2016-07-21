#include "stdafx.h"
#include "utf8_string.h"
#include "base_function.h"
#include "debug_helper.h"

#include <codecvt>
#include <iostream>

static bool check_utf8(const std::string& utf8)
{
	for (size_t i = 0; i < utf8.size(); ++i)
	{
		uint8_t c = (uint8_t)utf8.at(i);
		if (c >= 0 && c <= 127) i += 0;
		else if ((c & 0xE0) == 0xC0) i += 1;
		else if ((c & 0xF0) == 0xE0) i += 2;
		else if ((c & 0xF8) == 0xF0) i += 3;
		else if ((c & 0xFC) == 0xF8) i += 4;
		else if ((c & 0xFE) == 0xFC) i += 5;
		else return false;//invalid utf8
	}

	return true;
}

namespace base
{
	utf8_string::utf8_string()
	{
		this->m_szData = new std::string();
	}

	utf8_string::~utf8_string()
	{
		SAFE_DELETE(this->m_szData);
	}

	utf8_string::utf8_string(utf8_string&& rhs)
	{
		this->m_szData = rhs.m_szData;
		rhs.m_szData = nullptr;
	}

	utf8_string& utf8_string::operator = (utf8_string&& rhs)
	{
		this->m_szData = rhs.m_szData;
		rhs.m_szData = nullptr;

		return *this;
	}

	utf8_string::utf8_string(utf8_string& rhs)
	{
		*this->m_szData = *rhs.m_szData;
	}

	utf8_string& utf8_string::operator = (utf8_string& rhs)
	{
		*this->m_szData = *rhs.m_szData;
		
		return *this;
	}

	bool utf8_string::init(const std::string& utf8)
	{
		if (!check_utf8(utf8))
			return false;

		*this->m_szData = utf8;

		return true;
	}

	bool utf8_string::init(const std::wstring& ucs2)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> cv2;
		try
		{
			*this->m_szData = cv2.to_bytes(ucs2);
		}
		catch (...)
		{
			return false;
		}

		return true;
	}

	std::wstring utf8_string::cvt_ucs2() const
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> cv2;
		std::wstring result;

		try
		{
			result = cv2.from_bytes(*this->m_szData);
		}
		catch (...)
		{
			return L"";
		}

		return result;
	}

	const std::string& utf8_string::str() const
	{
		return *this->m_szData;
	}

	size_t utf8_string::length() const
	{
		size_t count = 0;
		for (size_t i = 0; i < this->m_szData->size(); ++i)
		{
			++count;
			uint8_t c = (uint8_t)this->m_szData->at(i);
			if (c >= 0 && c <= 127) i += 0;
			else if ((c & 0xE0) == 0xC0) i += 1;
			else if ((c & 0xF0) == 0xE0) i += 2;
			else if ((c & 0xF8) == 0xF0) i += 3;
			else if ((c & 0xFC) == 0xF8) i += 4;
			else if ((c & 0xFE) == 0xFC) i += 5;
			else return 0;//invalid utf8
		}

		return count;
	}

	std::string utf8_string::substr(size_t pos, size_t count) const
	{
		if (count == 0)
			return "";

		size_t utf8_pos = 0;
		size_t native_pos = 0;
		size_t min_pos = std::string::npos;
		size_t max_pos = std::string::npos;
		size_t size = this->m_szData->size();
		for (; native_pos < size; ++native_pos, ++utf8_pos)
		{
			if (utf8_pos == pos)
				min_pos = native_pos;
			if (utf8_pos <= pos + count || count == std::string::npos)
				max_pos = native_pos;

			uint8_t c = (unsigned char)this->m_szData->at(native_pos);
			if (c >= 0 && c <= 127) native_pos += 0;
			else if ((c & 0xE0) == 0xC0) native_pos += 1;
			else if ((c & 0xF0) == 0xE0) native_pos += 2;
			else if ((c & 0xF8) == 0xF0) native_pos += 3;
			else if ((c & 0xFC) == 0xF8) native_pos += 4;
			else if ((c & 0xFE) == 0xFC) native_pos += 5;
			else return "";//invalid utf8
		}

		if (utf8_pos <= pos + count || count == std::string::npos) 
			max_pos = native_pos;

		if (min_pos == std::string::npos || max_pos == std::string::npos)
			return "";

		return this->m_szData->substr(min_pos, max_pos - min_pos);
	}

	std::string utf8_string::at(size_t pos) const
	{
		return this->substr(pos, 1);
	}

	std::string utf8_string::operator[](size_t pos) const
	{
		return this->at(pos);
	}


	std::wstring utf8_string::cvt_ucs2(const std::string& utf8)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> cv2;

		std::wstring ucs2;
		try
		{
			ucs2 = cv2.from_bytes(utf8);
		}
		catch (...)
		{
			return L"";
		}

		return ucs2;
	}

	std::string utf8_string::cvt_utf8(const std::wstring& ucs2)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> cv2;

		std::string utf8;
		try
		{
			utf8 = cv2.to_bytes(ucs2);
		}
		catch (...)
		{
			return "";
		}

		return utf8;
	}
}