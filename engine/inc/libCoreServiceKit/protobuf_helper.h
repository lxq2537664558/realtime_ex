#pragma once

#include <string>

#include "google/protobuf/message.h"

#include "libCoreCommon/core_common.h"

namespace core
{
	google::protobuf::Message*	create_protobuf_message(const std::string& szMessageName);
	google::protobuf::Message*	unserialize_protobuf_message_from_buf(const std::string& szMessageName, const core::message_header* pHeader);
	int32_t						serialize_protobuf_message_to_buf(const google::protobuf::Message* pMessage, core::message_header* pHeader, uint32_t nSize);
}