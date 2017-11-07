#include "db_command_update_handler.h"

#include "db_protobuf.h"

#include "libBaseCommon/debug_helper.h"

#include <sstream>

namespace base
{

	CDbCommandUpdateHandler::CDbCommandUpdateHandler(CDbThread* pDbThread)
		: CDbCommandHandler(pDbThread)
	{

	}

	CDbCommandUpdateHandler::~CDbCommandUpdateHandler()
	{

	}

	uint32_t CDbCommandUpdateHandler::onDbCommand(const google::protobuf::Message* pRequestMessage, std::shared_ptr<google::protobuf::Message>* pResponseMessage)
	{
		std::string szMessageName = pRequestMessage->GetTypeName();
		const google::protobuf::Message* pMessage = pRequestMessage;

		std::string szTableName;
		DebugAstEx(getTableNameByMessageName(szMessageName, szTableName), db::eDBRC_ProtobufError);

		std::vector<SFieldInfo> vecFieldInfo;
		DebugAstEx(getMessageFieldInfos(pMessage, vecFieldInfo), db::eDBRC_ProtobufError);

		std::string szInsertClause;
		szInsertClause.reserve(1024);
		szInsertClause = "(";
		for (size_t i = 0; i < vecFieldInfo.size(); ++i)
		{
			const SFieldInfo& sFieldInfo = vecFieldInfo[i];

			if (i != 0)
				szInsertClause += ",";

			szInsertClause += sFieldInfo.szName;
		}
		szInsertClause += ") values (";

		for (size_t i = 0; i < vecFieldInfo.size(); ++i)
		{
			const SFieldInfo& sFieldInfo = vecFieldInfo[i];

			if (i != 0)
				szInsertClause += ",";

			if (sFieldInfo.bStr)
			{
				szInsertClause += "'";
				szInsertClause += this->m_pDbConnection->escape(sFieldInfo.szValue);
				szInsertClause += "'";
			}
			else
			{
				szInsertClause += sFieldInfo.szValue;
			}
		}
		szInsertClause += ")";

		std::string szUpdateClause;
		szUpdateClause.reserve(1024);
		for (size_t i = 0; i < vecFieldInfo.size(); ++i)
		{
			const SFieldInfo& sFieldInfo = vecFieldInfo[i];

			if (i != 0)
				szUpdateClause += ",";

			szUpdateClause += sFieldInfo.szName;
			szUpdateClause += "=";

			if (sFieldInfo.bStr)
			{
				szUpdateClause += "'";
				szUpdateClause += this->m_pDbConnection->escape(sFieldInfo.szValue);
				szUpdateClause += "'";
			}
			else
			{
				szUpdateClause += sFieldInfo.szValue;
			}
		}

		std::ostringstream oss;
		oss << "insert into " << szTableName << szInsertClause << " on duplicate key update " << szUpdateClause;
		std::string szSQL = oss.str();

#ifdef _DEBUG_SQL
		PrintInfo("{}", szSQL);
#endif

		uint32_t nErrorType = this->m_pDbConnection->execute(szSQL, nullptr);
		if (nErrorType == eMET_LostConnection)
			return db::eDBRC_LostConnection;
		else if (nErrorType != eMET_OK)
			return db::eDBRC_MysqlError;

		return (this->m_pDbConnection->getAffectedRow() >= 1) ? db::eDBRC_OK : db::eDBRC_MysqlError;
	}
}