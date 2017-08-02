#pragma once
#include "libCoreCommon\service_base.h"
#include "libCoreCommon\ticker.h"
#include "libCoreCommon\default_protobuf_factory.h"

using namespace std;
using namespace core;
using namespace base;

class CWatchdogServiceMessageHandler;
class CPlayerMessageHandler;
class CWatchdogService :
	public CServiceBase
{
	DECLARE_OBJECT(CWatchdogService)

public:
	CWatchdogService();
	virtual ~CWatchdogService();

private:
	virtual bool			onInit();
	virtual void			onFrame();
	virtual void			onQuit();

private:
	CWatchdogServiceMessageHandler*	m_pWatchdogServiceMessageHandler;
	CPlayerMessageHandler*			m_pPlayerMessageHandler;
	CDefaultProtobufFactory*		m_pDefaultProtobufFactory;
};