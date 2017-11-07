#include "db_cache.h"
#include "db_protobuf.h"
#include "db_cache_mgr.h"
#include "db_thread.h"

#include "libBaseCommon/logger.h"
#include "libBaseCommon/defer.h"
#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/time_util.h"

#define _CACHE_EXPIRED_TIME 1

namespace base
{

	CDbCache::CDbCache(CDbCacheMgr* pDbCacheMgr)
		: m_pDbCacheMgr(pDbCacheMgr)
		, m_nDataSize(0)
	{

	}

	CDbCache::~CDbCache()
	{

	}

	google::protobuf::Message* CDbCache::getData(uint32_t nDataID)
	{
		auto iter = this->m_mapCacheInfo.find(nDataID);
		if (iter == this->m_mapCacheInfo.end())
			return nullptr;

		const std::string& szDataName = this->m_pDbCacheMgr->getDataName(nDataID);
		google::protobuf::Message* pMessage = this->m_pDbCacheMgr->getDbThread()->createMessage(szDataName);
		if (nullptr == pMessage)
		{
			PrintWarning("CDbCache::getData error nullptr == pMessage data_name: {}", szDataName);
			return nullptr;
		}
		if (!pMessage->ParseFromArray(iter->second.szData.c_str(), (int32_t)iter->second.szData.size()))
		{
			PrintWarning("CDbCache::getData error pMessage->ParseFromArray data_name: {}", szDataName);
			SAFE_DELETE(pMessage);
			return nullptr;
		}

		return pMessage;
	}

	bool CDbCache::addData(uint32_t nDataID, const google::protobuf::Message* pData)
	{
		if (this->m_mapCacheInfo.find(nDataID) != this->m_mapCacheInfo.end())
			return false;

		std::string szData;
		if (!pData->SerializeToString(&szData))
			return false;

		int32_t nSize = (int32_t)szData.size();
		SCacheInfo& sCacheInfo = this->m_mapCacheInfo[nDataID];
		sCacheInfo.nTime = base::time_util::getGmtTime();
		sCacheInfo.szData = std::move(szData);
		this->m_nDataSize += nSize;

		return true;
	}

	bool CDbCache::setData(uint32_t nDataID, const std::string& szDataName, const google::protobuf::Message* pData)
	{
		auto iter = this->m_mapCacheInfo.find(nDataID);
		if (iter == this->m_mapCacheInfo.end())
			return this->addData(nDataID, pData);

		google::protobuf::Message* pDstData = this->m_pDbCacheMgr->getDbThread()->createMessage(szDataName);
		if (nullptr == pDstData)
			return false;

		defer([&]()
		{
			SAFE_DELETE(pDstData);
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
		this->m_nDataSize -= (int32_t)iter->second.szData.size();
		iter->second.nTime = base::time_util::getGmtTime();
		iter->second.szData = std::move(szData);
		this->m_nDataSize += nSize;

		return true;
	}

	bool CDbCache::delData(uint32_t nDataID)
	{
		auto iter = this->m_mapCacheInfo.find(nDataID);
		if (iter == this->m_mapCacheInfo.end())
			return false;

		this->m_nDataSize -= (int32_t)iter->second.szData.size();
		this->m_mapCacheInfo.erase(nDataID);

		return true;
	}

	int32_t CDbCache::getDataSize() const
	{
		return this->m_nDataSize;
	}

	bool CDbCache::writeback(int64_t nTime)
	{
		bool bDirty = false;
		for (auto iter = this->m_mapCacheInfo.begin(); iter != this->m_mapCacheInfo.end(); ++iter)
		{
			SCacheInfo& sCacheInfo = iter->second;
			if (sCacheInfo.nTime == 0)
				continue;

			int64_t nDeltaTime = nTime - sCacheInfo.nTime;
			if (nDeltaTime >= _CACHE_EXPIRED_TIME || nTime == 0)
			{
				const std::string& szDataName = this->m_pDbCacheMgr->getDataName(iter->first);
				if (szDataName.empty())
				{
					PrintWarning("CDbCache::writeback error szDataName.empty() index: {}", iter->first);
					continue;
				}
				google::protobuf::Message* pMessage = this->m_pDbCacheMgr->getDbThread()->createMessage(szDataName);
				if (nullptr == pMessage)
				{
					PrintWarning("CDbCache::writeback error nullptr == pMessage data_name: {}", szDataName);
					continue;
				}
				defer([&]()
				{
					SAFE_DELETE(pMessage);
				});

				if (!pMessage->ParseFromString(sCacheInfo.szData))
				{
					PrintWarning("CDbCache::writeback error pMessage->ParseFromString data_name: {}", szDataName);
					continue;
				}
				uint32_t nErrorCode = m_pDbCacheMgr->getDbThread()->getDbCommandHandlerProxy()->onDbCommand(db::eDBCT_Update, pMessage, nullptr);
				if (nErrorCode != db::eDBRC_OK)
				{
					PrintWarning("CDbCache::writeback error nErrorCode != db::eDBRC_OK data_name: {} error_code: {}", szDataName, nErrorCode);
				}
			}
			else
			{
				bDirty = true;
			}
		}

		return bDirty;
	}
}