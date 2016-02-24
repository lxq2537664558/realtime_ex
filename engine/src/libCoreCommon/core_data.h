#pragma once

#include "libBaseNetwork/network.h"

namespace core
{
	enum EMessageCommandType
	{
		eMCT_EXIT,
		eMCT_REQUEST_SOCKET_LISTEN,
		eMCT_REQUEST_SOCKET_CONNECT,
		eMCT_REQUEST_SOCKET_SHUTDOWN,
		eMCT_NOTIFY_SOCKET_CONNECT,
		eMCT_NOTIFY_SOCKET_DISCONNECT,
		eMCT_NOTIFY_CONNECTION_DESTROY,
		eMCT_NOTIFY_SOCKET_CONNECT_REFUSE,
		eMCT_SEND_SOCKET_DATA,
		eMCT_BROADCAST_SOCKET_DATA1,
		eMCT_BROADCAST_SOCKET_DATA2,
		eMCT_RECV_SOCKET_DATA1,			// 这里有两个recv command是因为，有可能逻辑层还没有设置m_pBaseConnection后续的消息就来了
		eMCT_RECV_SOCKET_DATA2,
		eMCT_TIMER,
	};

	struct SMCT_REQUEST_SOCKET_LISTEN
	{
		std::string			szHost;
		uint16_t			nPort;
		std::string			szContext;
		uint32_t			nClassID;
		uint32_t			nSendBufferSize;
		uint32_t			nRecvBufferSize;
		funRawDataParser	pfRawDataParser;
	};

	struct SMCT_REQUEST_SOCKET_CONNECT
	{
		std::string			szHost;
		uint16_t			nPort;
		std::string			szContext;
		uint32_t			nClassID;
		uint32_t			nSendBufferSize;
		uint32_t			nRecvBufferSize;
		funRawDataParser	pfRawDataParser;
	};

	struct SMCT_REQUEST_SOCKET_SHUTDOWN
	{
		int32_t		nSocketID;
		bool		bForce;
		std::string	szMsg;
	};

	struct SMCT_NOTIFY_SOCKET_CONNECT
	{
		std::string	szContext;
		uint32_t	nClassID;
		SNetAddr	sLocalAddr;
		SNetAddr	sRemoteAddr;
		int32_t		nSocketID;
	};

	class CBaseConnection;
	struct SMCT_NOTIFY_SOCKET_DISCONNECT
	{
		CBaseConnection* pBaseConnection;
	};

	struct SMCT_NOTIFY_CONNECTION_DESTROY
	{
		int32_t	nSocketID;
	};

	struct SMCT_NOTIFY_SOCKET_CONNECT_REFUSE
	{
		std::string			szContext;
	};

	struct SMCT_RECV_SOCKET_DATA1
	{
		int32_t		nSocketID;
		uint16_t	nMessageType;
	};

	struct SMCT_RECV_SOCKET_DATA2
	{
		CBaseConnection*	pBaseConnection;
		uint16_t			nMessageType;
	};

	struct SMCT_SEND_SOCKET_DATA
	{
		int32_t		nSocketID;
		uint16_t	nMessageType;
	};

	struct SMCT_BROADCAST_SOCKET_DATA1
	{
		uint16_t	nMessageType;
		uint32_t	nCount;
	};

	struct SMCT_BROADCAST_SOCKET_DATA2
	{
		uint32_t	nClassID;
		uint16_t	nMessageType;
	};
}