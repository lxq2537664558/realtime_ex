#include "stdafx.h"
#include "actor_base.h"
#include "core_actor.h"
#include "coroutine.h"
#include "core_app.h"
#include "core_common_define.h"
#include "actor_invoke_holder.h"

#include "libBaseCommon/base_time.h"


namespace core
{
	CActorBase::CActorBase()
		: m_pCoreActor(nullptr)
	{
	}

	CActorBase::~CActorBase()
	{
	}

	uint64_t CActorBase::getID() const
	{
		return this->m_pCoreActor->getID();
	}

	CServiceBase* CActorBase::getServiceBase() const
	{
		return this->m_pCoreActor->getCoreService()->getServiceBase();
	}

	void CActorBase::registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext)
	{
		this->m_pCoreActor->registerTicker(pTicker, nStartTime, nIntervalTime, nContext);
	}

	void CActorBase::unregisterTicker(CTicker* pTicker)
	{
		this->m_pCoreActor->unregisterTicker(pTicker);
	}

	bool CActorBase::send(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		return CCoreApp::Inst()->getLogicRunnable()->getTransporter()->invoke_a(this->m_pCoreActor->getCoreService(), 0, this->getID(), eType, nID, pMessage);
	}

	bool CActorBase::send(const SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage)
	{
		return this->getServiceBase()->getServiceInvoker()->send(sClientSessionInfo, pMessage);
	}

	bool CActorBase::send(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nServiceSelectorContext, google::protobuf::Message* pMessage)
	{
		return this->getServiceBase()->getServiceInvoker()->send(szServiceType, szServiceSelectorType, nServiceSelectorContext, pMessage);
	}

	bool CActorBase::broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const google::protobuf::Message* pMessage)
	{
		return this->getServiceBase()->getServiceInvoker()->broadcast(vecClientSessionInfo, pMessage);
	}

	void CActorBase::response(const SSessionInfo& sSessionInfo, const google::protobuf::Message* pMessage)
	{
		this->getServiceBase()->getServiceInvoker()->response(sSessionInfo, pMessage);
	}

	bool CActorBase::invoke(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, uint64_t nCoroutineID, const std::function<void(std::shared_ptr<google::protobuf::Message>, uint32_t)>& callback, CActorInvokeHolder* pActorInvokeHolder)
	{
		uint64_t nSessionID = CCoreApp::Inst()->getLogicRunnable()->getTransporter()->genSessionID();

		if (!CCoreApp::Inst()->getLogicRunnable()->getTransporter()->invoke_a(this->m_pCoreActor->getCoreService(), nSessionID, this->getID(), eType, nID, pMessage))
			return false;

		SPendingResponseInfo* pPendingResponseInfo = this->m_pCoreActor->addPendingResponseInfo(nSessionID, nCoroutineID, nID, pMessage->GetTypeName(), callback, pActorInvokeHolder != nullptr ? pActorInvokeHolder->getHolderID() : 0);
		DebugAstEx(nullptr != pPendingResponseInfo, false);
		
		return true;
	}
}