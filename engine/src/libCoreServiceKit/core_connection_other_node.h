#pragma once

#include "libCoreCommon/base_connection.h"

namespace core
{
	class CCoreConnectionOtherNode
		: public core::CBaseConnection
	{
	public:
		CCoreConnectionOtherNode();
		virtual ~CCoreConnectionOtherNode();

		virtual bool		init(uint32_t nType, const std::string& szContext);
		virtual void		release();

		virtual void		onConnect();
		virtual void		onDisconnect();
		virtual void		onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize);

		uint16_t			getNodeID() const;

	private:
		uint16_t	m_nNodeID;
	};
}