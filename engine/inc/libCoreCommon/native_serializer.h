#pragma once

#include "message_serializer.h"
#include "native_message.h"

#include "libBaseCommon/debug_helper.h"

namespace core
{
	class CNativeSerializer :
		public CMessageSerializer
	{
	public:
		CNativeSerializer() : m_writeBuf(UINT16_MAX) { }
		virtual ~CNativeSerializer() { }

		virtual uint8_t		getType() const { return eMST_Native; }
		virtual void		destroyMessage(void* pMessage);
		virtual void*		unserializeMessageFromBuf(const std::string& szMessageName, const void* pData, uint32_t nSize);
		virtual int32_t		serializeMessageToBuf(const void* pMessage, void* pBuf, uint32_t nSize);
		virtual bool		getMessageName(const void* pMessage, char* szBuf, size_t nBufSize);
	
	private:
		base::CWriteBuf m_writeBuf;
	};
}