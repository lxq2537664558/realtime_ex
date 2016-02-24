#pragma once
#include <string>

#include "db_facade.h"


namespace base
{
	class CDbConnection;
	class CDbParamBinder;
	class CDbResultBinder;
	class CDbStatement :
		public IDbStatement
	{
	public:
		CDbStatement();
		virtual ~CDbStatement();

		bool						init(CDbConnection* pDbConnection, MYSQL_STMT* pStmt, const char* szSql);
		virtual uint32_t			execute();
		virtual IDbParamBinder*		getParamBinder();
		virtual IDbResultBinder*	getResultBinder();
		virtual uint32_t			getParamCount() const;
		virtual uint32_t			getResultCount() const;
		virtual uint64_t			getRowCount() const;
		virtual int64_t				getInsertID() const;
		virtual bool				fatchNextRow();
		virtual bool				locate(uint64_t nRow);
		virtual void				release();

		MYSQL_STMT*					getMysqlStmt() const;
		const char*					getSql() const;

	private:
		MYSQL_STMT*			m_pStmt;

		CDbConnection*		m_pDbConnection;
		CDbParamBinder*		m_pDbParamBinder;
		CDbResultBinder*	m_pDbResultBinder;
		uint32_t			m_nResultCount;
		uint32_t			m_nParamCount;
		uint64_t			m_nRowCount;
		std::string			m_szSql;
	};
}