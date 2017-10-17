#include "native_serializer.h"
#include "core_common.h"

#include "libBaseCommon/function_util.h"

namespace core
{
	void* CNativeSerializer::unserializeMessageFromBuf(const std::string& szMessageName, const void* pData, uint32_t nSize)
	{
		DebugAstEx(pData != nullptr, nullptr);

		native_message_header* pNewMessage = native_message_header::createNativeMessage(szMessageName);
		if (nullptr == pNewMessage)
			return nullptr;

		if (!pNewMessage->unpack(pData, (uint16_t)nSize))
		{
			native_message_header::destroyNativeMessage(pNewMessage);
			return nullptr;
		}

		return pNewMessage;
	}

	int32_t CNativeSerializer::serializeMessageToBuf(const void* pMessage, void* pData, uint32_t nSize)
	{
		DebugAstEx(pMessage != nullptr && pData != nullptr, -1);

		const native_message_header* pHeader = static_cast<const native_message_header*>(pMessage);
		pHeader->pack(this->m_writeBuf);
		if (nSize < this->m_writeBuf.getCurSize())
			return -1;

		memcpy(pData, this->m_writeBuf.getBuf(), this->m_writeBuf.getCurSize());

		return this->m_writeBuf.getCurSize();
	}

	void CNativeSerializer::destroyMessage(void* pMessage)
	{
		if (pMessage == nullptr)
			return;

		native_message_header* pHeader = static_cast<native_message_header*>(pMessage);

		native_message_header::destroyNativeMessage(pHeader);
	}

	bool CNativeSerializer::getMessageName(const void* pMessage, char* szBuf, size_t nBufSize)
	{
		DebugAstEx(pMessage != nullptr, false);

		const native_message_header* pHeader = static_cast<const native_message_header*>(pMessage);
		const char* szMessageName = pHeader->getMessageName();
		if (szMessageName == nullptr)
			return false;

		size_t nMessageNameLen = base::function_util::strnlen(szMessageName, _TRUNCATE);
		if (nMessageNameLen >= nBufSize)
			return false;

		base::function_util::strcpy(szBuf, nBufSize, szMessageName);

		return true;
	}
}