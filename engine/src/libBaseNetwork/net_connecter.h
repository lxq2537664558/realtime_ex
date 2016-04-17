#pragma once

#include "net_socket.h"
#include "network.h"
#include "net_buffer.h"

namespace base
{
	class CNetConnecter :
		public INetConnecter,
		public CNetSocket
	{
	public:
		enum ECloseType
		{
			eCT_None = 0,
			eCT_Recv = 1,
			eCT_Send = 2,
		};

	public:
		CNetConnecter();
		virtual ~CNetConnecter();

		virtual void				onEvent(uint32_t nEvent) override;
		virtual void				forceClose() override;
		virtual bool				init(uint32_t nSendBufferSize, uint32_t nRecvBufferSize, CNetEventLoop* pNetEventLoop);

		virtual void				send(const void* pData, uint32_t nDataSize) override;
		virtual void				setHandler(INetConnecterHandler* pHandler) override;
		virtual void				shutdown(bool bForce, const char* szMsg) override;
		virtual const SNetAddr&		getLocalAddr() const override;
		virtual const SNetAddr&		getRemoteAddr() const override;
		virtual ENetConnecterType	getConnecterType() const override;
		virtual ENetConnecterState	getConnecterState() const override;
		virtual	uint32_t			getSendDataSize() const override;
		virtual	uint32_t			getRecvDataSize() const override;

		void						printInfo(const char* szMsg);
		void						activeSend();
		int32_t						getSendConnecterIndex() const;
		void						setSendConnecterIndex(int32_t nIndex);
		bool						connect(const SNetAddr& sNetAddr);
		void						setConnecterType(ENetConnecterType eConnecterType);
		void						setConnecterState(ENetConnecterState eConnecterState);

	private:
		void						onConnect();
		void						onRecv();
		void						commonSend();
		void						eventSend();

	private:
		ENetConnecterType		m_eConnecterType;
		ENetConnecterState		m_eConnecterState;
		uint32_t				m_nCloseType;
		CNetRecvBuffer*			m_pRecvBuffer;
		CNetSendBuffer*			m_pSendBuffer;
		INetConnecterHandler*	m_pHandler;
		uint32_t				m_nSendConnecterIndex;
	};
}