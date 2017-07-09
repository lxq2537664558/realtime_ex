#pragma once

#include "actor_base_impl.h"
#include "service_base.h"

#include <map>
#include <list>

namespace core
{
	class CActorScheduler
	{
	public:
		CActorScheduler();
		~CActorScheduler();

		bool				init();

		CActorBaseImpl*		createActorBase(CActorBase* pActor);
		void				destroyActorBase(CActorBaseImpl* pActorBase);

		bool				invoke(uint64_t nSessionID, uint64_t nFromActorID, uint64_t nToActorID, const google::protobuf::Message* pMessage);
		bool				response(uint64_t nSessionID, uint8_t nResult, uint64_t nToActorID, const google::protobuf::Message* pMessage);

		CActorBaseImpl*		getActorBase(uint64_t nID) const;

		void				addWorkActorBase(CActorBaseImpl* pActorBase);

		void				run();
	
	private:
		uint64_t								m_nNextActorID;
		std::map<uint64_t, CActorBaseImpl*>		m_mapActorBase;
		std::list<CActorBaseImpl*>				m_listWorkActorBase;
		std::vector<char>						m_vecBuf;
	};
}