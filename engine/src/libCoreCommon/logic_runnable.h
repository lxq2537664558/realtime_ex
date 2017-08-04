#pragma once

#include "libBaseCommon/thread_base.h"

#include "logic_message_queue.h"
#include "base_connection_mgr.h"
#include "base_connection_mgr_impl.h"
#include "node_connection_factory.h"
#include "core_service_mgr.h"
#include "transporter.h"
#include "service_registry_proxy.h"

#include "tinyxml2/tinyxml2.h"

namespace core
{
	class CLogicRunnable :
		public base::IRunnable
	{
	public:
		CLogicRunnable();
		virtual ~CLogicRunnable();

		bool					init(tinyxml2::XMLElement* pRootXML);

		CLogicMessageQueue*		getMessageQueue() const;
		CBaseConnectionMgr*		getBaseConnectionMgr() const;
		CBaseConnectionMgrImpl*	getBaseConnectionMgrImpl() const;
		CCoreServiceMgr*		getCoreServiceMgr() const;
		CTransporter*			getTransporter() const;
		CServiceRegistryProxy*	getServiceRegistryProxy() const;

		void					sendInsideMessage(const SMessagePacket& sMessagePacket);
		void					recvInsideMessage(std::vector<SMessagePacket>& vecMessagePacket);
		
		void					release();

	private:
		virtual bool			onInit();
		virtual bool			onProcess();
		virtual void			onDestroy();

		bool					dispatch(const SMessagePacket& sMessagePacket);

	private:
		base::CThreadBase*							m_pThreadBase;
		CBaseConnectionMgr*							m_pBaseConnectionMgr;
		CBaseConnectionMgrImpl*						m_pBaseConnectionMgrImpl;
		CLogicMessageQueue*							m_pMessageQueue;
		CCoreServiceMgr*							m_pCoreServiceMgr;
		CTransporter*								m_pTransporter;
		CServiceRegistryProxy*						m_pServiceRegistryProxy;
		CNodeConnectionFactory*						m_pNodeConnectionFactory;
		base::CCircleQueue<SMessagePacket, false>	m_insideQueue;

		int64_t										m_nTotalSamplingTime;
	};
}