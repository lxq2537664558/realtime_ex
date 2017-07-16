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

		virtual bool		init(uint32_t nType, const std::string& szContext);
		virtual void		release();

		uint16_t			getMasterID() const;

		virtual void		onConnect();
		virtual void		onDisconnect();
		virtual void		onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize, const void* pContext);

	private:
		uint16_t	m_nMasterID;
	};
}