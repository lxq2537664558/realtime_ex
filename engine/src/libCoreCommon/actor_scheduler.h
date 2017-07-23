#pragma once

#include "actor_base_impl.h"
#include "service_base.h"

#include <map>

namespace core
{
	class CServiceBaseImpl;
	class CActorScheduler
	{
	public:
		CActorScheduler(CServiceBaseImpl* pServiceBaseImpl);
		~CActorScheduler();

		bool				init();

		CActorBaseImpl*		createActorBase(uint64_t nActorID, CActorBase* pActorBase);
		void				destroyActorBase(CActorBaseImpl* pActorBase);

		CActorBaseImpl*		getActorBase(uint64_t nID) const;

		void				addWorkActorBase(CActorBaseImpl* pActorBase);
		void				addPendingActorBase(CActorBaseImpl* pActorBase);

		void				run();
	
	private:
		CServiceBaseImpl*						m_pServiceBaseImpl;
		std::map<uint64_t, CActorBaseImpl*>		m_mapActorBase;
		std::map<uint64_t, CActorBaseImpl*>		m_mapWorkActorBase;
		std::map<uint64_t, CActorBaseImpl*>		m_mapPendingActorBase;
	};
}