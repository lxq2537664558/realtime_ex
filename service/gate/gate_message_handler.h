#pragma once
#include "libBaseCommon/singleton.h"

#include "libCoreCommon/core_common.h"

#include "libCoreServiceKit/core_service_kit_define.h"

class CGateMessageHandler
	: public base::CSingleton<CGateMessageHandler>
{
public:
	CGateMessageHandler();
	~CGateMessageHandler();

	bool init();

private:
	void onLogin(uint64_t nSocketID, core::message_header_ptr pHeader);
};