#pragma once
#include "libCoreCommon/service_base.h"
#include "libCoreCommon/ticker.h"
#include "libCoreCommon/normal_protobuf_factory.h"

#include "test_actor1.h"
#include "test_actor0.h"
#include "my_actor_factory.h"

#include <map>

using namespace std;
using namespace core;
using namespace base;

class CTestActor0MessageHandler;
class CTestActorService1 :
	public CServiceBase
{
public:
	CTestActorService1();
	virtual ~CTestActorService1();

private:
	virtual bool				onInit();
	virtual void				onFrame();
	virtual void				onQuit();

	void						onServiceConnect(const std::string&, uint32_t nServiceID);
	void						onServiceDisconnect(const std::string&, uint32_t nServiceID);

	virtual CProtobufFactory*	getServiceProtobufFactory() const;
	virtual CActorFactory*		getActorFactory(const std::string& szType) const;

	virtual void				release();

private:
	CTestActor0*				m_pTestActor0;
	CTestActor1*				m_pTestActor1;

	CTestActor0MessageHandler*	m_pTestActor0MessageHandler;
	CMyActorFactory*			m_pMyActorFactory;
	CNormalProtobufFactory*		m_pNormalProtobufFactory;
};