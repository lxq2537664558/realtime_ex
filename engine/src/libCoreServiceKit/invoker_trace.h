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
		void		addTraceExtraInfo(const char* szFormat, ...);
		void		addTraceExtraInfo(uint64_t nTraceID, const char* szFormat, ...);

		void		traceBeginRecv(uint64_t nTraceID, uint16_t nMessageID, uint64_t nFromID);
		void		traceEndRecv();
		void		traceSend(uint64_t nTraceID, uint16_t nMessageID, uint64_t nToID, int64_t nBeginTime);

	private:
		void		addTraceExtraInfo(uint64_t nTraceID, const char* szFormat, va_list arg);

	private:
		bool		m_bEnable;
		uint64_t	m_nNextGenTraceID;

		uint64_t	m_nCurRecvTraceID;
		uint64_t	m_nCurRecvFromID;
		uint16_t	m_nCurRecvMessageID;
		int64_t		m_nCurRecvBeginTime;

		char		m_szExtraBuf[_INVOKER_TRACE_EXTRA_INFO];
	};
}