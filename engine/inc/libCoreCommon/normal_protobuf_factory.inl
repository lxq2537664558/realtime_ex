namespace core
{
	google::protobuf::Message* CNormalProtobufFactory::create_protobuf_message(const std::string& szMessageName)
	{
		const google::protobuf::Descriptor* pDescriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(szMessageName);
		if (pDescriptor == nullptr)
			return nullptr;

		const google::protobuf::Message* pProtoType = google::protobuf::MessageFactory::generated_factory()->GetPrototype(pDescriptor);
		if (pProtoType == nullptr)
			return nullptr;

		return pProtoType->New();
	}

	google::protobuf::Message* CNormalProtobufFactory::clone_protobuf_message(const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, nullptr);

		google::protobuf::Message* pNewMessage = this->create_protobuf_message(pMessage->GetTypeName());
		if (nullptr == pNewMessage)
			return nullptr;

		// 这里不能调用CopyFrom函数，因为 有可能有些服务是以proto的方式动态的创建的，有些是以源代码的方式创建的，两者的Descriptor不一致
		std::string szData = pMessage->SerializeAsString();
		if (!pNewMessage->ParseFromString(szData))
		{
			SAFE_DELETE(pNewMessage);
			return nullptr;
		}

		return pNewMessage;
	}

	google::protobuf::Message* CNormalProtobufFactory::unserialize_protobuf_message_from_buf(const std::string& szMessageName, const void* pData, uint32_t nSize)
	{
		DebugAstEx(pData != nullptr, nullptr);

		google::protobuf::Message* pMessage = this->create_protobuf_message(szMessageName);
		if (nullptr == pMessage)
			return nullptr;

		if (!pMessage->ParseFromArray(pData, nSize))
		{
			SAFE_DELETE(pMessage);
			return nullptr;
		}

		return pMessage;
	}

	int32_t CNormalProtobufFactory::serialize_protobuf_message_to_buf(const google::protobuf::Message* pMessage, void* pData, uint32_t nSize)
	{
		DebugAstEx(pMessage != nullptr && pData != nullptr, -1);

		if (!pMessage->SerializeToArray(pData, nSize))
			return -1;

		return pMessage->ByteSize();
	}
}