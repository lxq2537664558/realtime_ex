#pragma once

#include "core_common.h"
#include "core_common_define.h"
#include "service_base.h"
#include "actor_scheduler.h"
#include "service_invoker.h"
#include "logic_message_queue.h"
#include "message_dispatcher.h"

#include <map>

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

		bool					init(CServiceBase* pServiceBase, const SServiceBaseInfo& sServiceBaseInfo);
		void					quit();

		void					run();

		bool					onInit();
		void					doQuit();

		CServiceBase*			getServiceBase() const;

		uint32_t				getServiceID() const;
		const SServiceBaseInfo&	getServiceBaseInfo() const;

		void					registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		void					unregisterTicker(CTicker* pTicker);

		CServiceInvoker*		getServiceInvoker() const;
		CActorScheduler*		getActorScheduler() const;
		CMessageDispatcher*		getMessageDispatcher() const;

		void					setActorIDConverter(CActorIDConverter* pActorIDConverter);
		CActorIDConverter*		getActorIDConverter() const;

		void					setServiceIDConverter(CServiceIDConverter* pServiceIDConverter);
		CServiceIDConverter*	getServiceIDConverter() const;

		void					setProtobufFactory(CProtobufFactory* pProtobufFactory);
		CProtobufFactory*		getProtobufFactory() const;

		void					registerServiceMessageHandler(const std::string& szMessageName, const std::function<void(CServiceBase*, SSessionInfo, const google::protobuf::Message*)>& callback);
		void					registerServiceForwardHandler(const std::string& szMessageName, const std::function<void(CServiceBase*, SClientSessionInfo, const google::protobuf::Message*)>& callback);
		
		void					registerActorMessageHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SSessionInfo, const google::protobuf::Message*)>& callback);
		void					registerActorForwardHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)>& callback);
		
		std::function<void(CServiceBase*, SSessionInfo, const google::protobuf::Message*)>&
								getServiceMessageHandler(const std::string& szMessageName);
		std::function<void(CServiceBase*, SClientSessionInfo, const google::protobuf::Message*)>&
								getServiceForwardHandler(const std::string& szMessageName);

		std::function<void(CActorBase*, SSessionInfo, const google::protobuf::Message*)>&
								getActorMessageHandler(const std::string& szMessageName);
		std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)>&
								getActorForwardHandler(const std::string& szMessageName);

		void					setServiceConnectCallback(const std::function<void(uint32_t)>& callback);
		void					setServiceDisconnectCallback(const std::function<void(uint32_t)>& callback);
		std::function<void(uint32_t)>&
								getServiceConnectCallback();
		std::function<void(uint32_t)>&
								getServiceDisconnectCallback();

		EServiceRunState		getRunState() const;
		
	private:
		SServiceBaseInfo		m_sServiceBaseInfo;
		CServiceBase*			m_pServiceBase;
		EServiceRunState		m_eRunState;
		CServiceInvoker*		m_pServiceInvoker;
		CActorScheduler*		m_pActorScheduler;
		CMessageDispatcher*		m_pMessageDispatcher;
		CActorIDConverter*		m_pActorIDConverter;
		CServiceIDConverter*	m_pServiceIDConverter;
		CProtobufFactory*		m_pProtobufFactory;
		
		std::map<std::string, std::function<void(CServiceBase*, SSessionInfo, const google::protobuf::Message*)>>
								m_mapServiceMessageHandler;
		std::map<std::string, std::function<void(CServiceBase*, SClientSessionInfo, const google::protobuf::Message*)>>
								m_mapServiceForwardHandler;

		std::map<std::string, std::function<void(CActorBase*, SSessionInfo, const google::protobuf::Message*)>>
								m_mapActorMessageHandler;
		std::map<std::string, std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)>>
								m_mapActorForwardHandler;

		std::function<void(uint32_t)>
								m_fnServiceConnectCallback;
		std::function<void(uint32_t)>
								m_fnServiceDisconnectCallback;
	};
}