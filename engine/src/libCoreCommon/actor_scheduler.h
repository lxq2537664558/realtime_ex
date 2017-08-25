#pragma once

#include "core_actor.h"
#include "service_base.h"

#include <map>

namespace core
{
	class CCoreService;
	class CActorScheduler
	{
	public:
		CActorScheduler(CCoreService* pCoreService);
		~CActorScheduler();

		bool			init();

		CCoreActor*		createCoreActor(uint64_t nActorID, CActorBase* pActorBase);
		void			destroyCoreActor(CCoreActor* pCoreActor);

		CCoreActor*		getCoreActor(uint64_t nID) const;

		void			addWorkCoreActor(CCoreActor* pCoreActor);
		void			addPendingCoreActor(CCoreActor* pCoreActor);

		void			dispatch();

		uint64_t		getCurWorkActorID() const;
		void			setCurWorkActorID(uint64_t nID);

	private:
		CCoreService*					m_pCoreService;
		std::map<uint64_t, CCoreActor*>	m_mapCoreActor;
		std::map<uint64_t, CCoreActor*>	m_mapWorkCoreActor;
		std::map<uint64_t, CCoreActor*>	m_mapPendingCoreActor;

		uint64_t						m_nCurWorkActorID;
	};
}