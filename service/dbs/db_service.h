#pragma once
#include "libCoreCommon/service_base.h"
#include "libCoreCommon/normal_protobuf_factory.h"

#include "db_service_message_handler.h"

class CDbService :
	public core::CServiceBase
{
public:
	CDbService();
	virtual ~CDbService();
	
	virtual core::CProtobufFactory*	getServiceProtobufFactory() const;
	
	virtual void					release();

	uint32_t						getDbID() const;

private:
	virtual bool					onInit();
	virtual void					onFrame();
	virtual void					onQuit();

private:
	CDbServiceMessageHandler*		m_pDbServiceMessageHandler;
	core::CNormalProtobufFactory*	m_pNormalProtobufFactory;
	uint32_t						m_nDbID;
};