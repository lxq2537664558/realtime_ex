#include "stdafx.h"
#include "service_base_impl.h"
#include "service_base.h"
#include "core_app.h"

namespace core
{
	CServiceBaseImpl::CServiceBaseImpl()
		: m_eState(eSRS_Start)
		, m_nServiceID(0)
		, m_pServiceInvoker(nullptr)
		, m_pActorScheduler(nullptr)
	{

	}

	CServiceBaseImpl::~CServiceBaseImpl()
	{

	}

	bool CServiceBaseImpl::init(uint16_t nServiceID)
	{
		this->m_nServiceID = nServiceID;

		this->m_pActorScheduler = new CActorScheduler(this);
		this->m_pServiceInvoker = new CServiceInvoker(nServiceID);

		return true;
	}

	void CServiceBaseImpl::registerServiceMessageHandler(const std::string& szMessageName, const std::function<void(SSessionInfo, google::protobuf::Message*)>& callback)
	{
		this->m_mapServiceMessageHandler[szMessageName] = callback;
	}

	void CServiceBaseImpl::registerServiceForwardHandler(const std::string& szMessageName, const std::function<void(SClientSessionInfo, google::protobuf::Message*)>& callback)
	{
		this->m_mapServiceForwardHandler[szMessageName] = callback;
	}

	void CServiceBaseImpl::registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext)
	{
		CCoreApp::Inst()->registerTicker(CTicker::eTT_Service, this->getServiceBaseInfo().nID, pTicker, nStartTime, nIntervalTime, nContext);
	}

	void CServiceBaseImpl::unregisterTicker(CTicker* pTicker)
	{
		CCoreApp::Inst()->unregisterTicker(pTicker);
	}

	void CServiceBaseImpl::doQuit()
	{
		DebugAst(this->m_eState == eSRS_Quitting);

		this->m_eState = eSRS_Quit;
	}

	void CServiceBaseImpl::run()
	{
		this->m_pActorScheduler->run();
	}

}