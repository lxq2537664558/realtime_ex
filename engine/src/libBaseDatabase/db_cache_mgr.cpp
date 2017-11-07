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
		, m_nCurIndex(1)
		, m_nDataSize(0)
		, m_nMaxCacheSize(0)
		, m_nLastCleanCacheTime(0)
		, m_nLastWritebackTime(0)
		, m_nWritebackTime(0)
	{

	}

	CDbCacheMgr::~CDbCacheMgr()
	{

	}

	bool CDbCacheMgr::init(CDbThread* pDbThread, const db::SDbOptions& sDbOptions)
	{
		DebugAstEx(pDbThread != nullptr, false);

		this->m_pDbThread = pDbThread;
		this->m_nMaxCacheSize = sDbOptions.nMaxCacheSize;
		this->m_nWritebackTime = sDbOptions.nCacheWritebackTime;
		for (size_t i = 0; i < sDbOptions.vecCacheTable.size(); ++i)
		{
			this->m_setAllowDataName.insert(getMessageNameByTableName(sDbOptions.vecCacheTable[i]));
		}

		return true;
	}

	uint32_t CDbCacheMgr::getDataID(const std::string& szDataName)
	{
		if (this->m_setAllowDataName.find(szDataName) == this->m_setAllowDataName.end())
			return 0;

		uint32_t nDataID = 0;
		auto iter = this->m_mapDataIndex.find(szDataName);
		if (iter == this->m_mapDataIndex.end())
		{
			nDataID = this->m_nCurIndex++;
			this->m_mapDataIndex[szDataName] = nDataID;
			this->m_mapDataName[nDataID] = szDataName;
		}
		else
		{
			nDataID = iter->second;
		}

		return nDataID;
	}

	const std::string& CDbCacheMgr::getDataName(uint32_t nIndex) const
	{
		auto iter = this->m_mapDataName.find(nIndex);
		if (iter == this->m_mapDataName.end())
			return s_szErrorName;

		return iter->second;
	}

	google::protobuf::Message* CDbCacheMgr::getData(uint64_t nID, const std::string& szDataName)
	{
		uint32_t nDataID = this->getDataID(szDataName);

		if (nDataID == 0)
			return nullptr;

		auto iter = this->m_mapCache.find(nID);
		if (iter == this->m_mapCache.end())
			return nullptr;

		auto pDbCache = iter->second;

		return pDbCache->getData(nDataID);
	}

	bool CDbCacheMgr::setData(uint64_t nID, const google::protobuf::Message* pData)
	{
		if (this->m_nMaxCacheSize <= 0 || this->m_nWritebackTime <= 0)
			return false;

		const std::string szDataName = pData->GetTypeName();

		uint32_t nDataID = this->getDataID(szDataName);
		if (nDataID == 0)
			return false;

		if (this->m_mapCache.find(nID) == this->m_mapCache.end())
			return this->addData(nID, pData);

		auto pDbCache = this->m_mapCache[nID];

		int32_t nSize = pDbCache->getDataSize();
		if (!pDbCache->setData(nDataID, szDataName, pData))
			return false;

		this->m_nDataSize -= nSize;
		this->m_nDataSize += pDbCache->getDataSize();

		this->m_mapDirtyCache[nID] = pDbCache;

		return true;
	}

	bool CDbCacheMgr::addData(uint64_t nID, const google::protobuf::Message* pData)
	{
		if (this->m_nMaxCacheSize <= 0 || this->m_nWritebackTime <= 0)
			return false;

		const std::string szDataName = pData->GetTypeName();

		uint32_t nDataID = this->getDataID(szDataName);
		if (nDataID == 0)
			return false;

		if (this->m_mapCache.find(nID) != this->m_mapCache.end())
			return false;

		auto pDbCache = std::make_shared<CDbCache>(this);
		this->m_mapCache[nID] = pDbCache;

		int32_t nSize = pDbCache->getDataSize();
		if (!pDbCache->addData(nDataID, pData))
			return false;

		this->m_nDataSize -= nSize;
		this->m_nDataSize += pDbCache->getDataSize();

		return true;
	}

	bool CDbCacheMgr::delData(uint64_t nID, const std::string& szDataName)
	{
		if (this->m_nMaxCacheSize <= 0 || this->m_nWritebackTime <= 0)
			return false;

		uint32_t nDataID = this->getDataID(szDataName);
		if (nDataID == 0)
			return false;

		auto iter = this->m_mapCache.find(nID);
		if (iter == this->m_mapCache.end())
			return false;

		auto pDbCache = iter->second;

		int32_t nSize = pDbCache->getDataSize();
		if (!pDbCache->delData(nDataID))
			return false;

		this->m_nDataSize -= nSize;
		this->m_nDataSize += pDbCache->getDataSize();

		return true;
	}

	void CDbCacheMgr::cleanCache(int64_t nTime)
	{
		if (this->m_nMaxCacheSize <= 0 || this->m_nWritebackTime <= 0)
			return;

		if (this->m_nLastCleanCacheTime != 0 && this->m_nLastCleanCacheTime - nTime < _CLEAN_CACHE_TIME)
			return;

		if (this->m_nDataSize < this->m_nMaxCacheSize)
			return;

		this->m_nLastCleanCacheTime = nTime;

		std::vector<std::unordered_map<uint64_t, std::shared_ptr<CDbCache>>::const_local_iterator> vecElement;
		vecElement.reserve(5);
		for (size_t i = 0; i < 5; ++i)
		{
			if (this->m_mapCache.bucket_count() == 0)
				break;

			int32_t nPos = rand() % this->m_mapCache.bucket_count();
			if (this->m_mapCache.begin(nPos) != this->m_mapCache.end(nPos))
			{
				vecElement.push_back(this->m_mapCache.begin(nPos));
			}
		}

		if (vecElement.empty())
			return;

		uint32_t nPos = base::CRandGen::getGlobalRand((uint32_t)vecElement.size());
		auto pDbCache = vecElement[nPos]->second;
		uint64_t nID = vecElement[nPos]->first;
		this->m_nDataSize -= pDbCache->getDataSize();
		this->m_mapCache.erase(nID);
		this->m_mapDirtyCache.erase(nID);
		pDbCache->writeback(0);
	}

	void CDbCacheMgr::writeback(uint64_t nTime)
	{
		if (this->m_nMaxCacheSize <= 0 || this->m_nWritebackTime <= 0)
			return;

		if (this->m_nLastWritebackTime != 0 && this->m_nLastWritebackTime - nTime < this->m_nWritebackTime)
			return;

		this->m_nLastWritebackTime = nTime;

		for (auto iter = this->m_mapDirtyCache.begin(); iter != this->m_mapDirtyCache.end();)
		{
			if (!iter->second->writeback(nTime))
				this->m_mapDirtyCache.erase(iter++);
			else
				++iter;
		}
	}

	void CDbCacheMgr::flushCache(uint64_t nKey, bool bDel)
	{
		if (this->m_nMaxCacheSize <= 0 || this->m_nWritebackTime <= 0)
			return;

		if (nKey != 0)
		{
			auto iter = this->m_mapCache.find(nKey);
			if (iter == this->m_mapCache.end())
				return;

			iter->second->writeback(0);
			this->m_mapCache.erase(iter);
		}
		else
		{
			this->writeback(0);
			this->m_mapCache.clear();
		}
	}

	CDbThread* CDbCacheMgr::getDbThread() const
	{
		return this->m_pDbThread;
	}

	int64_t CDbCacheMgr::getMaxCacheSize() const
	{
		return this->m_nMaxCacheSize;
	}

	void CDbCacheMgr::setMaxCacheSize(uint64_t nSize)
	{
		this->m_nMaxCacheSize = nSize;
	}

	void CDbCacheMgr::update()
	{
		int64_t nCurTime = base::time_util::getGmtTime();

		this->cleanCache(nCurTime);
		this->writeback(nCurTime);
	}

}