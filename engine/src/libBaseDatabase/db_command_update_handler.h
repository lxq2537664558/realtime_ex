#pragma once
#include "db_command_handler.h"

namespace base
{
	class CDbCommandUpdateHandler :
		public CDbCommandHandler
	{
	public:
		CDbCommandUpdateHandler(CDbThread* pDbThread);
		virtual ~CDbCommandUpdateHandler();

		virtual uint32_t	onDbCommand(const google::protobuf::Message* pRequestMessage, std::shared_ptr<google::protobuf::Message>* pResponseMessage);
	};
}