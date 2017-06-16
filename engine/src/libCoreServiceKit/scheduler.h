#pragma once

#include "base_actor_impl.h"
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

		CActorBaseImpl*		createBaseActor(CActorBase* pActor);
		void				destroyBaseActor(CActorBaseImpl* pActorBase);

		bool				invoke(const SRequestMessageInfo& sRequestMessageInfo);
		bool				response(const SResponseMessageInfo& sResponseMessageInfo);

		CActorBaseImpl*		getBaseActor(uint64_t nID) const;

		void				addWorkBaseActor(CActorBaseImpl* pActorBase);

		void				run();
	
	private:
		uint64_t								m_nNextActorID;
		std::map<uint64_t, CActorBaseImpl*>		m_mapBaseActor;
		std::map<uint64_t, CActorBaseImpl*>		m_mapWorkBaseActor;
		std::vector<char>						m_vecBuf;
	};
}