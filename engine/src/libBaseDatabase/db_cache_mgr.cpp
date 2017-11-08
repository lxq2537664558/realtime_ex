#include "db_cache_mgr.h"
#include "db_protobuf.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/time_util.h"
#include "libBaseCommon/rand_gen.h"

#define _CLEAN_CACHE_TIME 1
#define _CHECK_WRITE_BACK_TIME 1

namespace base
{
	namespace 
	{
		std::string s_szErrorName;
	}

	CDbCacheMgr::CDbCacheMgr()
		: m_pDbThread(nullptr)
		, m_pTickerMgr(nullptr)
		, m_nDataSize(0)
		, m_nMaxCacheSize(0)
		, m_nWritebackTime(0)
	{
	}

	CDbCacheMgr::~CDbCacheMgr()
	{
		SAFE_DELETE(this->m_pTickerMgr);
	}

	bool CDbCacheMgr::init(CDbThread* pDbThread, const db::SDbOptions& sDbOptions)
	{
		DebugAstEx(pDbThread != nullptr, false);

		this->m_pDbThread = pDbThread;
		this->m_nMaxCacheSize = sDbOptions.nMaxCacheSize;
		this->m_nWritebackTime = sDbOptions.nCacheWritebackTime;
		for (size_t i = 0; i < sDbOptions.vecCacheTable.size(); ++i)
		{
			std::string szDataName = getMessageNameByTableName(sDbOptions.vecCacheTable[i]);
			this->m_mapCache[szDataName] = new CDbCache(this, szDataName);
		}

		this->m_pTickerMgr = new CTickerMgr(base::time_util::getGmtTime(), nullptr);

		return true;
	}

	google::protobuf::Message* CDbCacheMgr::getData(uint64_t nID, const std::string& szDataName)
	{
		auto iter = this->m_mapCache.find(szDataName);
		if (iter == this->m_mapCache.end())
			return nullptr;

		CDbCache* pDbCache = iter->second;

		return pDbCache->getData(nID);
	}

	bool CDbCacheMgr::setData(uint64_t nID, const google::protobuf::Message* pData)
	{
		const std::string szDataName = pData->GetTypeName();

		auto iter = this->m_mapCache.find(szDataName);
		if (iter == this->m_mapCache.end())
			return false;

		CDbCache* pDbCache = iter->second;

		int32_t nSize = pDbCache->getDataSize();
		if (!pDbCache->setData(nID, pData))
			return false;

		this->m_nDataSize -= nSize;
		this->m_nDataSize += pDbCache->getDataSize();

		if (this->m_nDataSize > this->m_nMaxCacheSize)
			this->m_nDataSize -= pDbCache->cleanData();

		return true;
	}

	bool CDbCacheMgr::addData(uint64_t nID, const google::protobuf::Message* pData)
	{
		const std::string szDataName = pData->GetTypeName();

		auto iter = this->m_mapCache.find(szDataName);
		if (iter == this->m_mapCache.end())
			return false;

		CDbCache* pDbCache = iter->second;

		int32_t nSize = pDbCache->getDataSize();
		if (!pDbCache->addData(nID, pData))
			return false;

		this->m_nDataSize -= nSize;
		this->m_nDataSize += pDbCache->getDataSize();

		if (this->m_nDataSize > this->m_nMaxCacheSize)
			this->m_nDataSize -= pDbCache->cleanData();

		return true;
	}

	bool CDbCacheMgr::delData(uint64_t nID, const std::string& szDataName)
	{
		auto iter = this->m_mapCache.find(szDataName);
		if (iter == this->m_mapCache.end())
			return false;

		CDbCache* pDbCache = iter->second;

		int32_t nSize = pDbCache->getDataSize();
		if (!pDbCache->delData(nID))
			return false;

		this->m_nDataSize -= nSize;
		this->m_nDataSize += pDbCache->getDataSize();

		return true;
	}

	CTickerMgr* CDbCacheMgr::getTickerMgr() const
	{
		return this->m_pTickerMgr;
	}

	void CDbCacheMgr::flushCache()
	{
		for (auto iter = this->m_mapCache.begin(); iter != this->m_mapCache.end(); ++iter)
		{
			iter->second->flush();
		}
	}

	CDbThread* CDbCacheMgr::getDbThread() const
	{
		return this->m_pDbThread;
	}

	bool CDbCacheMgr::enableCache() const
	{
		return this->m_nMaxCacheSize > 0;
	}

	void CDbCacheMgr::update()
	{
		int64_t nCurTime = base::time_util::getGmtTime();

		this->m_pTickerMgr->update(nCurTime);
	}

	uint32_t CDbCacheMgr::getWritebackTime() const
	{
		return this->m_nWritebackTime;
	}
}