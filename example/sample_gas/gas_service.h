#pragma once
#include "libCoreCommon/service_base.h"
#include "libCoreCommon/ticker.h"
#include "libCoreCommon/normal_protobuf_factory.h"
#include "libCoreCommon/json_protobuf_factory.h"

#include "player_factory.h"
#include "player_mgr.h"

using namespace std;
using namespace core;
using namespace base;

class CGasServiceMessageHandler;
class CPlayerMessageHandler;
class CGasService :
	public CServiceBase
{
public:
	CGasService();
	virtual ~CGasService();

	virtual core::CProtobufFactory*	getServiceProtobufFactory() const;
	virtual core::CProtobufFactory*	getForwardProtobufFactory() const;

	virtual CActorFactory*			getActorFactory(const std::string& szType) const;
	virtual void					release();

	CPlayerMgr*						getPlayerMgr() const;

private:
	virtual bool					onInit();
	virtual void					onFrame();
	virtual void					onQuit();

	void							onServiceConnect(const std::string& szType, uint32_t nServiceID);
	void							onNotifyOnlineCount(uint64_t nContext);

private:
	CGasServiceMessageHandler*	m_pGasServiceMessageHandler;
	CPlayerMessageHandler*		m_pPlayerMessageHandler;
	CNormalProtobufFactory*		m_pNormalProtobufFactory;
	core::CJsonProtobufFactory*	m_pJsonProtobufFactory;
	CPlayerFactory*				m_pPlayerFactory;
	CPlayerMgr*					m_pPlayerMgr;

	core::CTicker				m_tickerNotifyOnlineCount;
};