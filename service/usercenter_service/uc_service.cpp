#include "uc_service.h"
#include "uc_service_message_handler.h"

#include "libCoreCommon/service_invoker.h"

#include "tinyxml2/tinyxml2.h"

#include "server_proto_src/u2d_active_count_notify.pb.h"

CUCService::CUCService(const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName)
	: CServiceBase(sServiceBaseInfo, szConfigFileName)
{

}

CUCService::~CUCService()
{
}

CUserInfoMgr* CUCService::getUserInfoMgr() const
{
	return this->m_pUserInfoMgr.get();
}

void CUCService::release()
{
	delete this;

	google::protobuf::ShutdownProtobufLibrary();
}

bool CUCService::onInit()
{
	this->m_pNormalProtobufSerializer = std::make_unique<CNormalProtobufSerializer>();
	
	this->addServiceMessageSerializer(this->m_pNormalProtobufSerializer.get());

	this->setServiceMessageSerializer("", eMST_Protobuf);
	
	this->m_pUCServiceMessageHandler = std::make_unique<CUCServiceMessageHandler>(this);
	this->m_pUserInfoMgr = std::make_unique<CUserInfoMgr>(this);

	this->setServiceConnectCallback(std::bind(&CUCService::onServiceConnect, this, std::placeholders::_1, std::placeholders::_2));

	this->m_tickerNotifyActiveCount.setCallback(std::bind(&CUCService::onNotifyActiveCount, this, std::placeholders::_1));
	this->registerTicker(&this->m_tickerNotifyActiveCount, 5000, 5000, 0);

	PrintInfo("CUCService::onInit");

	return true;
}

void CUCService::onFrame()
{
}

void CUCService::onQuit()
{
	PrintInfo("CUCService::onQuit");
	this->doQuit();
}

void CUCService::onServiceConnect(const std::string& szType, uint32_t nServiceID)
{
	if (szType == "dispatch")
	{
		u2d_active_count_notify msg;
		msg.set_count(this->m_pUserInfoMgr->getUserCount());
		this->getServiceInvoker()->send(nServiceID, &msg);
	}
}

void CUCService::onNotifyActiveCount(uint64_t nContext)
{
	u2d_active_count_notify msg;
	msg.set_count(this->m_pUserInfoMgr->getUserCount());
	this->getServiceInvoker()->broadcast("dispatch", &msg);
}

extern "C" 
#ifdef _WIN32
__declspec(dllexport)
#endif
CServiceBase* createServiceBase(const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName)
{
	return new CUCService(sServiceBaseInfo, szConfigFileName);
}