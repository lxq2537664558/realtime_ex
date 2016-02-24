#pragma once

#include <functional>

#include "libBaseNetwork/network.h"

#include "ticker.h"

namespace core
{
#pragma pack(push,1)
	struct message_header
	{
		uint16_t nMsgSize;	// 包括消息头的
		uint16_t nMsgID;	

		message_header( uint16_t nMsgID ) : nMsgID(nMsgID) { }
		message_header() {}
	};
#pragma pack(pop)

	inline int32_t default_parser_raw_data(const void* pData, uint32_t nDataSize)
	{
		// 都不够消息头
		if (nDataSize < sizeof(core::message_header))
			return 0;

		const message_header* pMessageHead = reinterpret_cast<const message_header*>(pData);
		if (pMessageHead->nMsgSize < sizeof(message_header))
			return -1;

		// 不是完整的消息
		if (nDataSize < pMessageHead->nMsgSize)
			return 0;

		return pMessageHead->nMsgSize;
	}
}

#define Msg_Begin(MsgName, MsgID) \
class MsgName : public core::message_header\
{\
public:\
	MsgName() : message_header(MsgID) { nMsgSize = sizeof(MsgName); }\
	static  uint16_t	getMsgIDByType() { return MsgID; }\
	static  const char*	getMsgName() { return #MsgName; }

#define Msg_End };


#define _INVALID_SOCKET_ID		-1
#define _MAX_SERVICE_TYPE_LEN	64
#define _MAX_SERVICE_NAME_LEN	64

enum EMessageType
{
	eMT_HEARTBEAT	= 1,		// 心跳包
	eMT_REQUEST		= 2,		// 请求包
	eMT_RESPONSE	= 3,		// 响应包
	eMT_SYSTEM		= 4,		// 集群内部的系统包
	eMT_GATE		= 5,		// 网关服务转发的客户端消息
	eMT_CLIENT		= 6,		// 客户端包，不做任何处理，跟外部系统交互一般用这个，比如客户端
	
	eMT_BROADCAST	= 0x200,	// 广播包
};

#pragma pack(push,1)
struct gate_header
{
	uint64_t nID;
};
#pragma pack(pop)

struct SServiceBaseInfo
{
	char		szType[_MAX_SERVICE_TYPE_LEN];
	char		szName[_MAX_SERVICE_NAME_LEN];
	char		szHost[INET_ADDRSTRLEN];
	uint16_t	nPort;	// 0表示该服务没有监听地址
	uint32_t	nRecvBufSize;
	uint32_t	nSendBufSize;
};

namespace core
{
	typedef std::function<int32_t(const char*, uint32_t)>	funRawDataParser;
}