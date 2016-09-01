#pragma once
#include "libCoreCommon/core_common.h"

#include "message_ptr.h"

namespace core
{
	class CSerializeAdapter
	{
	public:
		CSerializeAdapter() { }
		virtual ~CSerializeAdapter() { }

		virtual message_header*		serialize(const void* pData, void* pBuf, uint16_t nBufSize) = 0;
		virtual CMessagePtr<char>	deserialize(const message_header* pData) = 0;
	};
}