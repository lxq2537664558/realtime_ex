#pragma once

#include "libCoreCommon/core_common.h"

#include "core_service_kit_define.h"

#include <list>

namespace core
{

	class CClusterInvoker;
	class CActor;
	class CResponseFuture
	{
		friend class CClusterInvoker;
		friend class CActor;
	public:
		CResponseFuture();
		~CResponseFuture();

		void			then(InvokeCallback callback);
		void			then(InvokeCallback callback, InvokeErrCallback err);
		CResponseFuture	then_r(InvokeCallbackEx callback);
		CResponseFuture	then_r(InvokeCallbackEx callback, InvokeErrCallback err);

	private:
		uint64_t	m_nSessionID;
		uint64_t	m_nActorID;
	};
}