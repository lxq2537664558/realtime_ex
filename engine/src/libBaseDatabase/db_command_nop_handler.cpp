#include "db_command_nop_handler.h"

namespace base
{
	CDbCommandNOPHandler::CDbCommandNOPHandler()
	{

	}

	CDbCommandNOPHandler::~CDbCommandNOPHandler()
	{

	}

	uint32_t CDbCommandNOPHandler::onDbCommand(const google::protobuf::Message* pRequestMessage, std::shared_ptr<google::protobuf::Message>* pResponseMessage)
	{
		return db::eDBRC_OK;
	}
}