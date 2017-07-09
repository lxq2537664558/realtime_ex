#pragma once

#include "base_connection.h"

namespace core
{
	class CBaseConnectionToMaster
		: public core::CBaseConnection
	{
	public:
		CBaseConnectionToMaster();
		virtual ~CBaseConnectionToMaster();

		virtual bool		init(uint32_t nType, const std::string& szContext);
		virtual void		release();

		virtual void		onConnect();
		virtual void		onDisconnect();
		virtual void		onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize);
	};
}