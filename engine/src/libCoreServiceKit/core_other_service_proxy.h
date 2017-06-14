#pragma once

#include "libCoreCommon/core_common.h"
#include "libBaseCommon/noncopyable.h"

#include "tinyxml2/tinyxml2.h"

#include "core_service_kit_define.h"
#include "core_connection_other_service.h"
#include "serialize_adapter.h"

#include <map>
#include <string>

namespace core
{
	// 其他节点在本地节点的代理
	class CCoreOtherServiceProxy :
		public base::noncopyable
	{
	public:
		CCoreOtherServiceProxy();
		~CCoreOtherServiceProxy();

		bool							init();
		
		void							addServiceBaseInfo(const SServiceBaseInfo& sServiceBaseInfo);
		void							delServiceBaseInfo(uint16_t nID, bool bForce);

		void							setSerializeAdapter(uint16_t nID, CSerializeAdapter* pSerializeAdapter);
		CSerializeAdapter*				getSerializeAdapter(uint16_t nID) const;

		uint16_t						getServiceID(const std::string& szName) const;

		const SServiceBaseInfo*			getServiceBaseInfo(uint16_t nID) const;
		
		CCoreConnectionOtherService*	getCoreConnectionOtherService(uint16_t nServiceID) const;
		bool							addCoreConnectionOtherService(uint16_t nID, CCoreConnectionOtherService* pCoreConnectionOtherService);
		void							delCoreConnectionOtherService(uint16_t nID);
		
	private:
		struct SServiceInfo
		{
			SServiceBaseInfo				sServiceBaseInfo;
			CCoreConnectionOtherService*	pCoreConnectionOtherService;
		};

		std::map<uint16_t, SServiceInfo>		m_mapServiceInfo;
		std::map<std::string, uint16_t>			m_mapServiceName;
		CSerializeAdapter*						m_pDefaultSerializeAdapter;
		std::map<uint16_t, CSerializeAdapter*>	m_mapSerializeAdapter;
	};
}