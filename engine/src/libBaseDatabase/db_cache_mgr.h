#pragma once
#include "db_cache.h"
#include "database.h"

#include "libBaseCommon/ticker.h"

#include <map>
#include <memory>

#include "google/protobuf/message.h"

namespace base
{
	/*
	CDbCacheMgr 类似于库的概念
	CDbCache 对应某一个表
	*/
	class CDbThread;
	class CDbCacheMgr
	{
	public:
		CDbCacheMgr();
		~CDbCacheMgr();

		bool				init(CDbThread* pDbThread, const db::SDbOptions& sDbOptions);
		google::protobuf::Message*
							getData(uint64_t nID, const std::string& szDataName);
		bool				setData(uint64_t nID, const google::protobuf::Message* pData);
		bool				addData(uint64_t nID, const google::protobuf::Message* pData);
		bool				delData(uint64_t nID, const std::string& szDataName);

		bool				enableCache() const;
		void				flushCache();
		CTickerMgr*			getTickerMgr() const;
		CDbThread*			getDbThread() const;
		void				update();

		uint32_t			getWritebackTime() const;

	private:
		CDbThread*							m_pDbThread;
		CTickerMgr*							m_pTickerMgr;
		std::map<std::string, CDbCache*>	m_mapCache;

		int64_t								m_nDataSize;
		
		uint32_t							m_nWritebackTime;
		int64_t								m_nMaxCacheSize;
	};
}