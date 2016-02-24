#include "stdafx.h"

#include "db_result_binder.h"
#include "db_statement.h"

#include "libBaseCommon/debug_helper.h"

namespace base
{
	CDbResultBinder::CDbResultBinder(CDbStatement* pDbStatement)
		: CDbBinder(pDbStatement->getMysqlStmt(), pDbStatement->getResultCount())
	{
	}

	CDbResultBinder::~CDbResultBinder()
	{
	}

	void CDbResultBinder::bind()
	{
		if (this->m_pMysqlStmt->mysql == nullptr)
		{
			PrintWarning("this->m_pMysqlStmt->mysql == nullptr");
			return;
		}

		my_bool nError = mysql_stmt_bind_result(this->m_pMysqlStmt, this->m_pBind);
		if (nError != 0)
		{
			PrintWarning("bind result error %d %s", nError, mysql_stmt_error(this->m_pMysqlStmt));
			return;
		}
		this->m_bBind = true;
	}

	void CDbResultBinder::bindInt64(uint32_t nIndex, int64_t& nValue, bool& bIsNull, bool& bError)
	{
		this->setValue(nIndex, MYSQL_TYPE_LONGLONG, &nValue, false, &bIsNull, &bError);
	}

	void CDbResultBinder::bindUInt64(uint32_t nIndex, uint64_t& nValue, bool& bIsNull, bool& bError)
	{
		this->setValue(nIndex, MYSQL_TYPE_LONGLONG, &nValue, true, &bIsNull, &bError);
	}

	void CDbResultBinder::bindInt32(uint32_t nIndex, int32_t& nValue, bool& bIsNull, bool& bError)
	{
		this->setValue(nIndex, MYSQL_TYPE_LONG, &nValue, false, &bIsNull, &bError);
	}

	void CDbResultBinder::bindUInt32(uint32_t nIndex, uint32_t& nValue, bool& bIsNull, bool& bError)
	{
		this->setValue(nIndex, MYSQL_TYPE_LONG, &nValue, true, &bIsNull, &bError);
	}

	void CDbResultBinder::bindInt16(uint32_t nIndex, int16_t& nValue, bool& bIsNull, bool& bError)
	{
		this->setValue(nIndex, MYSQL_TYPE_SHORT, &nValue, false, &bIsNull, &bError);
	}

	void CDbResultBinder::bindUInt16(uint32_t nIndex, uint16_t& nValue, bool& bIsNull, bool& bError)
	{
		this->setValue(nIndex, MYSQL_TYPE_SHORT, &nValue, true, &bIsNull, &bError);
	}

	void CDbResultBinder::bindInt8(uint32_t nIndex, int8_t& nValue, bool& bIsNull, bool& bError)
	{
		this->setValue(nIndex, MYSQL_TYPE_TINY, &nValue, false, &bIsNull, &bError);
	}

	void CDbResultBinder::bindUInt8(uint32_t nIndex, uint8_t& nValue, bool& bIsNull, bool& bError)
	{
		this->setValue(nIndex, MYSQL_TYPE_TINY, &nValue, true, &bIsNull, &bError);
	}

	void CDbResultBinder::bindFloat(uint32_t nIndex, float& fValue, bool& bIsNull, bool& bError)
	{
		this->setValue(nIndex, MYSQL_TYPE_FLOAT, &fValue, false, &bIsNull, &bError);
	}

	void CDbResultBinder::bindDouble(uint32_t nIndex, double& fValue, bool& bIsNull, bool& bError)
	{
		this->setValue(nIndex, MYSQL_TYPE_DOUBLE, &fValue, false, &bIsNull, &bError);
	}

	void CDbResultBinder::bindBlob(uint32_t nIndex, void* pBuf, uint32_t nBufSize, _ulong& nResultSize, bool& bIsNull, bool& bError)
	{
		this->setBuffer(nIndex, MYSQL_TYPE_BLOB, pBuf, nBufSize, &nResultSize, &bIsNull, &bError);
	}

	void CDbResultBinder::bindText(uint32_t nIndex, void* pBuf, uint32_t nBufSize, _ulong& nResultSize, bool& bIsNull, bool& bError)
	{
		this->setBuffer(nIndex, MYSQL_TYPE_VAR_STRING, pBuf, nBufSize, &nResultSize, &bIsNull, &bError);
	}
}