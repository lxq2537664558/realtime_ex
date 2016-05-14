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

	google::protobuf::Message* unserialize_protobuf_message_from_buf(const std::string& szMessageName, const core::message_header* pHeader)
	{
		google::protobuf::Message* pMessage = create_protobuf_message(szMessageName);
		if (nullptr == pMessage)
			return nullptr;

		if (!pMessage->ParseFromArray(pHeader + 1, pHeader->nMessageSize - sizeof(core::message_header)))
		{
			SAFE_DELETE(pMessage);
			return nullptr;
		}

		return pMessage;
	}

	int32_t serialize_protobuf_message_to_buf(const google::protobuf::Message* pMessage, core::message_header* pHeader, uint32_t nSize)
	{
		DebugAstEx(pMessage != nullptr, false);

		std::string szMessageData;
		if (!pMessage->SerializeToString(&szMessageData))
			return -1;

		if (szMessageData.size() > nSize)
			return -1;

		pHeader->nMessageID = base::hash(pMessage->GetTypeName().c_str());
		pHeader->nMessageSize = (uint16_t)(sizeof(core::message_header) + szMessageData.size());

		memcpy(pHeader + 1, szMessageData.c_str(), szMessageData.size());

		return (int32_t)(szMessageData.size() + sizeof(core::message_header));
	}
}