#pragma once

#include <functional>
#include <memory>

#include "google/protobuf/message.h"
#include "libBaseCommon/base_function.h"

#define _GET_MESSAGE_ID(szMessageName) (base::hash(szMessageName.c_str()))

enum EResponseResultType
{
	eRRT_OK,
	eRRT_TIME_OUT,
	eRRT_ERROR,
};

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

#pragma pack(pop)
}

#define message_begin(MessageName, nMessageID) \
class MessageName : public core::message_header\
{\
public:\
	MessageName() : core::message_header(nMessageID) { nMessageSize = sizeof(MessageName); }\
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

enum EMessageType
{
	eMT_HEARTBEAT			= 1,		// 服务之间心跳消息
	eMT_SYSTEM				= 2,		// 服务之间的系统消息
	eMT_REQUEST				= 3,		// 服务之间的请求消息
	eMT_RESPONSE			= 4,		// 服务之间的响应消息
	eMT_GATE_FORWARD		= 5,		// 客户端通过网关服务转发给其他服务消息
	eMT_TO_GATE				= 6,		// 其他服务通过网关服务转发客户端消息
	eMT_CLIENT				= 7,		// 客户端消息
	eMT_TICKER				= 8,		// 定时器
};

namespace core
{
	typedef std::function<int32_t(const char*, uint32_t, uint8_t&)>	MessageParser;	// 原生消息
	
	enum EMessageTargetType
	{
		eMTT_Actor		= 0,
		eMTT_Service	= 1,
	};

	struct	SMessagePacket
	{
		uint8_t		nType;
		uint32_t	nDataSize;
		void*		pData;
	};

	struct	SActorMessagePacket
	{
		uint8_t		nType;
		uint32_t	nFromServiceID;
		uint64_t	nData;
		uint64_t	nSessionID;
		google::protobuf::Message*		
					pMessage;
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
		std::string szClassName;	// 类名字
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
		EMessageTargetType	eFromType;
		uint64_t			nSessionID;
	};

	struct SSyncCallResultInfo
	{
		uint8_t	nResult;
		std::shared_ptr<google::protobuf::Message>
				pMessage;
	};

	typedef std::function<void(uint64_t, const message_header*)>						ClientCallback;				// 客户端消息处理函数类型
	typedef std::function<void(SSessionInfo, const void* pData, uint16_t nDataSize)>	NodeGlobalFilter;			// 全局的消息过滤器类型

#pragma pack(push,1)
	struct gate_forward_cookice
	{
		uint64_t	nSessionID;
		uint32_t	nFromServiceID;
		uint64_t	nToActorID;
		uint32_t	nToServiceID;
		uint16_t	nMessageNameLen;
		char		szMessageName[1];
	};

	struct gate_send_cookice
	{
		uint32_t	nToServiceID;
		uint64_t	nSessionID;
		uint16_t	nMessageNameLen;
		char		szMessageName[1];
	};

	struct gate_broadcast_cookice :
		public message_header
	{
		uint32_t	nToServiceID;
		uint16_t	nSessionCount;
		uint16_t	nMessageNameLen;
		char		szMessageName[1];
	};

	struct request_cookice
	{
		uint64_t	nSessionID;
		uint64_t	nFromActorID;
		uint32_t	nFromServiceID;
		uint64_t	nToActorID;
		uint32_t	nToServiceID;
		uint16_t	nMessageNameLen;
		char		szMessageName[1];
	};

	struct response_cookice
	{
		uint64_t	nSessionID;
		uint64_t	nToActorID;
		uint32_t	nToServiceID;
		uint8_t		nResult;
		uint16_t	nMessageNameLen;
		char		szMessageName[1];
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

#define _BASE_CONNECTION_TYPE_BEGIN	100