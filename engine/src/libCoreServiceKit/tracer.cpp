#include "stdafx.h"
#include "tracer.h"
#include "core_service_app_impl.h"

#include "libBaseCommon/base_time.h"

#define _MAX_TRACE_NODE_COUNT 10000

namespace core
{
	CTracer::CTracer()
		: m_nNextGenTraceID(0)
		, m_bEnable(true)
		, m_nCurRecvTraceID(0)
		, m_nCurRecvFromID(0)
		, m_nCurRecvMessageID(0)
		, m_nCurRecvBeginTime(0)
	{
		memset(this->m_szExtraBuf, 0, sizeof(this->m_szExtraBuf));
	}

	CTracer::~CTracer()
	{

	}

	bool CTracer::init()
	{
		this->enableTrace(false);
		return true;
	}

	void CTracer::enableTrace(bool bEnable)
	{
		this->m_bEnable = bEnable;
	}

	void CTracer::addTraceExtraInfo(uint64_t nTraceID, const char* szFormat, va_list arg)
	{
		if (!this->m_bEnable)
			return;

		if (base::crt::vsnprintf(this->m_szExtraBuf, sizeof(this->m_szExtraBuf), szFormat, arg) < 0)
		{
			PrintWarning("CInvokerTrace::addTraceExtraInfo trace_id: "UINT64FMT, nTraceID);
			return;
		}

		base::saveLogEx("trace", false, "trace_id: "UINT64FMT" extra_info: %s time: "UINT64FMT, nTraceID, this->m_szExtraBuf, base::getGmtTime());
	}

	void CTracer::addTraceExtraInfo(const char* szFormat, ...)
	{
		if (!this->m_bEnable)
			return;

		va_list arg;
		va_start(arg, szFormat);
		this->addTraceExtraInfo(this->m_nCurRecvTraceID, szFormat, arg);
		va_end(arg);
	}

	void CTracer::addTraceExtraInfo(uint64_t nTraceID, const char* szFormat, ...)
	{
		if (!this->m_bEnable)
			return;

		va_list arg;
		va_start(arg, szFormat);
		this->addTraceExtraInfo(nTraceID, szFormat, arg);
		va_end(arg);
	}

	void CTracer::traceBeginRecv(uint64_t nTraceID, uint16_t nMessageID, uint64_t nFromID)
	{
		if (!this->m_bEnable)
			return;
		
		if (nTraceID == 0)
			return;

		this->m_nCurRecvMessageID = nMessageID;
		this->m_nCurRecvFromID = nFromID;
		this->m_nCurRecvTraceID = nTraceID;
		this->m_nCurRecvBeginTime = base::getGmtTime();
	}

	void CTracer::traceEndRecv()
	{
		if (!this->m_bEnable)
			return;

		if (this->m_nCurRecvTraceID == 0)
			return;

		int64_t nEndTime = base::getGmtTime();
		base::saveLogEx("trace", false, "trace_id: "UINT64FMT" trace_recv message_id: %d from_id: "UINT64FMT"begin_time: "UINT64FMT" end_time: "UINT64FMT" cost: "INT64FMT,
			this->m_nCurRecvTraceID, this->m_nCurRecvMessageID, this->m_nCurRecvFromID, this->m_nCurRecvBeginTime, nEndTime, (nEndTime - this->m_nCurRecvBeginTime));

		this->m_nCurRecvTraceID = 0;
		this->m_nCurRecvFromID = 0;
		this->m_nCurRecvMessageID = 0;
		this->m_nCurRecvBeginTime = 0;
	}

	void CTracer::traceSend(uint64_t nTraceID, uint16_t nMessageID, uint64_t nToID, int64_t nBeginTime)
	{
		if (!this->m_bEnable)
			return;

		if (nTraceID == 0)
			return;

		int64_t nEndTime = base::getGmtTime();
		base::saveLogEx("trace", false, "trace_id: "UINT64FMT" trace_send message_id: %d to_id: "UINT64FMT"begin_time: "UINT64FMT" end_time: "UINT64FMT" cost: "INT64FMT,
			nTraceID, nMessageID, nToID, nBeginTime, nEndTime, (nEndTime - nBeginTime));
	}

	void CTracer::startNewTrace()
	{
		this->m_nCurRecvTraceID = this->m_nNextGenTraceID++;
	}

	uint64_t CTracer::getCurTraceID() const
	{
		return this->m_nCurRecvTraceID;
	}

}