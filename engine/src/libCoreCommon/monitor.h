#pragma once
#include "libBaseCommon/base_common.h"

namespace core
{
	bool initMonitor();
	void uninitMonitor();
	void pushMessageMonitor(uint32_t nID);
	void pushTickerMonitor(void* pAddr);
	void popMonitor();
}