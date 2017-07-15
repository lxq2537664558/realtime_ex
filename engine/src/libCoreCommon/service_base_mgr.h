#pragma once

#include "service_base_impl.h"

#include <list>
#include <map>

namespace core
{
	class CServiceBaseMgr
	{
	public:
		CServiceBaseMgr();
		~CServiceBaseMgr();
		
		bool				init(const std::vector<SServiceBaseInfo>& vecServiceBaseInfo);

		CServiceBaseImpl*	getServiceBase(uint16_t nID) const;

		void				sendMessage(uint16_t nToServiceID, const SMessagePacket& sMessagePacket);

		CServiceBaseImpl*	getWorkServiceBase();
		void				addWorkServiceBase(CServiceBaseImpl* pServiceBaseImpl);

	private:
		std::map<uint16_t, CServiceBaseImpl*>	m_mapServiceBase;

		std::list<CServiceBaseImpl*>			m_listWorkServiceBase;
		std::set<uint16_t>						m_setWorkServiceBaseID;
		std::mutex								m_lock;
		std::condition_variable					m_cond;
	};
}