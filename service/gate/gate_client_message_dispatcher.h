#pragma once
#include "libBaseCommon/singleton.h"
#include "libCoreCommon/core_common.h"
#include "libCoreCommon/normal_protobuf_serializer.h"

#include "gate_client_session.h"
#include "gate_connection_from_client.h"

#include <map>

typedef std::function<void(CGateConnectionFromClient*, const google::protobuf::Message*)>	ClientCallback;	// �ͻ�����Ϣ����������

class CGateService;
class CGateClientMessageDispatcher
{
public:
	CGateClientMessageDispatcher(CGateService* pGateService);
	~CGateClientMessageDispatcher();

	/**
	@brief: ��Ϣ�ɷ��������ɸ�����ϢԴ�������ɷ���Ϣ
	*/
	void	dispatch(CGateConnectionFromClient* pGateConnectionFromClient, const void* pData, uint16_t nSize);
	/**
	@brief: ע�ᾭ�ͻ�����Ϣ��Ӧ����
	*/
	void	registerMessageHandler(const std::string& szMessageName, const ClientCallback& callback);

private:
	void	forward(CGateClientSession* pGateClientSession, const core::message_header* pHeader);

	void	onToGateMessage(uint64_t nSessionID, const void* pData, uint16_t nDataSize);
	void	onToGateBroadcastMessage(const uint64_t* pSessionID, uint16_t nSessionCount, const void* pData, uint16_t nDataSize);

private:
	struct SClientMessageHandler
	{
		std::string		szMessageName;
		ClientCallback	callback;
	};
	std::map<uint32_t, SClientMessageHandler>	m_mapMessageHandler;
	CGateService*								m_pGateService;
};