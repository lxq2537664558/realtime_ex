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
			CCoreApp::Inst()->getActorScheduler()->destroyActorBase(this->m_pActorBaseImpl);
	}

	uint64_t CActorBase::getID() const
	{
		return this->m_pActorBaseImpl->getID();
	}

	bool CActorBase::send(uint64_t nID, const google::protobuf::Message* pMessage)
	{
		return CCoreApp::Inst()->getActorScheduler()->invoke(0, this->getID(), nID, pMessage);
	}

	void CActorBase::response(const SActorSessionInfo& sActorSessionInfo, const google::protobuf::Message* pMessage)
	{
		bool bRet = CCoreApp::Inst()->getActorScheduler()->response(sActorSessionInfo.nSessionID, eRRT_OK, sActorSessionInfo.nActorID, pMessage);
	}

	CActorBase* CActorBase::createActor(const std::string& szClassName, void* pContext)
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

	void CActorBase::release()
	{
		this->onDestroy();

		PrintInfo("destroy actor id: "UINT64FMT, this->getID());

		this->del();
	}

	void CActorBase::registerMessageHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SActorSessionInfo, const google::protobuf::Message*)>& handler)
	{
		CActorBaseImpl::registerActorMessageHandler(szMessageName, handler);
	}

	void CActorBase::registerForwardHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)>& handler)
	{
		CActorBaseImpl::registerForwardMessageHandler(szMessageName, handler);
	}

	bool CActorBase::invoke(uint64_t nID, const google::protobuf::Message* pMessage, uint64_t nCoroutineID, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		uint64_t nSessionID = CCoreApp::Inst()->getTransporter()->genSessionID();

		if (!CCoreApp::Inst()->getActorScheduler()->invoke(nSessionID, this->getID(), nID, pMessage))
			return false;

		SResponseWaitInfo* pResponseWaitInfo = this->m_pActorBaseImpl->addResponseWaitInfo(nSessionID, nCoroutineID, nID, pMessage->GetTypeName(), callback);
		DebugAstEx(nullptr != pResponseWaitInfo, false);

		return true;
	}
}