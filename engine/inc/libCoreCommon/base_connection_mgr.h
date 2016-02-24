#pragma once

#include <list>
#include <map>
#include <unordered_map>
#include <functional>

#include "core_common.h"


namespace core
{
	class CBaseConnection;
	class CCoreConnectionMgr;
	class CCoreConnection;
	class CBaseConnectionMgr :
		public base::noncopyable
	{
		friend class CBaseConnection;
		friend class CCoreConnectionMgr;
		friend class CCoreConnection;

	public:
		CBaseConnectionMgr();
		~CBaseConnectionMgr();

		bool				connect(const std::string& szHost, uint16_t nPort, const std::string& szContext, const std::string& szClassName, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, funRawDataParser pfRawDataParser);
		bool				listen(const std::string& szHost, uint16_t nPort, const std::string& szContext, const std::string& szClassName, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, funRawDataParser pfRawDataParser);

		CBaseConnection*	getBaseConnection(uint64_t nID) const;
		void				getBaseConnection(const std::string& szClassName, std::vector<CBaseConnection*> vecBaseConnection) const;
		uint32_t			getBaseConnectionCount(const std::string& szClassName) const;

		void				broadcast(const std::string& szClassName, uint16_t nMsgType, const void* pData, uint16_t nSize);

		void				setConnectCallback(std::function<void(CBaseConnection*)> funConnect);
		void				setDisconnectCallback(std::function<void(CBaseConnection*)> funDisconnect);
		void				setConnectRefuseCallback(std::function<void(const std::string&)> funConnectRefuse);

	private:
		void				onConnect(CBaseConnection* pBaseConnection);
		void				onDisconnect(CBaseConnection* pBaseConnection);
		void				onConnectRefuse(const std::string& szContext);

	private:
		CCoreConnectionMgr*						m_pCoreConnectionMgr;
		std::function<void(CBaseConnection*)>	m_funConnect;
		std::function<void(CBaseConnection*)>	m_funDisconnect;
		std::function<void(const std::string&)>	m_funConnectRefuse;
	};
}