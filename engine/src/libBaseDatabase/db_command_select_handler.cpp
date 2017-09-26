#include "db_command_select_handler.h"
#include "db_protobuf.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/defer.h"

#include "proto_src/select_command.pb.h"

#include <sstream>

namespace base
{
	CDbCommandSelectHandler::CDbCommandSelectHandler()
	{

	}

	CDbCommandSelectHandler::~CDbCommandSelectHandler()
	{

	}

	uint32_t CDbCommandSelectHandler::onDbCommand(const google::protobuf::Message* pRequestMessage, std::shared_ptr<google::protobuf::Message>* pResponseMessage)
	{
		const proto::db::select_command* pCommand = dynamic_cast<const proto::db::select_command*>(pRequestMessage);
		DebugAstEx(pCommand != nullptr, db::eDBRC_ProtobufError);

		std::string szMessageName = getMessageNameByTableName(pCommand->table_name());
		std::shared_ptr<google::protobuf::Message> pMessage(createMessage(szMessageName));
		DebugAstEx(pMessage != nullptr, db::eDBRC_ProtobufError);

		std::string szPrimaryName = getPrimaryName(pMessage.get());
		DebugAstEx(!szPrimaryName.empty(), db::eDBRC_ProtobufError);

		std::ostringstream oss;
		oss << "select * from " << pCommand->table_name() << " where " << szPrimaryName << "=" << pCommand->id() << " limit 1";
		std::string szSQL(oss.str());

#ifdef _DEBUG_SQL
		PrintInfo("{}", szSQL);
#endif

		CDbRecordset* pDbRecordset = nullptr;
		defer([&]()
		{
			SAFE_DELETE(pDbRecordset);
		});

		uint32_t nErrorType = this->m_pDbConnection->execute(szSQL, &pDbRecordset);
		if (nErrorType == eMET_LostConnection)
			return db::eDBRC_LostConnection;
		else if (nErrorType != eMET_OK)
			return db::eDBRC_MysqlError;

		DebugAstEx(pDbRecordset != nullptr, db::eDBRC_MysqlError);

		if (pDbRecordset->getRowCount() == 0)
			return db::eDBRC_EmptyRecordset;

		DebugAstEx(fillNormalMessage(pDbRecordset, pMessage.get()), db::eDBRC_ProtobufError);

		*pResponseMessage = pMessage;

		return db::eDBRC_OK;
	}
}