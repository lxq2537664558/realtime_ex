#pragma once
#include "db_connection.h"
#include "database.h"

#include <string>
#include <functional>

#include "google/protobuf/message.h"

namespace base
{
	struct SDbCommand
	{
		uint32_t	nType;
		google::protobuf::Message*
					pMessage;
		std::function<void(const google::protobuf::Message*, uint32_t)>
					callback;
	};

	struct SDbResultInfo
	{
		uint32_t	nErrorCode;
		std::shared_ptr<google::protobuf::Message>
					pMessage;
		std::function<void(const google::protobuf::Message*, uint32_t)>
					callback;
	};

	class CDbThread;
	class CDbCommandHandler
	{
	public:
		CDbCommandHandler(CDbThread* pDbThread) : m_pDbThread(pDbThread), m_pDbConnection(nullptr) { }
		virtual ~CDbCommandHandler() {}

		virtual void		onConnect(CDbConnection* pDbConnection) { this->m_pDbConnection = pDbConnection; }
		virtual void		onDisconnect() { this->m_pDbConnection = nullptr; }
		virtual uint32_t	onDbCommand(const google::protobuf::Message* pRequestMessage, std::shared_ptr<google::protobuf::Message>* pResponseMessage) = 0;

		CDbThread*			getDbThread() const { return this->m_pDbThread; }

	protected:
		CDbConnection*	m_pDbConnection;
		CDbThread*		m_pDbThread;
	};
}