#pragma once

#include "libCoreCommon/core_common.h"

#include "core_service_kit_define.h"

#include <list>

namespace core
{

	class CClusterInvoker;
	class CResponsePromise
	{
		friend class CClusterInvoker;

	public:
		CResponsePromise();
		~CResponsePromise();

		void				then(InvokeCallback callback);
		void				then(InvokeCallback callback, InvokeErrCallback err);
		CResponsePromise	then_r(InvokeCallbackEx callback);
		CResponsePromise	then_r(InvokeCallbackEx callback, InvokeErrCallback err);

	private:
		uint64_t	m_nSessionID;
	};
}