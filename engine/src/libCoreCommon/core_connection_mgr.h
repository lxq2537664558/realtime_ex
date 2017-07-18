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
		
		void				destroyCoreConnection(uint64_t nSocketID);
		CCoreConnection*	getCoreConnectionByID(uint64_t nID) const;
		uint32_t			getCoreConnectionCount(uint32_t nType) const;

		void				wakeup();

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
		@brief: ������������ʱ�ȴ����ӽ����Ĵ���������������ҪΪ�������߼����ڷ������������������ӳɹ�����һ��ʱ���������
		*/
		struct SNetActiveWaitConnecterHandler :
			public base::INetConnecterHandler
		{
			std::string			szContext;
			uint32_t			nType;
			CCoreConnectionMgr*	pCoreConnectionMgr;
			MessageParser		messageParser;

			virtual uint32_t	onRecv( const char* pData, uint32_t nDataSize ) { return 0; }
			virtual void		onSendComplete(uint32_t nSize) { }
			virtual void		onConnect();
			virtual void		onDisconnect();
			virtual void		onConnectFail();
		};

		base::INetEventLoop*							m_pNetEventLoop;
		std::vector<base::INetAccepterHandler*>			m_vecNetAccepterHandler;
		std::list<SNetActiveWaitConnecterHandler*>		m_listActiveNetWaitConnecterHandler;

		std::map<uint32_t, std::list<CCoreConnection*>>	m_mapCoreConnectionByTypeID;
		std::map<uint64_t, CCoreConnection*>			m_mapCoreConnectionByID;
		uint64_t										m_nNextCoreConnectionID;
		
	private:
		base::INetConnecterHandler*	onAccept( SNetAccepterHandler* pNetAccepterHandler, base::INetConnecter* pNetConnecter );
		void						onConnect( SNetActiveWaitConnecterHandler* pNetActiveWaitConnecterHandler );
		void						delActiveWaitConnecterHandler( SNetActiveWaitConnecterHandler* pWaitActiveConnecterHandler );

		CCoreConnection*			createCoreConnection(uint32_t nType, const std::string& szContext, const MessageParser& messageParser);
	};
}