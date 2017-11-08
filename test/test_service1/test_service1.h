#pragma once
#include "libBaseCommon/ticker.h"

#include "libCoreCommon/service_base.h"
#include "libCoreCommon/normal_protobuf_serializer.h"

#include <map>

using namespace std;
using namespace core;
using namespace base;

class CTestService1 :
	public CServiceBase
{
public:
	CTestService1(const core::SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName);
	virtual ~CTestService1();

private:
	virtual bool	onInit();
	virtual void	onFrame();
	virtual void	onQuit();

	virtual void	release();

	void			onServiceConnect(const std::string& szType, uint32_t nServiceID);
	void			onServiceDisconnect(const std::string& szType, uint32_t nServiceID);

	void			onTicker0(uint64_t nContext);
	void			onTicker1(uint64_t nContext);
	void			onTicker2(uint64_t nContext);
	void			onTicker3(uint64_t nContext);

private:
	std::vector<CTicker>		m_ticker0;
	CTicker						m_ticker1;
	CTicker						m_ticker2;
	CTicker						m_ticker3;
	std::map<uint32_t, bool>	m_mapConnectFlag;
	CNormalProtobufSerializer*	m_pNormalProtobufSerializer;
};