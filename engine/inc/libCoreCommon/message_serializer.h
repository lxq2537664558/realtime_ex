#pragma once

#include "core_common.h"

#include <string>

namespace core
{
	/**
	@brief: 消息系列化器，所有接口需要线程安全，这里必须要虚函数才能保证最终调用的才是具体模块的
	*/
	class CMessageSerializer
	{
	public:
		virtual ~CMessageSerializer() { }

		virtual uint8_t		getType() const = 0;
		virtual void		destroyMessage(void* pMessage) = 0;
		virtual void*		unserializeMessageFromBuf(const std::string& szMessageName, const void* pData, uint32_t nSize) = 0;
		virtual int32_t		serializeMessageToBuf(const void* pMessage, void* pBuf, uint32_t nSize) = 0;
		virtual bool		getMessageName(const void* pMessage, char* szBuf, size_t nBufSize) = 0;
	};
}