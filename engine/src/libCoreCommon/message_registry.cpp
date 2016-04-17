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

	void CMessageRegistry::registCallback(uint32_t nMessageID, ServiceCallback callback)
	{
		CCoreApp::Inst()->getMessageDirectory()->registCallback(nMessageID, callback);
	}

	void CMessageRegistry::registCallback(uint32_t nMessageID, GateClientCallback callback)
	{
		CCoreApp::Inst()->getMessageDirectory()->registCallback(nMessageID, callback);
	}

	void CMessageRegistry::registGlobalBeforeFilter(ServiceGlobalFilter callback)
	{
		CCoreApp::Inst()->getMessageDirectory()->registGlobalBeforeFilter(callback);
	}

	void CMessageRegistry::registGlobalAfterFilter(ServiceGlobalFilter callback)
	{
		CCoreApp::Inst()->getMessageDirectory()->registGlobalAfterFilter(callback);
	}
}