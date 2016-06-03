#include "stdafx.h"
#include "protobuf_helper.h"

namespace core
{
	google::protobuf::Message* create_protobuf_message(const std::string& szMessageName)
	{
		const google::protobuf::Descriptor* pDescriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(szMessageName);
		if (pDescriptor == nullptr)
			return nullptr;

		const google::protobuf::Message* pProtoType = google::protobuf::MessageFactory::generated_factory()->GetPrototype(pDescriptor);
		if (pProtoType == nullptr)
			return nullptr;

		return pProtoType->New();
	}

	google::protobuf::Message* unserialize_protobuf_message_from_buf(const std::string& szMessageName, const void* pData, uint16_t nSize)
	{
		DebugAstEx(pData != nullptr, nullptr);

		google::protobuf::Message* pMessage = create_protobuf_message(szMessageName);
		if (nullptr == pMessage)
			return nullptr;

		if (!pMessage->ParseFromArray(pData, nSize))
		{
			SAFE_DELETE(pMessage);
			return nullptr;
		}

		return pMessage;
	}

	int32_t serialize_protobuf_message_to_buf(const google::protobuf::Message* pMessage, void* pData, uint16_t nSize)
	{
		DebugAstEx(pMessage != nullptr && pData != nullptr, false);

		std::string szMessageData;
		if (!pMessage->SerializeToArray(pData, nSize))
			return -1;

		return pMessage->ByteSize();
	}
}