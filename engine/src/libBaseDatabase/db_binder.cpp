#include "stdafx.h"

#include "db_binder.h"
#include "db_statement.h"

#include "libBaseCommon/debug_helper.h"

namespace base
{
	CDbBinder::CDbBinder(MYSQL_STMT* pMysqlStmt, uint32_t nBindCount)
		: m_pMysqlStmt(pMysqlStmt)
		, m_bBind(false)
		, m_nBindCount(nBindCount)
		, m_pBind(new MYSQL_BIND[nBindCount])
	{
	}

	CDbBinder::~CDbBinder()
	{
		SAFE_DELETE_ARRAY(this->m_pBind);
	}

	void CDbBinder::setValue(uint32_t nIndex, enum_field_types eType, void* pBuf, bool bUnsigned, bool* pIsNull, bool* pError)
	{
		DebugAst(nIndex < this->m_nBindCount);

		MYSQL_BIND& bind = this->m_pBind[nIndex];
		memset(&bind, 0, sizeof(bind));
		bind.buffer_type = eType;
		bind.buffer = pBuf;
		bind.is_unsigned = bUnsigned;
		bind.is_null = (my_bool*)pIsNull;
		bind.error = (my_bool*)pError;
		this->m_bBind = false;
	}

	void CDbBinder::setBuffer(uint32_t nIndex, enum_field_types eType, void* pBuf, uint32_t nBufSize, _ulong* pResultSize, bool* pIsNull, bool* pError)
	{
		DebugAst(nIndex < this->m_nBindCount);
		DebugAst(pBuf != nullptr);

		MYSQL_BIND& bind = this->m_pBind[nIndex];
		memset(&bind, 0, sizeof(bind));
		bind.buffer_type = eType;
		bind.buffer = pBuf;
		bind.length = pResultSize;
		if (eType == MYSQL_TYPE_VARCHAR)
		{
			bind.buffer_length = nBufSize;
			bind.length_value = nBufSize;
		}
		else
		{
			bind.buffer_length = nBufSize;
		}
		bind.is_null = (my_bool*)pIsNull;
		bind.error = (my_bool*)pError;
		this->m_bBind = false;
	}

	bool CDbBinder::isBind() const
	{
		return this->m_bBind;
	}
}