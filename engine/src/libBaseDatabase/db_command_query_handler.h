#pragma once
#include "db_command_handler.h"

namespace base
{
	class CDbCommandQueryHandler :
		public CDbCommandHandler
	{
	public:
		CDbCommandQueryHandler(CDbThread* pDbThread);
		virtual ~CDbCommandQueryHandler();

		virtual uint32_t	onDbCommand(const google::protobuf::Message* pRequestMessage, std::shared_ptr<google::protobuf::Message>* pResponseMessage);
	};
}