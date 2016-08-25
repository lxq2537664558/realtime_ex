#pragma once
#include "libCoreCommon/core_common.h"
#include "libCoreServiceKit/promise.h"

#include <string>
#include <vector>
#include <map>
#include <memory>

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

	struct SNodeBaseInfo
	{
		uint16_t	nID;
		std::string	szName;			// 节点名字
		std::string	szGroup;		// 节点所属的组
		std::string	szHost;			// 节点器IP
		uint16_t	nPort;			// 0表示该节点没有监听地址
		uint32_t	nRecvBufSize;	// 接收缓冲区大小
		uint32_t	nSendBufSize;	// 发送缓存区大小
	};

	struct SClientSessionInfo
	{
		uint16_t	nGateNodeID;
		uint64_t	nSessionID;

		SClientSessionInfo(uint16_t	nGateNodeID, uint64_t nSessionID)
			: nGateNodeID(nGateNodeID), nSessionID(nSessionID)
		{}
	};

	struct SNodeSessionInfo
	{
		uint16_t	nNodeID;
		uint64_t	nSessionID;
	};

	struct SActorSessionInfo
	{
		uint64_t	nActorID;
		uint64_t	nSessionID;
	};

	typedef std::shared_ptr<message_header>	CMessage;
	
	typedef std::function<void(uint64_t, CMessage)>									ClientCallback;				// 客户端消息处理函数类型
	typedef std::function<bool(uint64_t, uint16_t, uint8_t, const void*, uint16_t)>	GlobalBeforeFilter;			// 全局的消息过滤器类型
	typedef std::function<void(uint64_t, uint16_t, uint8_t, const void*, uint16_t)>	GlobalAfterFilter;			// 全局的消息过滤器类型
	
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
		uint8_t		nResult;
	};

#pragma pack(pop)
}