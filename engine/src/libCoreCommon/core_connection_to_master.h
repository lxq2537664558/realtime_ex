#pragma once

#include "libCoreCommon/base_connection.h"

namespace core
{
	class CCoreConnectionToMaster
		: public core::CBaseConnection
	{
		DECLARE_OBJECT(CCoreConnectionToMaster)

	public:
		CCoreConnectionToMaster();
		virtual ~CCoreConnectionToMaster();

		virtual void	onConnect(const std::string& szContext);
		virtual void	onDisconnect();
		virtual void	onDispatch(uint32_t nMsgType, const void* pData, uint16_t nSize);
	};
}