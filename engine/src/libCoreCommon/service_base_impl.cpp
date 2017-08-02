#include "stdafx.h"
#include "service_base_impl.h"
#include "service_base.h"
#include "core_app.h"
#include "actor_base.h"
#include "hash_service_selector.h"
#include "random_service_selector.h"

#include <mutex>

namespace core
{
	CServiceBaseImpl::CServiceBaseImpl()
		: m_eRunState(eSRS_Start)
		, m_pServiceInvoker(nullptr)
		, m_pActorScheduler(nullptr)
		, m_pMessageDispatcher(nullptr)
		, m_pServiceBase(nullptr)
		, m_pActorIDConverter(nullptr)
		, m_pServiceIDConverter(nullptr)
		, m_pProtobufFactory(nullptr)
	{

	}

	CServiceBaseImpl::~CServiceBaseImpl()
	{

	}

	bool CServiceBaseImpl::init(CServiceBase* pServiceBase, const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName)
	{
		DebugAstEx(pServiceBase != nullptr, false);
		
		this->m_szConfigFileName = szConfigFileName;
		this->m_sServiceBaseInfo = sServiceBaseInfo;
		this->m_pServiceBase = pServiceBase;
		this->m_pServiceBase->m_pServiceBaseImpl = this;
		
		this->m_pActorScheduler = new CActorScheduler(this);
		this->m_pServiceInvoker = new CServiceInvoker(pServiceBase);
		this->m_pMessageDispatcher = new CMessageDispatcher(this);

		this->m_mapServiceSelector[eSST_Hash] = new CHashServiceSelector();
		this->m_mapServiceSelector[eSST_Random] = new CRandomServiceSelector();

		return true;
	}

	void CServiceBaseImpl::quit()
	{
		DebugAst(this->m_eRunState == eSRS_Normal);

		this->m_eRunState = eSRS_Quitting;
		this->m_pServiceBase->onQuit();
	}

	bool CServiceBaseImpl::onInit()
	{
		if (!this->m_pServiceBase->onInit())
			return false;

		this->m_eRunState = eSRS_Normal;

		return true;
	}

	CServiceBase* CServiceBaseImpl::getServiceBase() const
	{
		return this->m_pServiceBase;
	}

	uint32_t CServiceBaseImpl::getServiceID() const
	{
		return this->m_sServiceBaseInfo.nID;
	}

	const SServiceBaseInfo& CServiceBaseImpl::getServiceBaseInfo() const
	{
		return this->m_sServiceBaseInfo;
	}

	void CServiceBaseImpl::registerServiceMessageHandler(const std::string& szMessageName, const std::function<void(CServiceBase*, SSessionInfo, const google::protobuf::Message*)>& callback)
	{
		this->m_mapServiceMessageHandler[szMessageName] = callback;
	}

	void CServiceBaseImpl::registerServiceForwardHandler(const std::string& szMessageName, const std::function<void(CServiceBase*, SClientSessionInfo, const google::protobuf::Message*)>& callback)
	{
		uint32_t nMessageID = base::hash(szMessageName.c_str());

		std::unique_lock<base::spin_lock> guard(this->m_lockForwardMessage);

		auto iter = this->m_mapForwardMessageName.find(nMessageID);
		if (iter != this->m_mapForwardMessageName.end() && szMessageName != iter->second)
		{
			PrintWarning("dup forward message id message_id: %d old_message_name: %s new_message_name: %s", nMessageID, iter->second.c_str(), szMessageName.c_str());
			return;
		}

		this->m_mapForwardMessageName[nMessageID] = szMessageName;
		this->m_mapServiceForwardHandler[szMessageName] = callback;
	}

	void CServiceBaseImpl::registerActorMessageHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SSessionInfo, const google::protobuf::Message*)>& callback)
	{
		this->m_mapActorMessageHandler[szMessageName] = callback;
	}

	void CServiceBaseImpl::registerActorForwardHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)>& callback)
	{
		uint32_t nMessageID = base::hash(szMessageName.c_str());

		std::unique_lock<base::spin_lock> guard(this->m_lockForwardMessage);

		auto iter = this->m_mapForwardMessageName.find(nMessageID);
		if (iter != this->m_mapForwardMessageName.end() && szMessageName != iter->second)
		{
			PrintWarning("dup forward message id message_id: %d old_message_name: %s new_message_name: %s", nMessageID, iter->second.c_str(), szMessageName.c_str());
			return;
		}

		this->m_mapForwardMessageName[nMessageID] = szMessageName;
		this->m_mapActorForwardHandler[szMessageName] = callback;
	}

	std::function<void(CServiceBase*, SSessionInfo, const google::protobuf::Message*)>& CServiceBaseImpl::getServiceMessageHandler(const std::string& szMessageName)
	{
		auto iter = this->m_mapServiceMessageHandler.find(szMessageName);
		if (iter == this->m_mapServiceMessageHandler.end())
		{
			static std::function<void(CServiceBase*, SSessionInfo, const google::protobuf::Message*)> callback;
			return callback;
		}

		return iter->second;
	}

	std::function<void(CServiceBase*, SClientSessionInfo, const google::protobuf::Message*)>& CServiceBaseImpl::getServiceForwardHandler(const std::string& szMessageName)
	{
		auto iter = this->m_mapServiceForwardHandler.find(szMessageName);
		if (iter == this->m_mapServiceForwardHandler.end())
		{
			static std::function<void(CServiceBase*, SClientSessionInfo, const google::protobuf::Message*)> callback;
			return callback;
		}

		return iter->second;
	}

	std::function<void(CActorBase*, SSessionInfo, const google::protobuf::Message*)>& CServiceBaseImpl::getActorMessageHandler(const std::string& szMessageName)
	{
		auto iter = this->m_mapActorMessageHandler.find(szMessageName);
		if (iter == this->m_mapActorMessageHandler.end())
		{
			static std::function<void(CActorBase*, SSessionInfo, const google::protobuf::Message*)> callback;
			return callback;
		}

		return iter->second;
	}

	std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)>& CServiceBaseImpl::getActorForwardHandler(const std::string& szMessageName)
	{
		auto iter = this->m_mapActorForwardHandler.find(szMessageName);
		if (iter == this->m_mapActorForwardHandler.end())
		{
			static std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)> callback;
			return callback;
		}

		return iter->second;
	}

	void CServiceBaseImpl::registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext)
	{
		CCoreApp::Inst()->registerTicker(CTicker::eTT_Service, this->getServiceID(), 0, pTicker, nStartTime, nIntervalTime, nContext);
	}

	void CServiceBaseImpl::unregisterTicker(CTicker* pTicker)
	{
		CCoreApp::Inst()->unregisterTicker(pTicker);
	}

	void CServiceBaseImpl::doQuit()
	{
		DebugAst(this->m_eRunState == eSRS_Quitting);

		this->m_eRunState = eSRS_Quit;
	}

	void CServiceBaseImpl::run()
	{
		this->m_pActorScheduler->run();
		
		this->m_pServiceBase->onFrame();
	}

	void CServiceBaseImpl::setServiceConnectCallback(const std::function<void(const std::string&, uint32_t)>& callback)
	{
		this->m_fnServiceConnectCallback = callback;
	}

	void CServiceBaseImpl::setServiceDisconnectCallback(const std::function<void(const std::string&, uint32_t)>& callback)
	{
		this->m_fnServiceDisconnectCallback = callback;
	}

	std::function<void(const std::string&, uint32_t)>& CServiceBaseImpl::getServiceConnectCallback()
	{
		return this->m_fnServiceConnectCallback;
	}

	std::function<void(const std::string&, uint32_t)>& CServiceBaseImpl::getServiceDisconnectCallback()
	{
		return this->m_fnServiceDisconnectCallback;
	}

	CServiceInvoker* CServiceBaseImpl::getServiceInvoker() const
	{
		return this->m_pServiceInvoker;
	}

	CActorScheduler* CServiceBaseImpl::getActorScheduler() const
	{
		return this->m_pActorScheduler;
	}

	CMessageDispatcher* CServiceBaseImpl::getMessageDispatcher() const
	{
		return this->m_pMessageDispatcher;
	}

	void CServiceBaseImpl::setActorIDConverter(CActorIDConverter* pActorIDConverter)
	{
		this->m_pActorIDConverter = pActorIDConverter;
	}

	CActorIDConverter* CServiceBaseImpl::getActorIDConverter() const
	{
		return this->m_pActorIDConverter;
	}

	void CServiceBaseImpl::setServiceIDConverter(CServiceIDConverter* pServiceIDConverter)
	{
		this->m_pServiceIDConverter = pServiceIDConverter;
	}

	CServiceIDConverter* CServiceBaseImpl::getServiceIDConverter() const
	{
		return this->m_pServiceIDConverter;
	}

	EServiceRunState CServiceBaseImpl::getRunState() const
	{
		return this->m_eRunState;
	}

	void CServiceBaseImpl::setProtobufFactory(CProtobufFactory* pProtobufFactory)
	{
		this->m_pProtobufFactory = pProtobufFactory;
	}

	CProtobufFactory* CServiceBaseImpl::getProtobufFactory() const
	{
		return this->m_pProtobufFactory;
	}

	const std::string& CServiceBaseImpl::getConfigFileName() const
	{
		return this->m_szConfigFileName;
	}

	void CServiceBaseImpl::addServiceSelector(CServiceSelector* pServiceSelector)
	{
		DebugAst(pServiceSelector != nullptr);

		this->m_mapServiceSelector[pServiceSelector->getType()] = pServiceSelector;
	}

	CServiceSelector* CServiceBaseImpl::getServiceSelector(uint32_t nType) const
	{
		auto iter = this->m_mapServiceSelector.find(nType);
		if (iter == this->m_mapServiceSelector.end())
			return nullptr;

		return iter->second;
	}

	const std::string& CServiceBaseImpl::getForwardMessageName(uint32_t nMessageID)
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
}