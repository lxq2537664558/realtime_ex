#pragma once
#include "libCoreCommon/service_base.h"
#include "libCoreCommon/ticker.h"
#include "libCoreCommon/normal_protobuf_serializer.h"

using namespace std;
using namespace core;
using namespace base;

class CTestService3MessageHandler;
class CTestService3 :
	public CServiceBase
{
public:
	CTestService3(const core::SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName);
	virtual ~CTestService3();

	virtual void	release();

private:
	virtual bool	onInit();
	virtual void	onFrame();
	virtual void	onQuit();

private:
	CTestService3MessageHandler*	m_pTestService3MessageHandler;
	CNormalProtobufSerializer*		m_pNormalProtobufSerializer;
};