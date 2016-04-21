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

	void CMessageRegistry::registerCallback(uint32_t nMessageID, ServiceCallback callback)
	{
		CCoreApp::Inst()->getMessageDirectory()->registerCallback(nMessageID, callback);
	}

	void CMessageRegistry::registerCallback(uint32_t nMessageID, GateClientCallback callback)
	{
		CCoreApp::Inst()->getMessageDirectory()->registerCallback(nMessageID, callback);
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