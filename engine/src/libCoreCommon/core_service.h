#pragma once

#include "core_common.h"
#include "core_common_define.h"
#include "service_base.h"
#include "actor_scheduler.h"
#include "service_invoker.h"
#include "logic_message_queue.h"
#include "message_dispatcher.h"

#include "libBaseCommon/spin_lock.h"

#include <map>

namespace core
{
	class CCoreApp;
	class CCoreService :
		public base::noncopyable
	{
		friend class CCoreApp;

	public:
		CCoreService();
		~CCoreService();

		bool					init(CServiceBase* pServiceBase, const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName);
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

		void					setServiceSelector(uint32_t nType, CServiceSelector* pServiceSelector);
		CServiceSelector*		getServiceSelector(uint32_t nType) const;

		void					setToGateMessageCallback(const std::function<void(uint64_t, const void*, uint16_t)>& callback);
		std::function<void(uint64_t, const void*, uint16_t)>&
								getToGateMessageCallback();

		void					setToGateBroadcastMessageCallback(const std::function<void(const uint64_t*, uint16_t, const void*, uint16_t)>& callback);
		std::function<void(const uint64_t*, uint16_t, const void*, uint16_t)>&
								getToGateBroadcastMessageCallback();

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

		const std::string&		getForwardMessageName(uint32_t nMessageID);

		void					setServiceConnectCallback(const std::function<void(const std::string&, uint32_t)>& callback);
		void					setServiceDisconnectCallback(const std::function<void(const std::string&, uint32_t)>& callback);
		std::function<void(const std::string&, uint32_t)>&
								getServiceConnectCallback();
		std::function<void(const std::string&, uint32_t)>&
								getServiceDisconnectCallback();

		EServiceRunState		getRunState() const;
		const std::string&		getConfigFileName() const;
		
	private:
		SServiceBaseInfo		m_sServiceBaseInfo;
		std::string				m_szConfigFileName;
		CServiceBase*			m_pServiceBase;
		EServiceRunState		m_eRunState;
		CServiceInvoker*		m_pServiceInvoker;
		CActorScheduler*		m_pActorScheduler;
		CMessageDispatcher*		m_pMessageDispatcher;
		
		std::map<std::string, std::function<void(CServiceBase*, SSessionInfo, const google::protobuf::Message*)>>
								m_mapServiceMessageHandler;
		std::map<std::string, std::function<void(CServiceBase*, SClientSessionInfo, const google::protobuf::Message*)>>
								m_mapServiceForwardHandler;

		std::map<std::string, std::function<void(CActorBase*, SSessionInfo, const google::protobuf::Message*)>>
								m_mapActorMessageHandler;
		std::map<std::string, std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)>>
								m_mapActorForwardHandler;

		std::map<uint32_t, std::string>
								m_mapForwardMessageName;
		base::spin_lock			m_lockForwardMessage;	// 这个冲突概率非常小，直接用旋转锁，这样性能比读写锁还高

		std::function<void(const std::string&, uint32_t)>
								m_fnServiceConnectCallback;
		std::function<void(const std::string&, uint32_t)>
								m_fnServiceDisconnectCallback;

		std::function<void(uint64_t, const void*, uint16_t)>
								m_fnToGateMessageCallback;
		std::function<void(const uint64_t*, uint16_t, const void*, uint16_t)>
								m_fnToGateBroadcastMessageCallback;

		std::map<uint32_t, CServiceSelector*>
								m_mapServiceSelector;
	};
}