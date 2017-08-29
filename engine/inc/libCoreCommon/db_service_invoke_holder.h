#pragma once
#include "service_invoke_holder.h"

#include "libBaseCommon/base_common.h"
#include "libBaseDatabase/database.h"

#include "google/protobuf/message.h"

#include <string>
#include <vector>

namespace core
{
	class __CORE_COMMON_API__ CDbServiceInvokeHolder
	{
	public:
		CDbServiceInvokeHolder(CServiceBase* pServiceBase);
		CDbServiceInvokeHolder(CServiceBase* pServiceBase, uint32_t nDbServiceID);
		virtual ~CDbServiceInvokeHolder();

		bool		update(const google::protobuf::Message* pMessage);
		bool		call(uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg);
		bool		remove(uint64_t nID, const std::string& szTableName);
		bool		insert(const google::protobuf::Message* pMessage);
		bool		flush(uint64_t nID, base::db::EFlushCacheType eType);

		void		async_select(uint64_t nID, const std::string& szTableName, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		void		async_update(const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		void		async_remove(uint64_t nID, const std::string& szTableName, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		void		async_insert(const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		void		async_query(uint32_t nChannelID, const std::string& szTableName, const std::string& szWhereClause, const std::vector<std::string>* vecArg, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		void		async_call(uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		void		async_nop(uint32_t nChannelID, const std::function<void(uint32_t)>& callback);

		uint32_t	sync_select(uint64_t nID, const std::string& szTableName, std::shared_ptr<const google::protobuf::Message>& pResponseMessage);
		uint32_t	sync_update(const google::protobuf::Message* pMessage, std::shared_ptr<const google::protobuf::Message>& pResponseMessage);
		uint32_t	sync_remove(uint64_t nID, const std::string& szTableName, std::shared_ptr<const google::protobuf::Message>& pResponseMessage);
		uint32_t	sync_insert(const google::protobuf::Message* pMessage, std::shared_ptr<const google::protobuf::Message>& pResponseMessage);
		uint32_t	sync_query(uint32_t nChannelID, const std::string& szTableName, const std::string& szWhereClause, const std::vector<std::string>* vecArg, std::shared_ptr<const google::protobuf::Message>& pResponseMessage);
		uint32_t	sync_call(uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg, std::shared_ptr<const google::protobuf::Message>& pResponseMessage);
		uint32_t	sync_nop(uint32_t nChannelID);

		bool		update(uint32_t nDbServiceID, const google::protobuf::Message* pMessage);
		bool		call(uint32_t nDbServiceID, uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg);
		bool		remove(uint32_t nDbServiceID, uint64_t nID, const std::string& szTableName);
		bool		insert(uint32_t nDbServiceID, const google::protobuf::Message* pMessage);
		bool		flush(uint32_t nDbServiceID, uint64_t nID, base::db::EFlushCacheType eType);

		void		async_select(uint32_t nDbServiceID, uint64_t nID, const std::string& szTableName, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		void		async_update(uint32_t nDbServiceID, const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		void		async_remove(uint32_t nDbServiceID, uint64_t nID, const std::string& szTableName, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		void		async_insert(uint32_t nDbServiceID, const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		void		async_query(uint32_t nDbServiceID, uint32_t nChannelID, const std::string& szTableName, const std::string& szWhereClause, const std::vector<std::string>* vecArg, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		void		async_call(uint32_t nDbServiceID, uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		void		async_nop(uint32_t nDbServiceID, uint32_t nChannelID, const std::function<void(uint32_t)>& callback);

		uint32_t	sync_select(uint32_t nDbServiceID, uint64_t nID, const std::string& szTableName, std::shared_ptr<const google::protobuf::Message>& pResponseMessage);
		uint32_t	sync_update(uint32_t nDbServiceID, const google::protobuf::Message* pMessage, std::shared_ptr<const google::protobuf::Message>& pResponseMessage);
		uint32_t	sync_remove(uint32_t nDbServiceID, uint64_t nID, const std::string& szTableName, std::shared_ptr<const google::protobuf::Message>& pResponseMessage);
		uint32_t	sync_insert(uint32_t nDbServiceID, const google::protobuf::Message* pMessage, std::shared_ptr<const google::protobuf::Message>& pResponseMessage);
		uint32_t	sync_query(uint32_t nDbServiceID, uint32_t nChannelID, const std::string& szTableName, const std::string& szWhereClause, const std::vector<std::string>* vecArg, std::shared_ptr<const google::protobuf::Message>& pResponseMessage);
		uint32_t	sync_call(uint32_t nDbServiceID, uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg, std::shared_ptr<const google::protobuf::Message>& pResponseMessage);
		uint32_t	sync_nop(uint32_t nDbServiceID, uint32_t nChannelID);

	private:
		uint32_t				m_nDbServiceID;
		CServiceInvokeHolder	m_sServiceInvokeHolder;
	};
}