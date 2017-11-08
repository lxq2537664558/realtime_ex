#pragma once
#include "libBaseNetwork/network.h"

#include "libBaseCommon/ticker.h"

#include "core_common.h"

#include <atomic>

namespace core
{
	class CCoreConnectionMgr;
	class CBaseConnection;
	class CBaseConnectionMgr;
	class CLogicMessageQueue;
	class CCoreConnection :
		public base::net::INetConnecterHandler
	{
		friend class CCoreConnectionMgr;
		friend class CBaseConnectionMgr;

	public:
		// 因为CCoreConnection跟CBaseConnection存在于两个线程，他们之间通过消息队列交互，所以需要一些中间状态
		// eCCS_Connectting的存在是为了需要在逻辑线程创建CBaseConnection对象，但是在eCCS_Connectting状态下还是能收发消息的
		// eCCS_Disconnecting的存在是因为让CBaseConnection先销毁然后在CCoreConnection销毁，有些消息发送是直接传CCoreConnection指针的，这样多线程也是安全的
		enum
		{
			eCCS_None,
			eCCS_Connectting,
			eCCS_Connected,
			eCCS_Disconnecting,
		};

	public:
		CCoreConnection();
		virtual ~CCoreConnection();

		bool				init(CLogicMessageQueue* pMessageQueue, const std::string& szType, uint64_t nID, const std::string& szContext, const MessageParser& messageParser);
		virtual void		send(uint8_t nMessageType, const void* pData, uint16_t nSize);
		virtual void		send(uint8_t nMessageType, const void* pData, uint16_t nSize, const void* pExtraBuf, uint16_t nExtraSize);
		
		void				setMessageParser(MessageParser& parser);

		uint64_t			getID() const;

		const std::string&	getType() const;

		const std::string&	getContext() const;

		void				shutdown(bool bForce, const std::string& szMsg);

		uint32_t			getSendDataSize() const;
		uint32_t			getRecvDataSize() const;

		const SNetAddr&		getLocalAddr() const;
		const SNetAddr&		getRemoteAddr() const;

		void				enableHeartbeat(bool bEnable);

		uint32_t			getState() const;

		void				onConnectAck();

		base::net::ENetConnecterMode
							getMode() const;

	private:
		uint32_t			onRecv(const char* pData, uint32_t nDataSize);
		virtual void		onSendComplete(uint32_t nSize);
		void				onConnect();
		void				onDisconnect();
		virtual void		onConnectFail();

		void				onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize);

		void				onHeartbeat(uint64_t nContext);

	protected:
		std::atomic<uint8_t>	m_bHeartbeat;
		base::CTicker			m_heartbeat;
		uint32_t				m_nSendHeartbeatCount;

		uint64_t				m_nID;
		CLogicMessageQueue*		m_pMessageQueue;
		std::string				m_szType;
		uint32_t				m_nState;
		std::string				m_szContext;
		MessageParser			m_messageParser;
	};
}