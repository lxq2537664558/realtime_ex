#include "libBaseNetwork/network.h"
#include "libBaseCommon/thread_base.h"
#include "libBaseCommon/logger.h"
#include "libBaseCommon/debug_helper.h"

#include <iostream>
#include "connection_to_gate.h"
#include "connection_to_login.h"
using namespace std;

int main()
{
	base::log::init(false, false, "./log");
	base::net::startup();

	base::net::INetEventLoop* pNetEventLoop = base::net::createEventLoop();
	pNetEventLoop->init(1000);

	CMessageHandler::Inst()->init(pNetEventLoop);

	SNetAddr netAddr;
	netAddr.nPort = 8000;
	base::function_util::strcpy(netAddr.szHost, _countof(netAddr.szHost), "127.0.0.1");
	pNetEventLoop->connect(netAddr, 1024, 1024, new CConnectToLogin());

	while (true)
	{
		pNetEventLoop->update(10);
	}

	return 0;
}