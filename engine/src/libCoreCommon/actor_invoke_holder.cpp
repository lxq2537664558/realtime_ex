#include "stdafx.h"
#include "actor_invoke_holder.h"
#include "core_app.h"

namespace core
{
	CActorInvokeHolder::CActorInvokeHolder(CActorBase* pActorBase)
		: m_pActorBase(pActorBase)
	{
		static uint64_t s_nNextHolderID = 1;

		this->m_nID = s_nNextHolderID++;
	}

	CActorInvokeHolder::~CActorInvokeHolder()
	{
		if (this->m_pActorBase != nullptr)
			this->m_pActorBase->m_pCoreActor->delPendingResponseInfo(this->m_nID);
	}

	uint64_t CActorInvokeHolder::getHolderID() const
	{
		return this->m_nID;
	}

	CActorBase* CActorInvokeHolder::getActorBase() const
	{
		return this->m_pActorBase;
	}
}