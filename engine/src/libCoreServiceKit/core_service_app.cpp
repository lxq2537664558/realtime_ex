#include "stdafx.h"
#include "core_service_app.h"
#include "core_service_app_impl.h"

namespace core
{
	CCoreServiceApp::CCoreServiceApp()
	{

	}

	CCoreServiceApp::~CCoreServiceApp()
	{

	}

	CCoreServiceApp* CCoreServiceApp::Inst()
	{
		return static_cast<CCoreServiceApp*>(CBaseApp::Inst());
	}

	const SServiceBaseInfo& CCoreServiceApp::getServiceBaseInfo() const
	{
		return CCoreServiceAppImpl::Inst()->getServiceBaseInfo();
	}

	void CCoreServiceApp::startNewTrace()
	{
		CCoreServiceAppImpl::Inst()->getInvokerTrace()->startNewTrace();
	}

	void CCoreServiceApp::addTraceExtraInfo(const char* szFormat, ...)
	{
		va_list arg;
		va_start(arg, szFormat);
		CCoreServiceAppImpl::Inst()->getInvokerTrace()->addTraceExtraInfo(szFormat, arg);
		va_end(arg);
	}

	base::CLuaFacade* CCoreServiceApp::getLuaFacade() const
	{
		return CCoreServiceAppImpl::Inst()->getLuaFacade();
	}

	void CCoreServiceApp::setServiceConnectCallback(std::function<void(uint16_t)> funConnect)
	{
		CCoreServiceAppImpl::Inst()->setServiceConnectCallback(funConnect);
	}

	void CCoreServiceApp::setServiceDisconnectCallback(std::function<void(uint16_t)> funDisconnect)
	{
		CCoreServiceAppImpl::Inst()->setServiceDisconnectCallback(funDisconnect);
	}

	void CCoreServiceApp::registerServiceCallback(uint16_t nMessageID, ServiceCallback callback)
	{
		CCoreServiceAppImpl::Inst()->getCoreServiceInvoker()->registerCallback(nMessageID, callback);
	}

	void CCoreServiceApp::registerGateForwardCallback(uint16_t nMessageID, GateForwardCallback callback)
	{
		CCoreServiceAppImpl::Inst()->getCoreServiceInvoker()->registerCallback(nMessageID, callback);
	}

	void CCoreServiceApp::addGlobalBeforeFilter(ServiceGlobalFilter callback)
	{
		CCoreServiceAppImpl::Inst()->addGlobalBeforeFilter(callback);
	}

	void CCoreServiceApp::addGlobalAfterFilter(ServiceGlobalFilter callback)
	{
		CCoreServiceAppImpl::Inst()->addGlobalAfterFilter(callback);
	}

	bool CCoreServiceApp::onInit()
	{
		return CCoreServiceAppImpl::Inst()->init();
	}

	void CCoreServiceApp::onDestroy()
	{
		CCoreServiceAppImpl::Inst()->release();
	}

	void CCoreServiceApp::onProcess()
	{
		CCoreServiceAppImpl::Inst()->run();
	}

}