#include "stdafx.h"
#include "actor_base.h"
#include "core_actor.h"
#include "coroutine.h"
#include "core_app.h"
#include "core_common_define.h"

namespace core
{
	CActorBase::CActorBase()
		: m_pCoreActor(nullptr)
	{
	}

	CActorBase::~CActorBase()
	{
	}

	uint64_t CActorBase::getActorID() const
	{
		return this->m_pCoreActor->getID();
	}

	CServiceBase* CActorBase::getServiceBase() const
	{
		return this->m_pCoreActor->getCoreService()->getServiceBase();
	}

	void CActorBase::registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext)
	{
		CServiceBase* pServiceBase = this->getServiceBase();
		DebugAst(pServiceBase != nullptr);

		CCoreApp::Inst()->registerTicker(CTicker::eTT_Actor, pServiceBase->getServiceID(), this->getActorID(), pTicker, nStartTime, nIntervalTime, nContext);
	}

	void CActorBase::unregisterTicker(CTicker* pTicker)
	{
		CCoreApp::Inst()->unregisterTicker(pTicker);
	}

}