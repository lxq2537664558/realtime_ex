#pragma once
#include "libCoreCommon/service_base.h"
#include "libCoreCommon/ticker.h"
#include "libCoreCommon/default_protobuf_factory.h"

using namespace std;
using namespace core;
using namespace base;

class CTestService2MessageHandler;
class CTestService2 :
	public CServiceBase
{
public:
	CTestService2();
	virtual ~CTestService2();

	virtual CProtobufFactory*	getProtobufFactory() const;
	virtual void				release();

private:
	virtual bool				onInit();
	virtual void				onFrame();
	virtual void				onQuit();

private:
	CTestService2MessageHandler*	m_pTestService2MessageHandler;
	CDefaultProtobufFactory*		m_pDefaultProtobufFactory;
};