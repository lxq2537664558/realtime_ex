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
	pDbConnection->connect( "192.243.45.183", 3306, "CM_yf3", "8GRbg5eo@4QWae", "CM_EN2", "utf8" );
	base::IDbRecordset* pDbRecordset = pDbConnection->execute( "select name, player_id from player_base where player_id in(293091504, 298609469, 297373912, 273202212, 270517648, 301703830)" );
	for (uint32_t i = 0; i < pDbRecordset->getRowCount(); ++i)
	{
		pDbRecordset->fatchNextRow();
		std::string szName = pDbRecordset->getData(0);
		uint64_t nPlayerID = pDbRecordset->getData(1);
		PrintInfo("name %s", szName.c_str());
	}
	base::sleep(~0);

	return 0;
}