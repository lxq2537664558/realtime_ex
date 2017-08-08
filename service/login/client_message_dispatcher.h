#pragma once
#include "libBaseCommon/singleton.h"
#include "libCoreCommon/core_common.h"
#include "libCoreCommon/default_protobuf_factory.h"

#include "login_connection_from_client.h"

#include <map>

typedef std::function<void(CLoginConnectionFromClient*, const google::protobuf::Message*)>	ClientCallback;	// �ͻ�����Ϣ����������

class CGateService;
class CClientMessageDispatcher
{
public:
	CClientMessageDispatcher(CLoginService* pLoginService);
	~CClientMessageDispatcher();

	/**
	@brief: ��Ϣ�ɷ��������ɸ�����ϢԴ�������ɷ���Ϣ
	*/
	void	dispatch(CLoginConnectionFromClient* pConnectionFromClient, const void* pData, uint16_t nSize);
	/**
	@brief: ע�ᾭ�ͻ�����Ϣ��Ӧ����
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