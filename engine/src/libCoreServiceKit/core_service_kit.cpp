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

	const SMessageProxyGroupInfo* CCoreServiceKit::getMessageProxyGroupInfo(const std::string& szMessageName) const
	{
		return CCoreServiceKitImpl::Inst()->getCoreServiceProxy()->getMessageProxyGroupInfo(szMessageName);
	}

	const std::string& CCoreServiceKit::getMessageName(uint32_t nMessageID) const
	{
		return CCoreServiceKitImpl::Inst()->getCoreServiceProxy()->getMessageName(nMessageID);
	}

	void CCoreServiceKit::startNewTrace()
	{

	}

	void CCoreServiceKit::addTraceExtraInfo(const char* szFormat, ...)
	{

	}

}