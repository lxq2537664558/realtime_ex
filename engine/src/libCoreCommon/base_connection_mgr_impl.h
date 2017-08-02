#pragma once

#include <list>
#include <map>
#include <vector>
#include <functional>

#include "core_common.h"

#include "libBaseCommon\noncopyable.h"
#include "libBaseNetwork\network.h"

namespace core
{
	class CBaseConnection;
	class CCoreConnection;
	class CLogicRunnable;
	class CBaseConnectionFactory;
	
	class CBaseConnectionMgrImpl :
		public base::noncopyable
	{
		friend class CBaseConnection;
		friend class CLogicRunnable;

	public:
		CBaseConnectionMgrImpl();
		~CBaseConnectionMgrImpl();
		
		void							setBaseConnectionFactory(uint32_t nType, CBaseConnectionFactory* pBaseConnectionFactory);
		CBaseConnectionFactory*			getBaseConnectionFactory(uint32_t nType) const;
		
		void							enumBaseConnection(uint32_t nType, const std::function<bool(CBaseConnection* pBaseConnection)>& callback) const;
		CBaseConnection*				getBaseConnectionBySocketID(uint64_t nID) const;
		uint32_t						getBaseConnectionCount(uint32_t nType) const;
		
		void							addConnectCallback(const std::string& szKey, const std::function<void(CBaseConnection*)>& callback);
		void							delConnectCallback(const std::string& szKey);

		void							addDisconnectCallback(const std::string& szKey, const std::function<void(CBaseConnection*)>& callback);
		void							delDisconnectCallback(const std::string& szKey);
		
		void							addConnectFailCallback(const std::string& szKey, const std::function<void(const std::string&)>& callback);
		void							delConnectFailCallback(const std::string& szKey);

		void							addGlobalBeforeFilter(uint8_t nMessageType, const std::string& szKey, const NodeGlobalFilter& callback);
		void							delGlobalBeforeFilter(uint8_t nMessageType, const std::string& szKey);
		void							addGlobalAfterFilter(uint8_t nMessageType, const std::string& szKey, const NodeGlobalFilter& callback);
		void							delGlobalAfterFilter(uint8_t nMessageType, const std::string& szKey);
		const std::map<std::string, NodeGlobalFilter>*
										getGlobalBeforeFilter(uint8_t nMessageType) const;
		const std::map<std::string, NodeGlobalFilter>*
										getGlobalAfterFilter(uint8_t nMessageType) const;

	private:
		bool							onConnect(CCoreConnection* pCoreConnection);
		void							onDisconnect(uint64_t nSocketID);
		void							onConnectFail(const std::string& szContext);

	private:
		std::map<uint64_t, CBaseConnection*>							m_mapBaseConnectionByID;
		std::map<uint32_t, std::map<uint64_t, CBaseConnection*>>		m_mapBaseConnectionByType;
		std::map<uint32_t, CBaseConnectionFactory*>						m_mapBaseConnectionFactory;

		std::map<std::string, std::function<void(CBaseConnection*)>>	m_mapConnectCalback;
		std::map<std::string, std::function<void(CBaseConnection*)>>	m_mapDisconnectCallback;
		std::map<std::string, std::function<void(const char*)>>			m_mapConnectFailCallback;

		struct SNodeGlobalFilterInfo
		{
			std::map<std::string, NodeGlobalFilter>	mapGlobalBeforeFilter;
			std::map<std::string, NodeGlobalFilter>	mapGlobalAfterFilter;
		};
		std::map<uint8_t, SNodeGlobalFilterInfo>						m_mapGlobalFilterInfo;
	};
}