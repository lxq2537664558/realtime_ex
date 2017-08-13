#pragma once
#include "libCoreCommon/service_base.h"
#include "libCoreCommon/ticker.h"
#include "libCoreCommon/normal_protobuf_factory.h"

using namespace std;
using namespace core;
using namespace base;

class CTestService3MessageHandler;
class CTestService3 :
	public CServiceBase
{
public:
	CTestService3();
	virtual ~CTestService3();

	virtual CProtobufFactory*	getServiceProtobufFactory() const;
	virtual void				release();

private:
	virtual bool				onInit();
	virtual void				onFrame();
	virtual void				onQuit();

private:
	CTestService3MessageHandler*	m_pTestService3MessageHandler;
	CNormalProtobufFactory*			m_pNormalProtobufFactory;
};