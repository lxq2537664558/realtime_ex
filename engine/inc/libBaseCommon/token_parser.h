#pragma once

#include "base_common.h"

namespace base
{

	class CTokenParser
	{
	public:
		CTokenParser();
		~CTokenParser();

		bool	parse(const char* text, char delim);
		void	reset();
		size_t	getElementCount() const;
		bool	getInt32Element(size_t index, int32_t& value);
		bool	getUint32Element(size_t index, uint32_t& value);
		bool	getInt64Element(size_t index, int64_t& value);
		bool	getUint64Element(size_t index, uint64_t& value);
		bool	getStringElement(size_t index, const char*& value);

	private:
		char**	m_szElement;
		size_t	m_nCount;
	};
}