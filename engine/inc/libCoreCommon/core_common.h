#pragma once

#include <functional>

#include "libBaseNetwork/network.h"

#include "ticker.h"

namespace core
{
#pragma pack(push,1)
	// 消息头
	struct inside_message_header
	{
		uint16_t	nMessageSize;	// 包括消息头的
		uint8_t		nMessageType;	// 消息类型，类型见下面	

		inside_message_header(uint8_t nMessageType) : nMessageType(nMessageType) { }
		inside_message_header() {}
	};

	struct client_message_header
	{
		uint16_t	nMessageSize;	// 包括消息头的
		uint32_t	nMessageID;
	};

	struct inside_message_cookice
	{
		uint32_t	nMessageID;

		inside_message_cookice(uint32_t nMessageID) : nMessageID(nMessageID) { }
		inside_message_cookice() { nMessageID = 0; }
		~inside_message_cookice() { }
	};
#pragma pack(pop)
}

#define message_begin(MessageName, nMessageID) \
class MessageName : public core::inside_message_cookice\
{\
public:\
	MessageName() : core::inside_message_cookice(nMessageID) { }\
	static  uint16_t	getMessageID() { return nMessageID; }\
	static  const char*	getMessageName() { return #MessageName; }

#define message_end };

#define pack_begin(writeBuf)\
	writeBuf.clear();\
	writeBuf.write(this, sizeof(core::inside_message_cookice));

#define pack_end(writeBuf)\
	do\
	{\
		uint16_t nPos = (uint16_t)writeBuf.getCurSize(); \
		writeBuf.seek(base::eBST_Begin, 0); \
		writeBuf.write(nPos); \
		writeBuf.seek(base::eBST_Begin, nPos);\
	} while(0)

#define unpack_begin(buf, size)\
	base::CReadBuf readBuf;\
	readBuf.init(buf, size);\
	readBuf.read(this, sizeof(core::inside_message_cookice));

#define unpack_end()



#define _INVALID_SOCKET_ID			-1

enum EMessageType
{
	eMT_HEARTBEAT		= 1,		// 服务之间心跳消息
	eMT_REQUEST			= 2,		// 服务之间的请求消息
	eMT_RESPONSE		= 3,		// 服务之间的响应消息
	eMT_SYSTEM			= 4,		// 服务之间的系统消息
	eMT_GATE_FORWARD	= 5,		// 客户端通过网关服务转发给其他服务消息
	eMT_TO_GATE			= 6,		// 其他服务通过网关服务转发客户端消息
	eMT_CLIENT			= 7,		// 客户端消息
	
	eMT_TYPE_MASK		= 0x00ff,	// 类型掩码
	eMT_LUA				= 0x0100,	// lua消息

	// 特殊标记
	eMT_BROADCAST		= 0x200,	// 广播消息
};

namespace core
{
	typedef std::function<int32_t(const char*, uint32_t, uint8_t&)>	MessageParser;	// 原生消息
}

#define _BASE_CONNECTION_TYPE_BEGIN	100