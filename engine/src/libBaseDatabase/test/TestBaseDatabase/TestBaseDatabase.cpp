// TestBaseDatabase.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "libBaseDatabase/database.h"
#include "libBaseCommon/logger.h"

#include "player_base.pb.h"
#include "call_command.pb.h"
#include "select_command.pb.h"
#include "insert_command.pb.h"
#include "delete_command.pb.h"
#include "update_command.pb.h"
#include "query_command.pb.h"
#include "result_set.pb.h"

google::protobuf::Message* createMessage(const std::string& szMessageName)
{
	const google::protobuf::Descriptor* pDescriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(szMessageName);
	if (pDescriptor == nullptr)
		return nullptr;

	const google::protobuf::Message* pProtoType = google::protobuf::MessageFactory::generated_factory()->GetPrototype(pDescriptor);
	if (pProtoType == nullptr)
		return nullptr;

	return pProtoType->New();
}

void destroyMessage(google::protobuf::Message* pMessage)
{
	delete pMessage;
}

int main()
{
	base::log::init(false, false, "");

	base::db::SDbOptions sDbOptions;
	sDbOptions.nMaxCacheSize = 100;
	sDbOptions.nCacheWritebackTime = 100;
	sDbOptions.szProtoDir = "./db_proto/";
	sDbOptions.vecCacheTable.push_back("player_base");
	sDbOptions.nDbThreadCount = 1;
	sDbOptions.funcCreateMessage = std::bind(&createMessage, std::placeholders::_1);
	sDbOptions.funcDestroyMessage = std::bind(&destroyMessage, std::placeholders::_1);

	uint32_t dbID = base::db::create("127.0.0.1", 3306, "test_db", "root", "123456", "utf8", sDbOptions);

	// delete
	{
		proto::db::delete_command command;
		command.set_id(11);
		command.set_table_name("player_base");

		base::db::query(dbID, &command, [](const google::protobuf::Message* pResponse, uint32_t nErrorCode)
		{
			if (base::db::eDBRC_OK != nErrorCode)
			{
				PrintInfo("delete player_base error: {}", nErrorCode);
				return;
			}
		});
	}

	// insert
	{
		for (uint64_t i = 0; i < 10; ++i)
		{
			proto::db::player_base player_base;
			player_base.set_player_id(i);
			player_base.set_server_id((uint32_t)i);
			player_base.set_account_name("tavia");
			player_base.set_last_login_time(222);
			player_base.set_last_logout_time(3333);
			player_base.set_name("test_db");
			player_base.set_lv(1);
			player_base.set_gold(1222222);

			std::string szData = player_base.SerializeAsString();
			proto::db::insert_command command;
			command.set_message_name(player_base.GetTypeName());
			command.set_message_content(std::move(szData));

			base::db::query(dbID, &command, [i](const google::protobuf::Message* pResponse, uint32_t nErrorCode)
			{
				if (nErrorCode != base::db::eDBRC_OK)
				{
					PrintInfo("insert error");
				}

				PrintInfo("insert data {}", i);
			});
		}
	}

	// update
	{
		for (uint64_t i = 0; i < 10; ++i)
		{
			proto::db::player_base player_base;
			player_base.set_player_id(i);
			player_base.set_server_id((uint32_t)i);
			player_base.set_account_name("tavia2222");
			player_base.set_last_login_time(222);
			player_base.set_last_logout_time(3333);
			player_base.set_name("test_db");
			player_base.set_lv(1);
			player_base.set_gold(1222222);

			std::string szData = player_base.SerializeAsString();
			proto::db::update_command command;
			command.set_message_name(player_base.GetTypeName());
			command.set_message_content(std::move(szData));

			base::db::query(dbID, &command, [i](const google::protobuf::Message* pResponse, uint32_t nErrorCode)
			{
				if (nErrorCode != base::db::eDBRC_OK)
				{
					PrintInfo("update error");
				}

				PrintInfo("update data {}", i);
			});
		}
	}

	// select
	{
		proto::db::select_command command;
		command.set_id(11);
		command.set_table_name("player_base");

		base::db::query(dbID, &command, [](const google::protobuf::Message* pResponse, uint32_t nErrorCode)
		{
			if (base::db::eDBRC_OK != nErrorCode)
			{
				PrintInfo("base::db::eDBRC_OK != nErrorCode error: {}", nErrorCode);
				return;
			}

			const proto::db::player_base* pPlayerBase =
				dynamic_cast<const proto::db::player_base*>(pResponse);
			if (nullptr == pPlayerBase)
			{
				PrintInfo("nullptr == pPlayerBase");
				return;
			}

			uint64_t nLastLoginTime = pPlayerBase->last_login_time();
			uint64_t nLastLogoutTime = pPlayerBase->last_logout_time();
			std::string szAccountName = pPlayerBase->account_name();
			uint32_t nServerID = pPlayerBase->server_id();
			std::string szName = pPlayerBase->name();
			uint32_t nLv = pPlayerBase->lv();
			uint64_t nGold = pPlayerBase->gold();
		});
	}

	// query
	{
		proto::db::query_command command;
		command.set_channel_id(11);
		command.set_table_name("player_base");
		command.set_where_clause("name = {?}");
		command.add_args("test_db");
		
		base::db::query(dbID, &command, [](const google::protobuf::Message* pResponse, uint32_t nErrorCode)
		{
			if (base::db::eDBRC_OK != nErrorCode)
			{
				PrintInfo("base::db::eDBRC_OK != nErrorCode error: {}", nErrorCode);
				return;
			}

			const proto::db::player_base_set* pPlayerBaseSet =
				dynamic_cast<const proto::db::player_base_set*>(pResponse);
			if (nullptr == pPlayerBaseSet)
			{
				PrintInfo("nullptr == pPlayerBaseSet");
				return;
			}

			for (int32_t i = 0; i < pPlayerBaseSet->data_set_size(); ++i)
			{
				const proto::db::player_base& playerBase = pPlayerBaseSet->data_set(i);
				uint64_t nLastLoginTime = playerBase.last_login_time();
				uint64_t nLastLogoutTime = playerBase.last_logout_time();
				std::string szAccountName = playerBase.account_name();
				uint32_t nServerID = playerBase.server_id();
				std::string szName = playerBase.name();
				uint32_t nLv = playerBase.lv();
				uint64_t nGold = playerBase.gold();
			}
		});
	}

	// call
	{
		proto::db::call_command command;
		command.set_channel_id(11);
		command.set_sql("select player_id, name from player_base where name = {?}");
		command.add_args("test_db");

		base::db::query(dbID, &command, [](const google::protobuf::Message* pResponse, uint32_t nErrorCode)
		{
			if (base::db::eDBRC_OK != nErrorCode)
			{
				PrintInfo("base::db::eDBRC_OK != nErrorCode error: {}", nErrorCode);
				return;
			}

			const proto::db::result_set* pResultset =
				dynamic_cast<const proto::db::result_set*>(pResponse);
			if (nullptr == pResultset)
			{
				PrintInfo("nullptr == result_set");
				return;
			}

			for (int32_t i = 0; i < pResultset->rows_size(); ++i)
			{
				const proto::db::row& sRow = pResultset->rows(i);
				
				uint64_t nPlayerID = 0;
				base::string_util::convert_to_value(sRow.value(0), nPlayerID);
				std::string szName = sRow.value(1);
			}
		});
	}

	while (true)
	{
		base::db::update(dbID);
	}


    return 0;
}

