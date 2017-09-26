#pragma once
#include "native_message.h"

namespace core
{
native_message_begin(service_health_request)
	
	void pack(base::CWriteBuf& writeBuf) const
	{
		native_pack_begin(writeBuf);

		native_pack_end(writeBuf);
	}

	bool unpack(const void* pBuf, uint16_t nSize)
	{
		native_unpack_begin(pBuf, nSize);

		native_unpack_end();

		return true;
	}
native_message_end(service_health_request)

native_message_begin(service_health_response)

	void pack(base::CWriteBuf& writeBuf) const
	{
		native_pack_begin(writeBuf);

		native_pack_end(writeBuf);
	}

	bool unpack(const void* pBuf, uint16_t nSize)
	{
		native_unpack_begin(pBuf, nSize);

		native_unpack_end();

		return true;
	}
native_message_end(service_health_response)
}