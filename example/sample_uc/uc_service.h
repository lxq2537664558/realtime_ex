#pragma once
#include "libCoreCommon/service_base.h"
#include "libCoreCommon/ticker.h"
#include "libCoreCommon/normal_protobuf_factory.h"
#include "user_info_mgr.h"

using namespace std;
using namespace core;
using namespace base;

class CUCServiceMessageHandler;
class CPlayerMessageHandler;
class CUCService :
	public CServiceBase
{
public:
	CUCService();
	virtual ~CUCService();

	virtual core::CProtobufFactory*	getServiceProtobufFactory() const;
	
	CUserInfoMgr*					getUserInfoMgr() const;

	virtual void					release();

private:
	virtual bool					onInit();
	virtual void					onFrame();
	virtual void					onQuit();

	void							onServiceConnect(const std::string& szType, uint32_t nServiceID);
	void							onNotifyActiveCount(uint64_t nContext);

private:
	CUCServiceMessageHandler*	m_pUCServiceMessageHandler;
	CNormalProtobufFactory*		m_pNormalProtobufFactory;
	core::CTicker				m_tickerNotifyActiveCount;

	CUserInfoMgr*				m_pUserInfoMgr;
};