#pragma once

#include <functional>

#include "libBaseNetwork/network.h"

#include "ticker.h"

namespace core
{
#pragma pack(push,1)
	// 消息头
	struct message_header
	{
		uint16_t	nMessageSize;	// 包括消息头的
		uint16_t	nMessageID;	

		message_header(uint16_t nMessageID) : nMessageID(nMessageID) { }
		message_header() {}
	};

	struct request_cookice
	{
		uint64_t nSessionID;
	};

	struct response_cookice
	{
		uint64_t nSessionID;
		uint8_t  nResult;
	};

#pragma pack(pop)

	inline int32_t default_parser_native_data(const void* pData, uint32_t nDataSize)
	{
		// 都不够消息头
		if (nDataSize < sizeof(message_header))
			return 0;

		const message_header* pHeader = reinterpret_cast<const message_header*>(pData);
		if (pHeader->nMessageSize < sizeof(message_header))
			return -1;

		// 不是完整的消息
		if (nDataSize < pHeader->nMessageSize)
			return 0;

		return pHeader->nMessageSize;
	}
}

#define message_begin(MessageName, nMessageID) \
class MessageName : public core::message_header\
{\
public:\
	MessageName() : message_header(nMessageID) { nMessageSize = sizeof(MessageName); }\
	static  uint16_t	getMessageID() { return nMessageID; }\
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
#define _MAX_SERVICE_TYPE_LEN		64
#define _MAX_SERVICE_NAME_LEN		64
#define _MAX_SERVICE_API_NAME_LEN	128

enum EMessageType
{
	eMT_HEARTBEAT	= 1,		// 服务之间心跳消息
	eMT_REQUEST		= 2,		// 服务之间的请求消息
	eMT_RESPONSE	= 3,		// 服务之间的响应消息
	eMT_SYSTEM		= 4,		// 服务之间的系统消息
	eMT_FROM_GATE	= 5,		// 客户端通过网关服务转发给其他服务消息
	eMT_TO_GATE		= 6,		// 其他服务通过网关服务转发客户端消息
	eMT_CLIENT		= 7,		// 客户端消息
	
	eMT_TYPE_MASK	= 0x00ff,	// 类型掩码

	// 特殊标记
	eMT_NATIVE		= 0,		// 原生消息格式
	eMT_PROTOBUF	= 0x8000,	// protobuf格式
	eMT_BROADCAST	= 0x200,	// 广播消息
};

#pragma pack(push,1)
struct gate_cookice
{
	uint64_t nSessionID;
};
struct gate_cookice_broadcast
{
	uint16_t nCount;
};
#pragma pack(pop)

namespace core
{
	typedef std::function<int32_t(const char*, uint32_t)>	ClientDataCallback;

	struct SClientSessionInfo
	{
		const std::string&	szServiceName;
		uint64_t			nSessionID;

		SClientSessionInfo(const std::string&	szServiceName, uint64_t nSessionID)
			: szServiceName(szServiceName), nSessionID(nSessionID)
		{}
	};

	struct SServiceSessionInfo
	{
		std::string	szServiceName;
		uint64_t	nSessionID;
	};

	enum EResponseResultType
	{
		eRRT_OK,
		eRRT_TIME_OUT,
		eRRT_ERROR,
	};

	typedef std::function<void(uint16_t, const message_header*, EResponseResultType)>		InvokeCallback;			// RPC消息响应回调函数类型
	typedef std::function<void(const std::string&, uint16_t, const message_header*)>		ServiceCallback;		// 服务消息处理函数类型
	typedef std::function<void(const SClientSessionInfo&, uint16_t, const message_header*)>	GateClientCallback;		// 经网关服务转发的客户端消息处理函数类型
	typedef std::function<void(const message_header*)>										ClientCallback;			// 客户端消息处理函数类型
	typedef std::function<bool(const std::string&, uint16_t, const void*, uint16_t)>		ServiceGlobalCallback;	// 全局的服务消息处理函数类型
}

struct SServiceBaseInfo
{
	std::string	szType;
	std::string	szName;
	std::string	szHost;
	uint16_t	nPort;	// 0表示该服务没有监听地址
	uint32_t	nRecvBufSize;
	uint32_t	nSendBufSize;
};

struct SMessageSyncInfo
{
	uint8_t		nGate;
	uint32_t	nMessageID;
};