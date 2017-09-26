#pragma once
#include "libCoreCommon/service_base.h"
#include "libCoreCommon/json_protobuf_serializer.h"
#include "libCoreCommon/core_common.h"

#include "login_client_connection_factory.h"
#include "login_client_message_dispatcher.h"
#include "login_client_message_handler.h"

class CLoginService :
	public core::CServiceBase
{
public:
	CLoginService(const core::SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName);
	virtual ~CLoginService();

	CLoginClientMessageDispatcher*	getLoginClientMessageDispatcher() const;

	virtual void					release();

private:
	virtual bool					onInit();
	virtual void					onFrame();
	virtual void					onQuit();

private:
	std::unique_ptr<CLoginClientConnectionFactory>		m_pLoginClientConnectionFactory;
	std::unique_ptr<CLoginClientMessageDispatcher>		m_pLoginClientMessageDispatcher;
	std::unique_ptr<CLoginClientMessageHandler>			m_pLoginClientMessageHandler;

	std::unique_ptr<core::CNormalProtobufSerializer>	m_pNormalProtobufSerializer;
	std::unique_ptr<core::CJsonProtobufSerializer>		m_pJsonProtobufSerializer;
};