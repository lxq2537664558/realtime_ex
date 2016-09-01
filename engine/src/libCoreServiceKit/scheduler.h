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

		CBaseActorImpl*		createBaseActor(CBaseActor* pActor);
		void				destroyBaseActor(CBaseActorImpl* pActorBase);

		bool				invoke(const SRequestMessageInfo& sRequestMessageInfo);
		bool				response(const SResponseMessageInfo& sResponseMessageInfo);

		CBaseActorImpl*		getBaseActor(uint64_t nID) const;

		void				addWorkBaseActor(CBaseActorImpl* pActorBase);

		void				run();
	
	private:
		uint64_t								m_nNextActorID;
		std::map<uint64_t, CBaseActorImpl*>		m_mapBaseActor;
		std::map<uint64_t, CBaseActorImpl*>		m_mapWorkBaseActor;
		std::vector<char>						m_vecBuf;
	};
}