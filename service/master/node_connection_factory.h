#pragma once

#include "libCoreCommon/base_connection_factory.h"

class CMasterService;
class CNodeConnectionFactory :
	public core::CBaseConnectionFactory
{
public:
	CNodeConnectionFactory(CMasterService* pMasterService);
	virtual ~CNodeConnectionFactory();

	virtual core::CBaseConnection*	createBaseConnection(const std::string& szType);

private:
	CMasterService*	m_pMasterService;
};