#pragma once
#include "libBaseCommon/singleton.h"

#include "libCoreCommon/core_common.h"

#include <map>

class CGateMessageDispatcher
	: public base::CSingleton<CGateMessageDispatcher>
{
public:
	CGateMessageDispatcher();
	~CGateMessageDispatcher();

	bool	init();

	/**
	@brief: 消息派发函数，由各个消息源调用来派发消息
	*/
	void	dispatch(uint16_t nMessageType, const void* pData, uint16_t nSize);
	/**
	@brief: 注册经客户端消息响应函数
	*/
	void	registCallback(uint32_t nMessageID, core::ClientCallback callback);
	/**
	@brief: 获取消息响应函数
	*/
	core::ClientCallback&
			getCallback(uint32_t nMessageID);

private:
	std::map<uint32_t, core::ClientCallback>	m_mapClientCallback;
};