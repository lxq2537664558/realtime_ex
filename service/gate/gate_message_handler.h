#pragma once
#include "libBaseCommon/singleton.h"

#include "core_common.h"

class CGateMessageHandler
	: public base::CSingleton<CGateMessageHandler>
{
public:
	CGateMessageHandler();
	~CGateMessageHandler();

	bool init();

private:
};