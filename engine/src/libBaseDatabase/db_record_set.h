#pragma once

#include <string>

#include "db_facade.h"

namespace base
{
	class CDbConnection;
	class CDbRecordset :
		public IDbRecordset
	{
	public:
		CDbRecordset();
		virtual ~CDbRecordset();

		bool				init(CDbConnection* pDbConnection, MYSQL_RES* pRes, const char* szSql);
		virtual uint64_t	getRowCount() const;
		virtual uint32_t	getColCount() const;
		virtual bool		fatchNextRow();
		virtual bool		locate(uint64_t nRow);
		virtual CVariant	getData(uint32_t nCol) const;
		virtual void		release();

		const char*			getSql() const;

	private:
		CDbConnection*		m_pConnection;
		MYSQL_RES*			m_pRes;
		MYSQL_ROW			m_ppCurRow;
		uint64_t			m_nRowCount;
		uint32_t			m_nColCount;
		enum_field_types*	m_pFieldType;
		std::string			m_szSql;
	};
}