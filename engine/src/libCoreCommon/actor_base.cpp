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

	uint64_t CActorBase::getID() const
	{
		return this->m_pCoreActor->getID();
	}

	CServiceBase* CActorBase::getServiceBase() const
	{
		return this->m_pCoreActor->getCoreService()->getServiceBase();
	}
}