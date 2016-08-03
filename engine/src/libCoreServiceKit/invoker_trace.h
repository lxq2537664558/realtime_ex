#pragma once
#include "libCoreCommon/core_common.h"

#include <list>

namespace core
{
#define _INVOKER_TRACE_EXTRA_INFO 4096

	class CInvokerTrace
	{
	public:
		CInvokerTrace();
		~CInvokerTrace();
		
		bool		init();
		void		enableTrace(bool bEnable);
		void		startNewTrace();
		uint64_t	getCurTraceID() const;
		void		addTraceExtraInfo(uint64_t nTraceID, const char* szFormat, va_list arg);
		void		addTraceExtraInfo(const char* szFormat, ...);
		void		addTraceExtraInfo(uint64_t nTraceID, const char* szFormat, ...);
		void		beginRecv(uint64_t nTraceID, uint16_t nMessageID, uint16_t nFromServiceID);
		void		endRecv();
		void		send(uint16_t nMessageID);

	private:
		bool		m_bEnable;
		uint64_t	m_nNextGenTraceID;
		uint64_t	m_nCurTraceID;
		char		m_szExtraInfo[_INVOKER_TRACE_EXTRA_INFO];
	};
}