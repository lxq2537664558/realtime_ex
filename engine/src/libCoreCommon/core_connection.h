#pragma once
#include "libBaseNetwork/network.h"

#include "core_common.h"
#include "core_connection_monitor.h"

namespace core
{
	class CCoreConnectionMgr;
	class CBaseConnection;
	class CBaseConnectionMgr;
	class CCoreConnection :
		public base::INetConnecterHandler
	{
		friend class CCoreConnectionMgr;
		friend class CBaseConnectionMgr;

	public:
		enum
		{
			eCCS_None,
			eCCS_Connectting,
			eCCS_Connected,
			eCCS_Disconnectting,
		};

	public:
		CCoreConnection();
		~CCoreConnection();

		bool				init(uint32_t nType, uint64_t nID, const std::string& szContext, const MessageParser& messageParser);
		void				send(uint8_t nMessageType, const void* pData, uint16_t nSize);
		void				send(uint8_t nMessageType, const void* pData, uint16_t nSize, const void* pExtraBuf, uint16_t nExtraSize);
		
		void				setMessageParser(MessageParser& parser);

		uint64_t			getID() const;

		uint32_t			getType() const;

		void				shutdown(bool bForce, const std::string& szMsg);

		uint32_t			getSendDataSize() const;
		uint32_t			getRecvDataSize() const;

		const SNetAddr&		getLocalAddr() const;
		const SNetAddr&		getRemoteAddr() const;

		void				onHeartbeat();
		void				enableHeartbeat(bool bEnable);

		void				setState(uint32_t nState);
		uint32_t			getState() const;

	private:
		uint32_t			onRecv(const char* pData, uint32_t nDataSize);
		void				onConnect();
		void				onDisconnect();

		void				onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize);

	private:
		bool					m_bHeartbeat;
		uint32_t				m_nSendHeartbeatCount;

		uint64_t				m_nID;
		uint32_t				m_nState;
		uint32_t				m_nType;
		std::string				m_szContext;
		MessageParser			m_messageParser;
		CCoreConnectionMonitor	m_monitor;
	};
}