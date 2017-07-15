#pragma once

#include "core_common.h"
#include "core_common_define.h"
#include "service_base.h"
#include "actor_scheduler.h"
#include "service_invoker.h"
#include "logic_message_queue.h"
#include "message_dispatcher.h"

#include <map>
#include <atomic>


namespace core
{
	class CCoreApp;
	class CServiceBaseImpl :
		public base::noncopyable
	{
		friend class CCoreApp;

	public:
		CServiceBaseImpl();
		~CServiceBaseImpl();

		bool					init(const SServiceBaseInfo& sServiceBaseInfo, CServiceBase* pServiceBase);
		CServiceBase*			getServiceBase() const;

		const SServiceBaseInfo&	getServiceBaseInfo() const;

		void					registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		void					unregisterTicker(CTicker* pTicker);

		CServiceInvoker*		getServiceInvoker() const;
		CActorScheduler*		getActorScheduler() const;
		CLogicMessageQueue*		getMessageQueue() const;
		CMessageDispatcher*		getMessageDispatcher() const;

		void					setActorIDConverter(CActorIDConverter* pActorIDConverter);
		CActorIDConverter*		getActorIDConverter() const;

		bool					isWorking() const;

		void					setWorking(bool bFlag);
		
		void					registerServiceMessageHandler(const std::string& szMessageName, const std::function<void(SSessionInfo, google::protobuf::Message*)>& callback);
		void					registerServiceForwardHandler(const std::string& szMessageName, const std::function<void(SClientSessionInfo, google::protobuf::Message*)>& callback);
		
		void					registerActorMessageHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SSessionInfo, const google::protobuf::Message*)>& callback);
		void					registerActorForwardHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)>& callback);
		
		std::function<void(SSessionInfo, google::protobuf::Message*)>&
								getServiceMessageHandler(const std::string& szMessageName);
		std::function<void(SClientSessionInfo, google::protobuf::Message*)>&
								getServiceForwardHandler(const std::string& szMessageName);

		std::function<void(CActorBase*, SSessionInfo, google::protobuf::Message*)>&
								getActorMessageHandler(const std::string& szMessageName);
		std::function<void(CActorBase*, SClientSessionInfo, google::protobuf::Message*)>&
								getActorForwardHandler(const std::string& szMessageName);

		void					setServiceConnectCallback(const std::function<void(uint16_t)>& callback);
		void					setServiceDisconnectCallback(const std::function<void(uint16_t)>& callback);
		std::function<void(uint16_t)>&
								getServiceConnectCallback();
		std::function<void(uint16_t)>&
								getServiceDisconnectCallback();

		const std::string&		getConfigFileName() const;
		
		base::CWriteBuf&		getWriteBuf() const;
		
		uint32_t				getQPS() const;
		
		EServiceRunState		getRunState() const;
		void					setRunState(EServiceRunState eState);
		
		void					doQuit();

		void					run();

	private:
		SServiceBaseInfo	m_sServiceBaseInfo;
		CServiceBase*		m_pServiceBase;
		EServiceRunState	m_eState;
		CServiceInvoker*	m_pServiceInvoker;
		CActorScheduler*	m_pActorScheduler;
		CLogicMessageQueue*	m_pMessageQueue;
		CMessageDispatcher*	m_pMessageDispatcher;
		CActorIDConverter*	m_pActorIDConverter;
		std::atomic<bool>	m_bWorking;

		std::map<std::string, std::function<void(SSessionInfo, google::protobuf::Message*)>>		
							m_mapServiceMessageHandler;
		std::map<std::string, std::function<void(SClientSessionInfo, google::protobuf::Message*)>>	
							m_mapServiceForwardHandler;

		std::map<std::string, std::function<void(CActorBase*, SSessionInfo, const google::protobuf::Message*)>>
							m_mapActorMessageHandler;
		std::map<std::string, std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)>>
							m_mapActorForwardHandler;

		std::function<void(uint16_t)>
							m_fnServiceConnectCallback;
		std::function<void(uint16_t)>
							m_fnServiceDisconnectCallback;
	};
}