#pragma once
#include "libCoreCommon\service_base.h"
#include "libCoreCommon\ticker.h"
#include "libCoreCommon\default_protobuf_factory.h"

#include "test_actor1.h"
#include "test_actor0.h"
#include "my_actor_id_converter.h"

#include <map>

using namespace std;
using namespace core;
using namespace base;

class CTestActor0MessageHandler;
class CTestActorService1 :
	public CServiceBase
{
	DECLARE_OBJECT(CTestService1)

public:
	CTestActorService1();
	virtual ~CTestActorService1();

private:
	virtual bool	onInit();
	virtual void	onFrame();
	virtual void	onQuit();

	void			onServiceConnect(const std::string&, uint32_t nServiceID);
	void			onServiceDisconnect(const std::string&, uint32_t nServiceID);

private:
	CTestActor0*				m_pTestActor0;
	CTestActor1*				m_pTestActor1;

	CTestActor0MessageHandler*	m_pTestActor0MessageHandler;
	CMyActorIDConverter*		m_pMyActorIDConverter;
	CDefaultProtobufFactory*	m_pDefaultProtobufFactory;
};