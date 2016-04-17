#pragma once
#include "db_facade.h"
#include "libBaseCommon/noncopyable.h"

namespace base
{
	class CDbStatement;
	class CDbBinder :
		public noncopyable
	{
	public:
		bool isBind() const;

	protected:
		CDbBinder(MYSQL_STMT* pMysqlStmt, uint32_t nBindCount);
		~CDbBinder();

		void setValue(uint32_t nIndex, enum_field_types eType, void* pBuf, bool bUnsigned, bool* pIsNull, bool* pError);
		void setBuffer(uint32_t nIndex, enum_field_types eType, void* pBuf, uint32_t nBufSize, _ulong* pResultSize, bool* pIsNull, bool* pError);

	protected:
		uint32_t	m_nBindCount;
		MYSQL_BIND* m_pBind;
		bool        m_bBind;
		MYSQL_STMT* m_pMysqlStmt;
	};
}