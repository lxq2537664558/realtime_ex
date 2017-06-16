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
	@brief: ��Ϣ�ɷ��������ɸ�����ϢԴ�������ɷ���Ϣ
	*/
	void	dispatch(uint64_t nSocketID, uint8_t nMessageType, const void* pData, uint16_t nSize);
	/**
	@brief: ע�ᾭ�ͻ�����Ϣ��Ӧ����
	*/
	void	registerCallback(uint16_t nMessageID, core::ClientCallback callback);
	/**
	@brief: ������ϢID��ȡ��Ϣ��Ӧ����
	*/
	core::ClientCallback
			getCallback(uint16_t nMessageID) const;

private:
	void	forward(uint64_t nSessionID, const core::message_header* pHeader);

private:
	std::map<uint16_t, core::ClientCallback>	m_mapClientCallbackInfo;
};