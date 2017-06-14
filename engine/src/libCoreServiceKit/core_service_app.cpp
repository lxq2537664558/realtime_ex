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

	const SServiceBaseInfo& CCoreServiceApp::getNodeBaseInfo() const
	{
		return CCoreServiceAppImpl::Inst()->getNodeBaseInfo();
	}

	uint16_t CCoreServiceApp::getServiceID(const std::string& szName) const
	{
		return CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getServiceID(szName);
	}

	base::CLuaFacade* CCoreServiceApp::getLuaFacade() const
	{
		return CCoreServiceAppImpl::Inst()->getLuaFacade();
	}

	void CCoreServiceApp::setNodeConnectCallback(const std::function<void(uint16_t)>& callback)
	{
		CCoreServiceAppImpl::Inst()->setNodeConnectCallback(callback);
	}

	void CCoreServiceApp::setNodeDisconnectCallback(const std::function<void(uint16_t)>& callback)
	{
		CCoreServiceAppImpl::Inst()->setNodeDisconnectCallback(callback);
	}

	void CCoreServiceApp::registerMessageHandler(uint16_t nMessageID, const std::function<bool(uint16_t, CMessagePtr<char>)>& callback)
	{
		CCoreServiceAppImpl::Inst()->getCoreMessageRegistry()->registerCallback(nMessageID, callback);
	}

	void CCoreServiceApp::registerForwardHandler(uint16_t nMessageID, const std::function<bool(SClientSessionInfo, CMessagePtr<char>)>& callback)
	{
		CCoreServiceAppImpl::Inst()->getCoreMessageRegistry()->registerGateForwardCallback(nMessageID, callback);
	}

	void CCoreServiceApp::addGlobalBeforeFilter(GlobalBeforeFilter callback)
	{
		CCoreServiceAppImpl::Inst()->addGlobalBeforeFilter(callback);
	}

	void CCoreServiceApp::addGlobalAfterFilter(GlobalAfterFilter callback)
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

	void CCoreServiceApp::setSerializeAdapter(uint16_t nNodeID, CSerializeAdapter* pSerializeAdapter)
	{
		CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->setSerializeAdapter(nNodeID, pSerializeAdapter);
	}
}