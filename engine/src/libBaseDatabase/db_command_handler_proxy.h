#pragma once

#include "db_command_handler.h"

#include <map>

namespace base
{
	class CDbThread;
	class CDbCommandHandlerProxy
	{
	public:
		CDbCommandHandlerProxy(CDbThread* pDbThread);
		~CDbCommandHandlerProxy();

		void		onConnect(CDbConnection* pDbConnection);
		void		onDisconnect();
		uint32_t	onDbCommand(uint32_t nType, google::protobuf::Message* pRequestMessage, std::shared_ptr<google::protobuf::Message>* pResponseMessage);

	private:
		std::map<uint32_t, CDbCommandHandler*>
					m_mapDbCommandHandler;
	};
}