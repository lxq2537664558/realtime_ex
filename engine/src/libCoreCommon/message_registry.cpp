#include "stdafx.h"
#include "message_registry.h"
#include "core_app.h"

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
		CCoreApp::Inst()->getMessageDirectory()->registerCallback(szMessageName, callback);
	}

	void CMessageRegistry::registerGateForwardCallback(const std::string& szMessageName, GateForwardCallback callback)
	{
		CCoreApp::Inst()->getMessageDirectory()->registerCallback(szMessageName, callback);
	}

	void CMessageRegistry::addGlobalBeforeFilter(ServiceGlobalFilter callback)
	{
		CCoreApp::Inst()->getMessageDirectory()->addGlobalBeforeFilter(callback);
	}

	void CMessageRegistry::addGlobalAfterFilter(ServiceGlobalFilter callback)
	{
		CCoreApp::Inst()->getMessageDirectory()->addGlobalAfterFilter(callback);
	}
}