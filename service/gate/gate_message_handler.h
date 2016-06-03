#pragma once
#include "libBaseCommon/singleton.h"

#include "libCoreCommon/core_common.h"

class CGateMessageHandler
	: public base::CSingleton<CGateMessageHandler>
{
public:
	CGateMessageHandler();
	~CGateMessageHandler();

	bool init();

private:
	void onLogin(uint64_t nSocketID, const core::client_message_header* pHeader);
};