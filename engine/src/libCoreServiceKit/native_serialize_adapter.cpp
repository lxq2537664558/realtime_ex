#include "stdafx.h"
#include "native_serialize_adapter.h"

namespace core
{
	CNativeSerializeAdapter::CNativeSerializeAdapter()
	{

	}

	CNativeSerializeAdapter::~CNativeSerializeAdapter()
	{

	}

	message_header* CNativeSerializeAdapter::serialize(const void* pData, void* pBuf, uint16_t nBufSize)
	{
		return const_cast<message_header*>(reinterpret_cast<const message_header*>(pData));
	}

	CMessagePtr<char> CNativeSerializeAdapter::deserialize(const message_header* pData)
	{
		char* pNewData = new char[pData->nMessageSize];
		memcpy(pNewData, pData, pData->nMessageSize);
		CMessagePtr<char> pMessage = CMessagePtr<char>(pNewData, pData->nMessageID, [pNewData](void*) { delete[] pNewData; });

		return pMessage;
	}
}