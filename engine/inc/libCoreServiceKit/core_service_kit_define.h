#pragma once
#include "libCoreCommon/core_common.h"

#include "google/protobuf/message.h"

#include <string>
#include <vector>
#include <map>

#define _SERVICE_WEIGHT_PERCENT_MULT	10000	// 服务权重倍数
#define _MAX_SERVICE_TYPE_LEN			64
#define _MAX_SERVICE_NAME_LEN			64
#define _MAX_SERVICE_API_NAME_LEN		128

#define _GET_MESSAGE_ID(szMessageName) (base::hash(szMessageName.c_str()))

namespace core
{
	struct SMetaMessageProxyInfo
	{
		std::string	szMessageName;		// 消息名字
		std::string	szLoadbalanceName;	// 负载均衡器名字
		std::string	szFail;				// 失败策略名字
		uint32_t	nRetries;			// 失败重试次数
		uint32_t	nTimeout;			// 远程调用超时时间
	};

	struct SMessageProxyInfo
	{
		std::string	szServiceName;		// 服务名字
		std::string	szServiceGroup;		// 服务组
		std::string	szMessageName;		// 消息名字
		int32_t		nWeight;			// 负载均衡权重
	};

	struct SMessageProxyGroupInfo
	{
		std::map<std::string, SMessageProxyInfo>	mapMessageProxyInfo;
		std::map<std::string, int32_t>				mapGroupWeight;
		int32_t										nTotalWeight;
	};

	struct SServiceBaseInfo
	{
		std::string	szType;			// 服务类型
		std::string	szName;			// 服务名字
		std::string	szGroup;		// 服务所属的组
		uint32_t	nWeight;		// 服务作为提供者在所有相同提供者中的权重
		std::string	szHost;
		uint16_t	nPort;			// 0表示该服务没有监听地址
		uint32_t	nRecvBufSize;
		uint32_t	nSendBufSize;
	};

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

	typedef std::function<void(uint32_t, const google::protobuf::Message*, EResponseResultType)>		InvokeCallback;			// RPC消息响应回调函数类型
	typedef std::function<void(const std::string&, uint32_t, const google::protobuf::Message*)>			ServiceCallback;		// 服务消息处理函数类型
	typedef std::function<void(const SClientSessionInfo&, uint32_t, const google::protobuf::Message*)>	GateForwardCallback;	// 经网关服务转发的客户端消息处理函数类型
	typedef std::function<void(uint64_t, const message_header*)>										ClientCallback;			// 客户端消息处理函数类型
	typedef std::function<bool(const std::string&, uint32_t, const void*, uint16_t)>					ServiceGlobalFilter;	// 全局的消息过滤器类型

#pragma pack(push,1)
	struct gate_cookice
	{
		uint64_t nSessionID;
	};
	
	struct gate_cookice_broadcast
	{
		uint16_t nCount;
	};

	struct request_cookice
	{
		uint64_t nSessionID;
	};

	struct response_cookice
	{
		uint64_t	nSessionID;
		uint8_t		nResult;
		uint16_t	nMessageNameLen;
		char		szMessageName[1];
	};

#pragma pack(pop)
}