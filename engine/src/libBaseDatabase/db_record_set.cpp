#include "db_record_set.h"
#include "db_connection.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/string_util.h"
#include "libBaseCommon/variant.h"

namespace base
{
	CDbRecordset::CDbRecordset()
		: m_pConnection(nullptr)
		, m_pRes(nullptr)
		, m_curRow(nullptr)
		, m_nRowCount(0)
		, m_nFieldCount(0)
	{
	}

	CDbRecordset::~CDbRecordset()
	{
		mysql_free_result(this->m_pRes);
	}

	bool CDbRecordset::init(CDbConnection* pDbConnection, MYSQL_RES* pRes, const std::string& szSQL)
	{
		DebugAstEx(pDbConnection != nullptr && pRes != nullptr, false);

		this->m_pConnection = pDbConnection;
		this->m_pRes = pRes;
		this->m_curRow = nullptr;
		this->m_nFieldCount = mysql_num_fields(pRes);
		this->m_nRowCount = (uint64_t)mysql_num_rows(pRes);
		this->m_pField = mysql_fetch_fields(pRes);

		this->m_szSQL = szSQL;

		return true;
	}

	uint32_t CDbRecordset::getFieldCount() const
	{
		return this->m_nFieldCount;
	}

	uint64_t CDbRecordset::getRowCount() const
	{
		return this->m_nRowCount;
	}

	bool CDbRecordset::fatchNextRow()
	{
		this->m_curRow = mysql_fetch_row(this->m_pRes);

		return nullptr != this->m_curRow;
	}

	bool CDbRecordset::locate(uint64_t nRow)
	{
		DebugAstEx(nRow < this->m_nRowCount, false);

		mysql_data_seek(this->m_pRes, nRow);
		this->m_curRow = mysql_fetch_row(this->m_pRes);

		return nullptr != this->m_curRow;
	}

	CVariant CDbRecordset::getVariantData(uint32_t nCol) const
	{
		DebugAstEx(nCol < this->m_nFieldCount, CVariant());

		if (nullptr == this->m_curRow || nullptr == this->m_pField)
			return CVariant();

		switch (this->m_pField[nCol].type)
		{
		case MYSQL_TYPE_TINY:
		case MYSQL_TYPE_SHORT:
		case MYSQL_TYPE_LONG:
			{
				int32_t nVal = 0;
				if (!base::string_util::convert_to_value(this->m_curRow[nCol], nVal))
					return CVariant();

				return CVariant(nVal);
			}
			break;

		case MYSQL_TYPE_LONGLONG:
			{
				int64_t nVal = 0;
				if (!base::string_util::convert_to_value(this->m_curRow[nCol], nVal))
					return CVariant();

				return CVariant(nVal);
			}
			break;

		case MYSQL_TYPE_STRING:
		case MYSQL_TYPE_VAR_STRING:
		case MYSQL_TYPE_VARCHAR:
		case MYSQL_TYPE_BLOB:
		case MYSQL_TYPE_LONG_BLOB:
		case MYSQL_TYPE_TINY_BLOB:
			{
				return CVariant(this->m_curRow[nCol], this->m_pField[nCol].max_length);
			}
			break;

		default:
			{
				PrintWarning("CDbRecordset::getData invaild type {}", (int32_t)this->m_pField[nCol].type);
			}
		}

		return CVariant();
	}

	std::string CDbRecordset::getData(uint32_t nCol) const
	{
		std::string szData;
		DebugAstEx(nCol < this->m_nFieldCount, szData);

		if (nullptr == this->m_curRow)
			return szData;

		const char* szRawData = this->m_curRow[nCol];
		if (nullptr == szRawData)
			return szData;

		szData.assign(szRawData, this->m_pField[nCol].max_length);

		return szData;
	}

	const std::string& CDbRecordset::getSQL() const
	{
		return this->m_szSQL;
	}

	const char* CDbRecordset::getFieldName(uint32_t nCol) const
	{
		DebugAstEx(nCol < this->m_nFieldCount, nullptr);

		if (nullptr == this->m_pField)
			return nullptr;

		return this->m_pField[nCol].name;
	}
}