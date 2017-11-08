#pragma once
#include "libCoreCommon/service_base.h"

#include "gate_client_connection_factory.h"
#include "gate_client_session_mgr.h"
#include "gate_client_message_dispatcher.h"
#include "gate_client_message_handler.h"
#include "gate_service_message_handler.h"

#include "libCoreCommon/json_protobuf_serializer.h"

class CGateService :
	public core::CServiceBase
{
public:
	CGateService(const core::SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName);
	virtual ~CGateService();

	CGateClientSessionMgr*			getGateClientSessionMgr() const;
	CGateClientMessageDispatcher*	getGateClientMessageDispatcher() const;
	CGateClientMessageHandler*		getGateClientMessageHandler() const;

	virtual void					release();

private:
	virtual bool					onInit();
	virtual void					onFrame();
	virtual void					onQuit();

	void							onServiceConnect(const std::string& szType, uint32_t nServiceID);
	void							onServiceDisconnect(const std::string& szType, uint32_t nServiceID);
	void							onNotifyOnlineCount(uint64_t nContext);

private:
	std::unique_ptr<CGateClientConnectionFactory>		m_pGateClientConnectionFactory;
	std::unique_ptr<CGateClientSessionMgr>				m_pGateClientSessionMgr;
	std::unique_ptr<CGateClientMessageDispatcher>		m_pGateClientMessageDispatcher;
	std::unique_ptr<CGateClientMessageHandler>			m_pGateClientMessageHandler;
	std::unique_ptr<CGateServiceMessageHandler>			m_pGateServiceMessageHandler;

	std::unique_ptr<core::CNormalProtobufSerializer>	m_pNormalProtobufSerializer;
	std::unique_ptr<core::CJsonProtobufSerializer>		m_pJsonProtobufSerializer;

	base::CTicker										m_tickerNotifyOnlineCount;
	std::string											m_szAddr;
};