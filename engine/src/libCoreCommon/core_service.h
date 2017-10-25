#pragma once

#include "core_common.h"
#include "core_common_define.h"
#include "service_base.h"
#include "service_invoker.h"
#include "logic_message_queue.h"
#include "message_dispatcher.h"
#include "base_connection_mgr.h"
#include "local_service_registry_proxy.h"
#include "transporter.h"

#include "libBaseCommon/spin_lock.h"

#include <map>
#include <set>

namespace core
{
	class CCoreApp;
	class CCoreService :
		public base::noncopyable
	{
		friend class CCoreApp;

	public:
		CCoreService(CServiceBase* pServiceBase, const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName);
		~CCoreService();

		void				quit();

		void				onFrame();

		bool				onInit();
		void				doQuit();

		CServiceBase*		getServiceBase() const;

		uint32_t			getServiceID() const;
		const SServiceBaseInfo&
							getServiceBaseInfo() const;

		CServiceInvoker*	getServiceInvoker() const;
		CMessageDispatcher*	getMessageDispatcher() const;

		CLogicMessageQueue*	getMessageQueue() const;

		CTransporter*		getTransporter() const;

		void				setServiceSelector(uint32_t nType, CServiceSelector* pServiceSelector);
		CServiceSelector*	getServiceSelector(uint32_t nType) const;

		CBaseConnectionMgr*	getBaseConnectionMgr() const;
		
		void				setToGateMessageCallback(const std::function<void(uint64_t, const void*, uint16_t)>& callback);
		std::function<void(uint64_t, const void*, uint16_t)>&
							getToGateMessageCallback();

		void				setToGateBroadcastMessageCallback(const std::function<void(const uint64_t*, uint16_t, const void*, uint16_t)>& callback);
		std::function<void(const uint64_t*, uint16_t, const void*, uint16_t)>&
							getToGateBroadcastMessageCallback();

		void				registerServiceMessageHandler(const std::string& szMessageName, const std::function<void(CServiceBase*, SSessionInfo, const void*)>& callback);
		void				registerServiceForwardHandler(const std::string& szMessageName, const std::function<void(CServiceBase*, SClientSessionInfo, const void*)>& callback);
		
		std::function<void(CServiceBase*, SSessionInfo, const void*)>&
							getServiceMessageHandler(const std::string& szMessageName);
		std::function<void(CServiceBase*, SClientSessionInfo, const void*)>&
							getServiceForwardHandler(const std::string& szMessageName);

		const std::string&	getForwardMessageName(uint32_t nMessageID);

		void				setServiceConnectCallback(const std::function<void(const std::string&, uint32_t)>& callback);
		void				setServiceDisconnectCallback(const std::function<void(const std::string&, uint32_t)>& callback);
		std::function<void(const std::string&, uint32_t)>&
							getServiceConnectCallback();
		std::function<void(const std::string&, uint32_t)>&
							getServiceDisconnectCallback();

		EServiceRunState	getRunState() const;
		const std::string&	getConfigFileName() const;
		
		bool				isServiceHealth(uint32_t nServiceID) const;
		void				updateServiceHealth(uint32_t nServiceID, bool bTimeout);
		
		SPendingResponseInfo*
							getPendingResponseInfo(uint64_t nSessionID);
		SPendingResponseInfo*
							addPendingResponseInfo(uint32_t nToServiceID, uint64_t nSessionID, uint64_t nCoroutineID, const std::function<void(std::shared_ptr<void>, uint32_t)>& callback, uint32_t nTimeout, uint64_t nHolderID);
		void				delPendingResponseInfo(uint64_t nHolderID);

		uint64_t			genSessionID();

		void				addServiceMessageSerializer(CMessageSerializer* pMessageSerializer);
		void				setServiceMessageSerializer(uint32_t nServiceID, uint32_t nType);
		void				setForwardMessageSerializer(CMessageSerializer* pMessageSerializer);
		uint32_t			getServiceMessageSerializerType(uint32_t nServiceID) const;
		CMessageSerializer*	getServiceMessageSerializer(uint32_t nServiceID) const;
		CMessageSerializer*	getServiceMessageSerializerByType(uint8_t nType) const;
		CMessageSerializer*	getForwardMessageSerializer() const;

		uint32_t			getQPS() const;
		void				incQPS();

		CLocalServiceRegistryProxy*
							getLocalServiceRegistryProxy() const;

	private:
		void				onCheckServiceHealth(uint64_t nContext);
		void				onRequestMessageTimeout(uint64_t nContext);
		void				onQPS(uint64_t nContext);

	private:
		SServiceBaseInfo		m_sServiceBaseInfo;
		std::string				m_szConfigFileName;
		CServiceBase*			m_pServiceBase;
		EServiceRunState		m_eRunState;
		CServiceInvoker*		m_pServiceInvoker;
		CMessageDispatcher*		m_pMessageDispatcher;
		CLogicMessageQueue*		m_pMessageQueue;
		CBaseConnectionMgr*		m_pBaseConnectionMgr;
		CTransporter*			m_pTransporter;
		CLocalServiceRegistryProxy*
								m_pLocalServiceRegistryProxy;

		std::atomic<uint32_t>	m_nQPS;
		uint32_t				m_nCurQPS;
		CTicker					m_tickerQPS;

		CTicker					m_tickerCheckHealth;

		uint64_t				m_nNextSessionID;
		std::map<uint64_t, SPendingResponseInfo*>	
								m_mapPendingResponseInfo;
		std::map<uint64_t, std::list<uint64_t>>
								m_mapHolderSessionIDList;

		std::map<uint32_t, int32_t>	
								m_mapServiceHealth;	// 服务健康度

		std::map<std::string, std::function<void(CServiceBase*, SSessionInfo, const void*)>>
								m_mapServiceMessageHandler;
		std::map<std::string, std::function<void(CServiceBase*, SClientSessionInfo, const void*)>>
								m_mapServiceForwardHandler;

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

		std::map<uint32_t, CMessageSerializer*>
								m_mapMessageSerializer;
		std::map<uint32_t, uint32_t>
								m_mapServiceMessageSerializerType;
		uint32_t				m_nDefaultServiceMessageSerializerType;
		CMessageSerializer*		m_pForwardMessageSerializer;
	};
}