#include "stdafx.h"

#include "db_connection.h"
#include "db_record_set.h"
#include "db_facade.h"
#include "db_statement.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/logger.h"

namespace base
{
	CDbConnection::CDbConnection()
		: m_pMySql(nullptr)
		, m_pDbFacade(nullptr)
	{
	}

	bool CDbConnection::connect(const char* szHost, uint16_t nPort, const char* szUser, const char* szPassword, const char* szDbname, const char* szCharacterSet)
	{
		DebugAstEx(!this->isConnect(), false);

		this->m_pMySql = mysql_init(nullptr);
		if (nullptr == this->m_pMySql)
		{
			PrintWarning("mysql_init error");
			return false;
		}

		/*
		CLIENT_FOUND_ROWS 为了使mysql_affected_rows返回的数量完全有where语句控制，而不是因为数据没有变动就返回0
		CLIENT_MULTI_STATEMENTS 为了可以在一次执行mysql_real_query中可以执行多条sql（用;隔开），但是这样会导致出现SQL注入的情况，所以这里禁用了
		*/
		if (this->m_pMySql != mysql_real_connect(this->m_pMySql, szHost, szUser, szPassword, szDbname, nPort, nullptr, CLIENT_FOUND_ROWS))
		{
			PrintWarning("mysql_real_connect error: %s", mysql_error(this->m_pMySql));
			mysql_close(this->m_pMySql);
			this->m_pMySql = nullptr;
			return false;
		}
		if (0 != mysql_set_character_set(this->m_pMySql, szCharacterSet))
		{
			PrintWarning("mysql_set_character_set error: %s", mysql_error(this->m_pMySql));
			mysql_close(this->m_pMySql);
			this->m_pMySql = nullptr;
			return false;
		}
		return true;
	}

	bool CDbConnection::isConnect() const
	{
		return this->m_pMySql != nullptr;
	}

	CDbConnection::~CDbConnection()
	{
		this->m_pDbFacade->delConnectionCount(this);

		if (this->isConnect())
			this->close();
	}

	void CDbConnection::close()
	{
		if (!this->isConnect())
			return;

		for (auto iter = this->m_listDbStatement.begin(); iter != this->m_listDbStatement.end(); ++iter)
		{
			CDbStatement* pDbStatement = *iter;
			SAFE_RELEASE(pDbStatement);
		}

		for (auto iter = this->m_listDbRecordset.begin(); iter != this->m_listDbRecordset.end(); ++iter)
		{
			CDbRecordset* pDbRecordset = *iter;
			SAFE_RELEASE(pDbRecordset);
		}

		mysql_close(this->m_pMySql);
		this->m_pMySql = nullptr;
	}

	void CDbConnection::release()
	{
		delete this;
	}

	uint32_t CDbConnection::getRecordsetCount() const
	{
		return (uint32_t)this->m_listDbRecordset.size();
	}

	uint32_t CDbConnection::getStatementCount() const
	{
		return (uint32_t)this->m_listDbStatement.size();
	}

	IDbStatement* CDbConnection::createStatement(const char* szSql)
	{
		DebugAstEx(szSql != nullptr, nullptr);
		DebugAstEx(this->isConnect(), nullptr);

		MYSQL_STMT* pStmt = mysql_stmt_init(this->m_pMySql);
		size_t nLen = strlen(szSql);
		if (0 != mysql_stmt_prepare(pStmt, szSql, (unsigned long)nLen))
		{
			PrintWarning("mysql_stmt_prepare error: %d, error: %s sql: %s", mysql_stmt_errno(pStmt), mysql_stmt_error(pStmt), szSql);
			return nullptr;
		}

		CDbStatement* pDbStatement = new CDbStatement();
		if (!pDbStatement->init(this, pStmt, szSql))
		{
			SAFE_RELEASE(pDbStatement);
			return nullptr;
		}

		this->m_listDbStatement.push_back(pDbStatement);

		return pDbStatement;
	}

	IDbRecordset* CDbConnection::execute(const char* szSql)
	{
		DebugAstEx(szSql != nullptr, nullptr);
		DebugAstEx(this->isConnect(), nullptr);

		size_t nLen = strlen(szSql);
		int32_t nError = mysql_real_query(this->m_pMySql, szSql, (unsigned long)nLen);
		if (0 != nError)
		{
			PrintWarning("mysql_real_query error: %d, error: %s sql: %s", nError, mysql_error(this->m_pMySql), szSql);
			return nullptr;
		}
		MYSQL_RES* pRes = mysql_store_result(this->m_pMySql);
		if (nullptr == pRes)
		{
			if (0 != mysql_errno(this->m_pMySql))
				PrintWarning("mysql_store_result error: %s sql: %s", mysql_error(this->m_pMySql), szSql);
			return nullptr;
		}

		CDbRecordset *pDbRecordset = new CDbRecordset();
		if (!pDbRecordset->init(this, pRes, szSql))
			SAFE_RELEASE(pDbRecordset);

		this->m_listDbRecordset.push_back(pDbRecordset);

		return pDbRecordset;
	}

	bool CDbConnection::ping()
	{
		DebugAstEx(this->isConnect(), false);

		return 0 == mysql_ping(this->m_pMySql);
	}

	uint64_t CDbConnection::getAffectedRow() const
	{
		DebugAstEx(this->isConnect(), 0);

		return (uint64_t)mysql_affected_rows(this->m_pMySql);
	}

	void CDbConnection::delRecordsetCount(CDbRecordset* pDbRecordset)
	{
		DebugAst(pDbRecordset != nullptr);

		this->m_listDbRecordset.remove(pDbRecordset);
	}

	void CDbConnection::delStmtCountCount(CDbStatement* pDbStatement)
	{
		DebugAst(pDbStatement != nullptr);

		this->m_listDbStatement.remove(pDbStatement);
	}
}