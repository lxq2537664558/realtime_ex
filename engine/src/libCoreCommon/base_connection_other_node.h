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

		virtual void	release();

		virtual void	onConnect();
		virtual void	onDisconnect();
		virtual void	onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize);

		uint32_t		getNodeID() const;
		void			setNodeID(uint32_t nNodeID);

		void			responseHandshake();

	private:
		uint32_t	m_nNodeID;
	};
}