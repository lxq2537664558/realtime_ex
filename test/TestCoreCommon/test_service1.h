#pragma once
#include "libCoreCommon\service_base.h"
#include "libCoreCommon\ticker.h"

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

	void					onTicker(uint64_t nContext);

private:
	CTicker	m_ticker;
};