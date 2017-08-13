#pragma once
#include "libCoreCommon/service_base.h"
#include "libCoreCommon/ticker.h"
#include "libCoreCommon/normal_protobuf_factory.h"

#include "online_count_mgr.h"

using namespace std;
using namespace core;
using namespace base;

class CDispatchServiceMessageHandler;
class CDispatchService :
	public CServiceBase
{
public:
	CDispatchService();
	virtual ~CDispatchService();

	virtual core::CProtobufFactory*	getServiceProtobufFactory() const;
	virtual void					release();

	COnlineCountMgr*				getOnlineCountMgr() const;

private:
	virtual bool					onInit();
	virtual void					onFrame();
	virtual void					onQuit();

private:
	CDispatchServiceMessageHandler*	m_pDispatchServiceMessageHandler;
	CNormalProtobufFactory*			m_pNormalProtobufFactory;
	COnlineCountMgr*				m_pOnlineCountMgr;
};