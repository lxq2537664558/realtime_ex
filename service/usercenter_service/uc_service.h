#pragma once
#include "libCoreCommon/service_base.h"
#include "libCoreCommon/ticker.h"
#include "libCoreCommon/normal_protobuf_serializer.h"
#include "user_info_mgr.h"

using namespace std;
using namespace core;
using namespace base;

class CUCServiceMessageHandler;
class CPlayerMessageHandler;
class CUCService :
	public CServiceBase
{
public:
	CUCService(const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName);
	virtual ~CUCService();

	CUserInfoMgr*	getUserInfoMgr() const;

	virtual void	release();

private:
	virtual bool	onInit();
	virtual void	onFrame();
	virtual void	onQuit();

	void			onServiceConnect(const std::string& szType, uint32_t nServiceID);
	void			onNotifyActiveCount(uint64_t nContext);

private:
	std::unique_ptr<CUCServiceMessageHandler>	m_pUCServiceMessageHandler;
	std::unique_ptr<CNormalProtobufSerializer>	m_pNormalProtobufSerializer;
	std::unique_ptr<CUserInfoMgr>				m_pUserInfoMgr;

	core::CTicker								m_tickerNotifyActiveCount;
};