#include "stdafx.h"
#include "invoker_trace.h"
#include "core_service_app_impl.h"

#include "libBaseCommon/base_time.h"

namespace core
{
	CInvokerTrace::CInvokerTrace()
		: m_nCurTraceID(0)
		, m_nNextGenTraceID(0)
		, m_bEnable(true)
	{
		memset(this->m_szExtraInfo, 0, sizeof(this->m_szExtraInfo));
	}

	CInvokerTrace::~CInvokerTrace()
	{

	}

	bool CInvokerTrace::init()
	{
		this->enableTrace(false);
		return true;
	}

	void CInvokerTrace::enableTrace(bool bEnable)
	{
		this->m_bEnable = bEnable;
	}

	void CInvokerTrace::addTraceExtraInfo(uint64_t nTraceID, const char* szFormat, va_list arg)
	{
		if (!this->m_bEnable)
			return;

		if (base::crt::vsnprintf(this->m_szExtraInfo, sizeof(this->m_szExtraInfo), szFormat, arg) < 0)
		{
			PrintWarning("CInvokerTrace::addTraceExtraInfo trace_id: "UINT64FMT, nTraceID);
			return;
		}

		base::saveLogEx("trace", false, "trace_id: "UINT64FMT" extra_info: %s time: "UINT64FMT, nTraceID, this->m_szExtraInfo, base::getGmtTime());
	}

	void CInvokerTrace::addTraceExtraInfo(const char* szFormat, ...)
	{
		if (!this->m_bEnable)
			return;

		va_list arg;
		va_start(arg, szFormat);
		this->addTraceExtraInfo(this->m_nCurTraceID, szFormat, arg);
		va_end(arg);
	}

	void CInvokerTrace::addTraceExtraInfo(uint64_t nTraceID, const char* szFormat, ...)
	{
		if (!this->m_bEnable)
			return;

		va_list arg;
		va_start(arg, szFormat);
		this->addTraceExtraInfo(nTraceID, szFormat, arg);
		va_end(arg);
	}

	void CInvokerTrace::beginRecv(uint64_t nTraceID, uint16_t nMessageID, uint16_t nFromServiceID)
	{
		if (!this->m_bEnable)
			return;

		base::saveLogEx("trace", false, "trace_id: "UINT64FMT" begin recv time: "UINT64FMT" message_id: %d service_name: %s from_service_id: %d",
			nTraceID, base::getGmtTime(), nMessageID, CCoreServiceAppImpl::Inst()->getServiceBaseInfo().szName.c_str(), nFromServiceID);
		
		this->m_nCurTraceID = nTraceID;
	}

	void CInvokerTrace::endRecv()
	{
		if (!this->m_bEnable)
			return;

		base::saveLogEx("trace", false, "trace_id: "UINT64FMT" end recv time: "UINT64FMT, this->m_nCurTraceID, base::getGmtTime());
	}

	void CInvokerTrace::send(uint16_t nMessageID)
	{
		if (!this->m_bEnable)
			return;

		base::saveLogEx("trace", false, "trace_id: "UINT64FMT" send time: "UINT64FMT" message_id: %d service_name: %s",
			this->m_nCurTraceID, base::getGmtTime(), nMessageID, CCoreServiceAppImpl::Inst()->getServiceBaseInfo().szName.c_str());
	}

	void CInvokerTrace::startNewTrace()
	{
		this->m_nCurTraceID = this->m_nNextGenTraceID++;
	}

	uint64_t CInvokerTrace::getCurTraceID() const
	{
		return this->m_nCurTraceID;
	}

}