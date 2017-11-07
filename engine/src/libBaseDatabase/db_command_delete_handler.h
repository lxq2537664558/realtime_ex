#pragma once
#include "db_command_handler.h"

namespace base
{
	class CDbCommandDeleteHandler :
		public CDbCommandHandler
	{
	public:
		CDbCommandDeleteHandler(CDbThread* pDbThread);
		virtual ~CDbCommandDeleteHandler();

		virtual uint32_t	onDbCommand(const google::protobuf::Message* pRequestMessage, std::shared_ptr<google::protobuf::Message>* pResponseMessage);
	};
}