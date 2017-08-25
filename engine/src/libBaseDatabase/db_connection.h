#pragma once

#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

#include "mysql/mysql.h"

namespace base
{
	enum EMysqlErrorType
	{
		eMET_OK						= 0,
		eMET_LostConnection			= 1,	// 连接丢失
		eMET_NoPrivileges			= 1044, // 数据库用户权限不足
		eMET_NoDiskSpace			= 1021, // 硬盘剩余空间不足
		eMET_KeyDupForUpdate		= 1022, // 关键字重复，更改记录失败
		eMET_NoRecord				= 1032, // 记录不存在
		eMET_NoMemory				= 1037, // 系统内存不足
		eMET_OutofConnection		= 1040, // 已到达数据库的最大连接数，请加大数据库可用连接数
		eMET_SQLSyntax				= 1149, // SQL语句语法错误
		eMET_KeyDupForInsert		= 1062,	// 关键字重复，插入记录失败
		eMET_CommitTrans			= 1180, // 提交事务失败
		eMET_RollbackTrans			= 1181, // 回滚事务失败
		eMET_Deadloop				= 1205, // 加锁超时
		eMET_StatementReprepared	= 1615,	// Statement需要重新准备
		
		eMET_Unknwon = -1
	};


	class CDbRecordset;
	class CDbConnection
	{
		friend class CDbFacade;

	public:
		CDbConnection();
		virtual ~CDbConnection();

		bool			connect(const std::string& szHost, uint16_t nPort, const std::string& szUser, const std::string& szPassword, const std::string& szDbname, const std::string& szCharacterset);
		bool			isConnect() const;
		void			close();
		
		uint32_t		execute(const std::string& szSQL, CDbRecordset** pDbRecordset);
		bool			ping();
		uint64_t		getAffectedRow() const;
		void			autoCommit(bool enable);
		void			begin();
		void			commit();
		void			rollback();
		std::string		escape(const std::string& szSQL);

	private:
		MYSQL*	m_pMysql;	
	};
}