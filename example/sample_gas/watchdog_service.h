#pragma once
#include "libCoreCommon/service_base.h"
#include "libCoreCommon/ticker.h"
#include "libCoreCommon/normal_protobuf_factory.h"

#include "player_factory.h"

#include "libCoreCommon/json_protobuf_factory.h"

using namespace std;
using namespace core;
using namespace base;

class CWatchdogServiceMessageHandler;
class CPlayerMessageHandler;
class CWatchdogService :
	public CServiceBase
{
public:
	CWatchdogService();
	virtual ~CWatchdogService();

	virtual core::CProtobufFactory*	getServiceProtobufFactory() const;
	virtual core::CProtobufFactory*	getForwardProtobufFactory() const;

	virtual CActorFactory*			getActorFactory(const std::string& szType) const;
	virtual void					release();

private:
	virtual bool					onInit();
	virtual void					onFrame();
	virtual void					onQuit();

	void							onServiceConnect(const std::string& szType, uint32_t nServiceID);
	void							onNotifyGateOnlineCount(uint64_t nContext);

private:
	CWatchdogServiceMessageHandler*	m_pWatchdogServiceMessageHandler;
	CPlayerMessageHandler*			m_pPlayerMessageHandler;
	CNormalProtobufFactory*			m_pNormalProtobufFactory;
	core::CJsonProtobufFactory*		m_pJsonProtobufFactory;
	CPlayerFactory*					m_pPlayerFactory;

	core::CTicker					m_tickerNotifyGateOnlineCount;
};