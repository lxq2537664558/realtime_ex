#include "db_command_delete_handler.h"
#include "db_protobuf.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/defer.h"

#include "proto_src/delete_command.pb.h"

#include <sstream>

namespace base
{
	CDbCommandDeleteHandler::CDbCommandDeleteHandler()
	{

	}

	CDbCommandDeleteHandler::~CDbCommandDeleteHandler()
	{

	}

	uint32_t CDbCommandDeleteHandler::onDbCommand(const google::protobuf::Message* pRequestMessage, std::shared_ptr<google::protobuf::Message>* pResponseMessage)
	{
		const proto::db::delete_command* pCommand = dynamic_cast<const proto::db::delete_command*>(pRequestMessage);
		DebugAstEx(pCommand != nullptr, db::eDBRC_ProtobufError);

		std::string szMessageName = getMessageNameByTableName(pCommand->table_name());
		google::protobuf::Message* pMessage = createMessage(szMessageName);
		DebugAstEx(pMessage != nullptr, db::eDBRC_ProtobufError);

		defer([&]() 
		{
			SAFE_DELETE(pMessage);
		});

		std::string szPrimaryName = getPrimaryName(pMessage);
		DebugAstEx(!szPrimaryName.empty(), db::eDBRC_ProtobufError);

		std::ostringstream oss;
		oss << "delete from " << pCommand->table_name().c_str() << " where " << szPrimaryName << "=" << pCommand->id() << " limit 1";
		std::string szSQL(oss.str());

#ifdef _DEBUG_SQL
		PrintInfo("{}", szSQL);
#endif

		uint32_t nErrorType = this->m_pDbConnection->execute(szSQL, nullptr);
		if (nErrorType == eMET_LostConnection)
			return db::eDBRC_LostConnection;
		else if (nErrorType != eMET_OK)
			return db::eDBRC_MysqlError;

		return db::eDBRC_OK;
	}
}