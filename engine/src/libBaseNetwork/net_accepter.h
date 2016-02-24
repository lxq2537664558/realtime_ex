#pragma once

#include "net_socket.h"
#include "network.h"

namespace base
{
	class CNetAccepter :
		public INetAccepter,
		public CNetSocket
	{
	public:
		CNetAccepter();
		virtual ~CNetAccepter();

		virtual void            onEvent(uint32_t nEvent) override;
		virtual void            forceClose() override;
		virtual bool			init(uint32_t nSendBufferSize, uint32_t nRecvBufferSize, CNetEventLoop* pNetEventLoop);

		virtual void            setHandler(INetAccepterHandler* pHandler) override;
		virtual const SNetAddr& getListenAddr() const override;
		virtual void            shutdown() override;

		bool                    listen(const SNetAddr& netAddr);

	private:
		INetAccepterHandler* m_pHandler;
	};
}