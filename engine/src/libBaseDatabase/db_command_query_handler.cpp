#include "db_command_query_handler.h"
#include "db_protobuf.h"

#include "libBaseCommon/defer.h"
#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/string_util.h"

#include "proto_src/query_command.pb.h"

#include <sstream>

namespace
{
	const std::string szPlaceholder = "{?}";
}

namespace base
{

	CDbCommandQueryHandler::CDbCommandQueryHandler()
	{

	}

	CDbCommandQueryHandler::~CDbCommandQueryHandler()
	{

	}

	uint32_t CDbCommandQueryHandler::onDbCommand(const google::protobuf::Message* pRequestMessage, std::shared_ptr<google::protobuf::Message>* pResponseMessage)
	{
		const proto::db::query_command* pCommand = dynamic_cast<const proto::db::query_command*>(pRequestMessage);
		DebugAstEx(pCommand != nullptr, db::eDBRC_ProtobufError);
		
		std::string szWhereClause = pCommand->where_clause();
		std::string::size_type nPos = 0;
		int32_t nIndex = 0;
		while (std::string::npos != (nPos = szWhereClause.find(szPlaceholder, nPos)) && nIndex < pCommand->args_size())
		{
			const std::string& szArg = pCommand->args(nIndex);
			std::string szSafeArg = "'";
			szSafeArg += this->m_pDbConnection->escape(szArg);
			szSafeArg += "'";

			szWhereClause.replace(nPos, szPlaceholder.size(), szSafeArg);
			nPos += szSafeArg.size();
			nIndex++;
		}

		std::ostringstream oss;
		if (!szWhereClause.empty())
			oss << "select * from " << pCommand->table_name().c_str() << " where " << szWhereClause;
		else
			oss << "select * from " << pCommand->table_name().c_str();
		
		std::string szSQL(oss.str());

#ifdef _DEBUG_SQL
		PrintInfo("{}", szSQL);
#endif

		CDbRecordset* pDbRecordset = nullptr;
		defer([&]()
		{
			delete pDbRecordset;
		});

		uint32_t nErrorType = this->m_pDbConnection->execute(szSQL, &pDbRecordset);
		if (nErrorType == eMET_LostConnection)
			return db::eDBRC_LostConnection;
		else if (nErrorType != eMET_OK)
			return db::eDBRC_MysqlError;

		DebugAstEx(pDbRecordset != nullptr, db::eDBRC_MysqlError);

		std::string szMessageName = getMessageNameByTableName(pCommand->table_name());
		*pResponseMessage = std::shared_ptr<google::protobuf::Message>(createRepeatMessage(pDbRecordset, szMessageName));
		DebugAstEx(*pResponseMessage != nullptr, db::eDBRC_ProtobufError);

		return db::eDBRC_OK;
	}
}