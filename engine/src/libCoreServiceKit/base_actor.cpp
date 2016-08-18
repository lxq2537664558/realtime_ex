#include "stdafx.h"
#include "base_actor.h"
#include "base_actor_impl.h"
#include "core_service_app_impl.h"
#include "cluster_invoker.h"
#include "base_actor_factory.h"

#include "libCoreCommon/coroutine.h"

#define _REMOTE_BIT 48

namespace core
{
	CBaseActor::CBaseActor()
		: m_pBaseActorImpl(nullptr)
	{
	}

	CBaseActor::~CBaseActor()
	{
		if (this->m_pBaseActorImpl != nullptr)
			CCoreServiceAppImpl::Inst()->getScheduler()->destroyBaseActor(this->m_pBaseActorImpl);
	}

	uint64_t CBaseActor::getID() const
	{
		return this->m_pBaseActorImpl->getID();
	}

	bool CBaseActor::invoke(uint64_t nID, const message_header* pData)
	{
		SRequestMessageInfo sRequestMessageInfo;
		sRequestMessageInfo.pData = const_cast<message_header*>(pData);
		sRequestMessageInfo.nSessionID = 0;
		sRequestMessageInfo.nFromActorID = this->getID();
		sRequestMessageInfo.nToActorID = nID;

		return CCoreServiceAppImpl::Inst()->getScheduler()->invoke(sRequestMessageInfo);
	}

// 	uint32_t CBaseActor::invoke(uint64_t nID, const message_header* pData, CMessage& pResultData)
// 	{
// 		SRequestMessageInfo sRequestMessageInfo;
// 		sRequestMessageInfo.pData = const_cast<message_header*>(pData);
// 		sRequestMessageInfo.nSessionID = CCoreServiceAppImpl::Inst()->getTransporter()->genSessionID();
// 		sRequestMessageInfo.nCoroutineID = coroutine::getCurrentID();
// 		sRequestMessageInfo.nFromActorID = this->getID();
// 		sRequestMessageInfo.nToActorID = nID;
// 
// 		if (!CCoreServiceAppImpl::Inst()->getScheduler()->invoke(sRequestMessageInfo))
// 			return eRRT_ERROR;
// 
// 		this->m_pBaseActorImpl->addResponseWaitInfo(sRequestMessageInfo.nSessionID, 0, coroutine::getCurrentID());
// 
// 		coroutine::yield();
// 
// 		CMessage* pMessage = reinterpret_cast<CMessage*>(coroutine::recvMessage(coroutine::getCurrentID()));
// 		uint32_t nRet = (uint32_t)reinterpret_cast<uint64_t>(coroutine::recvMessage(coroutine::getCurrentID()));
// 
// 		pResultData = *pMessage;
// 		SAFE_DELETE(pMessage);
// 
// 		return nRet;
// 	}

	SActorSessionInfo CBaseActor::getActorSessionInfo() const
	{
		return this->m_pBaseActorImpl->getActorSessionInfo();
	}

	void CBaseActor::response(const message_header* pData)
	{
		this->response(this->getActorSessionInfo(), pData);
	}

	void CBaseActor::response(const SActorSessionInfo& sActorSessionInfo, const message_header* pData)
	{
		SResponseMessageInfo sResponseMessageInfo;
		sResponseMessageInfo.nSessionID = sActorSessionInfo.nSessionID;
		sResponseMessageInfo.pData = const_cast<message_header*>(pData);
		sResponseMessageInfo.nResult = eRRT_OK;
		sResponseMessageInfo.nFromActorID = this->getID();
		sResponseMessageInfo.nToActorID = sActorSessionInfo.nActorID;

		bool bRet = CCoreServiceAppImpl::Inst()->getScheduler()->response(sResponseMessageInfo);
	}

	uint16_t CBaseActor::getServiceID(uint64_t nActorID)
	{
		return (uint16_t)(nActorID >> _REMOTE_ACTOR_BIT);
	}

	uint64_t CBaseActor::getLocalActorID(uint64_t nActorID)
	{
		return nActorID & 0x0000ffffffffffff;
	}

	uint64_t CBaseActor::makeRemoteActorID(uint16_t nServiceID, uint64_t nActorID)
	{
		return (uint64_t)nServiceID << _REMOTE_ACTOR_BIT | nActorID;
	}

	CBaseActor* CBaseActor::createActor(void* pContext, CBaseActorFactory* pBaseActorFactory)
	{
		DebugAstEx(pBaseActorFactory != nullptr, nullptr);

		CBaseActor* pBaseActor = pBaseActorFactory->createBaseActor();
		pBaseActor->m_pBaseActorImpl = CCoreServiceAppImpl::Inst()->getScheduler()->createBaseActor(pBaseActor);
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

	void CBaseActor::release()
	{
		this->onDestroy();

		PrintInfo("destroy actor id: "UINT64FMT, this->getID());

		delete this;
	}

	bool CBaseActor::invokeImpl(uint64_t nID, const message_header* pData, uint64_t nCoroutineID, const std::function<void(std::shared_ptr<message_header>, uint32_t)>& callback)
	{
		SRequestMessageInfo sRequestMessageInfo;
		sRequestMessageInfo.pData = const_cast<message_header*>(pData);
		sRequestMessageInfo.nSessionID = CCoreServiceAppImpl::Inst()->getTransporter()->genSessionID();
		sRequestMessageInfo.nFromActorID = this->getID();
		sRequestMessageInfo.nToActorID = nID;

		if (!CCoreServiceAppImpl::Inst()->getScheduler()->invoke(sRequestMessageInfo))
			return false;

		SResponseWaitInfo* pResponseWaitInfo = this->m_pBaseActorImpl->addResponseWaitInfo(sRequestMessageInfo.nSessionID, 0, nCoroutineID);
		DebugAstEx(nullptr != pResponseWaitInfo, false);
	
		pResponseWaitInfo->callback = callback;

		return true;
	}

}