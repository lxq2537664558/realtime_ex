#pragma once
#include "db_thread.h"
#include "database.h"

#include <list>
#include <vector>

namespace base
{
	struct SDbConnectionInfo
	{
		std::string	szHost;
		uint16_t	nPort;
		std::string	szDb;
		std::string	szUser;
		std::string	szPassword;
		std::string	szCharacterset;
	};

	class CDbThreadMgr
	{
	public:
		CDbThreadMgr();
		~CDbThreadMgr();

		bool		init(const std::string& szHost, uint16_t nPort, const std::string& szDb, const std::string& szUser, const std::string& szPassword, const std::string& szCharacterset, uint32_t nDbThreadCount, const db::SCacheConfigInfo& sCacheConfigInfo);

		uint32_t	getThreadCount() const;
		void		query(uint32_t nThreadIndex, const SDbCommand& sDbCommand);
		const SDbConnectionInfo&
					getDbConnectionInfo() const;

		void		addResultInfo(const SDbResultInfo& sResultInfo);

		void		exit();

		void		update();

		uint32_t	getQPS(uint32_t nThreadIndex);
		uint32_t	getQueueSize(uint32_t nThreadIndex);

		void		setMaxCacheSize(uint64_t nSize);
		
	private:
		std::vector<CDbThread*>		m_vecDbThread;
		SDbConnectionInfo			m_sDbConnectionInfo;
		std::mutex					m_tResultLock;
		std::list<SDbResultInfo>	m_listResultInfo;
	};
}