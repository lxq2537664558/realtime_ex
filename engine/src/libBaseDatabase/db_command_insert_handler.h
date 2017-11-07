#pragma once
#include "db_command_handler.h"

namespace base
{
	class CDbCommandInsertHandler :
		public CDbCommandHandler
	{
	public:
		CDbCommandInsertHandler(CDbThread* pDbThread);
		virtual ~CDbCommandInsertHandler();

		virtual uint32_t	onDbCommand(const google::protobuf::Message* pRequestMessage, std::shared_ptr<google::protobuf::Message>* pResponseMessage);
	};
}