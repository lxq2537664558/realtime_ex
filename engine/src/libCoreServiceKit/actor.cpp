#include "stdafx.h"
#include "actor.h"
#include "core_service_app_impl.h"
#include "cluster_invoker.h"

#include "libCoreCommon/coroutine.h"

#define _REMOTE_BIT 48

namespace core
{
	CActor::CActor()
	{
		this->m_pActorBase = CCoreServiceAppImpl::Inst()->getScheduler()->createActorBase(this);
	}

	CActor::~CActor()
	{
		CCoreServiceAppImpl::Inst()->getScheduler()->destroyActorBase(this->m_pActorBase);
	}

	uint64_t CActor::getID() const
	{
		return this->m_pActorBase->getID();
	}

	bool CActor::invoke(uint64_t nID, const message_header* pData)
	{
		SRequestMessageInfo sRequestMessageInfo;
		sRequestMessageInfo.pData = const_cast<message_header*>(pData);
		sRequestMessageInfo.nSessionID = 0;
		sRequestMessageInfo.nCoroutineID = 0;
		sRequestMessageInfo.nFromActorID = this->getID();
		sRequestMessageInfo.nToActorID = nID;

		return CCoreServiceAppImpl::Inst()->getScheduler()->invoke(sRequestMessageInfo);
	}

	uint32_t CActor::invoke(uint64_t nID, const message_header* pData, CMessage& pResultData)
	{
		SRequestMessageInfo sRequestMessageInfo;
		sRequestMessageInfo.pData = const_cast<message_header*>(pData);
		sRequestMessageInfo.nSessionID = CCoreServiceAppImpl::Inst()->getTransporter()->genSessionID();
		sRequestMessageInfo.nCoroutineID = coroutine::getCurrentID();
		sRequestMessageInfo.nFromActorID = this->getID();
		sRequestMessageInfo.nToActorID = nID;

		if (!CCoreServiceAppImpl::Inst()->getScheduler()->invoke(sRequestMessageInfo))
			return eRRT_ERROR;

		this->m_pActorBase->addResponseWaitInfo(sRequestMessageInfo.nSessionID, 0, coroutine::getCurrentID());

		coroutine::yield();

		SResponseWaitInfo* pResponseWaitInfo = reinterpret_cast<SResponseWaitInfo*>(coroutine::recvMessage(coroutine::getCurrentID()));
		DebugAstEx(pResponseWaitInfo != nullptr, eRRT_ERROR);
		uint32_t nRet = (uint32_t)reinterpret_cast<uint64_t>(coroutine::recvMessage(coroutine::getCurrentID()));

		pResultData = pResponseWaitInfo->pResponseMessage;
		return nRet;
	}

	bool CActor::invoke_r(uint64_t nID, const message_header* pData, CResponseFuture& sResponseFuture)
	{
		SRequestMessageInfo sRequestMessageInfo;
		sRequestMessageInfo.pData = const_cast<message_header*>(pData);
		sRequestMessageInfo.nSessionID = CCoreServiceAppImpl::Inst()->getTransporter()->genSessionID();
		sRequestMessageInfo.nCoroutineID = coroutine::getCurrentID();
		sRequestMessageInfo.nFromActorID = this->getID();
		sRequestMessageInfo.nToActorID = nID;

		if (!CCoreServiceAppImpl::Inst()->getScheduler()->invoke(sRequestMessageInfo))
			return false;

		this->m_pActorBase->addResponseWaitInfo(sRequestMessageInfo.nSessionID, 0, 0);

		sResponseFuture.m_nSessionID = sRequestMessageInfo.nSessionID;
		sResponseFuture.m_nActorID = this->getID();

		return true;
	}

	bool CActor::invoke_r(uint64_t nID, const message_header* pData, InvokeCallback& callback)
	{
		SRequestMessageInfo sRequestMessageInfo;
		sRequestMessageInfo.pData = const_cast<message_header*>(pData);
		sRequestMessageInfo.nSessionID = CCoreServiceAppImpl::Inst()->getTransporter()->genSessionID();
		sRequestMessageInfo.nCoroutineID = coroutine::getCurrentID();
		sRequestMessageInfo.nFromActorID = this->getID();
		sRequestMessageInfo.nToActorID = nID;

		if (!CCoreServiceAppImpl::Inst()->getScheduler()->invoke(sRequestMessageInfo))
			return false;

		this->m_pActorBase->addResponseWaitInfo(sRequestMessageInfo.nSessionID, 0, 0);

		SResponseWaitInfo* pResponseWaitInfo = this->m_pActorBase->getResponseWaitInfo(sRequestMessageInfo.nSessionID, false);
		DebugAstEx(nullptr != pResponseWaitInfo, false);
		
		pResponseWaitInfo->callback = [callback](SResponseWaitInfo*, uint8_t nMessageType, CMessage pMessage)->void
		{
			callback(nMessageType, pMessage);
		};

		return true;
	}

	SActorSessionInfo CActor::getActorSessionInfo() const
	{
		return this->m_pActorBase->getActorSessionInfo();
	}

	void CActor::response(const message_header* pData)
	{
		this->response(this->getActorSessionInfo(), pData);
	}

	void CActor::response(const SActorSessionInfo& sActorSessionInfo, const message_header* pData)
	{
		SResponseMessageInfo sResponseMessageInfo;
		sResponseMessageInfo.nSessionID = sActorSessionInfo.nSessionID;
		sResponseMessageInfo.pData = const_cast<message_header*>(pData);
		sResponseMessageInfo.nResult = eRRT_OK;
		sResponseMessageInfo.nFromActorID = this->getID();
		sResponseMessageInfo.nToActorID = sActorSessionInfo.nActorID;

		bool bRet = CCoreServiceAppImpl::Inst()->getScheduler()->response(sResponseMessageInfo);
	}

	uint16_t CActor::getServiceID(uint64_t nActorID)
	{
		return (uint16_t)(nActorID >> _REMOTE_ACTOR_BIT);
	}

	uint64_t CActor::getLocalActorID(uint64_t nActorID)
	{
		return nActorID & 0x0000ffffffffffff;
	}

	uint64_t CActor::getRemoteActorID(uint16_t nServiceID, uint64_t nActorID)
	{
		return (uint64_t)nServiceID << _REMOTE_ACTOR_BIT | nActorID;
	}
}