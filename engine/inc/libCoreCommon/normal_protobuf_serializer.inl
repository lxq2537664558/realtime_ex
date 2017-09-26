#include "libBaseCommon/function_util.h"

namespace core
{
	void* CNormalProtobufSerializer::create_message(const std::string& szMessageName)
	{
		const google::protobuf::Descriptor* pDescriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(szMessageName);
		if (pDescriptor == nullptr)
			return nullptr;

		const google::protobuf::Message* pProtoType = google::protobuf::MessageFactory::generated_factory()->GetPrototype(pDescriptor);
		if (pProtoType == nullptr)
			return nullptr;

		return pProtoType->New();
	}

	void* CNormalProtobufSerializer::unserializeMessageFromBuf(const std::string& szMessageName, const void* pData, uint32_t nSize)
	{
		DebugAstEx(pData != nullptr, nullptr);

		google::protobuf::Message* pMessage = static_cast<google::protobuf::Message*>(this->create_message(szMessageName));
		if (nullptr == pMessage)
			return nullptr;

		if (!pMessage->ParseFromArray(pData, nSize))
		{
			SAFE_DELETE(pMessage);
			return nullptr;
		}

		return pMessage;
	}

	int32_t CNormalProtobufSerializer::serializeMessageToBuf(const void* pMessage, void* pData, uint32_t nSize)
	{
		DebugAstEx(pMessage != nullptr && pData != nullptr, -1);

		const google::protobuf::Message* pPBMessage = static_cast<const google::protobuf::Message*>(pMessage);

		if (!pPBMessage->SerializeToArray(pData, nSize))
			return -1;

		return pPBMessage->ByteSize();
	}

	void CNormalProtobufSerializer::destroyMessage(void* pMessage)
	{
		if (pMessage == nullptr)
			return;

		const google::protobuf::Message* pPBMessage = static_cast<const google::protobuf::Message*>(pMessage);
		SAFE_DELETE(pPBMessage);
	}

	bool CNormalProtobufSerializer::getMessageName(const void* pMessage, char* szBuf, size_t nBufSize)
	{
		DebugAstEx(pMessage != nullptr, false);

		const google::protobuf::Message* pPBMessage = static_cast<const google::protobuf::Message*>(pMessage);
		const std::string szMessageName = pPBMessage->GetTypeName();
		if (szMessageName.size() >= nBufSize)
			return false;

		base::function_util::strcpy(szBuf, nBufSize, szMessageName.c_str());
		return true;
	}
}