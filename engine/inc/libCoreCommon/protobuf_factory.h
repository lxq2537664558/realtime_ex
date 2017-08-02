#pragma once

#include <stdint.h>
#include <string.h>

#include "google\protobuf\message.h"

namespace core
{
	/**
	@brief: protobuf �����������нӿ���Ҫ�̰߳�ȫ���������Ҫ�麯�����ܱ�֤���յ��õĲ��Ǿ���ģ���CDefaultProtobufFactory��������core_common��CDefaultProtobufFactory
	*/
	class CProtobufFactory
	{
	public:
		virtual ~CProtobufFactory() { }

		virtual google::protobuf::Message*	create_protobuf_message(const std::string& szMessageName) = 0;
		virtual google::protobuf::Message*	clone_protobuf_message(const google::protobuf::Message* pMessage) = 0;
		virtual google::protobuf::Message*	unserialize_protobuf_message_from_buf(const std::string& szMessageName, const void* pData, uint32_t nSize) = 0;
		virtual int32_t						serialize_protobuf_message_to_buf(const google::protobuf::Message* pMessage, void* pBuf, uint32_t nSize) = 0;
	};
}