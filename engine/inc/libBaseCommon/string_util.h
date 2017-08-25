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
		// ��������תΪ�ַ���
		template <class T>
		inline std::string	convert_to_str(const T& v);

		inline std::string	convert_to_str(const CVariant& v);


		// �ַ���תΪĳһ����
		template <class T>
		inline bool			convert_to_value(const std::string& str, T& v);

		// �ַ���תΪĳһ����
		template <class T>
		inline bool			convert_to_value(const char* str, T& v);

		// ���ַ���תΪСд������
		inline std::string&	to_lower(std::string& str);

		// ���ַ���תΪСд������
		inline std::string&	to_upper(std::string& str);

		// �ж��ַ����Ƿ���ĳһ�Ӵ�Ϊ��ʼ
		inline bool			start_with(const std::string& str, const std::string& substr);

		// �ж��ַ����Ƿ���ĳһ�Ӵ�Ϊ��β
		inline bool			end_with(const std::string& str, const std::string& substr);

		// ȥ���ַ�������������ַ���delim����һ�ַ��������ַ�(Ĭ��ȥ���ո�)
		inline std::string& ltrim(std::string& str, const char* delim = " ");

		// ȥ���ַ������ұ������ַ���delim����һ�ַ��������ַ�(Ĭ��ȥ���ո�)
		inline std::string& rtrim(std::string& str, const char* delim = " ");

		// ȥ���ַ��������������ַ���delim����һ�ַ��������ַ�(Ĭ��ȥ���ո�)
		inline std::string& trim(std::string& str, const char* delim = " ");

		// ȥ���ַ����е������ض���һ�ַ�
		inline std::string&	erase(std::string& str, char c = ' ');

		// �ַ����滻 ȥ���ַ����е�ĳ�ض��ַ���delim�������ַ���s����
		inline std::string&	replace_all(std::string& str, const char* delim, const char* s = "");

		// �ַ����ָ�
		inline void			split(const std::string& str, std::vector<std::string>& result, const char* delim = " ");

		// ��ʽ����� format(xxx{}xxx{}xx,����1������2)
		template<class ...Args>
		inline std::string	format(const char* fmt, const Args&... args);

		// ��ʽ����� format(xxx{}xxx{}xx,����1������2)
		template<class ...Args>
		inline std::string	format(const std::string& fmt, const Args&... args);

		// �ַ������Դ�Сд�Ƚ�, ����������������map��set���ıȽ���
		struct string_cmp_nocase : 
			public std::binary_function<std::string, std::string, bool>
		{
		public:
			inline bool operator()(const std::string& lhs, const std::string& rhs) const;
		};
	}
}

#include "string_util.inl"