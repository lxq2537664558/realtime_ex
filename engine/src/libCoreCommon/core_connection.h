#pragma once
#include "libBaseNetwork/network.h"

#include "core_common.h"

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
		CCoreConnection();
		virtual ~CCoreConnection();

		bool				init(CBaseConnection* pBaseConnection, uint64_t nID, ClientDataCallback clientDataCallback);
		void				send(uint16_t nMessageType, const void* pData, uint16_t nSize);
		void				send(uint16_t nMessageType, const void* pData, uint16_t nSize, const void* pExtraBuf, uint16_t nExtraSize);

		uint64_t			getID() const;

		void				shutdown(bool bForce, const std::string& szMsg);

		CBaseConnection*	getBaseConnection() const;

		uint32_t			getSendDataSize() const;
		uint32_t			getRecvDataSize() const;

		const SNetAddr&		getLocalAddr() const;
		const SNetAddr&		getRemoteAddr() const;

		void				onHeartbeat(uint64_t nContext);

	private:
		virtual uint32_t	onRecv(const char* pData, uint32_t nDataSize);
		virtual void		onConnect();
		virtual void		onDisconnect();

		void				onPacketMsg(uint32_t nMessageType, const void* pData, uint16_t nSize);
		void				sendHeartbeat();

	private:
		bool				m_bHeartbeat;
		CTicker*			m_pHeartbeat;
		uint32_t			m_nSendHeartbeatCount;

		std::string			m_szContext;
		uint64_t			m_nID;
		CBaseConnection*	m_pBaseConnection;	// 这个必须奥保证赋值操作是原子的，所以应该在边界上对齐，不然又可能不是原子的
		ClientDataCallback	m_clientDataCallback;
	};
}