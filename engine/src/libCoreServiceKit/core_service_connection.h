#pragma once

#include "libCoreCommon/base_connection.h"

namespace core
{
	class CCoreServiceConnection
		: public core::CBaseConnection
	{
	public:
		CCoreServiceConnection();
		virtual ~CCoreServiceConnection();

		virtual bool		init(const std::string& szContext);
		virtual uint32_t	getType() const;
		virtual void		release();

		virtual void		onConnect();
		virtual void		onDisconnect();
		virtual bool		onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize);

		const std::string&	getServiceName() const;

	private:
		std::string	m_szServiceName;
	};
}