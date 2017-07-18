#include "libCoreCommon/base_app.h"

using namespace std;
using namespace core;
using namespace base;

int32_t main(int32_t argc, char* argv[])
{
	CBaseApp::Inst()->run(argc, argv, "test_service_config1.xml");
	return 0;
}