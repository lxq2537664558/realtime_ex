#include "stdafx.h"
#include "libBaseNetwork/network.h"
#include "libBaseCommon/thread_base.h"
#include "libBaseCommon/base_time.h"
#include "libBaseCommon/logger.h"

#include <iostream>
#include "libBaseCommon/debug_helper.h"
#include "connection_to_gate.h"
using namespace std;

int main()
{
	base::initLog(false, false, "./log");
	base::startupNetwork();

	CMessageHandler::Inst()->init();

	base::INetEventLoop* pNetEventLoop = base::createNetEventLoop();
	pNetEventLoop->init(1000);

	SNetAddr netAddr;
	netAddr.nPort = 8000;
	base::crt::strcpy(netAddr.szHost, _countof(netAddr.szHost), "127.0.0.1");
	pNetEventLoop->connect(netAddr, 1024, 1024, new CConnectToGate());

	while (true)
	{
		pNetEventLoop->update(10);
	}

	return 0;
}