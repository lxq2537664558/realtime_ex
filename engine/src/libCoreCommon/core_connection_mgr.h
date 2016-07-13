#pragma once

#include <vector>
#include <list>
#include <unordered_map>
#include <map>
#include <functional>

#include "libBaseNetwork/network.h"

#include "core_connection.h"
#include "core_common.h"

namespace core
{
	class CCoreConnection;
	class CBaseConnectionMgr;
	class CCoreConnectionMgr :
		public base::noncopyable
	{
		friend class CCoreConnection;

	public:
		CCoreConnectionMgr();
		~CCoreConnectionMgr();

		bool				init(uint32_t nMaxConnectionCount);
		bool				connect(const std::string& szHost, uint16_t nPort, uint32_t nType, const std::string& szContext, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, MessageParser messageParser);
		bool				listen(const std::string& szHost, uint16_t nPort, uint32_t nType, const std::string& szContext, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, MessageParser messageParser);
		void				update(int64_t nTime);

		void				broadcast(uint32_t nType, uint8_t nMessageType, const void* pData, uint16_t nSize, const std::vector<uint64_t>* vecExcludeID);
		
		void				destroyConnection(uint64_t nSocketID);
		CCoreConnection*	getCoreConnectionByID(uint64_t nID) const;
		uint32_t			getCoreConnectionCount(uint32_t nType) const;

		void				onTimer(int64_t nTime);

	private:
		struct SNetAccepterHandler :
			public base::INetAccepterHandler
		{
			std::string			szContext;
			uint32_t			nType;
			CCoreConnectionMgr*	pCoreConnectionMgr;
			MessageParser		messageParser;

			virtual base::INetConnecterHandler* onAccept( base::INetConnecter* pNetConnecter );
		};


		/**
		@brief: 主动发起连接时等待连接建立的处理器，这个设计主要为了屏蔽逻辑层在发起连接请求到真正连接成功会有一段时间这个问题
		*/
		struct SNetActiveWaitConnecterHandler :
			public base::INetConnecterHandler
		{
			std::string			szContext;
			uint32_t			nType;
			CCoreConnectionMgr*	pCoreConnectionMgr;
			MessageParser		messageParser;

			virtual uint32_t	onRecv( const char* pData, uint32_t nDataSize ) { return 0; }
			virtual void		onConnect();
			virtual void		onDisconnect();
		};

		base::INetEventLoop*							m_pNetEventLoop;
		std::vector<base::INetAccepterHandler*>			m_vecNetAccepterHandler;
		std::list<SNetActiveWaitConnecterHandler*>		m_listActiveNetWaitConnecterHandler;

		std::map<uint32_t, std::list<CCoreConnection*>>	m_mapCoreConnectionByTypeID;
		std::map<uint64_t, CCoreConnection*>			m_mapCoreConnectionByID;
		uint64_t										m_nNextCoreConnectionID;
		CBaseConnectionMgr*								m_pBaseConnectionMgr;
		
	private:
		base::INetConnecterHandler*	onAccept( SNetAccepterHandler* pNetAccepterHandler, base::INetConnecter* pNetConnecter );
		void						onConnect( SNetActiveWaitConnecterHandler* pNetActiveWaitConnecterHandler );
		void						delActiveWaitConnecterHandler( SNetActiveWaitConnecterHandler* pWaitActiveConnecterHandler );

		CCoreConnection*			createConnection(uint32_t nType, const std::string& szContext, const MessageParser& messageParser);
	};
}