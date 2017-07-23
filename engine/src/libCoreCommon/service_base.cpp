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

	uint32_t CServiceBase::getServiceID() const
	{
		return this->m_pServiceBaseImpl->getServiceID();
	}

	const SServiceBaseInfo& CServiceBase::getServiceBaseInfo() const
	{
		return this->m_pServiceBaseImpl->getServiceBaseInfo();
	}

	CServiceBase::~CServiceBase()
	{

	}

	void CServiceBase::registerServiceMessageHandler(const std::string& szMessageName, const std::function<void(SSessionInfo, const google::protobuf::Message*)>& callback)
	{
		this->m_pServiceBaseImpl->registerServiceMessageHandler(szMessageName, callback);
	}

	void CServiceBase::registerServiceForwardHandler(const std::string& szMessageName, const std::function<void(SClientSessionInfo, const google::protobuf::Message*)>& callback)
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

	void CServiceBase::setServiceConnectCallback(const std::function<void(uint32_t)>& callback)
	{
		this->m_pServiceBaseImpl->setServiceConnectCallback(callback);
	}

	void CServiceBase::setServiceDisconnectCallback(const std::function<void(uint32_t)>& callback)
	{
		this->m_pServiceBaseImpl->setServiceDisconnectCallback(callback);
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

	CActorBase* CServiceBase::createActor(const std::string& szClassName, uint64_t nActorID, const std::string& szContext)
	{
		DebugAstEx(nActorID != 0, nullptr);

		if (this->m_pServiceBaseImpl->getActorScheduler()->getActorBase(nActorID) != nullptr)
		{
			PrintWarning("CServiceBase::createActor error actor id exist actor_id: "UINT64FMT" class_name: %s", nActorID, szClassName.c_str());
			return nullptr;
		}

		CActorBase* pActorBase = dynamic_cast<CActorBase*>(CBaseObject::createObject(szClassName));
		DebugAstEx(pActorBase != nullptr, nullptr);

		pActorBase->m_pActorBaseImpl = this->m_pServiceBaseImpl->getActorScheduler()->createActorBase(nActorID, pActorBase);
		if (pActorBase->m_pActorBaseImpl == nullptr)
		{
			SAFE_RELEASE(pActorBase);
			return nullptr;
		}

		uint64_t nCoroutineID = coroutine::create(0, [pActorBase, szContext](uint64_t){ pActorBase->onInit(szContext); });
		coroutine::resume(nCoroutineID, 0);
		
		PrintInfo("create actor id: "UINT64FMT, pActorBase->getID());

		return pActorBase;
	}

	void CServiceBase::release()
	{
		CBaseObject::destroyObject(this);
	}

	void CServiceBase::setActorIDConverter(CActorIDConverter* pActorIDConverter)
	{
		this->m_pServiceBaseImpl->setActorIDConverter(pActorIDConverter);
	}

	void CServiceBase::setServiceIDConverter(CServiceIDConverter* pServiceIDConverter)
	{
		this->m_pServiceBaseImpl->setServiceIDConverter(pServiceIDConverter);
	}
}