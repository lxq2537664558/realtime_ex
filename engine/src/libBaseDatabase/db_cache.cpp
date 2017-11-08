#include "db_cache.h"
#include "db_protobuf.h"
#include "db_cache_mgr.h"
#include "db_thread.h"

#include "libBaseCommon/logger.h"
#include "libBaseCommon/defer.h"
#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/time_util.h"
#include "libBaseCommon/rand_gen.h"

namespace base
{
	CDbCache::CDbCache(CDbCacheMgr* pDbCacheMgr, const std::string szDataName)
		: m_pDbCacheMgr(pDbCacheMgr)
		, m_nDataSize(0)
		, m_szDataName(szDataName)
	{

	}

	CDbCache::~CDbCache()
	{

	}

	google::protobuf::Message* CDbCache::getData(uint64_t nID)
	{
		auto iter = this->m_mapCacheInfo.find(nID);
		if (iter == this->m_mapCacheInfo.end())
			return nullptr;

		google::protobuf::Message* pMessage = this->m_pDbCacheMgr->getDbThread()->createMessage(this->m_szDataName);
		if (nullptr == pMessage)
		{
			PrintWarning("CDbCache::getData error nullptr == pMessage data_name: {}", this->m_szDataName);
			return nullptr;
		}

		if (!pMessage->ParseFromArray(iter->second.szData.c_str(), (int32_t)iter->second.szData.size()))
		{
			PrintWarning("CDbCache::getData error pMessage->ParseFromArray data_name: {}", this->m_szDataName);
			this->m_pDbCacheMgr->getDbThread()->destroyMessage(pMessage);
			return nullptr;
		}

		return pMessage;
	}

	bool CDbCache::addData(uint64_t nID, const google::protobuf::Message* pData)
	{
		if (this->m_mapCacheInfo.find(nID) != this->m_mapCacheInfo.end())
			return false;

		std::string szData;
		if (!pData->SerializeToString(&szData))
			return false;

		int32_t nSize = (int32_t)szData.size();
		SCacheInfo& sCacheInfo = this->m_mapCacheInfo[nID];
		sCacheInfo.szData = std::move(szData);
		this->m_nDataSize += nSize;
		sCacheInfo.ticker = std::make_unique<CTicker>();
		sCacheInfo.ticker->setCallback(std::bind(&CDbCache::onBackup, this, std::placeholders::_1), false);

		return true;
	}

	bool CDbCache::setData(uint64_t nID, const google::protobuf::Message* pData)
	{
		DebugAstEx(pData != nullptr, false);
		
		auto iter = this->m_mapCacheInfo.find(nID);
		if (iter == this->m_mapCacheInfo.end())
		{
			this->addData(nID, pData);
			// 这里返回false是为了让db操作直接落到db上，而不是只在cache中，因为是新的数据。
			return false;
		}

		SCacheInfo& sCacheInfo = iter->second;

		google::protobuf::Message* pDstData = this->m_pDbCacheMgr->getDbThread()->createMessage(this->m_szDataName);
		if (nullptr == pDstData)
			return false;

		defer([&]()
		{
			this->m_pDbCacheMgr->getDbThread()->destroyMessage(pDstData);
		});

		if (!pDstData->ParseFromString(iter->second.szData))
			return false;

		const google::protobuf::Descriptor* pSrcDescriptor = pData->GetDescriptor();
		DebugAstEx(pSrcDescriptor != nullptr, false);

		const google::protobuf::Reflection* pSrcReflection = pData->GetReflection();
		DebugAstEx(pSrcReflection != nullptr, false);

		const google::protobuf::Descriptor* pDstDescriptor = pDstData->GetDescriptor();
		DebugAstEx(pDstDescriptor != nullptr, false);

		const google::protobuf::Reflection* pDstReflection = pDstData->GetReflection();
		DebugAstEx(pDstReflection != nullptr, false);

		for (int32_t i = 0; i < pSrcDescriptor->field_count(); ++i)
		{
			const google::protobuf::FieldDescriptor* pSrcFieldDescriptor = pSrcDescriptor->field(i);
			DebugAstEx(pSrcFieldDescriptor != nullptr, false);
			const google::protobuf::FieldDescriptor* pDstFieldDescriptor = pDstDescriptor->FindFieldByName(pSrcFieldDescriptor->name());
			DebugAstEx(pDstFieldDescriptor != nullptr, false);

			if (!pSrcReflection->HasField(*pData, pSrcFieldDescriptor))
				continue;

			switch (pSrcFieldDescriptor->type())
			{
			case google::protobuf::FieldDescriptor::TYPE_INT32:
			case google::protobuf::FieldDescriptor::TYPE_SINT32:
			case google::protobuf::FieldDescriptor::TYPE_SFIXED32:
				pDstReflection->SetInt32(pDstData, pDstFieldDescriptor, pSrcReflection->GetInt32(*pData, pSrcFieldDescriptor));
				break;

			case google::protobuf::FieldDescriptor::TYPE_UINT32:
			case google::protobuf::FieldDescriptor::TYPE_FIXED32:
				pDstReflection->SetUInt32(pDstData, pDstFieldDescriptor, pSrcReflection->GetUInt32(*pData, pSrcFieldDescriptor));
				break;

			case google::protobuf::FieldDescriptor::TYPE_INT64:
			case google::protobuf::FieldDescriptor::TYPE_SINT64:
			case google::protobuf::FieldDescriptor::TYPE_SFIXED64:
				pDstReflection->SetInt64(pDstData, pDstFieldDescriptor, pSrcReflection->GetInt64(*pData, pSrcFieldDescriptor));
				break;

			case google::protobuf::FieldDescriptor::TYPE_UINT64:
			case google::protobuf::FieldDescriptor::TYPE_FIXED64:
				pDstReflection->SetUInt64(pDstData, pDstFieldDescriptor, pSrcReflection->GetUInt64(*pData, pSrcFieldDescriptor));
				break;

			case google::protobuf::FieldDescriptor::TYPE_DOUBLE:
				pDstReflection->SetDouble(pDstData, pDstFieldDescriptor, pSrcReflection->GetDouble(*pData, pSrcFieldDescriptor));
				break;

			case google::protobuf::FieldDescriptor::TYPE_FLOAT:
				pDstReflection->SetFloat(pDstData, pDstFieldDescriptor, pSrcReflection->GetFloat(*pData, pSrcFieldDescriptor));
				break;

			case google::protobuf::FieldDescriptor::TYPE_STRING:
			case google::protobuf::FieldDescriptor::TYPE_BYTES:
				pDstReflection->SetString(pDstData, pDstFieldDescriptor, pSrcReflection->GetString(*pData, pSrcFieldDescriptor));
				break;

			case google::protobuf::FieldDescriptor::TYPE_MESSAGE:
			{
#ifdef GetMessage
#undef GetMessage
#endif
				google::protobuf::Message* pDstSubMessage = pDstReflection->MutableMessage(pDstData, pDstFieldDescriptor);
				const google::protobuf::Message& srcSubMessage = pSrcReflection->GetMessage(*pData, pSrcFieldDescriptor);
				pDstSubMessage->CopyFrom(srcSubMessage);
			}
			break;

			default:
				DebugAstEx(false, false);
			}
		}

		std::string szData;
		if (!pDstData->SerializeToString(&szData))
			return false;

		int32_t nSize = (int32_t)szData.size();
		this->m_nDataSize -= (int32_t)sCacheInfo.szData.size();
		sCacheInfo.szData = std::move(szData);
		this->m_nDataSize += nSize;

		if (!sCacheInfo.ticker->isRegister())
			this->m_pDbCacheMgr->getTickerMgr()->registerTicker(sCacheInfo.ticker.get(), this->m_pDbCacheMgr->getWritebackTime(), 0, 0);

		return true;
	}

	bool CDbCache::delData(uint64_t nID)
	{
		auto iter = this->m_mapCacheInfo.find(nID);
		if (iter == this->m_mapCacheInfo.end())
			return false;

		this->m_nDataSize -= (int32_t)iter->second.szData.size();
		this->m_mapCacheInfo.erase(nID);

		return true;
	}

	int32_t CDbCache::getDataSize() const
	{
		return this->m_nDataSize;
	}

	void CDbCache::flush()
	{
		for (auto iter = this->m_mapCacheInfo.begin(); iter != this->m_mapCacheInfo.end(); ++iter)
		{
			this->onBackup(iter->first);
		}
	}

	int32_t CDbCache::cleanData()
	{
		std::vector<std::unordered_map<uint64_t, SCacheInfo>::local_iterator> vecElement;
		vecElement.reserve(5);
		for (size_t i = 0; i < 5; ++i)
		{
			if (this->m_mapCacheInfo.bucket_count() == 0)
				break;

			int32_t nPos = rand() % this->m_mapCacheInfo.bucket_count();
			if (this->m_mapCacheInfo.begin(nPos) != this->m_mapCacheInfo.end(nPos))
			{
				vecElement.push_back(this->m_mapCacheInfo.begin(nPos));
			}
		}

		if (vecElement.empty())
			return 0;

		uint32_t nPos = base::CRandGen::getGlobalRand((uint32_t)vecElement.size());
		SCacheInfo& sCacheInfo = vecElement[nPos]->second;
		uint64_t nID = vecElement[nPos]->first;

		if (sCacheInfo.ticker->isRegister())
			this->onBackup(nID);

		int32_t nDataSize = (int32_t)sCacheInfo.szData.size();
		this->m_nDataSize -= nDataSize;

		this->m_mapCacheInfo.erase(nID);

		return nDataSize;
	}

	void CDbCache::onBackup(uint64_t nContext)
	{
		auto iter = this->m_mapCacheInfo.find(nContext);
		if (iter == this->m_mapCacheInfo.end())
			return;

		SCacheInfo& sCacheInfo = iter->second;

		google::protobuf::Message* pMessage = this->m_pDbCacheMgr->getDbThread()->createMessage(this->m_szDataName);
		if (nullptr == pMessage)
		{
			PrintWarning("CDbCache::writeback error nullptr == pMessage data_name: {}", this->m_szDataName);
			return;
		}

		defer([&]()
		{
			this->m_pDbCacheMgr->getDbThread()->destroyMessage(pMessage);
		});

		if (!pMessage->ParseFromString(sCacheInfo.szData))
		{
			PrintWarning("CDbCache::writeback error pMessage->ParseFromString data_name: {}", this->m_szDataName);
			return;
		}

		uint32_t nErrorCode = m_pDbCacheMgr->getDbThread()->getDbCommandHandlerProxy()->onDbCommand(db::eDBCT_Update, pMessage, nullptr);
		if (nErrorCode != db::eDBRC_OK)
		{
			PrintWarning("CDbCache::writeback error nErrorCode != db::eDBRC_OK data_name: {} error_code: {}", this->m_szDataName, nErrorCode);
		}
	}
}