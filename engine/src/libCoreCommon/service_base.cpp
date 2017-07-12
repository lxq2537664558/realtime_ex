#include "stdafx.h"
#include "service_base.h"
#include "core_app.h"
#include "service_base_impl.h"
#include "actor_base.h"

namespace core
{
	CServiceBase::CServiceBase()
		: m_pServiceBaseImpl(nullptr)
	{

	}

	const SServiceBaseInfo& CServiceBase::getServiceBaseInfo() const
	{
		return this->m_pServiceBaseImpl->getServiceBaseInfo();
	}

	CServiceBase::~CServiceBase()
	{

	}

	void CServiceBase::registerServiceMessageHandler(const std::string& szMessageName, const std::function<void(SSessionInfo, google::protobuf::Message*)>& callback)
	{
		this->m_pServiceBaseImpl->registerServiceMessageHandler(szMessageName, callback);
	}

	void CServiceBase::registerServiceForwardHandler(const std::string& szMessageName, const std::function<void(SClientSessionInfo, google::protobuf::Message*)>& callback)
	{
		this->m_pServiceBaseImpl->registerServiceForwardHandler(szMessageName, callback);
	}

	void CServiceBase::registerActorMessageHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SSessionInfo, const google::protobuf::Message*)>& callback)
	{
		this->m_pServiceBaseImpl->registerActorMessageHandler(szMessageName, callback);
	}

	void CServiceBase::registerActorForwardHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)>& callback)
	{
		this->m_pServiceBaseImpl->registerActorForwardHandler(szMessageName, callback);
	}

	void CServiceBase::registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext)
	{
		this->m_pServiceBaseImpl->registerTicker(pTicker, nStartTime, nIntervalTime, nContext);
	}

	void CServiceBase::unregisterTicker(CTicker* pTicker)
	{
		this->m_pServiceBaseImpl->unregisterTicker(pTicker);
	}

	void CServiceBase::doQuit()
	{
		this->m_pServiceBaseImpl->doQuit();
	}

	CServiceInvoker* CServiceBase::getServiceInvoker() const
	{
		return this->m_pServiceBaseImpl->getServiceInvoker();
	}

	CActorBase* CServiceBase::createActor(const std::string& szClassName, void* pContext)
	{
		CActorBase* pActorBase = dynamic_cast<CActorBase*>(CBaseObject::createObject(szClassName));
		DebugAstEx(pActorBase != nullptr, nullptr);

		pActorBase->m_pActorBaseImpl = CCoreApp::Inst()->getActorScheduler()->createActorBase(pActorBase);
		if (pActorBase->m_pActorBaseImpl == nullptr)
		{
			pActorBase->del();
			return nullptr;
		}

		if (!pActorBase->onInit(pContext))
		{
			pActorBase->del();
			return nullptr;
		}

		PrintInfo("create actor id: "UINT64FMT, pActorBase->getID());

		return pActorBase;
	}
}