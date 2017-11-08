#pragma once
#include "libBaseCommon/ticker.h"

#include "libCoreCommon/service_base.h"
#include "libCoreCommon/normal_protobuf_serializer.h"

#include "online_count_mgr.h"

using namespace std;
using namespace core;
using namespace base;

class CDispatchServiceMessageHandler;
class CDispatchService :
	public CServiceBase
{
public:
	CDispatchService(const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName);
	virtual ~CDispatchService();

	virtual void		release();

	COnlineCountMgr*	getOnlineCountMgr() const;

	uint32_t			getGlobalDbServiceID() const;
	uint32_t			getAccountTblCount() const;

private:
	virtual bool		onInit();
	virtual void		onFrame();
	virtual void		onQuit();

private:
	std::unique_ptr<CDispatchServiceMessageHandler>	m_pDispatchServiceMessageHandler;
	std::unique_ptr<CNormalProtobufSerializer>		m_pNormalProtobufSerializer;
	std::unique_ptr<COnlineCountMgr>				m_pOnlineCountMgr;
	uint32_t										m_nGlobalDbServiceID;
	uint32_t										m_nAccountTblCount;
};