#pragma once

#include "google/protobuf/message.h"
#include "libBaseNetwork/network.h"

#include <functional>
#include <string>
#include <map>

#include "message_handler.h"

class CConnectToLogin :
	public base::INetConnecterHandler
{
public:
	CConnectToLogin();
	virtual ~CConnectToLogin();

	virtual void		onSendComplete(uint32_t nSize) { }
	virtual void		onConnectFail() { }

	virtual uint32_t	onRecv(const char* pData, uint32_t nDataSize);

	virtual void		onConnect();

	virtual void		onDisconnect();

	uint64_t			getAccountID() const;

private:
	void				onDispatch(const message_header* pData);

private:
	uint64_t	m_nAccountID;
};