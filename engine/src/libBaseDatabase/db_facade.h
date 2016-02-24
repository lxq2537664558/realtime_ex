#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#endif

#include <list>

#include "mysql/mysql.h"

#include "database.h"

namespace base
{
	class CDbConnection;
	class CDbFacade :
		public IDbFacade
	{
	public:
		CDbFacade();
		virtual ~CDbFacade();

		virtual IDbConnection*	createConnection();
		virtual uint32_t		getConnectionCount() const;
		virtual void			release();

		void					delConnectionCount(CDbConnection* pDbConnection);

	private:
		std::list<CDbConnection*>	m_listConnection;
	};
}