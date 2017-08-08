#pragma once

#include "google/protobuf/message.h"
#include "libBaseNetwork/network.h"

#include <functional>
#include <string>
#include <map>

#include "message_handler.h"

class CConnectToGate :
	public base::INetConnecterHandler
{
public:
	CConnectToGate(const std::string& szKey);
	virtual ~CConnectToGate();

	virtual void		onSendComplete(uint32_t nSize) { }
	virtual void		onConnectFail() { }

	virtual uint32_t	onRecv(const char* pData, uint32_t nDataSize);

	virtual void		onConnect();

	virtual void		onDisconnect();

	uint64_t			getPlayerID() const;

private:
	void				onDispatch(const message_header* pData);

private:
	uint64_t	m_nPlayerID;
	std::string	m_szKey;
};