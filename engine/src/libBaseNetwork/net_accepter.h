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

		virtual bool			init(uint32_t nSendBufferSize, uint32_t nRecvBufferSize, CNetEventLoop* pNetEventLoop);
		virtual void            release();
		virtual void            onEvent(uint32_t nEvent);
		virtual uint32_t		getSocketType() const { return eNST_Acceptor; }
		
		virtual void            setHandler(INetAccepterHandler* pHandler);
		virtual const SNetAddr& getListenAddr() const;
		virtual void            shutdown();

		bool                    listen(const SNetAddr& netAddr, bool bReusePort);

	private:
		INetAccepterHandler*	m_pHandler;
#ifndef _WIN32
		int32_t					m_nIdleID;	// �����ҪΪ�˽���������������þ�����ᵼ��accept()ʧ�ܣ�������һ��ENFILE ���󣬵��ǲ�û�оܾ�������ӣ�������Ȼ�����Ӷ����У��⵼������һ�ε�����ʱ����Ȼ�ᴥ�������������Ŀɶ��¼����⵼�³���busy loop
#endif
	};
}