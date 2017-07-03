#include "stdafx.h"
#include "actor_base.h"
#include "actor_base_impl.h"
#include "core_service_app_impl.h"
#include "cluster_invoker.h"
#include "actor_base_factory.h"
#include "coroutine.h"
#include "core_service_define.h"

#include "libBaseCommon/base_time.h"

#define _REMOTE_BIT 48

namespace core
{
	CActorBase::CActorBase()
		: m_pBaseActorImpl(nullptr)
	{
	}

	CActorBase::~CActorBase()
	{
		if (this->m_pBaseActorImpl != nullptr)
			CCoreServiceAppImpl::Inst()->getScheduler()->destroyActorBase(this->m_pBaseActorImpl);
	}

	uint64_t CActorBase::getID() const
	{
		return this->m_pBaseActorImpl->getID();
	}

	bool CActorBase::invoke(uint64_t nID, const void* pData)
	{
		SRequestMessageInfo sRequestMessageInfo;
		sRequestMessageInfo.pData = pData;
		sRequestMessageInfo.nSessionID = 0;
		sRequestMessageInfo.nFromActorID = this->getID();
		sRequestMessageInfo.nToActorID = nID;

		return CCoreServiceAppImpl::Inst()->getScheduler()->invoke(sRequestMessageInfo);
	}

	SActorSessionInfo CActorBase::getActorSessionInfo() const
	{
		return this->m_pBaseActorImpl->getActorSessionInfo();
	}

	void CActorBase::response(const void* pData)
	{
		this->response(this->getActorSessionInfo(), pData);
	}

	void CActorBase::response(const SActorSessionInfo& sActorSessionInfo, const void* pData)
	{
		SResponseMessageInfo sResponseMessageInfo;
		sResponseMessageInfo.nSessionID = sActorSessionInfo.nSessionID;
		sResponseMessageInfo.pData = pData;
		sResponseMessageInfo.nResult = eRRT_OK;
		sResponseMessageInfo.nFromActorID = this->getID();
		sResponseMessageInfo.nToActorID = sActorSessionInfo.nActorID;

		bool bRet = CCoreServiceAppImpl::Inst()->getScheduler()->response(sResponseMessageInfo);
	}

	uint16_t CActorBase::getServiceID(uint64_t nActorID)
	{
		return (uint16_t)(nActorID >> _REMOTE_ACTOR_BIT);
	}

	uint64_t CActorBase::getLocalActorID(uint64_t nActorID)
	{
		return nActorID & 0x0000ffffffffffff;
	}

	uint64_t CActorBase::makeRemoteActorID(uint16_t nServiceID, uint64_t nActorID)
	{
		return (uint64_t)nServiceID << _REMOTE_ACTOR_BIT | nActorID;
	}

	CActorBase* CActorBase::createActor(void* pContext, CActorBaseFactory* pBaseActorFactory)
	{
		DebugAstEx(pBaseActorFactory != nullptr, nullptr);

		CActorBase* pBaseActor = pBaseActorFactory->createActorBase();
		pBaseActor->m_pBaseActorImpl = CCoreServiceAppImpl::Inst()->getScheduler()->createActorBase(pBaseActor);
		if (pBaseActor->m_pBaseActorImpl == nullptr)
		{
			SAFE_DELETE(pBaseActor);
			return nullptr;
		}

		if (!pBaseActor->onInit(pContext))
		{
			SAFE_DELETE(pBaseActor);
			return nullptr;
		}

		PrintInfo("create actor id: "UINT64FMT, pBaseActor->getID());

		return pBaseActor;
	}

	void CActorBase::release()
	{
		this->onDestroy();

		PrintInfo("destroy actor id: "UINT64FMT, this->getID());

		delete this;
	}

	void CActorBase::registerMessageHandler(uint16_t nMessageID, const std::function<void(CActorBase*, uint64_t, CMessagePtr<char>)>& handler, bool bAsync)
	{
		CActorBaseImpl::registerMessageHandler(nMessageID, handler, bAsync);
	}

	void CActorBase::registerForwardHandler(uint16_t nMessageID, const std::function<void(CActorBase*, SClientSessionInfo, CMessagePtr<char>)>& handler, bool bAsync)
	{
		CActorBaseImpl::registerForwardHandler(nMessageID, handler, bAsync);
	}

	bool CActorBase::invokeImpl(uint64_t nID, const void* pData, uint64_t nCoroutineID, const std::function<void(CMessagePtr<char>, uint32_t)>& callback)
	{
		SRequestMessageInfo sRequestMessageInfo;
		sRequestMessageInfo.pData = pData;
		sRequestMessageInfo.nSessionID = CCoreServiceAppImpl::Inst()->getTransporter()->genSessionID();
		sRequestMessageInfo.nFromActorID = this->getID();
		sRequestMessageInfo.nToActorID = nID;

		if (!CCoreServiceAppImpl::Inst()->getScheduler()->invoke(sRequestMessageInfo))
			return false;

		SResponseWaitInfo* pResponseWaitInfo = this->m_pBaseActorImpl->addResponseWaitInfo(sRequestMessageInfo.nSessionID, nCoroutineID);
		DebugAstEx(nullptr != pResponseWaitInfo, false);
	
		pResponseWaitInfo->callback = callback;
		pResponseWaitInfo->nToID = nID;
		//pResponseWaitInfo->nMessageID = pData->nMessageID;
		pResponseWaitInfo->nBeginTime = base::getGmtTime();

		return true;
	}

}