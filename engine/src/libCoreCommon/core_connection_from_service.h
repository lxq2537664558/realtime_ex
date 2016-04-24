#pragma once

#include "libCoreCommon/base_connection.h"

namespace core
{
	class CCoreConnectionFromService
		: public core::CBaseConnection
	{
		DECLARE_OBJECT(CCoreConnectionFromService)

	public:
		CCoreConnectionFromService();
		virtual ~CCoreConnectionFromService();

		virtual void		onConnect(const std::string& szContext);
		virtual void		onDisconnect();
		virtual void		onDispatch(uint32_t nMessageType, const void* pData, uint16_t nSize);

		const std::string&	getServiceName() const;

	private:
		std::string	m_szServiceName;
	};
}