#include "libBaseDatabase/database.h"
#include "libBaseCommon/base_function.h"
#include "libBaseCommon/thread_base.h"
#include "libBaseCommon/base_time.h"
#include "libBaseCommon/logger.h"

#include <iostream>
#include <string>
using namespace std;

int main()
{
	base::initLog(true);
	base::startupDatabase();
	base::IDbConnection* pDbConnection = base::createDbFacade()->createConnection();
	pDbConnection->connect( "127.0.0.1", 3306, "root", "123456", "game_db", "utf8" );
	char szBuf[4096] = { 0 };
	base::crt::snprintf( szBuf, _countof(szBuf), "drop table if exists tbl_user;" );
	pDbConnection->execute( szBuf );
	base::crt::snprintf( szBuf, _countof(szBuf), 
		"create table `tbl_user` ("
		"`id` int(10) unsigned NOT NULL,"
		"`name` varchar(255) NOT NULL,"
		"PRIMARY KEY (`id`)"
		") ENGINE=InnoDB DEFAULT CHARSET=utf8;" );
	pDbConnection->execute( szBuf );

	int64_t nBeginTime = base::getGmtTime();
	string szSQL;
	for( uint32_t i = 0; i < 1000; ++i )
	{
		char szName[256] = { "abc" };
		base::crt::snprintf( szBuf, _countof(szBuf), "insert into `tbl_user` values (%d, '%s');", i, szName );
		szSQL = szBuf;
		pDbConnection->execute( szSQL.c_str() );
	}
	int64_t nEndTime = base::getGmtTime();
	
	cout << "InnoDB insert 100000 rows cost " << nEndTime-nBeginTime << endl;


	base::IDbRecordset* pDbRecordset = pDbConnection->execute( "select name from tbl_user" );
// 	base::CVariant var( "aaaa" );
// 	base::CVariant aaa;
// 	aaa = std::move( var );
// 	const char* szName = std::move( base::CVariant( "aaaa" ) );
	base::sleep( ~0 );

	return 0;
}