#include "stdafx.h"

#include "db_record_set.h"
#include "db_connection.h"

#include "libBaseCommon/debug_helper.h"

namespace base
{
	CDbRecordset::CDbRecordset()
		: m_pConnection(nullptr)
		, m_pRes(nullptr)
		, m_ppCurRow(nullptr)
		, m_nRowCount(0)
		, m_nColCount(0)
		, m_pFieldType(nullptr)
	{
	}

	CDbRecordset::~CDbRecordset()
	{
		mysql_free_result(this->m_pRes);
		this->m_pConnection->delRecordsetCount(this);

		SAFE_DELETE_ARRAY(this->m_pFieldType);
	}

	bool CDbRecordset::init(CDbConnection* pDbConnection, MYSQL_RES* pRes, const char* szSql)
	{
		DebugAstEx(pDbConnection != nullptr && pRes != nullptr && szSql != nullptr, false);

		this->m_pConnection = pDbConnection;
		this->m_pRes = pRes;
		this->m_ppCurRow = nullptr;
		this->m_nColCount = mysql_num_fields(pRes);
		this->m_nRowCount = (uint64_t)mysql_num_rows(pRes);
		if (this->m_nColCount > 0)
			this->m_pFieldType = new enum_field_types[this->m_nColCount];

		for (uint32_t i = 0; i < this->m_nColCount; ++i)
		{
			MYSQL_FIELD* pField = mysql_fetch_field_direct(this->m_pRes, i);
			if (pField == nullptr)
				return false;

			this->m_pFieldType[i] = pField->type;
		}

		this->m_szSql = szSql;

		return true;
	}

	uint32_t CDbRecordset::getColCount() const
	{
		return this->m_nColCount;
	}

	uint64_t CDbRecordset::getRowCount() const
	{
		return this->m_nRowCount;
	}

	bool CDbRecordset::fatchNextRow()
	{
		this->m_ppCurRow = mysql_fetch_row(this->m_pRes);

		return nullptr != this->m_ppCurRow;
	}

	bool CDbRecordset::locate(uint64_t nRow)
	{
		DebugAstEx(nRow < this->m_nRowCount, false);

		mysql_data_seek(this->m_pRes, nRow);
		this->m_ppCurRow = mysql_fetch_row(this->m_pRes);

		return nullptr != this->m_ppCurRow;
	}

	CVariant CDbRecordset::getData(uint32_t nCol) const
	{
		DebugAstEx(nCol < this->m_nColCount, CVariant());

		if (nullptr == this->m_ppCurRow || nullptr == this->m_pFieldType)
			return CVariant();

		switch (this->m_pFieldType[nCol])
		{
		case MYSQL_TYPE_TINY:
		case MYSQL_TYPE_SHORT:
		case MYSQL_TYPE_LONG:
			{
				if (this->m_ppCurRow[nCol] != nullptr)
				{
					int32_t nVal = 0;
					base::crt::atoi(this->m_ppCurRow[nCol], nVal);
					return CVariant(nVal);
				}
				else
				{
					return CVariant(0);
				}
			}
			break;

		case MYSQL_TYPE_LONGLONG:
			{
				if (this->m_ppCurRow[nCol] != nullptr)
				{
					int64_t nVal = 0;
					base::crt::atoi64(this->m_ppCurRow[nCol], nVal);
					return CVariant(nVal);
				}
				else
				{
					return CVariant(0);
				}
			}
			break;

		case MYSQL_TYPE_STRING:
		case MYSQL_TYPE_VAR_STRING:
			{
				return CVariant(this->m_ppCurRow[nCol]);
			}
			break;

		case MYSQL_TYPE_BLOB:
			{
				_ulong* pLength = mysql_fetch_lengths(this->m_pRes);
				return CVariant(this->m_ppCurRow[nCol], pLength[nCol]);
			}
			break;

		default:
			{
				PrintWarning("CDbRecordset::getData invaild type %d", (int32_t)this->m_pFieldType[nCol]);
			}
		}
		return CVariant();
	}

	void CDbRecordset::release()
	{
		delete this;
	}

	const char* CDbRecordset::getSql() const
	{
		return this->m_szSql.c_str();
	}
}