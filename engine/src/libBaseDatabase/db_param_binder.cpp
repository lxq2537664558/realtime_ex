#include "stdafx.h"

#include "db_param_binder.h"
#include "db_statement.h"

#include "libBaseCommon/debug_helper.h"

namespace base
{
	CDbParamBinder::CDbParamBinder(CDbStatement* pDbStatement)
		: CDbBinder(pDbStatement->getMysqlStmt(), pDbStatement->getParamCount())
	{
	}

	CDbParamBinder::~CDbParamBinder()
	{

	}

	void CDbParamBinder::bind()
	{
		if (this->m_pMysqlStmt->mysql == nullptr)
		{
			PrintWarning("this->m_pMysqlStmt->mysql == nullptr");
			return;
		}
		my_bool nError = mysql_stmt_bind_param(this->m_pMysqlStmt, this->m_pBind);
		if (nError != 0)
		{
			PrintWarning("bind param error %d %s", nError, mysql_stmt_error(this->m_pMysqlStmt));
			return;
		}
		this->m_bBind = true;
	}

	void CDbParamBinder::setInt64(uint32_t nIndex, int64_t& nValue)
	{
		this->setValue(nIndex, MYSQL_TYPE_LONGLONG, &nValue, false, nullptr, nullptr);
	}

	void CDbParamBinder::setUInt64(uint32_t nIndex, uint64_t& nValue)
	{
		this->setValue(nIndex, MYSQL_TYPE_LONGLONG, &nValue, true, nullptr, nullptr);
	}

	void CDbParamBinder::setInt32(uint32_t nIndex, int32_t& nValue)
	{
		this->setValue(nIndex, MYSQL_TYPE_LONG, &nValue, false, nullptr, nullptr);
	}

	void CDbParamBinder::setUInt32(uint32_t nIndex, uint32_t& nValue)
	{
		this->setValue(nIndex, MYSQL_TYPE_LONG, &nValue, true, nullptr, nullptr);
	}

	void CDbParamBinder::setInt16(uint32_t nIndex, int16_t& nValue)
	{
		this->setValue(nIndex, MYSQL_TYPE_SHORT, &nValue, false, nullptr, nullptr);
	}

	void CDbParamBinder::setUInt16(uint32_t nIndex, uint16_t& nValue)
	{
		this->setValue(nIndex, MYSQL_TYPE_SHORT, &nValue, true, nullptr, nullptr);
	}

	void CDbParamBinder::setInt8(uint32_t nIndex, int8_t& nValue)
	{
		this->setValue(nIndex, MYSQL_TYPE_TINY, &nValue, false, nullptr, nullptr);
	}

	void CDbParamBinder::setUInt8(uint32_t nIndex, uint8_t& nValue)
	{
		this->setValue(nIndex, MYSQL_TYPE_TINY, &nValue, true, nullptr, nullptr);
	}

	void CDbParamBinder::setFloat(uint32_t nIndex, float& fValue)
	{
		this->setValue(nIndex, MYSQL_TYPE_FLOAT, &fValue, false, nullptr, nullptr);
	}

	void CDbParamBinder::setDouble(uint32_t nIndex, double& fValue)
	{
		this->setValue(nIndex, MYSQL_TYPE_DOUBLE, &fValue, false, nullptr, nullptr);
	}

	void CDbParamBinder::setBlob(uint32_t nIndex, void* pBuf, uint32_t nBufSize)
	{
		this->setBuffer(nIndex, MYSQL_TYPE_BLOB, pBuf, nBufSize, nullptr, nullptr, nullptr);
	}

	void CDbParamBinder::setText(uint32_t nIndex, void* pBuf, uint32_t nBufSize)
	{
		this->setBuffer(nIndex, MYSQL_TYPE_VARCHAR, pBuf, nBufSize, nullptr, nullptr, nullptr);
	}
}