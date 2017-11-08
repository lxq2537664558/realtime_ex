#pragma once
#include "libBaseCommon/ticker.h"

#include "libCoreCommon/service_base.h"
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

	void			TestTickCallBack3(uint64_t  ucontext);
	void			TestTickCallBack0(uint64_t  ucontext);

private:
	struct  ST_TickData
	{
		CTicker ticker;
		int		nFlag; // when nFlag ==2 , to do unRegis;
	};

	CTicker							m_ticker;
	int64_t							m_nLastTime;
	std::vector<ST_TickData>		m_vecTickData;
	CTestService2MessageHandler*	m_pTestService2MessageHandler;
	CNormalProtobufSerializer*		m_pNormalProtobufSerializer;
};