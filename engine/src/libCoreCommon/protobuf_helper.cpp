#include "protobuf_helper.h"

namespace core
{
	int32_t serialize_protobuf_message_to_buf(const google::protobuf::Message* pMessage, void* pBuf, uint32_t nSize)
	{
		DebugAstEx(pMessage != nullptr, false);

		std::string szMessageData;
		if (!pMessage->SerializeToString(&szMessageData))
			return -1;

		if (szMessageData.size() > nSize)
			return -1;

		memcpy(pBuf, szMessageData.c_str(), szMessageData.size());

		return (int32_t)(szMessageData.size());
	}

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

	google::protobuf::Message* unserialize_protobuf_message_from_buf(const std::string& szMessageName, const void* pBuf, uint32_t nSize)
	{
		google::protobuf::Message* pMessage = create_protobuf_message(szMessageName);
		if (nullptr == pMessage)
			return nullptr;

		if (!pMessage->ParseFromArray(pBuf, nSize))
		{
			SAFE_DELETE(pMessage);
			return nullptr;
		}

		return pMessage;
	}
}