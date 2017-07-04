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
		: m_pActorBaseImpl(nullptr)
	{
	}

	CActorBase::~CActorBase()
	{
		if (this->m_pActorBaseImpl != nullptr)
			CCoreServiceAppImpl::Inst()->getScheduler()->destroyActorBase(this->m_pActorBaseImpl);
	}

	uint64_t CActorBase::getID() const
	{
		return this->m_pActorBaseImpl->getID();
	}

	bool CActorBase::send(uint64_t nID, const google::protobuf::Message* pMessage)
	{
		SRequestMessageInfo sRequestMessageInfo;
		sRequestMessageInfo.pMessage = pMessage;
		sRequestMessageInfo.nSessionID = 0;
		sRequestMessageInfo.nFromActorID = this->getID();
		sRequestMessageInfo.nToActorID = nID;

		return CCoreServiceAppImpl::Inst()->getScheduler()->invoke(sRequestMessageInfo);
	}

	SActorSessionInfo CActorBase::getActorSessionInfo() const
	{
		return this->m_pActorBaseImpl->getActorSessionInfo();
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

	CActorBase* CActorBase::createActorBase(void* pContext, CActorBaseFactory* pBaseActorFactory)
	{
		DebugAstEx(pBaseActorFactory != nullptr, nullptr);

		CActorBase* pBaseActor = pBaseActorFactory->createActorBase();
		pBaseActor->m_pActorBaseImpl = CCoreServiceAppImpl::Inst()->getScheduler()->createActorBase(pBaseActor);
		if (pBaseActor->m_pActorBaseImpl == nullptr)
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

	void CActorBase::registerMessageHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SActorSessionInfo, const google::protobuf::Message*)>& handler)
	{
		CActorBaseImpl::registerMessageHandler(szMessageName, handler);
	}

	void CActorBase::registerForwardHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)>& handler)
	{
		CActorBaseImpl::registerForwardHandler(szMessageName, handler);
	}

	bool CActorBase::invokeImpl(uint64_t nID, const google::protobuf::Message* pMessage, uint64_t nCoroutineID, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		SRequestMessageInfo sRequestMessageInfo;
		sRequestMessageInfo.pMessage = pMessage;
		sRequestMessageInfo.nSessionID = CCoreServiceAppImpl::Inst()->getTransporter()->genSessionID();
		sRequestMessageInfo.nFromActorID = this->getID();
		sRequestMessageInfo.nToActorID = nID;

		if (!CCoreServiceAppImpl::Inst()->getScheduler()->invoke(sRequestMessageInfo))
			return false;

		SResponseWaitInfo* pResponseWaitInfo = this->m_pActorBaseImpl->addResponseWaitInfo(sRequestMessageInfo.nSessionID, nCoroutineID);
		DebugAstEx(nullptr != pResponseWaitInfo, false);
	
		pResponseWaitInfo->callback = callback;
		pResponseWaitInfo->nToID = nID;
		pResponseWaitInfo->nBeginTime = base::getGmtTime();

		return true;
	}

}