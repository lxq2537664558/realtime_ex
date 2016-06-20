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

	void CMessageRegistry::registerServiceCallback(uint16_t nMessageID, ServiceCallback callback)
	{
		CCoreServiceKitImpl::Inst()->getCoreServiceInvoker()->registerCallback(nMessageID, callback);
	}

	void CMessageRegistry::registerGateForwardCallback(uint16_t nMessageID, GateForwardCallback callback)
	{
		CCoreServiceKitImpl::Inst()->getCoreServiceInvoker()->registerCallback(nMessageID, callback);
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