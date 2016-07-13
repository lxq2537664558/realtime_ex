#pragma once

#include <list>

#include "db_facade.h"

namespace base
{
	class CDbRecordset;
	class CDbStatement;
	class CDbConnection :
		public IDbConnection
	{
		friend class CDbFacade;

	public:
		CDbConnection();
		virtual ~CDbConnection();

		virtual bool			connect(const char* szHost, uint16_t nPort, const char* szUser, const char* szPassword, const char* szDbname, const char* szCharacterSet);
		virtual bool			isConnect() const;

		virtual void			close();
		virtual void			release();
		virtual uint32_t		getStatementCount() const;
		virtual uint32_t		getRecordsetCount() const;
		virtual IDbStatement*	createStatement(const char* szSql);
		virtual IDbRecordset*	execute(const char* szSql);
		virtual bool			ping();
		virtual uint64_t		getAffectedRow() const;
		virtual void*			getMysql() const;
		virtual void			escape(char* szDst, const char* szSrc, size_t nLength);

		void					delRecordsetCount(CDbRecordset* pDbRecordset);
		void					delStmtCountCount(CDbStatement* pDbStatement);

	private:
		MYSQL*						m_pMysql;
		CDbFacade*					m_pDbFacade;
		std::list<CDbRecordset*>	m_listDbRecordset;
		std::list<CDbStatement*>	m_listDbStatement;
	};
}