#include "stdafx.h"
#include "invoker_trace.h"
#include "core_service_kit_impl.h"

#include "libBaseCommon\base_time.h"

namespace core
{
	CInvokerTrace::CInvokerTrace()
		: m_nCurTraceID(0)
		, m_nNextGenTraceID(0)
	{
		memset(this->m_szExtraInfo, 0, sizeof(this->m_szExtraInfo));
	}

	CInvokerTrace::~CInvokerTrace()
	{

	}

	void CInvokerTrace::addTraceExtraInfo(uint64_t nTraceID, const char* szFormat, va_list arg)
	{
		if (base::crt::vsnprintf(this->m_szExtraInfo, sizeof(this->m_szExtraInfo), szFormat, arg) < 0)
		{
			PrintWarning("CInvokerTrace::addTraceExtraInfo");
			return;
		}

		PrintTrace("trace_id: "UINT64FMT" extra_info: %s time: "UINT64FMT, nTraceID, this->m_szExtraInfo, base::getGmtTime());
	}

	void CInvokerTrace::addTraceExtraInfo(const char* szFormat, ...)
	{
		va_list arg;
		va_start(arg, szFormat);
		this->addTraceExtraInfo(this->m_nCurTraceID, szFormat, arg);
		va_end(arg);
	}

	void CInvokerTrace::addTraceExtraInfo(uint64_t nTraceID, const char* szFormat, ...)
	{
		va_list arg;
		va_start(arg, szFormat);
		this->addTraceExtraInfo(nTraceID, szFormat, arg);
		va_end(arg);
	}

	void CInvokerTrace::beginRecv(uint64_t nTraceID, const std::string& szMessageName, const std::string& szFromServiceName)
	{
		PrintTrace("trace_id: "UINT64FMT" begin recv time: "UINT64FMT" message_name: %s service_name: %s from_service_name: %s",
			nTraceID, base::getGmtTime(), szMessageName.c_str(), CCoreServiceKitImpl::Inst()->getServiceBaseInfo().szName.c_str(), szFromServiceName.c_str());
		
		this->m_nCurTraceID = nTraceID;
	}

	void CInvokerTrace::endRecv()
	{
		PrintTrace("trace_id: "UINT64FMT" end recv time: "UINT64FMT, this->m_nCurTraceID, base::getGmtTime());
	}

	void CInvokerTrace::send(const std::string& szMessageName, const std::string& szToServiceName)
	{
		PrintTrace("trace_id: "UINT64FMT" send time: "UINT64FMT" message_id: %s service_name: %s to_service_name: %s",
			this->m_nCurTraceID, base::getGmtTime(), szMessageName.c_str(), CCoreServiceKitImpl::Inst()->getServiceBaseInfo().szName.c_str(), szToServiceName.c_str());
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