#include "stdafx.h"
#include "service_base_impl.h"
#include "service_base.h"
#include "core_app.h"

namespace core
{
	CServiceBaseImpl::CServiceBaseImpl()
		: m_eState(eSRS_Start)
		, m_pServiceInvoker(nullptr)
		, m_pActorScheduler(nullptr)
		, m_bWorking(false)
	{

	}

	CServiceBaseImpl::~CServiceBaseImpl()
	{

	}

	bool CServiceBaseImpl::init(const SServiceBaseInfo& sServiceBaseInfo, CServiceBase* pServiceBase)
	{
		DebugAstEx(pServiceBase != nullptr, false);

		this->m_sServiceBaseInfo = sServiceBaseInfo;
		this->m_pServiceBase = pServiceBase;

		this->m_pActorScheduler = new CActorScheduler(this);
		this->m_pServiceInvoker = new CServiceInvoker(this);

		if (!this->m_pServiceBase->onInit())
			return false;

		this->m_eState = eSRS_Normal;

		return true;
	}

	CServiceBase* CServiceBaseImpl::getServiceBase() const
	{
		return this->m_pServiceBase;
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
		
		this->m_pServiceBase->onFrame();
	}

	bool CServiceBaseImpl::isWorking() const
	{
		return this->m_bWorking.load(std::memory_order_relaxed);
	}

	void CServiceBaseImpl::setWorking(bool bFlag)
	{
		this->m_bWorking.store(bFlag, std::memory_order_relaxed);
	}

	void CServiceBaseImpl::setServiceConnectCallback(const std::function<void(uint16_t)>& callback)
	{
		this->m_fnServiceConnectCallback = callback;
	}

	void CServiceBaseImpl::setServiceDisconnectCallback(const std::function<void(uint16_t)>& callback)
	{
		this->m_fnServiceDisconnectCallback = callback;
	}

	std::function<void(uint16_t)>& CServiceBaseImpl::getServiceConnectCallback()
	{
		return this->m_fnServiceConnectCallback;
	}

	std::function<void(uint16_t)>& CServiceBaseImpl::getServiceDisconnectCallback()
	{
		return this->m_fnServiceDisconnectCallback;
	}
}