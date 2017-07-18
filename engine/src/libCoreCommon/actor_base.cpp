#include "stdafx.h"
#include "actor_base.h"
#include "actor_base_impl.h"
#include "service_invoker.h"
#include "coroutine.h"
#include "core_app.h"
#include "core_common_define.h"

#include "libBaseCommon/base_time.h"

namespace core
{
	CActorBase::CActorBase()
		: m_pActorBaseImpl(nullptr)
	{
	}

	CActorBase::~CActorBase()
	{
		if (this->m_pActorBaseImpl != nullptr)
			this->m_pActorBaseImpl->getServiceBaseImpl()->getActorScheduler()->destroyActorBase(this->m_pActorBaseImpl);
	}

	uint64_t CActorBase::getID() const
	{
		return this->m_pActorBaseImpl->getID();
	}

	void CActorBase::registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext)
	{
		this->m_pActorBaseImpl->registerTicker(pTicker, nStartTime, nIntervalTime, nContext);
	}

	void CActorBase::unregisterTicker(CTicker* pTicker)
	{
		this->m_pActorBaseImpl->unregisterTicker(pTicker);
	}

	bool CActorBase::send(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		return CCoreApp::Inst()->getTransporter()->invoke_a(this->m_pActorBaseImpl->getServiceBaseImpl(), eType, 0, this->getID(), nID, pMessage);
	}

	bool CActorBase::broadcast(const std::string& szServiceType, const google::protobuf::Message* pMessage)
	{
		return this->m_pActorBaseImpl->getServiceBaseImpl()->getServiceInvoker()->broadcast(szServiceType, pMessage);
	}

	bool CActorBase::send(const SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage)
	{
		return CServiceInvoker::send(sClientSessionInfo, pMessage);
	}

	bool CActorBase::broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const google::protobuf::Message* pMessage)
	{
		return CServiceInvoker::broadcast(vecClientSessionInfo, pMessage);
	}

	void CActorBase::response(const SSessionInfo& sSessionInfo, const google::protobuf::Message* pMessage)
	{
		this->m_pActorBaseImpl->getServiceBaseImpl()->getServiceInvoker()->response(sSessionInfo, pMessage);
	}

	void CActorBase::release()
	{
		this->onDestroy();

		PrintInfo("destroy actor id: "UINT64FMT, this->getID());

		this->del();
	}

	bool CActorBase::invoke(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, uint64_t nCoroutineID, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		uint64_t nSessionID = CCoreApp::Inst()->getTransporter()->genSessionID();

		if (!CCoreApp::Inst()->getTransporter()->invoke_a(this->m_pActorBaseImpl->getServiceBaseImpl(), eType, nSessionID, this->getID(), nID, pMessage))
			return false;

		SPendingResponseInfo* pPendingResponseInfo = this->m_pActorBaseImpl->addPendingResponseInfo(nSessionID, nCoroutineID, nID, pMessage->GetTypeName(), callback);
		DebugAstEx(nullptr != pPendingResponseInfo, false);
		
		return true;
	}
}