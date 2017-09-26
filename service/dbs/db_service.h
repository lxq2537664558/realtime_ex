#pragma once
#include "libCoreCommon/service_base.h"
#include "libCoreCommon/normal_protobuf_serializer.h"

#include "db_service_message_handler.h"

class CDbService :
	public core::CServiceBase
{
public:
	CDbService(const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName);
	virtual ~CDbService();
	
	virtual void	release();

	uint32_t		getDbID() const;

private:
	virtual bool	onInit();
	virtual void	onFrame();
	virtual void	onQuit();

private:
	std::unique_ptr<CDbServiceMessageHandler>			m_pDbServiceMessageHandler;
	std::unique_ptr<core::CNormalProtobufSerializer>	m_pNormalProtobufSerializer;
	uint32_t											m_nDbID;
};