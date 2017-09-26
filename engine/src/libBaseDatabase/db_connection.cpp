#include "db_connection.h"
#include "db_record_set.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/logger.h"

#include <sstream>

namespace base
{
	CDbConnection::CDbConnection()
		: m_pMysql(nullptr)
	{
	}

	bool CDbConnection::connect(const std::string& szHost, uint16_t nPort, const std::string& szUser, const std::string& szPassword, const std::string& szDbname, const std::string& szCharacterset)
	{
		DebugAstEx(!this->isConnect(), false);

		this->m_pMysql = mysql_init(nullptr);
		if (nullptr == this->m_pMysql)
		{
			PrintWarning("mysql_init error");
			return false;
		}

		/*
		CLIENT_FOUND_ROWS Ϊ��ʹmysql_affected_rows���ص�������ȫ��where�����ƣ���������Ϊ����û�б䶯�ͷ���0
		CLIENT_MULTI_STATEMENTS Ϊ�˿�����һ��ִ��mysql_real_query�п���ִ�ж���sql����;�����������������ᵼ��SQLע��ǳ����ף�ͨ��union����������һ���������SQLע�룩���������������
		CLIENT_MULTI_RESULTS ����call�п��ܵ��ô洢���̣����û�����������ã����ô洢���̻ᱨ�������������Ǻ���Ҫ��ȡrecord_set��ʱ���ȡ��û��Ϊֹ
		*/
		if (this->m_pMysql != mysql_real_connect(this->m_pMysql, szHost.c_str(), szUser.c_str(), szPassword.c_str(), szDbname.c_str(), nPort, nullptr, CLIENT_FOUND_ROWS| CLIENT_MULTI_RESULTS))
		{
			PrintWarning("mysql_real_connect error_no: {} error: {}", mysql_errno(this->m_pMysql), mysql_error(this->m_pMysql));
			mysql_close(this->m_pMysql);
			this->m_pMysql = nullptr;
			return false;
		}

		if (0 != mysql_set_character_set(this->m_pMysql, szCharacterset.c_str()))
		{
			PrintWarning("mysql_set_character_set error_no: {} error: {}", mysql_errno(this->m_pMysql), mysql_error(this->m_pMysql));
			mysql_close(this->m_pMysql);
			this->m_pMysql = nullptr;
			return false;
		}

		return true;
	}

	bool CDbConnection::isConnect() const
	{
		return this->m_pMysql != nullptr;
	}

	CDbConnection::~CDbConnection()
	{
		this->close();
	}

	void CDbConnection::close()
	{
		if (!this->isConnect())
			return;

		mysql_close(this->m_pMysql);
		this->m_pMysql = nullptr;
	}

	uint32_t CDbConnection::execute(const std::string& szSQL, CDbRecordset** pDbRecordset)
	{
		DebugAstEx(this->isConnect(), eMET_Unknwon);

		if (0 != mysql_real_query(this->m_pMysql, szSQL.c_str(), (unsigned long)szSQL.size()))
		{
			uint32_t nError = mysql_errno(this->m_pMysql);
			PrintWarning("mysql_real_query error_no: {}, error: {} sql: {}", nError, mysql_error(this->m_pMysql), szSQL);
			return nError;
		}
		MYSQL_RES* pRes = mysql_store_result(this->m_pMysql);
		if (nullptr == pRes)
		{
			uint32_t nError = mysql_errno(this->m_pMysql);
			if (0 != nError)
				PrintWarning("mysql_store_result error_no: {} error: {} sql: {}", nError, mysql_error(this->m_pMysql), szSQL);

			return nError;
		}

		while (true)
		{
			int32_t nRet = mysql_next_result(this->m_pMysql);
			if (nRet < 0)
				break;

			if (nRet == 0)
			{
				MYSQL_RES* pNextRes = mysql_store_result(this->m_pMysql);
				if (pNextRes != nullptr)
				{
					mysql_free_result(pNextRes);
				}
			}
			else
			{
				PrintWarning("mysql_next_result error: {} sql: {}", mysql_error(this->m_pMysql), szSQL);
				break;
			}
		}

		DebugAstEx(pDbRecordset != nullptr, eMET_Unknwon);

		*pDbRecordset = new CDbRecordset();
		if (!(*pDbRecordset)->init(this, pRes, szSQL))
		{
			SAFE_DELETE(*pDbRecordset);
			return eMET_Unknwon;
		}
		return eMET_OK;
	}

	bool CDbConnection::ping()
	{
		DebugAstEx(this->isConnect(), false);

		return 0 == mysql_ping(this->m_pMysql);
	}

	uint64_t CDbConnection::getAffectedRow() const
	{
		DebugAstEx(this->isConnect(), 0);

		return (uint64_t)mysql_affected_rows(this->m_pMysql);
	}

	std::string CDbConnection::escape(const std::string& szSQL)
	{
		DebugAstEx(this->m_pMysql != nullptr, "");
		if (szSQL.empty())
			return "";

		std::string szBuf;
		szBuf.resize(szSQL.size() * 2);

		unsigned long nSize = mysql_real_escape_string(this->m_pMysql, &szBuf[0], szSQL.c_str(), (unsigned long)szSQL.size());
		szBuf.resize(nSize);

		return szBuf;
	}

	void CDbConnection::autoCommit(bool enable)
	{
		std::ostringstream oss;
		oss << "set autocommit = " << (enable ? 1 : 0);
		this->execute(oss.str(), nullptr);
	}

	void CDbConnection::begin()
	{
		this->execute("begin", nullptr);
	}

	void CDbConnection::commit()
	{
		this->execute("commit", nullptr);
	}

	void CDbConnection::rollback()
	{
		this->execute("rollback", nullptr);
	}
}