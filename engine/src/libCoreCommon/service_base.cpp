#include "stdafx.h"
#include "service_base.h"
#include "core_app.h"

namespace core
{
	CServiceBase::CServiceBase()
		: m_eState(eSRS_Start)
		, m_nServiceID(0)
		, m_pServiceInvoker(nullptr)
	{

	}

	CServiceBase::~CServiceBase()
	{

	}

	void CServiceBase::registerMessageHandler(const std::string& szMessageName, const std::function<void(SSessionInfo, google::protobuf::Message*)>& callback)
	{
		CCoreApp::Inst()->getCoreMessageRegistry()->registerCallback(this->m_nServiceID, szMessageName, callback);
	}

	void CServiceBase::registerForwardHandler(const std::string& szMessageName, const std::function<void(SClientSessionInfo, google::protobuf::Message*)>& callback)
	{
		CCoreApp::Inst()->getCoreMessageRegistry()->registerGateForwardCallback(this->m_nServiceID, szMessageName, callback);
	}

	void CServiceBase::registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext)
	{
		CCoreApp::Inst()->registerTicker(CTicker::eTT_Logic, 0, pTicker, nStartTime, nIntervalTime, nContext);
	}

	void CServiceBase::unregisterTicker(CTicker* pTicker)
	{
		CCoreApp::Inst()->unregisterTicker(pTicker);
	}

	void CServiceBase::doQuit()
	{
		DebugAst(this->m_eState == eSRS_Quitting);

		this->m_eState = eSRS_Quit;
	}

}