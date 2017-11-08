#pragma once

#include <unordered_map>
#include <string>
#include <memory>

#include "google/protobuf/message.h"
#include "libBaseCommon/ticker.h"

namespace base
{
	class CDbCacheMgr;
	class CDbCache
	{
	public:
		CDbCache(CDbCacheMgr* pDbCacheMgr, const std::string szDataName);
		~CDbCache();

		google::protobuf::Message*
				getData(uint64_t nID);
		bool	setData(uint64_t nID, const google::protobuf::Message* pData);
		bool	addData(uint64_t nID, const google::protobuf::Message* pData);
		bool	delData(uint64_t nID);
		int32_t	getDataSize() const;
		void	flush();
		int32_t	cleanData();

	private:
		void	onBackup(uint64_t nContext);

	private:
		struct SCacheInfo
		{
			std::string	szData;
			std::unique_ptr<CTicker>
						ticker;
		};

		CDbCacheMgr*	m_pDbCacheMgr;
		std::string		m_szDataName;
		std::unordered_map<uint64_t, SCacheInfo>
						m_mapCacheInfo;
		int32_t			m_nDataSize;
	};
}
