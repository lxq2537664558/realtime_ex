#include "libBaseNetwork/network.h"
#include "libBaseCommon/thread_base.h"
#include "libBaseCommon/base_time.h"
#include "libBaseCommon/logger.h"

#include <iostream>
using namespace std;

struct SNetConnecterHandler :
	public base::INetConnecterHandler
{
	virtual uint32_t onRecv( const char* pData, uint32_t nDataSize ) override
	{
		this->m_pNetConnecter->send( pData, nDataSize );
		return nDataSize;
	}

	virtual void   onConnect() override
	{
		PrintDebug( "onConnect" );
	}

	virtual void   onDisconnect() override
	{
		PrintDebug( "onDisconnect" );
	}
};

struct SNetAccepterHandler :
	public base::INetAccepterHandler
{
	virtual base::INetConnecterHandler* onAccept( base::INetConnecter* pNetConnecter )
	{
		return new SNetConnecterHandler();
	}
};

int main()
{
	base::initLog(true);
	base::startupNetwork();
	
	SNetAddr netAddr;
	netAddr.nPort = 8000;
	strcpy( netAddr.szHost, "0.0.0.0" );
	SNetAccepterHandler* pNetAccepterHandler = new SNetAccepterHandler();
	base::INetEventLoop* pNetEventLoop = base::createNetEventLoop();
	pNetEventLoop->init(50000);

	pNetEventLoop->listen( netAddr, 10*1024, 10*1024, pNetAccepterHandler );

	while( true )
	{
		pNetEventLoop->update(100);
	}

	return 0;
}