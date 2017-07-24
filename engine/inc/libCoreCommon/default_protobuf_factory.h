#pragma once

#include "protobuf_factory.h"

#include "libBaseCommon/debug_helper.h"

namespace core
{
	class CDefaultProtobufFactory :
		public CProtobufFactory
	{
	public:
		virtual ~CDefaultProtobufFactory() { }

		inline google::protobuf::Message*	create_protobuf_message(const std::string& szMessageName);
		inline google::protobuf::Message*	clone_protobuf_message(const google::protobuf::Message* pMessage);
		inline google::protobuf::Message*	unserialize_protobuf_message_from_buf(const std::string& szMessageName, const void* pData, uint32_t nSize);
		inline int32_t						serialize_protobuf_message_to_buf(const google::protobuf::Message* pMessage, void* pBuf, uint32_t nSize);
	};
}

#include "default_protobuf_factory.inl"