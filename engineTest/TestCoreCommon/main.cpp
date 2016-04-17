#include "libBaseCommon/base_time.h"
#include "libCoreCommon/core_app.h"
#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/base_connection.h"
#include "libCoreCommon/core_common.h"
#include "libCoreCommon/ticker.h"
#include "libCoreCommon/message_dispatch.h"

#include <map>

Msg_Begin(STestMsg, 2)
	uint32_t	nIndex;
	char szBuf[1024];
Msg_End

class CTestApp;
CTestApp* g_pApp;

class CTestConnection :
	public core::CBaseConnection
{
	DECLARE_OBJECT(CTestConnection)

public:
	virtual void	onConnect(const std::string& szContext);
	virtual void	onDisconnect();
	virtual void	onDispatch(const void* pData, uint16_t nSize);
};

class CTestApp :
	public core::CCoreApp
{
public:
	virtual bool onInit()
	{
		if (!core::CCoreApp::onInit())
			return false;

		CTestConnection::registClassInfo();

		this->getBaseConnectionMgr()->listen("0.0.0.0", 8888, "", _GET_CLASS_NAME(CTestConnection), 100 * 1024, 100 * 1024, nullptr);

		return true;
	}

	virtual void onQuit()
	{
		this->doQuit();
	}
};

DEFINE_OBJECT(CTestConnection, 1000)

void CTestConnection::onDispatch(const void* pData, uint16_t nSize)
{
	
}

void CTestConnection::onConnect(const std::string& szContext)
{
	PrintDebug("CTestConnection::onConnect");
}

void CTestConnection::onDisconnect()
{
	PrintDebug("CTestConnection::onDisconnect");
}

#include <functional>

int32_t main(int32_t argc, char* argv[])
{
	g_pApp = new CTestApp();
	g_pApp->run(argc, argv, "gate_config.xml");

	return 0;
}