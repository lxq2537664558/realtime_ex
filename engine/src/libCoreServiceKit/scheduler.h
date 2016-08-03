#pragma once

#include "actor_base.h"
#include "service_base.h"

#include <map>
#include <list>

namespace core
{
	class CScheduler
	{
	public:
		CScheduler();
		~CScheduler();

		bool				init();

		CActorBase*			createActorBase(CActor* pActor);
		void				destroyActorBase(CActorBase* pActorBase);

		bool				invoke(const SRequestMessageInfo& sRequestMessageInfo);
		bool				response(const SResponseMessageInfo& sResponseMessageInfo);

		CActorBase*			getActorBase(uint64_t nID) const;

		void				addWorkActorBase(CActorBase* pActorBase);

		void				run();
	
	private:
		uint64_t								m_nNextActorID;
		std::map<uint64_t, CActorBase*>			m_mapActorBase;
		std::map<uint64_t, CActorBase*>			m_mapWorkActorBase;
	};
}