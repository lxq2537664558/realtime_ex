#pragma once

#include "libCoreCommon/base_connection.h"

namespace core
{
	class CCoreConnectionOtherService
		: public core::CBaseConnection
	{
	public:
		CCoreConnectionOtherService();
		virtual ~CCoreConnectionOtherService();

		virtual bool		init(uint32_t nType, const std::string& szContext);
		virtual void		release();

		virtual void		onConnect();
		virtual void		onDisconnect();
		virtual void		onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize);

		uint16_t			getServiceID() const;

	private:
		uint16_t	m_nServiceID;
	};
}