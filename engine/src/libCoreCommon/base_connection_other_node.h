#pragma once

#include "base_connection.h"

namespace core
{
	class CBaseConnectionOtherNode
		: public CBaseConnection
	{
	public:
		CBaseConnectionOtherNode();
		virtual ~CBaseConnectionOtherNode();

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