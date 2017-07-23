#pragma once
#include "libCoreCommon\service_base.h"
#include "libCoreCommon\ticker.h"
#include "test_actor2.h"

using namespace std;
using namespace core;
using namespace base;

class CTestActorService2MessageHandler;
class CTestActor2MessageHandler;
class CTestActorService2 :
	public CServiceBase
{
	DECLARE_OBJECT(CTestActorService2)

public:
	CTestActorService2();
	virtual ~CTestActorService2();

private:
	virtual bool			onInit();
	virtual void			onFrame();
	virtual void			onQuit();

	
private:
	CTestActorService2MessageHandler*	m_pTestActorService2MessageHandler;
	CTestActor2MessageHandler*			m_pTestActor2MessageHandler;
	CTestActor2*						m_pTestActor2;
};