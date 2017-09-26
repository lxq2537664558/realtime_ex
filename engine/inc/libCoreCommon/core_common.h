#pragma once

#include <functional>
#include <memory>

#include "libBaseCommon/function_util.h"

#ifdef _WIN32

#	ifdef __BUILD_CORE_COMMON_DLL__
#		define __CORE_COMMON_API__ __declspec(dllexport)
#	else
#		define __CORE_COMMON_API__ __declspec(dllimport)
#	endif

#else

#	define __CORE_COMMON_API__

#endif

#define _GET_MESSAGE_ID(szMessageName) (base::function_util::hash(szMessageName.c_str()))
#define _GET_MESSAGE_ID_EX(szMessageName) (base::function_util::hash(szMessageName))

enum EResponseResultType
{
	eRRT_OK			= 1,
	eRRT_TIME_OUT	= 2,
	eRRT_ERROR		= 3,
};

namespace core
{
#pragma pack(push,1)
	// 消息头
	struct message_header
	{
		uint16_t	nMessageSize;	// 包括消息头的
		uint32_t	nMessageID;

		message_header(uint32_t nMessageID) : nMessageID(nMessageID) { }
		message_header() {}
	};

#pragma pack(pop)
}

#define message_begin(MessageName, nMessageID) \
class MessageName : public core::message_header\
{\
public:\
	MessageName() : core::message_header(nMessageID) { nMessageSize = sizeof(MessageName); }\
	static  uint32_t	getMessageID() { return nMessageID; }\
	static  const char*	getMessageName() { return #MessageName; }

#define message_end };

#define pack_begin(writeBuf)\
	writeBuf.clear();\
	writeBuf.write(this, sizeof(core::message_header));

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
	readBuf.read(this, sizeof(core::message_header));

#define unpack_end()


#define _INVALID_SOCKET_ID			-1

enum EMessageType
{
	eMT_NONE				= 0,
	eMT_HEARTBEAT			= 1,		// 服务之间心跳消息
	eMT_SYSTEM				= 2,		// 服务之间的系统消息
	eMT_REQUEST				= 3,		// 服务之间的请求消息
	eMT_RESPONSE			= 4,		// 服务之间的响应消息
	eMT_GATE_FORWARD		= 5,		// 客户端通过网关服务转发给其他服务消息
	eMT_TO_GATE				= 6,		// 其他服务通过网关服务转发客户端消息
	eMT_TO_GATE_BROADCAST	= 7,		// 其他服务通过网关服务广播客户端消息
	eMT_CLIENT				= 8,		// 客户端消息
};

enum ECoreConnectionType
{
	eCCT_Normal,
	eCCT_Websocket,
};

enum EServiceSelectorType
{
	eSST_Random = 1,
	eSST_Hash	= 2,
};

enum EMessageSerializerType
{
	eMST_Protobuf		= 1,
	eMST_JsonProtobuf	= 2,
	eMST_Native			= 3,
};

#define _MAX_MESSAGE_NAME_LEN 128

namespace core
{
	typedef std::function<int32_t(const char*, uint32_t, uint8_t&)>	MessageParser;	// 原生消息

	struct	SMessagePacket
	{
		uint8_t		nType;
		uint32_t	nDataSize;
		void*		pData;
	};

	struct	SActorMessagePacket
	{
		uint8_t		nType;
		uint8_t		nMessageSerializerType;
		uint32_t	nFromServiceID;
		uint64_t	nData;
		uint64_t	nSessionID;
		void*		pMessage;
	};

	struct SNodeBaseInfo
	{
		uint32_t	nID;
		std::string	szName;			// 节点名字
		std::string	szHost;			// 服务器IP
		uint16_t	nPort;			// 0表示该节点没有监听地址
		uint32_t	nRecvBufSize;	// 接收缓冲区大小
		uint32_t	nSendBufSize;	// 发送缓存区大小
	};

	struct SServiceBaseInfo
	{
		uint32_t	nID;
		std::string	szName;			// 服务名字
		std::string	szType;			// 服务类型（比如gate, gas）
	};

	struct SClientSessionInfo
	{
		uint32_t	nGateServiceID;
		uint64_t	nSessionID;
	};

	struct SSessionInfo
	{
		uint32_t			nFromServiceID;
		uint64_t			nFromActorID;
		uint64_t			nSessionID;
	};

	struct SSyncCallResultInfo
	{
		uint8_t	nResult;
		std::shared_ptr<void>
				pMessage;
	};

#pragma pack(push,1)
	struct gate_forward_cookice
	{
		uint64_t	nSessionID;
		uint32_t	nFromServiceID;
		uint64_t	nToActorID;
		uint32_t	nToServiceID;
	};

	struct gate_send_cookice
	{
		uint64_t	nSessionID;
		uint32_t	nToServiceID;
	};

	struct gate_broadcast_cookice
	{
		uint32_t	nToServiceID;
		uint16_t	nSessionCount;
	};

	struct request_cookice
	{
		uint64_t	nSessionID;
		uint64_t	nFromActorID;
		uint32_t	nFromServiceID;
		uint64_t	nToActorID;
		uint32_t	nToServiceID;
		uint8_t		nMessageSerializerType;
		uint16_t	nMessageNameLen;
		char		szMessageName[1];
	};

	struct response_cookice
	{
		uint64_t	nSessionID;
		uint32_t	nFromServiceID;
		uint64_t	nToActorID;
		uint32_t	nToServiceID;
		uint8_t		nMessageSerializerType;
		uint8_t		nResult;
		uint16_t	nMessageNameLen;
		char		szMessageName[1];
	};

	struct health_request_cookice
	{
		uint64_t	nSessionID;
		uint32_t	nFromServiceID;
		uint32_t	nToServiceID;
	};

	struct health_response_cookice
	{
		uint64_t	nSessionID;
		uint32_t	nToServiceID;
		uint8_t		nResult;
	};

#pragma pack(pop)
}

inline int32_t default_client_message_parser(const char* pData, uint32_t nSize, uint8_t& nMessageType)
{
	if (nSize < sizeof(core::message_header))
		return 0;

	const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pData);
	if (pHeader->nMessageSize < sizeof(core::message_header))
		return -1;

	// 不是完整的消息
	if (nSize < pHeader->nMessageSize)
		return 0;

	nMessageType = eMT_CLIENT;

	return pHeader->nMessageSize;
}