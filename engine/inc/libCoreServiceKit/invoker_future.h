#pragma once

#include "libCoreCommon/core_common.h"
#include "core_service_kit_define.h"

#include <list>

namespace core
{
	class CInvokerFuture
	{
	public:
		CInvokerFuture();

		CInvokerFuture&	then(InvokeCallback callback);

	private:
		std::list<InvokeCallback>	m_listCallback;
	};
}