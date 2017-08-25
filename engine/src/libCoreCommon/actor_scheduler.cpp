#include "stdafx.h"
#include "actor_scheduler.h"
#include "message_dispatcher.h"
#include "actor_base.h"
#include "coroutine.h"
#include "core_app.h"
#include "core_common_define.h"
#include "core_actor.h"
#include "core_service.h"

#include "libCoreCommon/base_app.h"
#include "libBaseCommon/time_util.h"

namespace core
{
	CActorScheduler::CActorScheduler(CCoreService* pCoreService)
		: m_pCoreService(pCoreService)
		, m_nCurWorkActorID(0)
	{
	}

	CActorScheduler::~CActorScheduler()
	{

	}

	bool CActorScheduler::init()
	{
		return true;
	}

	CCoreActor* CActorScheduler::getCoreActor(uint64_t nID) const
	{
		auto iter = this->m_mapCoreActor.find(nID);
		if (iter == this->m_mapCoreActor.end())
			return nullptr;

		return iter->second;
	}

	void CActorScheduler::dispatch()
	{
		int64_t nCurTime = base::time_util::getGmtTime();
		for (auto iter = this->m_mapPendingCoreActor.begin(); iter != this->m_mapPendingCoreActor.end();)
		{
			CCoreActor* pCoreActor = iter->second;
			if (pCoreActor == nullptr)
			{
				this->m_mapPendingCoreActor.erase(iter++);
				continue;
			}

			if (!pCoreActor->onPendingTimer(nCurTime))
			{
				++iter;
				continue;
			}

			this->m_mapPendingCoreActor.erase(iter++);
		}

		std::map<uint64_t, CCoreActor*> mapWorkCoreActor = std::move(this->m_mapWorkCoreActor);
		for (auto iter = mapWorkCoreActor.begin(); iter != mapWorkCoreActor.end(); ++iter)
		{
			CCoreActor* pCoreActor = iter->second;
			if (pCoreActor == nullptr)
				continue;
			
			this->m_nCurWorkActorID = pCoreActor->getID();

			pCoreActor->process();
		}
		this->m_nCurWorkActorID = 0;
	}

	CCoreActor* CActorScheduler::createCoreActor(uint64_t nActorID, CActorBase* pActorBase)
	{
		DebugAstEx(pActorBase != nullptr, nullptr);
		DebugAstEx(nActorID != 0, nullptr);
		DebugAstEx(this->m_mapCoreActor.find(nActorID) == this->m_mapCoreActor.end(), nullptr);

		CCoreActor* pCoreActor = new CCoreActor(nActorID, pActorBase, this->m_pCoreService);
		
		this->m_mapCoreActor[pCoreActor->getID()] = pCoreActor;

		return pCoreActor;
	}

	void CActorScheduler::destroyCoreActor(CCoreActor* pCoreActor)
	{
		DebugAst(pCoreActor != nullptr);

		this->m_mapCoreActor.erase(pCoreActor->getID());
		this->m_mapWorkCoreActor.erase(pCoreActor->getID());
		this->m_mapPendingCoreActor.erase(pCoreActor->getID());

		SAFE_DELETE(pCoreActor);
	}

	void CActorScheduler::addWorkCoreActor(CCoreActor* pCoreActor)
	{
		DebugAst(pCoreActor != nullptr);

		if (pCoreActor->getState() == CCoreActor::eABS_Normal || pCoreActor->getState() == CCoreActor::eABS_RecvPending)
			this->m_mapWorkCoreActor[pCoreActor->getID()] = pCoreActor;
	}

	void CActorScheduler::addPendingCoreActor(CCoreActor* pCoreActor)
	{
		DebugAst(pCoreActor != nullptr);

		DebugAst(pCoreActor->getState() == CCoreActor::eABS_Normal);

		this->m_mapPendingCoreActor[pCoreActor->getID()] = pCoreActor;

		pCoreActor->setState(CCoreActor::eABS_Pending);
	}

	uint64_t CActorScheduler::getCurWorkActorID() const
	{
		return this->m_nCurWorkActorID;
	}

	void CActorScheduler::setCurWorkActorID(uint64_t nID)
	{
		this->m_nCurWorkActorID = nID;
	}
}