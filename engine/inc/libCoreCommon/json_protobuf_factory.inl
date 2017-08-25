namespace core
{
	google::protobuf::Message* CJsonProtobufFactory::create_protobuf_message(const std::string& szMessageName)
	{
		const google::protobuf::Descriptor* pDescriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(szMessageName);
		if (pDescriptor == nullptr)
			return nullptr;

		const google::protobuf::Message* pProtoType = google::protobuf::MessageFactory::generated_factory()->GetPrototype(pDescriptor);
		if (pProtoType == nullptr)
			return nullptr;

		return pProtoType->New();
	}

	google::protobuf::Message* CJsonProtobufFactory::clone_protobuf_message(const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, nullptr);

		google::protobuf::Message* pNewMessage = this->create_protobuf_message(pMessage->GetTypeName());
		if (nullptr == pNewMessage)
			return nullptr;
		
		// ���ﲻ�ܵ���CopyFrom��������Ϊ �п�����Щ��������proto�ķ�ʽ��̬�Ĵ����ģ���Щ����Դ����ķ�ʽ�����ģ����ߵ�Descriptor��һ��
		std::string szData = pMessage->SerializeAsString();
		if (!pNewMessage->ParseFromString(szData))
		{
			SAFE_DELETE(pNewMessage);
			return nullptr;
		}

		return pNewMessage;
	}

	google::protobuf::Message* CJsonProtobufFactory::unserialize_protobuf_message_from_buf(const std::string& szMessageName, const void* pData, uint32_t nSize)
	{
		DebugAstEx(pData != nullptr, nullptr);

		google::protobuf::Message* pMessage = this->create_protobuf_message(szMessageName);
		if (nullptr == pMessage)
			return nullptr;

		std::string szJson(reinterpret_cast<const char*>(pData), nSize);
		if (!google::protobuf::util::JsonStringToMessage(szJson, pMessage).ok())
		{
			SAFE_DELETE(pMessage);
			return nullptr;
		}

		return pMessage;
	}

	int32_t CJsonProtobufFactory::serialize_protobuf_message_to_buf(const google::protobuf::Message* pMessage, void* pData, uint32_t nSize)
	{
		DebugAstEx(pMessage != nullptr && pData != nullptr, -1);

		std::string szJson;
		if (!google::protobuf::util::MessageToJsonString(*pMessage, &szJson).ok())
			return -1;
		
		if (szJson.size() > nSize)
			return -1;

		memcpy(pData, szJson.c_str(), szJson.size());

		return (int32_t)szJson.size();
	}
}