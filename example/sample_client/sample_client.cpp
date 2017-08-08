#include "stdafx.h"
#include "libBaseNetwork/network.h"
#include "libBaseCommon/thread_base.h"
#include "libBaseCommon/base_time.h"
#include "libBaseCommon/logger.h"
#include "libBaseCommon/debug_helper.h"

#include <iostream>
#include "connection_to_gate.h"
#include "connection_to_login.h"
using namespace std;

int main()
{
	base::initLog(false, false, "./log");
	base::startupNetwork();

	base::INetEventLoop* pNetEventLoop = base::createNetEventLoop();
	pNetEventLoop->init(1000);

	CMessageHandler::Inst()->init(pNetEventLoop);

	SNetAddr netAddr;
	netAddr.nPort = 8000;
	base::crt::strcpy(netAddr.szHost, _countof(netAddr.szHost), "192.168.222.137");
	pNetEventLoop->connect(netAddr, 1024, 1024, new CConnectToLogin());

	while (true)
	{
		pNetEventLoop->update(10);
	}

	return 0;
}