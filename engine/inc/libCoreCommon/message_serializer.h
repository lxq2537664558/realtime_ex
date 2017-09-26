#pragma once

#include "core_common.h"

#include <string>

namespace core
{
	/**
	@brief: ��Ϣϵ�л��������нӿ���Ҫ�̰߳�ȫ���������Ҫ�麯�����ܱ�֤���յ��õĲ��Ǿ���ģ���
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