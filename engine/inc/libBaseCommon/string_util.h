#pragma once
#include <string>
#include <sstream>
#include <algorithm>
#include <functional>
#include <vector>

#include "function_util.h"
#include "variant.h"

namespace base
{
	namespace string_util
	{
		// 任意类型转为字符串
		template <class T>
		inline std::string	convert_to_str(const T& v);

		inline std::string	convert_to_str(const CVariant& v);


		// 字符串转为某一类型
		template <class T>
		inline bool			convert_to_value(const std::string& str, T& v);

		// 字符串转为某一类型
		template <class T>
		inline bool			convert_to_value(const char* str, T& v);

		// 将字符串转为小写并返回
		inline std::string&	to_lower(std::string& str);

		// 将字符串转为小写并返回
		inline std::string&	to_upper(std::string& str);

		// 判断字符串是否以某一子串为开始
		inline bool			start_with(const std::string& str, const std::string& substr);

		// 判断字符串是否以某一子串为结尾
		inline bool			end_with(const std::string& str, const std::string& substr);

		// 去掉字符串中左边属于字符串delim中任一字符的所有字符(默认去除空格)
		inline std::string& ltrim(std::string& str, const char* delim = " ");

		// 去掉字符串中右边属于字符串delim中任一字符的所有字符(默认去除空格)
		inline std::string& rtrim(std::string& str, const char* delim = " ");

		// 去掉字符串中两端属于字符串delim中任一字符的所有字符(默认去除空格)
		inline std::string& trim(std::string& str, const char* delim = " ");

		// 去掉字符串中的所有特定单一字符
		inline std::string&	erase(std::string& str, char c = ' ');

		// 字符串替换 去掉字符串中的某特定字符串delim并以新字符串s代替
		inline std::string&	replace_all(std::string& str, const char* delim, const char* s = "");

		// 字符串分隔
		inline void			split(const std::string& str, std::vector<std::string>& result, const char* delim = " ");

		// 格式化输出 format(xxx{}xxx{}xx,变量1，变量2)
		template<class ...Args>
		inline std::string	format(const char* fmt, const Args&... args);

		// 格式化输出 format(xxx{}xxx{}xx,变量1，变量2)
		template<class ...Args>
		inline std::string	format(const std::string& fmt, const Args&... args);

		// 字符串忽略大小写比较, 可用作容器（比如map、set）的比较子
		struct string_cmp_nocase : 
			public std::binary_function<std::string, std::string, bool>
		{
		public:
			inline bool operator()(const std::string& lhs, const std::string& rhs) const;
		};
	}
}

#include "string_util.inl"