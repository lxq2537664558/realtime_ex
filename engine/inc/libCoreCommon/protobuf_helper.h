#pragma once
#include "core_common.h"

#include "google/protobuf/message.h"

namespace core
{
	google::protobuf::Message*	create_protobuf_message(const std::string& szMessageName);
	google::protobuf::Message*	unserialize_protobuf_message_from_buf(const std::string& szMessageName, const void* pBuf, uint32_t nSize);
	int32_t						serialize_protobuf_message_to_buf(const google::protobuf::Message* pMessage, void* pBuf, uint32_t nSize);
}