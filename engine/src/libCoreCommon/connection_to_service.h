#pragma once

#include "libCoreCommon/base_connection.h"

namespace core
{
	class CConnectionToService
		: public core::CBaseConnection
	{
		DECLARE_OBJECT(CConnectionToService)

	public:
		CConnectionToService();
		virtual ~CConnectionToService();

		virtual void		onConnect(const std::string& szContext);
		virtual void		onDisconnect();
		virtual void		onDispatch(uint16_t nMessageType, const void* pData, uint16_t nSize);

		const std::string&	getServiceName() const;

	private:
		std::string	m_szServiceName;
	};
}