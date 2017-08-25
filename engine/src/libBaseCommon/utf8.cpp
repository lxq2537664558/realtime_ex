#include "stdafx.h"
#include "utf8.h"
#include "function_util.h"
#include "debug_helper.h"

#ifdef _WIN32
#include <codecvt>
#endif

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
	CUTF8::CUTF8()
	{
		this->m_szData = new std::string();
	}

	CUTF8::~CUTF8()
	{
		SAFE_DELETE(this->m_szData);
	}

	CUTF8::CUTF8(CUTF8&& rhs)
	{
		this->m_szData = rhs.m_szData;
		rhs.m_szData = nullptr;
	}

	CUTF8& CUTF8::operator = (CUTF8&& rhs)
	{
		this->m_szData = rhs.m_szData;
		rhs.m_szData = nullptr;

		return *this;
	}

	CUTF8::CUTF8(CUTF8& rhs)
	{
		*this->m_szData = *rhs.m_szData;
	}

	CUTF8& CUTF8::operator = (CUTF8& rhs)
	{
		*this->m_szData = *rhs.m_szData;
		
		return *this;
	}

	bool CUTF8::init(const std::string& utf8)
	{
		if (!check_utf8(utf8))
			return false;

		*this->m_szData = utf8;

		return true;
	}

	bool CUTF8::init(const std::wstring& ucs2)
	{

#ifdef _WIN32
		try
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>> cv2;
			*this->m_szData = cv2.to_bytes(ucs2);
		}
		catch (...)
		{
			return false;
		}
#endif

		return true;
	}

	std::wstring CUTF8::cvt_ucs2() const
	{
		std::wstring result;

#ifdef _WIN32
		try
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>> cv2;
			result = cv2.from_bytes(*this->m_szData);
		}
		catch (...)
		{
			return L"";
		}
#endif

		return result;
	}

	const std::string& CUTF8::str() const
	{
		return *this->m_szData;
	}

	size_t CUTF8::length() const
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

	std::string CUTF8::substr(size_t pos, size_t count) const
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

	std::string CUTF8::at(size_t pos) const
	{
		return this->substr(pos, 1);
	}

	std::string CUTF8::operator[](size_t pos) const
	{
		return this->at(pos);
	}

	std::wstring CUTF8::cvt_ucs2(const std::string& utf8)
	{
		std::wstring ucs2;

#ifdef _WIN32
		try
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>> cv2;
			ucs2 = cv2.from_bytes(utf8);
		}
		catch (...)
		{
			return L"";
		}
#endif

		return ucs2;
	}

	std::string CUTF8::cvt_utf8(const std::wstring& ucs2)
	{
		std::string utf8;

#ifdef _WIN32
		try
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>> cv2;
			utf8 = cv2.to_bytes(ucs2);
		}
		catch (...)
		{
			return "";
		}
#endif

		return utf8;
	}
}