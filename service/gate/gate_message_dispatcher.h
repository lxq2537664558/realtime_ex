#pragma once
#include "libBaseCommon/singleton.h"
#include "libCoreCommon/core_common.h"
#include "libCoreServiceKit/core_service_kit_common.h"

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
	void	dispatch(uint64_t nSocketID, uint8_t nMessageType, const void* pData, uint16_t nSize);
	/**
	@brief: 注册经客户端消息响应函数
	*/
	void	registerCallback(uint16_t nMessageID, core::ClientCallback callback);
	/**
	@brief: 根据消息ID获取消息响应函数
	*/
	core::ClientCallback
			getCallback(uint16_t nMessageID) const;

private:
	void	forward(uint64_t nSessionID, const core::message_header* pHeader);

private:
	std::map<uint16_t, core::ClientCallback>	m_mapClientCallbackInfo;
};