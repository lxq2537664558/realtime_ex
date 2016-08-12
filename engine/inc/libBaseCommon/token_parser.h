#pragma once

#include "base_common.h"

namespace base
{

	class __BASE_COMMON_API__ CTokenParser
	{
	public:
		CTokenParser();
		~CTokenParser();

		bool	parse(const char* szBuf, char nDelim);
		void	reset();
		size_t	getElementCount() const;
		bool	getInt32Element(size_t nIndex, int32_t& nValue);
		bool	getUint32Element(size_t nIndex, uint32_t& nValue);
		bool	getInt64Element(size_t nIndex, int64_t& nValue);
		bool	getUint64Element(size_t nIndex, uint64_t& nValue);
		bool	getStringElement(size_t nIndex, char* szBuf, size_t nBufSize);

	private:
		char**	m_szElement;
		size_t	m_nCount;
	};
}