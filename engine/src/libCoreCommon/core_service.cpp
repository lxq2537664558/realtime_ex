#include "stdafx.h"
#include "core_service.h"
#include "service_base.h"
#include "core_app.h"
#include "actor_base.h"
#include "hash_service_selector.h"
#include "random_service_selector.h"

#include <mutex>

namespace core
{
	CCoreService::CCoreService()
		: m_eRunState(eSRS_Start)
		, m_pServiceInvoker(nullptr)
		, m_pActorScheduler(nullptr)
		, m_pMessageDispatcher(nullptr)
		, m_pServiceBase(nullptr)
	{

	}

	CCoreService::~CCoreService()
	{

	}

	bool CCoreService::init(CServiceBase* pServiceBase, const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName)
	{
		DebugAstEx(pServiceBase != nullptr, false);
		
		this->m_szConfigFileName = szConfigFileName;
		this->m_sServiceBaseInfo = sServiceBaseInfo;
		this->m_pServiceBase = pServiceBase;
		this->m_pServiceBase->m_pCoreService = this;
		
		this->m_pActorScheduler = new CActorScheduler(this);
		this->m_pServiceInvoker = new CServiceInvoker(pServiceBase);
		this->m_pMessageDispatcher = new CMessageDispatcher(this);

		this->m_mapServiceSelector[eSST_Random] = new CRandomServiceSelector();
		this->m_mapServiceSelector[eSST_Hash] = new CHashServiceSelector();

		return true;
	}

	void CCoreService::quit()
	{
		DebugAst(this->m_eRunState == eSRS_Normal);

		this->m_eRunState = eSRS_Quitting;
		this->m_pServiceBase->onQuit();
	}

	bool CCoreService::onInit()
	{
		if (!this->m_pServiceBase->onInit())
			return false;

		this->m_eRunState = eSRS_Normal;

		return true;
	}

	CServiceBase* CCoreService::getServiceBase() const
	{
		return this->m_pServiceBase;
	}

	uint32_t CCoreService::getServiceID() const
	{
		return this->m_sServiceBaseInfo.nID;
	}

	const SServiceBaseInfo& CCoreService::getServiceBaseInfo() const
	{
		return this->m_sServiceBaseInfo;
	}

	void CCoreService::registerServiceMessageHandler(const std::string& szMessageName, const std::function<void(CServiceBase*, SSessionInfo, const google::protobuf::Message*)>& callback)
	{
		this->m_mapServiceMessageHandler[szMessageName] = callback;
	}

	void CCoreService::registerServiceForwardHandler(const std::string& szMessageName, const std::function<void(CServiceBase*, SClientSessionInfo, const google::protobuf::Message*)>& callback)
	{
		uint32_t nMessageID = _GET_MESSAGE_ID(szMessageName);

		std::unique_lock<base::spin_lock> guard(this->m_lockForwardMessage);

		auto iter = this->m_mapForwardMessageName.find(nMessageID);
		if (iter != this->m_mapForwardMessageName.end() && szMessageName != iter->second)
		{
			PrintWarning("dup forward message id message_id: {} old_message_name: {} new_message_name: {}", nMessageID, iter->second, szMessageName);
			return;
		}

		this->m_mapForwardMessageName[nMessageID] = szMessageName;
		this->m_mapServiceForwardHandler[szMessageName] = callback;
	}

	void CCoreService::registerActorMessageHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SSessionInfo, const google::protobuf::Message*)>& callback)
	{
		this->m_mapActorMessageHandler[szMessageName] = callback;
	}

	void CCoreService::registerActorForwardHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)>& callback)
	{
		uint32_t nMessageID = _GET_MESSAGE_ID(szMessageName);

		std::unique_lock<base::spin_lock> guard(this->m_lockForwardMessage);

		auto iter = this->m_mapForwardMessageName.find(nMessageID);
		if (iter != this->m_mapForwardMessageName.end() && szMessageName != iter->second)
		{
			PrintWarning("dup forward message id message_id: {} old_message_name: {} new_message_name: {}", nMessageID, iter->second, szMessageName);
			return;
		}

		this->m_mapForwardMessageName[nMessageID] = szMessageName;
		this->m_mapActorForwardHandler[szMessageName] = callback;
	}

	std::function<void(CServiceBase*, SSessionInfo, const google::protobuf::Message*)>& CCoreService::getServiceMessageHandler(const std::string& szMessageName)
	{
		auto iter = this->m_mapServiceMessageHandler.find(szMessageName);
		if (iter == this->m_mapServiceMessageHandler.end())
		{
			static std::function<void(CServiceBase*, SSessionInfo, const google::protobuf::Message*)> callback;
			return callback;
		}

		return iter->second;
	}

	std::function<void(CServiceBase*, SClientSessionInfo, const google::protobuf::Message*)>& CCoreService::getServiceForwardHandler(const std::string& szMessageName)
	{
		auto iter = this->m_mapServiceForwardHandler.find(szMessageName);
		if (iter == this->m_mapServiceForwardHandler.end())
		{
			static std::function<void(CServiceBase*, SClientSessionInfo, const google::protobuf::Message*)> callback;
			return callback;
		}

		return iter->second;
	}

	std::function<void(CActorBase*, SSessionInfo, const google::protobuf::Message*)>& CCoreService::getActorMessageHandler(const std::string& szMessageName)
	{
		auto iter = this->m_mapActorMessageHandler.find(szMessageName);
		if (iter == this->m_mapActorMessageHandler.end())
		{
			static std::function<void(CActorBase*, SSessionInfo, const google::protobuf::Message*)> callback;
			return callback;
		}

		return iter->second;
	}

	std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)>& CCoreService::getActorForwardHandler(const std::string& szMessageName)
	{
		auto iter = this->m_mapActorForwardHandler.find(szMessageName);
		if (iter == this->m_mapActorForwardHandler.end())
		{
			static std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)> callback;
			return callback;
		}

		return iter->second;
	}

	void CCoreService::registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext)
	{
		uint64_t nCurWorkActorID = this->m_pActorScheduler->getCurWorkActorID();
		if (nCurWorkActorID != 0)
		{
			CCoreApp::Inst()->registerTicker(CTicker::eTT_Actor, this->getServiceID(), nCurWorkActorID, pTicker, nStartTime, nIntervalTime, nContext);
		}
		else
		{
			CCoreApp::Inst()->registerTicker(CTicker::eTT_Service, this->getServiceID(), 0, pTicker, nStartTime, nIntervalTime, nContext);
		}
	}

	void CCoreService::unregisterTicker(CTicker* pTicker)
	{
		CCoreApp::Inst()->unregisterTicker(pTicker);
	}

	void CCoreService::doQuit()
	{
		DebugAst(this->m_eRunState == eSRS_Quitting);

		this->m_eRunState = eSRS_Quit;
	}

	void CCoreService::run()
	{
		this->m_pServiceBase->onFrame();
	}

	void CCoreService::setServiceConnectCallback(const std::function<void(const std::string&, uint32_t)>& callback)
	{
		this->m_fnServiceConnectCallback = callback;
	}

	void CCoreService::setServiceDisconnectCallback(const std::function<void(const std::string&, uint32_t)>& callback)
	{
		this->m_fnServiceDisconnectCallback = callback;
	}

	std::function<void(const std::string&, uint32_t)>& CCoreService::getServiceConnectCallback()
	{
		return this->m_fnServiceConnectCallback;
	}

	std::function<void(const std::string&, uint32_t)>& CCoreService::getServiceDisconnectCallback()
	{
		return this->m_fnServiceDisconnectCallback;
	}

	CServiceInvoker* CCoreService::getServiceInvoker() const
	{
		return this->m_pServiceInvoker;
	}

	CActorScheduler* CCoreService::getActorScheduler() const
	{
		return this->m_pActorScheduler;
	}

	CMessageDispatcher* CCoreService::getMessageDispatcher() const
	{
		return this->m_pMessageDispatcher;
	}

	EServiceRunState CCoreService::getRunState() const
	{
		return this->m_eRunState;
	}

	const std::string& CCoreService::getConfigFileName() const
	{
		return this->m_szConfigFileName;
	}

	void CCoreService::setServiceSelector(uint32_t nType, CServiceSelector* pServiceSelector)
	{
		if (pServiceSelector == nullptr)
			this->m_mapServiceSelector.erase(nType);
		else
			this->m_mapServiceSelector[nType] = pServiceSelector;
	}

	CServiceSelector* CCoreService::getServiceSelector(uint32_t nType) const
	{
		auto iter = this->m_mapServiceSelector.find(nType);
		if (iter == this->m_mapServiceSelector.end())
			return nullptr;

		return iter->second;
	}

	const std::string& CCoreService::getForwardMessageName(uint32_t nMessageID)
	{
		std::unique_lock<base::spin_lock> guard(this->m_lockForwardMessage);

		auto iter = this->m_mapForwardMessageName.find(nMessageID);
		if (iter == this->m_mapForwardMessageName.end())
		{
			static std::string s_Default;
			return s_Default;
		}

		return iter->second;
	}

	void CCoreService::setToGateMessageCallback(const std::function<void(uint64_t, const void*, uint16_t)>& callback)
	{
		this->m_fnToGateMessageCallback = callback;
	}

	std::function<void(uint64_t, const void*, uint16_t)>& CCoreService::getToGateMessageCallback()
	{
		return this->m_fnToGateMessageCallback;
	}

	void CCoreService::setToGateBroadcastMessageCallback(const std::function<void(const uint64_t*, uint16_t, const void*, uint16_t)>& callback)
	{
		this->m_fnToGateBroadcastMessageCallback = callback;
	}

	std::function<void(const uint64_t*, uint16_t, const void*, uint16_t)>& CCoreService::getToGateBroadcastMessageCallback()
	{
		return this->m_fnToGateBroadcastMessageCallback;
	}
}