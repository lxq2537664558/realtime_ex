#pragma once
#include "service_invoke_holder.h"
#include "db_native_message.h"

#include "libBaseCommon/base_common.h"
#include "libBaseCommon/string_util.h"

#include "libBaseDatabase/database.h"

#include "google/protobuf/message.h"

#include <string>
#include <vector>

namespace core
{
	class CDbServiceInvokeHolder
	{
	public:
		CDbServiceInvokeHolder(CServiceBase* pServiceBase) : m_sServiceInvokeHolder(pServiceBase), m_nDbServiceID(0) { }
		CDbServiceInvokeHolder(CServiceBase* pServiceBase, uint32_t nDbServiceID) : m_sServiceInvokeHolder(pServiceBase), m_nDbServiceID(nDbServiceID) { }
		virtual ~CDbServiceInvokeHolder() { }

		inline bool		update(const google::protobuf::Message* pMessage);
		inline bool		call(uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg);
		inline bool		remove(uint64_t nID, const std::string& szTableName);
		inline bool		insert(const google::protobuf::Message* pMessage);
		inline bool		flush(uint64_t nID, base::db::EFlushCacheType eType);

		inline void		async_select(uint64_t nID, const std::string& szTableName, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		inline void		async_update(const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		inline void		async_remove(uint64_t nID, const std::string& szTableName, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		inline void		async_insert(const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		inline void		async_query(uint32_t nChannelID, const std::string& szTableName, const std::string& szWhereClause, const std::vector<std::string>* vecArg, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		inline void		async_call(uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		inline void		async_nop(uint32_t nChannelID, const std::function<void(uint32_t)>& callback);

		inline uint32_t	sync_select(uint64_t nID, const std::string& szTableName, std::shared_ptr<const google::protobuf::Message>& pResponseMessage);
		inline uint32_t	sync_update(const google::protobuf::Message* pMessage, std::shared_ptr<const google::protobuf::Message>& pResponseMessage);
		inline uint32_t	sync_remove(uint64_t nID, const std::string& szTableName, std::shared_ptr<const google::protobuf::Message>& pResponseMessage);
		inline uint32_t	sync_insert(const google::protobuf::Message* pMessage, std::shared_ptr<const google::protobuf::Message>& pResponseMessage);
		inline uint32_t	sync_query(uint32_t nChannelID, const std::string& szTableName, const std::string& szWhereClause, const std::vector<std::string>* vecArg, std::shared_ptr<const google::protobuf::Message>& pResponseMessage);
		inline uint32_t	sync_call(uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg, std::shared_ptr<const google::protobuf::Message>& pResponseMessage);
		inline uint32_t	sync_nop(uint32_t nChannelID);

		inline bool		update(uint32_t nDbServiceID, const google::protobuf::Message* pMessage);
		inline bool		call(uint32_t nDbServiceID, uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg);
		inline bool		remove(uint32_t nDbServiceID, uint64_t nID, const std::string& szTableName);
		inline bool		insert(uint32_t nDbServiceID, const google::protobuf::Message* pMessage);
		inline bool		flush(uint32_t nDbServiceID, uint64_t nID, base::db::EFlushCacheType eType);

		inline void		async_select(uint32_t nDbServiceID, uint64_t nID, const std::string& szTableName, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		inline void		async_update(uint32_t nDbServiceID, const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		inline void		async_remove(uint32_t nDbServiceID, uint64_t nID, const std::string& szTableName, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		inline void		async_insert(uint32_t nDbServiceID, const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		inline void		async_query(uint32_t nDbServiceID, uint32_t nChannelID, const std::string& szTableName, const std::string& szWhereClause, const std::vector<std::string>* vecArg, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		inline void		async_call(uint32_t nDbServiceID, uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		inline void		async_nop(uint32_t nDbServiceID, uint32_t nChannelID, const std::function<void(uint32_t)>& callback);

		inline uint32_t	sync_select(uint32_t nDbServiceID, uint64_t nID, const std::string& szTableName, std::shared_ptr<const google::protobuf::Message>& pResponseMessage);
		inline uint32_t	sync_update(uint32_t nDbServiceID, const google::protobuf::Message* pMessage, std::shared_ptr<const google::protobuf::Message>& pResponseMessage);
		inline uint32_t	sync_remove(uint32_t nDbServiceID, uint64_t nID, const std::string& szTableName, std::shared_ptr<const google::protobuf::Message>& pResponseMessage);
		inline uint32_t	sync_insert(uint32_t nDbServiceID, const google::protobuf::Message* pMessage, std::shared_ptr<const google::protobuf::Message>& pResponseMessage);
		inline uint32_t	sync_query(uint32_t nDbServiceID, uint32_t nChannelID, const std::string& szTableName, const std::string& szWhereClause, const std::vector<std::string>* vecArg, std::shared_ptr<const google::protobuf::Message>& pResponseMessage);
		inline uint32_t	sync_call(uint32_t nDbServiceID, uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg, std::shared_ptr<const google::protobuf::Message>& pResponseMessage);
		inline uint32_t	sync_nop(uint32_t nDbServiceID, uint32_t nChannelID);

	private:
		uint32_t				m_nDbServiceID;
		CServiceInvokeHolder	m_sServiceInvokeHolder;
	};
}

#include "db_service_invoke_holder.inl"
