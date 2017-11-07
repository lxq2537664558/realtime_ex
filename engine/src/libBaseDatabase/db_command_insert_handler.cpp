#include "db_command_insert_handler.h"
#include "db_protobuf.h"

#include "libBaseCommon/debug_helper.h"

#include <sstream>

namespace base
{
	CDbCommandInsertHandler::CDbCommandInsertHandler(CDbThread* pDbThread)
		: CDbCommandHandler(pDbThread)
	{

	}

	CDbCommandInsertHandler::~CDbCommandInsertHandler()
	{

	}

	uint32_t CDbCommandInsertHandler::onDbCommand(const google::protobuf::Message* pRequestMessage, std::shared_ptr<google::protobuf::Message>* pResponseMessage)
	{
		const google::protobuf::Message* pMessage = pRequestMessage;
		const std::string szMessageName = pMessage->GetTypeName();

		std::string szTableName;
		DebugAstEx(getTableNameByMessageName(szMessageName, szTableName), db::eDBRC_ProtobufError);

		std::vector<SFieldInfo> vecFieldInfo;
		DebugAstEx(getMessageFieldInfos(pMessage, vecFieldInfo), db::eDBRC_ProtobufError);

		std::string szClause;
		szClause.reserve(1024);

		szClause = "(";
		for (size_t i = 0; i < vecFieldInfo.size(); ++i)
		{
			const SFieldInfo& sFieldInfo = vecFieldInfo[i];

			if (i != 0)
				szClause += ",";

			szClause += sFieldInfo.szName;
		}
		szClause += ") values (";

		for (size_t i = 0; i < vecFieldInfo.size(); ++i)
		{
			const SFieldInfo& sFieldInfo = vecFieldInfo[i];

			if (i != 0)
				szClause += ",";

			if (sFieldInfo.bStr)
			{
				szClause += "'";
				szClause += this->m_pDbConnection->escape(sFieldInfo.szValue);
				szClause += "'";
			}
			else
			{
				szClause += sFieldInfo.szValue;
			}
		}
		szClause += ")";
		std::ostringstream oss;
		oss << "insert into " << szTableName << szClause;
		std::string szSQL = oss.str();

#ifdef _DEBUG_SQL
		PrintInfo("{}", szSQL);
#endif

		uint32_t nErrorType = this->m_pDbConnection->execute(szSQL, nullptr);
		if (nErrorType == eMET_LostConnection)
			return db::eDBRC_LostConnection;
		else if (nErrorType != eMET_OK)
			return db::eDBRC_MysqlError;

		return this->m_pDbConnection->getAffectedRow() == 1 ? db::eDBRC_OK : db::eDBRC_MysqlError;
	}

}