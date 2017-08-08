#pragma once
#include "libCoreCommon/service_base.h"
#include "libCoreCommon/ticker.h"
#include "libCoreCommon/default_protobuf_factory.h"

#include "test_actor2.h"
#include "my_actor_factory.h"

using namespace std;
using namespace core;
using namespace base;

class CTestActorService2MessageHandler;
class CTestActor2MessageHandler;
class CTestActorService2 :
	public CServiceBase
{
public:
	CTestActorService2();
	virtual ~CTestActorService2();

	virtual CProtobufFactory*	getProtobufFactory() const;
	virtual CActorFactory*		getActorFactory(const std::string& szType) const;
	virtual void				release();

private:
	virtual bool				onInit();
	virtual void				onFrame();
	virtual void				onQuit();
	
private:
	CTestActorService2MessageHandler*	m_pTestActorService2MessageHandler;
	CTestActor2MessageHandler*			m_pTestActor2MessageHandler;
	CTestActor2*						m_pTestActor2;
	CMyActorFactory*					m_pMyActorFactory;
	CDefaultProtobufFactory*			m_pDefaultProtobufFactory;
};