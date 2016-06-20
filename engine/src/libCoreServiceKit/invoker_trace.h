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
		
		void		startNewTrace();
		uint64_t	getCurTraceID() const;
		void		addTraceExtraInfo(uint64_t nTraceID, const char* szFormat, va_list arg);
		void		addTraceExtraInfo(const char* szFormat, ...);
		void		addTraceExtraInfo(uint64_t nTraceID, const char* szFormat, ...);
		void		beginRecv(uint64_t nTraceID, uint16_t nMessageID, const std::string& szFromServiceName);
		void		endRecv();
		void		send(uint16_t nMessageID);

	private:
		uint64_t	m_nNextGenTraceID;
		uint64_t	m_nCurTraceID;
		char		m_szExtraInfo[_INVOKER_TRACE_EXTRA_INFO];
	};
}