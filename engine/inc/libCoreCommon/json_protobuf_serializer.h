#pragma once

#include "message_serializer.h"

#include "google/protobuf/message.h"
#include "google/protobuf/util/json_util.h"

#include "libBaseCommon/debug_helper.h"

namespace core
{
	class CJsonProtobufSerializer :
		public CMessageSerializer
	{
	public:
		virtual ~CJsonProtobufSerializer() { }

		virtual uint8_t 	getType() const { return eMST_JsonProtobuf; }
		inline void			destroyMessage(void* pMessage);
		inline void*		unserializeMessageFromBuf(const std::string& szMessageName, const void* pData, uint32_t nSize);
		inline int32_t		serializeMessageToBuf(const void* pMessage, void* pBuf, uint32_t nSize);
		inline bool			getMessageName(const void* pMessage, char* szBuf, size_t nBufSize);

	private:
		inline void*	create_message(const std::string& szMessageName);
	};
}

#include "json_protobuf_serializer.inl"