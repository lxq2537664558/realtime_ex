#pragma once

#include "libCoreCommon/base_connection.h"

namespace core
{
	class CCoreConnectionFromOtherNode
		: public core::CBaseConnection
	{
	public:
		CCoreConnectionFromOtherNode();
		virtual ~CCoreConnectionFromOtherNode();

		virtual bool		init(const std::string& szContext);
		virtual uint32_t	getType() const;
		virtual void		release();

		virtual void		onConnect();
		virtual void		onDisconnect();
		virtual void		onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize);

		uint16_t			getNodeID() const;

	private:
		uint16_t	m_nNodeID;
	};
}