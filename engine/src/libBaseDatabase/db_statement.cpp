#include "stdafx.h"

#include "db_statement.h"
#include "db_connection.h"
#include "db_param_binder.h"
#include "db_result_binder.h"

#include "libBaseCommon/debug_helper.h"

// stmt操作函数返回值不表示错误码，需要调用mysql_stmt_errno去取

namespace base
{
	CDbStatement::CDbStatement()
		: m_pDbConnection(nullptr)
		, m_pStmt(nullptr)
		, m_nParamCount(0)
		, m_nResultCount(0)
		, m_nRowCount(0)
		, m_pDbParamBinder(nullptr)
		, m_pDbResultBinder(nullptr)
	{
	}

	CDbStatement::~CDbStatement()
	{
		SAFE_DELETE(this->m_pDbParamBinder);
		SAFE_DELETE(this->m_pDbResultBinder);

		if (mysql_stmt_free_result(this->m_pStmt) != 0)
			PrintWarning("mysql_stmt_free_result error: %d, error: %s sql: %s", mysql_stmt_errno(this->m_pStmt), mysql_stmt_error(this->m_pStmt), this->m_szSql.c_str());

		mysql_stmt_close(this->m_pStmt);
		this->m_pDbConnection->delStmtCountCount(this);
	}

	bool CDbStatement::init(CDbConnection* pDbConnection, MYSQL_STMT* pStmt, const char* szSql)
	{
		DebugAstEx(pDbConnection != nullptr && pStmt != nullptr && szSql != nullptr, false);

		this->m_pDbConnection = pDbConnection;
		this->m_pStmt = pStmt;
		this->m_nParamCount = mysql_stmt_param_count(pStmt);
		this->m_nResultCount = mysql_stmt_field_count(pStmt);
		this->m_nRowCount = 0;
		this->m_pDbParamBinder = nullptr;
		this->m_pDbResultBinder = nullptr;
		if (0 != this->m_nParamCount)
			this->m_pDbParamBinder = new CDbParamBinder(this);
		if (0 != this->m_nResultCount)
			this->m_pDbResultBinder = new CDbResultBinder(this);

		this->m_szSql = szSql;

		return true;
	}

	uint32_t CDbStatement::execute()
	{
		if (this->m_pStmt->mysql == nullptr)
		{
			PrintWarning("this->m_pMysqlStmt->mysql == nullptr");
			return eDBET_LostConnection;
		}

		if (this->m_pDbParamBinder != nullptr)
			DebugAstEx(this->m_pDbParamBinder->isBind(), INVALID_32BIT);

		if (this->m_pDbResultBinder != nullptr)
			DebugAstEx(this->m_pDbResultBinder->isBind(), INVALID_32BIT);

		if (mysql_stmt_free_result(this->m_pStmt) != 0)
		{
			int32_t nError = mysql_stmt_errno(this->m_pStmt);
			PrintWarning("mysql_stmt_free_result error: %d, error: %s sql: %s", nError, mysql_stmt_error(this->m_pStmt), this->m_szSql.c_str());
			return nError;
		}
		if (mysql_stmt_execute(this->m_pStmt) != 0)
		{
			int32_t nError = mysql_stmt_errno(this->m_pStmt);
			PrintWarning("mysql_stmt_execute error: %d, error: %s sql: %s", nError, mysql_stmt_error(this->m_pStmt), this->m_szSql.c_str());
			return nError;
		}
		if (mysql_stmt_store_result(this->m_pStmt) != 0)
		{
			int32_t nError = mysql_stmt_errno(this->m_pStmt);
			PrintWarning("store reslut error: %d, error: %s sql: %s", nError, mysql_stmt_error(this->m_pStmt), this->m_szSql.c_str());
			return nError;
		}
		this->m_nRowCount = (uint64_t)mysql_stmt_num_rows(this->m_pStmt);

		return eDBET_Success;
	}

	IDbParamBinder* CDbStatement::getParamBinder()
	{
		return this->m_pDbParamBinder;
	}

	IDbResultBinder* CDbStatement::getResultBinder()
	{
		return this->m_pDbResultBinder;
	}

	uint32_t CDbStatement::getParamCount() const
	{
		return this->m_nParamCount;
	}

	uint32_t CDbStatement::getResultCount() const
	{
		return this->m_nResultCount;
	}

	uint64_t CDbStatement::getRowCount() const
	{
		return this->m_nRowCount;
	}

	int64_t CDbStatement::getInsertID() const
	{
		return mysql_stmt_insert_id(this->m_pStmt);
	}

	bool CDbStatement::fatchNextRow()
	{
		return mysql_stmt_fetch(this->m_pStmt) == 0;
	}

	bool CDbStatement::locate(uint64_t nRow)
	{
		DebugAstEx(nRow < this->m_nRowCount, false);

		mysql_stmt_data_seek(this->m_pStmt, nRow);
		
		return mysql_stmt_fetch(this->m_pStmt) == 0;
	}

	void CDbStatement::release()
	{
		delete this;
	}

	MYSQL_STMT* CDbStatement::getMysqlStmt() const
	{
		return this->m_pStmt;
	}

	const char* CDbStatement::getSql() const
	{
		return this->m_szSql.c_str();
	}
}