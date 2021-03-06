#pragma once

#include "libBaseCommon/thread_base.h"

#include "db_connection.h"
#include "db_cache_mgr.h"
#include "db_command_handler_proxy.h"

#include <list>
#include <mutex>
#include <condition_variable>

namespace base
{
	class CDbThreadMgr;
	class CDbThread :
		public IRunnable
	{
		friend class CDbThreadMgr;

	public:
		CDbThread();
		~CDbThread();

		bool		init(CDbThreadMgr* pDbThreadMgr, const db::SDbOptions& sDbOptions);
		void		query(const SDbCommand& sDbCommand);
		bool		isConnectDb() const;
		uint32_t	getQueueSize();
		CDbCommandHandlerProxy*
					getDbCommandHandlerProxy() const;
		google::protobuf::Message*
					createMessage(const std::string& szMessageName);
		void		destroyMessage(google::protobuf::Message* pMessage);

	private:
		bool		connectDb(bool bInit);
		bool		onProcess();
		void		onDestroy();

		bool		onPreCache(uint32_t nType, const google::protobuf::Message* pRequest, std::shared_ptr<google::protobuf::Message>& pResponse);
		void		onPostCache(uint32_t nType, const google::protobuf::Message* pRequest, std::shared_ptr<google::protobuf::Message>& pResponse);
		void		flushCache();

	private:
		volatile uint32_t		m_quit;
		std::condition_variable	m_condition;
		std::mutex				m_tCommandLock;
		std::list<SDbCommand>	m_listCommand;
		CDbConnection			m_dbConnection;
		CDbCommandHandlerProxy*	m_pCommandHandlerProxy;
		CDbThreadMgr*			m_pDbThreadMgr;
		CThreadBase*			m_pThreadBase;
		CDbCacheMgr				m_dbCacheMgr;
	};
}