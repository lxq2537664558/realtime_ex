#pragma once

#include "libCoreCommon/base_connection.h"

namespace core
{
	class CCoreConnectionToMaster
		: public core::CBaseConnection
	{
	public:
		CCoreConnectionToMaster();
		virtual ~CCoreConnectionToMaster();

		virtual bool		init(uint32_t nType, const std::string& szContext);
		virtual void		release();

		virtual void		onConnect();
		virtual void		onDisconnect();
		virtual void		onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize);
	};
}