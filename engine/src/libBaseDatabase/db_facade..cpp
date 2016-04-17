#include "stdafx.h"

#include "db_facade.h"
#include "db_connection.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/logger.h"

namespace base
{
	CDbFacade::CDbFacade()
	{
	}

	CDbFacade::~CDbFacade()
	{

	}

	void CDbFacade::release()
	{
		for (auto iter = this->m_listConnection.begin(); iter != this->m_listConnection.end(); ++iter)
		{
			CDbConnection* pDbConnection = *iter;
			SAFE_RELEASE(pDbConnection);
		}
	}

	IDbConnection* CDbFacade::createConnection()
	{
		CDbConnection* pDbConnection = new CDbConnection();
		this->m_listConnection.push_back(pDbConnection);
		pDbConnection->m_pDbFacade = this;
		return pDbConnection;
	}

	uint32_t CDbFacade::getConnectionCount() const
	{
		return (uint32_t)this->m_listConnection.size();
	}

	void CDbFacade::delConnectionCount(CDbConnection* pDbConnection)
	{
		this->m_listConnection.remove(pDbConnection);
	}

	//================================================================

	void startupDatabase()
	{
		mysql_library_init(0, nullptr, nullptr);

		if (!mysql_thread_safe())
			PrintWarning("non-threadsafe client detected!  please rebuild and link with libmysql_r");
	}

	void cleanupDatabase()
	{
		mysql_library_end();
	}

	IDbFacade* createDbFacade()
	{
		return new CDbFacade();
	}
}