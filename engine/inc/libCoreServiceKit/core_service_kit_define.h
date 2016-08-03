#pragma once
#include "libCoreCommon/core_common.h"
#include "libCoreCommon/message.h"

#include <string>
#include <vector>
#include <map>
#include <memory>

#define _SERVICE_WEIGHT_PERCENT_MULT	10000	// 服务权重倍数
#define _MAX_SERVICE_TYPE_LEN			64
#define _MAX_SERVICE_NAME_LEN			64
#define _MAX_SERVICE_API_NAME_LEN		128

#define _GET_MESSAGE_ID(szMessageName) (base::hash(szMessageName.c_str()))

enum EResponseResultType
{
	eRRT_OK,
	eRRT_TIME_OUT,
	eRRT_ERROR,
};

#define _REMOTE_ACTOR_BIT 48

namespace core
{

	struct SServiceBaseInfo
	{
		uint16_t	nID;
		std::string	szType;			// 服务类型
		std::string	szName;			// 服务名字
		std::string	szGroup;		// 服务所属的组
		std::string	szHost;			// 服务器IP
		uint16_t	nPort;			// 0表示该服务没有监听地址
		uint32_t	nRecvBufSize;	// 接收缓冲区大小
		uint32_t	nSendBufSize;	// 发送缓存区大小
	};

	struct SClientSessionInfo
	{
		uint16_t	nServiceID;
		uint64_t	nSessionID;

		SClientSessionInfo(uint16_t	nServiceID, uint64_t nSessionID)
			: nServiceID(nServiceID), nSessionID(nSessionID)
		{}
	};

	struct SServiceSessionInfo
	{
		uint16_t	nServiceID;
		uint64_t	nSessionID;
	};

	struct SActorSessionInfo
	{
		uint64_t	nActorID;
		uint64_t	nSessionID;
	};

	class CResponseFuture;
	
	typedef std::function<void(uint8_t, CMessage)>								InvokeCallback;			// RPC消息响应回调函数类型
	typedef std::function<CResponseFuture(uint8_t, CMessage)>					InvokeCallbackEx;		// RPC消息响应回调函数类型
	typedef std::function<void(uint32_t)>										InvokeErrCallback;		// RPC错误响应回调函数类型
	typedef std::function<void(uint16_t, uint8_t, CMessage)>					ServiceCallback;		// 服务消息处理函数类型(这里服务名字必须是值，不能是引用，因为有协程)
	typedef std::function<void(SClientSessionInfo, uint8_t, CMessage)>			GateForwardCallback;	// 经网关服务转发的客户端消息处理函数类型
	typedef std::function<void(uint64_t, CMessage)>								ClientCallback;			// 客户端消息处理函数类型
	typedef std::function<void(uint16_t, uint8_t, const void*, uint16_t)>		ServiceGlobalFilter;	// 全局的消息过滤器类型

#pragma pack(push,1)
	struct gate_forward_cookice
	{
		uint64_t nActorID;
		uint64_t nSessionID;
		uint64_t nTraceID;
	};

	struct gate_send_cookice
	{
		uint64_t nSessionID;
		uint64_t nTraceID;
	};
	
	struct gate_broadcast_cookice :
		public message_header
	{
		uint16_t nCount;
	};

	struct request_cookice
	{
		uint64_t nSessionID;
		uint64_t nFromActorID;
		uint64_t nToActorID;
		uint64_t nTraceID;
	};

	struct response_cookice
	{
		uint64_t	nActorID;
		uint64_t	nSessionID;
		uint64_t	nTraceID;
		uint8_t		nResult;
	};

#pragma pack(pop)
}