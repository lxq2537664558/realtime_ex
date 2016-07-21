#pragma once
#include <string>

#include "base_common.h"

namespace base
{
	class __BASE_COMMON_API__ utf8_string
	{
	public:
		utf8_string();
		~utf8_string();

		utf8_string(utf8_string& rhs);
		utf8_string(utf8_string&& rhs);
		utf8_string& operator = (utf8_string& rhs);
		utf8_string& operator = (utf8_string&& rhs);

		bool				init(const std::string& utf8);
		bool				init(const std::wstring& ucs2);

		std::wstring		cvt_ucs2() const;
		size_t				length() const;
		const std::string&	str() const;
		std::string			substr(size_t pos, size_t count) const;
		std::string			at(size_t pos) const;
		std::string	operator[](size_t pos) const;

		static std::wstring	cvt_ucs2(const std::string& utf8);
		static std::string	cvt_utf8(const std::wstring& ucs2);

	private:
		std::string*	m_szData;
	};
}