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

		CActorBaseImpl*		createActorBase(CActorBase* pActor);
		void				destroyActorBase(CActorBaseImpl* pActorBase);

		bool				invoke(const SRequestMessageInfo& sRequestMessageInfo);
		bool				response(const SResponseMessageInfo& sResponseMessageInfo);

		CActorBaseImpl*		getBaseActor(uint64_t nID) const;

		void				addWorkActorBase(CActorBaseImpl* pActorBase);

		void				run();
	
	private:
		uint64_t								m_nNextActorID;
		std::map<uint64_t, CActorBaseImpl*>		m_mapActorBase;
		std::list<CActorBaseImpl*>				m_listWorkActorBase;
		std::vector<char>						m_vecBuf;
	};
}