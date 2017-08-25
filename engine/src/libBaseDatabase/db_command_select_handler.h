#pragma once
#include "db_command_handler.h"

namespace base
{
	class CDbCommandSelectHandler :
		public CDbCommandHandler
	{
	public:
		CDbCommandSelectHandler();
		virtual ~CDbCommandSelectHandler();

		virtual uint32_t	onDbCommand(const google::protobuf::Message* pRequestMessage, std::shared_ptr<google::protobuf::Message>* pResponseMessage);
	};
}