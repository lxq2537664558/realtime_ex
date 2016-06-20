#include "stdafx.h"
#include "core_service_kit.h"
#include "core_service_kit_impl.h"

namespace core
{
	CCoreServiceKit::CCoreServiceKit()
	{

	}

	CCoreServiceKit::~CCoreServiceKit()
	{

	}

	void CCoreServiceKit::release()
	{
		CCoreServiceKitImpl::Inst()->release();
	}

	bool CCoreServiceKit::init()
	{
		return CCoreServiceKitImpl::Inst()->init();
	}

	const SServiceBaseInfo& CCoreServiceKit::getServiceBaseInfo() const
	{
		return CCoreServiceKitImpl::Inst()->getServiceBaseInfo();
	}

	void CCoreServiceKit::startNewTrace()
	{
		CCoreServiceKitImpl::Inst()->getInvokerTrace()->startNewTrace();
	}

	void CCoreServiceKit::addTraceExtraInfo(const char* szFormat, ...)
	{
		va_list arg;
		va_start(arg, szFormat);
		CCoreServiceKitImpl::Inst()->getInvokerTrace()->addTraceExtraInfo(szFormat, arg);
		va_end(arg);
	}

	base::CLuaFacade* CCoreServiceKit::getLuaFacade() const
	{
		return CCoreServiceKitImpl::Inst()->getLuaFacade();
	}
}