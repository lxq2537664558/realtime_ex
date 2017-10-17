#pragma once

#include <map>
#include <functional>

#include "core_common.h"

#include "libBaseCommon/noncopyable.h"

namespace core
{
	class CBaseConnection;
	class CCoreConnection;
	class CLogicRunnable;
	class CLogicMessageQueue;
	class CBaseConnectionFactory;
	
	class CBaseConnectionMgrImpl :
		public base::noncopyable
	{
		friend class CBaseConnection;
		friend class CLogicRunnable;

	public:
		CBaseConnectionMgrImpl(CLogicMessageQueue* pMessageQueue);
		~CBaseConnectionMgrImpl();
		
		CLogicMessageQueue*		getMessageQueue() const;

		void					setBaseConnectionFactory(const std::string& szType, CBaseConnectionFactory* pBaseConnectionFactory);
		CBaseConnectionFactory*	getBaseConnectionFactory(const std::string& szType) const;

		void					enumBaseConnection(const std::string& szType, const std::function<bool(CBaseConnection* pBaseConnection)>& callback) const;
		CBaseConnection*		getBaseConnectionBySocketID(uint64_t nID) const;
		uint32_t				getBaseConnectionCount(const std::string& szType) const;

		void					addConnectCallback(const std::string& szKey, const std::function<void(CBaseConnection*)>& callback);
		void					delConnectCallback(const std::string& szKey);

		void					addDisconnectCallback(const std::string& szKey, const std::function<void(CBaseConnection*)>& callback);
		void					delDisconnectCallback(const std::string& szKey);

		void					addConnectFailCallback(const std::string& szKey, const std::function<void(const std::string&)>& callback);
		void					delConnectFailCallback(const std::string& szKey);

	private:
		bool					onConnect(CCoreConnection* pCoreConnection);
		void					onDisconnect(uint64_t nSocketID);
		void					onConnectFail(const std::string& szContext);
		
	private:
		CLogicMessageQueue*												m_pMessageQueue;
		std::map<uint64_t, CBaseConnection*>							m_mapBaseConnectionByID;
		std::map<std::string, std::map<uint64_t, CBaseConnection*>>		m_mapBaseConnectionByType;
		std::map<std::string, CBaseConnectionFactory*>					m_mapBaseConnectionFactory;

		std::map<std::string, std::function<void(CBaseConnection*)>>	m_mapConnectCalback;
		std::map<std::string, std::function<void(CBaseConnection*)>>	m_mapDisconnectCallback;
		std::map<std::string, std::function<void(const char*)>>			m_mapConnectFailCallback;
	};
}