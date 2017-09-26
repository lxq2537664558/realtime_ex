#pragma once
#include "libBaseCommon/singleton.h"
#include "libCoreCommon/core_common.h"
#include "libCoreCommon/normal_protobuf_serializer.h"

#include "login_connection_from_client.h"

#include <map>

typedef std::function<void(CLoginConnectionFromClient*, const google::protobuf::Message*)>	ClientCallback;	// 客户端消息处理函数类型

class CGateService;
class CLoginClientMessageDispatcher
{
public:
	CLoginClientMessageDispatcher(CLoginService* pLoginService);
	~CLoginClientMessageDispatcher();

	/**
	@brief: 消息派发函数，由各个消息源调用来派发消息
	*/
	void	dispatch(CLoginConnectionFromClient* pConnectionFromClient, const void* pData, uint16_t nSize);
	/**
	@brief: 注册经客户端消息响应函数
	*/
	void	registerMessageHandler(const std::string& szMessageName, const ClientCallback& callback);

private:
	struct SClientMessageHandler
	{
		std::string		szMessageName;
		ClientCallback	callback;
	};
	std::map<uint32_t, SClientMessageHandler>	m_mapMessageHandler;
	CLoginService*								m_pLoginService;
};