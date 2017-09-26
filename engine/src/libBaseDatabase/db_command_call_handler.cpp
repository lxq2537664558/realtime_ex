#include "db_command_call_handler.h"
#include "db_protobuf.h"

#include "libBaseCommon/string_util.h"
#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/defer.h"

#include <sstream>

#include "proto_src/result_set.pb.h"
#include "proto_src/call_command.pb.h"


namespace
{
	const uint32_t nTryDeadloopCount = 5;

	const std::string szPlaceholder = "{?}";
}

namespace base
{
	CDbCommandCallHandler::CDbCommandCallHandler()
	{

	}

	CDbCommandCallHandler::~CDbCommandCallHandler()
	{

	}

	uint32_t CDbCommandCallHandler::onDbCommand(const google::protobuf::Message* pRequestMessage, std::shared_ptr<google::protobuf::Message>* pResponseMessage)
	{
		const proto::db::call_command* pCommand = dynamic_cast<const proto::db::call_command*>(pRequestMessage);
		DebugAstEx(pCommand != nullptr, db::eDBRC_ProtobufError);

		std::string szSQL = pCommand->sql();
		std::string::size_type nPos = 0;
		int32_t nIndex = 0;
		while (std::string::npos != (nPos = szSQL.find(szPlaceholder, nPos)) && nIndex < pCommand->args_size())
		{
			const std::string& szArg = pCommand->args(nIndex);
			std::string szSafeArg = "'";
			szSafeArg += this->m_pDbConnection->escape(szArg);
			szSafeArg += "'";

			szSQL.replace(nPos, szPlaceholder.size(), szSafeArg);
			nPos += szSafeArg.size();
			nIndex++;
		}

#ifdef _DEBUG_SQL
		PrintInfo("{}", szSQL);
#endif

		CDbRecordset* pDbRecordset = nullptr;
		defer([&]()
		{
			SAFE_DELETE(pDbRecordset);
		});
		
		bool bOK = false;
		for (size_t i = 0; i < nTryDeadloopCount; ++i)
		{
			uint32_t nErrorType = this->m_pDbConnection->execute(szSQL, &pDbRecordset);

			if (nErrorType == eMET_Deadloop)
			{
				// 死锁回滚继续重来
				continue;
			}
			else if (nErrorType == eMET_LostConnection)
			{
				return db::eDBRC_LostConnection;
			}
			else if (nErrorType != eMET_OK)
			{
				return db::eDBRC_MysqlError;
			}

			bOK = true;
			break;
		}

		if (!bOK)
			return db::eDBRC_MysqlError;

		if (pDbRecordset == nullptr)
			return db::eDBRC_OK;

		std::shared_ptr<proto::db::result_set> pResultset = std::make_shared<proto::db::result_set>();
		for (uint32_t i = 0; i < pDbRecordset->getFieldCount(); ++i)
		{
			pResultset->add_field_name(pDbRecordset->getFieldName(i));
		}

		for (uint64_t i = 0; i < pDbRecordset->getRowCount(); ++i)
		{
			pDbRecordset->fatchNextRow();

			proto::db::row* pRow = pResultset->add_rows();
			DebugAstEx(pRow != nullptr, db::eDBRC_ProtobufError);

			for (uint32_t j = 0; j < pDbRecordset->getFieldCount(); ++j)
			{
				std::string szValue = pDbRecordset->getData(j);
				
				pRow->add_value(szValue);
			}
		}

		*pResponseMessage = pResultset;

		return db::eDBRC_OK;
	}
}