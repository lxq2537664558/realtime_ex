#pragma once
#include "core_common.h"
#include "promise.h"

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

namespace core
{

	struct SNodeBaseInfo
	{
		uint16_t	nID;
		std::string	szName;			// 节点名字
		std::string	szHost;			// 服务器IP
		uint16_t	nPort;			// 0表示该节点没有监听地址
		uint32_t	nRecvBufSize;	// 接收缓冲区大小
		uint32_t	nSendBufSize;	// 发送缓存区大小
	};

	struct SServiceBaseInfo
	{
		uint16_t	nID;
		std::string	szName;			// 服务名字
		std::string	szType;			// 服务类型（比如gate, gas）
	};

	struct SClientSessionInfo
	{
		uint16_t	nGateServiceID;
		uint64_t	nSessionID;

		SClientSessionInfo(uint16_t	nGateServiceID, uint64_t nSessionID)
			: nGateServiceID(nGateServiceID), nSessionID(nSessionID)
		{}

		SClientSessionInfo()
			: nGateServiceID(0), nSessionID(0)
		{}
	};

	struct SServiceSessionInfo
	{
		uint16_t	nServiceID;
		uint64_t	nSessionID;

		SServiceSessionInfo(uint16_t nServiceID, uint64_t nSessionID)
			: nServiceID(nServiceID), nSessionID(nSessionID)
		{}

		SServiceSessionInfo()
			: nServiceID(0), nSessionID(0)
		{}
	};

	struct SActorSessionInfo
	{
		uint64_t	nActorID;
		uint64_t	nSessionID;
	};

	typedef std::function<void(uint64_t, const message_header*)> ClientCallback;	// 客户端消息处理函数类型

#pragma pack(push,1)
	struct gate_forward_cookice
	{
		uint16_t	nFromServiceID;
		uint16_t	nToServiceID;
		uint64_t	nSessionID;
		uint16_t	nMessageNameLen;
		char		szMessageName[1];
	};

	struct actor_gate_forward_cookice
	{
		uint16_t	nFromServiceID;
		uint64_t	nToActorID;
		uint64_t	nSessionID;
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
		uint16_t	nFromServiceID;
		uint16_t	nToServiceID;
		uint16_t	nMessageNameLen;
		char		szMessageName[1];
	};

	struct response_cookice
	{
		uint64_t	nSessionID;
		uint8_t		nResult;
		uint16_t	nMessageNameLen;
		char		szMessageName[1];
	};

	struct actor_request_cookice
	{
		uint64_t	nSessionID;
		uint64_t	nFromActorID;
		uint64_t	nToActorID;
		uint16_t	nMessageNameLen;
		char		szMessageName[1];
	};

	struct actor_response_cookice
	{
		uint64_t	nToActorID;
		uint64_t	nSessionID;
		uint8_t		nResult;
		uint16_t	nMessageNameLen;
		char		szMessageName[1];
	};

#pragma pack(pop)
}