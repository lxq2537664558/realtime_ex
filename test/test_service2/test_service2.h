#pragma once
#include "libCoreCommon/service_base.h"
#include "libCoreCommon/ticker.h"
#include "libCoreCommon/normal_protobuf_serializer.h"

using namespace std;
using namespace core;
using namespace base;

class CTestService2MessageHandler;
class CTestService2 :
	public CServiceBase
{
public:
	CTestService2(const core::SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName);
	virtual ~CTestService2();

	virtual void	release();

private:
	virtual bool	onInit();
	virtual void	onFrame();
	virtual void	onQuit();

private:
	CTestService2MessageHandler*	m_pTestService2MessageHandler;
	CNormalProtobufSerializer*		m_pNormalProtobufSerializer;
};