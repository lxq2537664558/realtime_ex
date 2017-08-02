#pragma once
#include "libCoreCommon\service_base.h"
#include "libCoreCommon\ticker.h"
#include "libCoreCommon\default_protobuf_factory.h"

#include <map>

using namespace std;
using namespace core;
using namespace base;

class CTestService1 :
	public CServiceBase
{
	DECLARE_OBJECT(CTestService1)

public:
	CTestService1();
	virtual ~CTestService1();

private:
	virtual bool			onInit();
	virtual void			onFrame();
	virtual void			onQuit();

	void					onServiceConnect(const std::string& szType, uint32_t nServiceID);
	void					onServiceDisconnect(const std::string& szType, uint32_t nServiceID);

	void					onTicker1(uint64_t nContext);
	void					onTicker2(uint64_t nContext);
	void					onTicker3(uint64_t nContext);

private:
	CTicker						m_ticker1;
	CTicker						m_ticker2;
	CTicker						m_ticker3;
	std::map<uint32_t, bool>	m_mapConnectFlag;
	CDefaultProtobufFactory*	m_pDefaultProtobufFactory;
};