#include "libBaseDatabase/database.h"
#include "libBaseCommon/base_function.h"
#include "libBaseCommon/thread_base.h"
#include "libBaseCommon/base_time.h"
#include "libBaseCommon/logger.h"

#include <iostream>
#include <string>
#include "../../3rd/mysql/include/mysql/mysql.h"
using namespace std;

int main()
{
	base::initLog(true);
	base::startupDatabase();
	base::IDbConnection* pDbConnection = base::createDbFacade()->createConnection();
	pDbConnection->connect( "10.0.18.160", 3306, "root", "123456", "test", "gbk" );
	char szSQL[256] = { 0 };
	char szName[256] = { 0xd5, '\'', ' ', 'o', 'r', ' ', '1', '=', '1', ';', '-', '-', ' ' };
	char szEscapeName[256] = { 0 };
	mysql_real_escape_string((MYSQL*)pDbConnection->getMysql(), szEscapeName, szName, strlen(szName));
	base::crt::snprintf(szSQL, _countof(szSQL), "select id from tbl_player where name = '%s';", szEscapeName);
	base::IDbRecordset* pDbRecordset = pDbConnection->execute(szSQL);
	for (uint64_t i = 0; i < pDbRecordset->getRowCount(); ++i)
	{
		pDbRecordset->fatchNextRow();
		uint32_t nID = pDbRecordset->getData(0);
		cout << "ID: " << nID << endl;
	}
	base::sleep(~0);

	return 0;
}