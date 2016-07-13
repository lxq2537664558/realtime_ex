#pragma once

#include "libBaseCommon/base_common.h"
#include "libBaseCommon/variant.h"

/*
mysql 获取结果的时候可选择是否缓冲整个结果集，我根据下列分析得出自己的选择结果
1. 查询执行之后，整个结果集肯定被放在mysql内部，这是根据mysql文档:(you must retrieve all the rows even if you determine in mid-retrieval that you've found the information you were looking for.)
2. 结果集在调用StoreResult或者UseResult+全部FetchRow的前后，要么放在mysql内部，要么放在外部，内存没有什么节省,
3. 使用UseResult结果集必须要Fetch全部Row之后才能知道RowCount,这很不方便

因此无论是IDbRecordset还是IDbStatement,都选用StoreResult的方案

这里mysql禁掉了多语句的查询，这样做是为了防止SQL注入放生，如果只有一个语句让执行，那么SQL一旦被注入就会报错，另外用statemenet的方式是不会发生SQL注入，所以尽量用statemenet方式
*/

#ifdef _WIN32
#	ifdef __BUILD_BASE_DATABASE_DLL__
#		define __BASE_DATABASE_API__ __declspec(dllexport)
#	else
#		define __BASE_DATABASE_API__ __declspec(dllimport)
#	endif
#else
#	define __BASE_DATABASE_API__
#endif

namespace base
{
	enum EDBErrorType
	{
		eDBET_Success				= 0,
		eDBET_LostConnection		= 1,	// 连接丢失
		eDBET_NoPrivileges			= 1044, // 数据库用户权限不足
		eDBET_NoDiskSpace			= 1021, // 硬盘剩余空间不足
		eDBET_KeyDupForUpdate		= 1022, // 关键字重复，更改记录失败
		eDBET_NoRecord				= 1032, // 记录不存在
		eDBET_NoMemory				= 1037, // 系统内存不足
		eDBET_OutofConnection		= 1040, // 已到达数据库的最大连接数，请加大数据库可用连接数
		eDBET_SQLSyntax				= 1149, // SQL语句语法错误
		eDBET_KeyDupForInsert		= 1062,	// 关键字重复，插入记录失败
		eDBET_CommitTrans			= 1180, // 提交事务失败
		eDBET_RollbackTrans			= 1181, // 回滚事务失败
		eDBET_Deadloop				= 1205, // 加锁超时
		eDBET_StatementReprepared	= 1615,	// Statement需要重新准备

		eDBET_Unknwon = INVALID_32BIT
	};

	class IDbConnection;
	class IDbFacade
	{
	public:
		virtual IDbConnection*	createConnection() = 0;
		virtual uint32_t		getConnectionCount() const = 0;
		virtual void			release() = 0;

	protected:
		virtual ~IDbFacade() { }
	};

	class IDbStatement;
	class IDbRecordset;
	class IDbConnection
	{
	public:
		virtual bool			connect(const char* szHost, uint16_t nPort, const char* szUser, const char* szPassword, const char* szDbname, const char* szCharacterSet) = 0;
		virtual bool			isConnect() const = 0;
		virtual void			close() = 0;
		virtual void			release() = 0;
		virtual uint32_t		getStatementCount() const = 0;
		virtual uint32_t		getRecordsetCount() const = 0;
		virtual IDbStatement*	createStatement(const char* szSQL) = 0;
		virtual IDbRecordset*	execute(const char* szSql) = 0;
		virtual bool			ping() = 0;
		virtual uint64_t		getAffectedRow() const = 0;
		virtual void*			getMysql() const = 0;
		virtual void			escape(char* szDst, const char* szSrc, size_t nLength) = 0;

	protected:
		virtual ~IDbConnection() { };
	};

	class IDbParamBinder;
	class IDbResultBinder;
	class IDbStatement
	{
	public:
		virtual uint32_t			execute() = 0;
		virtual IDbParamBinder*		getParamBinder() = 0;
		virtual IDbResultBinder*	getResultBinder() = 0;
		virtual uint32_t			getParamCount() const = 0;
		virtual uint32_t			getResultCount() const = 0;
		virtual uint64_t			getRowCount() const = 0;
		virtual int64_t				getInsertID() const = 0;
		virtual bool				fatchNextRow() = 0;
		virtual bool				locate(uint64_t nRow) = 0;
		virtual void				release() = 0;

	protected:
		virtual ~IDbStatement() { }
	};

	class IDbParamBinder
	{
	public:
		virtual void setInt64(uint32_t nIndex, int64_t& nValue) = 0;
		virtual void setUInt64(uint32_t nIndex, uint64_t& nValue) = 0;
		virtual void setInt32(uint32_t nIndex, int32_t& nValue) = 0;
		virtual void setUInt32(uint32_t nIndex, uint32_t& nValue) = 0;
		virtual void setInt16(uint32_t nIndex, int16_t& nValue) = 0;
		virtual void setUInt16(uint32_t nIndex, uint16_t& nValue) = 0;
		virtual void setInt8(uint32_t nIndex, int8_t& nValue) = 0;
		virtual void setUInt8(uint32_t nIndex, uint8_t& nValue) = 0;
		virtual void setFloat(uint32_t nIndex, float& fValue) = 0;
		virtual void setDouble(uint32_t nIndex, double& fValue) = 0;
		virtual void setBlob(uint32_t nIndex, void* pBuf, uint32_t nBufSize) = 0;
		// nBufSize 必须是字符串的长度而不是缓冲的长度
		virtual void setText(uint32_t nIndex, void* pBuf, uint32_t nBufSize) = 0;

		virtual void bind() = 0;

	protected:
		virtual ~IDbParamBinder() { }
	};

	class IDbResultBinder
	{
	public:
		virtual void bindInt64(uint32_t nIndex, int64_t& nValue, bool& bIsNull, bool& bError) = 0;
		virtual void bindUInt64(uint32_t nIndex, uint64_t& nValue, bool& bIsNull, bool& bError) = 0;
		virtual void bindInt32(uint32_t nIndex, int32_t& nValue, bool& bIsNull, bool& bError) = 0;
		virtual void bindUInt32(uint32_t nIndex, uint32_t& nValue, bool& bIsNull, bool& bError) = 0;
		virtual void bindInt16(uint32_t nIndex, int16_t& nValue, bool& bIsNull, bool& bError) = 0;
		virtual void bindUInt16(uint32_t nIndex, uint16_t& nValue, bool& bIsNull, bool& bError) = 0;
		virtual void bindInt8(uint32_t nIndex, int8_t& nValue, bool& bIsNull, bool& bError) = 0;
		virtual void bindUInt8(uint32_t nIndex, uint8_t& nValue, bool& bIsNull, bool& bError) = 0;
		virtual void bindFloat(uint32_t nIndex, float& fValue, bool& bIsNull, bool& bError) = 0;
		virtual void bindDouble(uint32_t nIndex, double& fValue, bool& bIsNull, bool& bError) = 0;
		virtual void bindBlob(uint32_t nIndex, void* pBuf, uint32_t nBufSize, _ulong& nResultSize, bool& bIsNull, bool& bError) = 0;
		virtual void bindText(uint32_t nIndex, void* pBuf, uint32_t nBufSize, _ulong& nResultSize, bool& bIsNull, bool& bError) = 0;

		virtual void bind() = 0;

	protected:
		virtual ~IDbResultBinder() { }
	};

	class IDbRecordset
	{
	public:
		virtual uint64_t	getRowCount() const = 0;
		virtual uint32_t	getColCount() const = 0;
		virtual bool        fatchNextRow() = 0;
		virtual bool        locate(uint64_t nRow) = 0;
		virtual CVariant	getData(uint32_t nCol) const = 0;
		virtual void        release() = 0;

	protected:
		virtual ~IDbRecordset() { }
	};

	__BASE_DATABASE_API__ void			startupDatabase();
	__BASE_DATABASE_API__ void			cleanupDatabase();

	__BASE_DATABASE_API__ IDbFacade*	createDbFacade();
}