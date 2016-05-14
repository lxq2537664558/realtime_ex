#include "stdafx.h"
#include "message_registry.h"
#include "core_service_kit_impl.h"

namespace core
{
	CMessageRegistry::CMessageRegistry()
	{

	}

	CMessageRegistry::~CMessageRegistry()
	{

	}

	bool CMessageRegistry::init()
	{
		return true;
	}

	void CMessageRegistry::registerServiceCallback(const std::string& szMessageName, ServiceCallback callback)
	{
		CCoreServiceKitImpl::Inst()->getCoreServiceInvoker()->registerCallback(szMessageName, callback);
	}

	void CMessageRegistry::registerGateForwardCallback(const std::string& szMessageName, GateForwardCallback callback)
	{
		CCoreServiceKitImpl::Inst()->getCoreServiceInvoker()->registerCallback(szMessageName, callback);
	}

	void CMessageRegistry::addGlobalBeforeFilter(ServiceGlobalFilter callback)
	{
		CCoreServiceKitImpl::Inst()->addGlobalBeforeFilter(callback);
	}

	void CMessageRegistry::addGlobalAfterFilter(ServiceGlobalFilter callback)
	{
		CCoreServiceKitImpl::Inst()->addGlobalAfterFilter(callback);
	}
}