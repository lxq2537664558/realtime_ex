#pragma once
#include "db_command_handler.h"

namespace base
{
	class CDbCommandCallHandler :
		public CDbCommandHandler
	{
	public:
		CDbCommandCallHandler(CDbThread* pDbThread);
		virtual ~CDbCommandCallHandler();

		virtual uint32_t	onDbCommand(const google::protobuf::Message* pRequestMessage, std::shared_ptr<google::protobuf::Message>* pResponseMessage);
	};
}