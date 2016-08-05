#pragma once

#include "libCoreCommon/core_common.h"

#include "core_service_kit_define.h"

#include <list>

namespace core
{
	class CResponseFuture
	{
	public:
		CResponseFuture();
		~CResponseFuture();

		void			then(InvokeCallback callback);
		void			then(InvokeCallback callback, InvokeErrCallback err);
		CResponseFuture	then_r(InvokeCallbackEx callback);
		CResponseFuture	then_r(InvokeCallbackEx callback, InvokeErrCallback err);

		void			setSessionID(uint64_t nSessionID);
		void			setActorID(uint64_t nActorID);

	private:
		uint64_t	m_nSessionID;
		uint64_t	m_nActorID;
	};
}