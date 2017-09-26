#pragma once

#include "base_connection.h"

namespace core
{
	class CBaseConnectionToMaster
		: public CBaseConnection
	{
	public:
		CBaseConnectionToMaster();
		virtual ~CBaseConnectionToMaster();

		virtual void	release();

		uint32_t		getMasterID() const;

		virtual void	onConnect();
		virtual void	onDisconnect();
		virtual void	onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize);

	private:
		uint32_t	m_nMasterID;
	};
}