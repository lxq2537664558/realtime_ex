#pragma once

#include "libBaseNetwork/network.h"
#include "core_common.h"

namespace core
{
	enum EMessageCommandType
	{
		eMCT_QUIT,
		eMCT_FRAME,
		eMCT_REQUEST_SOCKET_LISTEN,
		eMCT_REQUEST_SOCKET_CONNECT,
		eMCT_REQUEST_SOCKET_SHUTDOWN,
		eMCT_NOTIFY_SOCKET_CONNECT,
		eMCT_NOTIFY_SOCKET_CONNECT_ACK,
		eMCT_NOTIFY_SOCKET_DISCONNECT,
		eMCT_NOTIFY_SOCKET_CONNECT_FAIL,
		eMCT_SEND_SOCKET_DATA,
		eMCT_RECV_SOCKET_DATA,
		eMCT_REQUEST,
		eMCT_RESPONSE,
		eMCT_GATE_FORWARD,
		eMCT_BROADCAST_SOCKET_DATA1,
		eMCT_BROADCAST_SOCKET_DATA2,
		eMCT_TICKER,
		eMCT_ENABLE_HEARTBEAT,
		eMCT_SEND_HEARTBEAT,
	};

	struct SMCT_REQUEST_SOCKET_LISTEN
	{
		std::string			szHost;
		uint16_t			nPort;
		std::string			szContext;
		uint32_t			nType;
		uint32_t			nSendBufferSize;
		uint32_t			nRecvBufferSize;
		MessageParser		messageParser;
	};

	struct SMCT_REQUEST_SOCKET_CONNECT
	{
		std::string			szHost;
		uint16_t			nPort;
		std::string			szContext;
		uint32_t			nType;
		uint32_t			nSendBufferSize;
		uint32_t			nRecvBufferSize;
		MessageParser		messageParser;
	};

	struct SMCT_REQUEST_SOCKET_SHUTDOWN
	{
		uint64_t	nSocketID;
		uint32_t	nType;
		std::string	szMsg;
	};

	struct SMCT_NOTIFY_SOCKET_CONNECT
	{
		std::string	szContext;
		uint32_t	nType;
		SNetAddr	sLocalAddr;
		SNetAddr	sRemoteAddr;
		uint64_t	nSocketID;
	};

	struct SMCT_NOTIFY_SOCKET_CONNECT_ACK
	{
		uint64_t nSocketID;
	};

	struct SMCT_NOTIFY_SOCKET_DISCONNECT
	{
		uint64_t nSocketID;
	};

	struct SMCT_NOTIFY_SOCKET_CONNECT_FAIL
	{
		std::string	szContext;
	};

	struct SMCT_RECV_SOCKET_DATA
	{
		uint64_t	nSocketID;
		uint64_t	nSessionID;
		uint64_t	nData;
		uint64_t	nToID;
		uint32_t	nToServiceID;
		uint8_t		nMessageType;
		uint16_t	nDataSize;
		void*		pData;
	};

	struct SMCT_REQUEST
	{
		uint64_t	nSessionID;
		uint64_t	nFromActorID;
		uint32_t	nFromServiceID;
		uint64_t	nToActorID;
		uint32_t	nToServiceID;
		google::protobuf::Message*
					pMessage;
	};

	struct SMCT_RESPONSE
	{
		uint64_t	nSessionID;
		uint64_t	nToActorID;
		uint32_t	nToServiceID;
		uint8_t		nResult;
		google::protobuf::Message*
					pMessage;
	};

	struct SMCT_GATE_FORWARD
	{
		uint64_t	nSessionID;
		uint32_t	nFromServiceID;
		uint64_t	nToActorID;
		uint32_t	nToServiceID;
		google::protobuf::Message*
					pMessage;
	};

	struct SMCT_SEND_SOCKET_DATA
	{
		uint64_t	nSocketID;
		uint8_t		nMessageType;
	};

	struct SMCT_BROADCAST_SOCKET_DATA1
	{
		uint8_t		nMessageType;
		uint16_t	nSocketIDCount;
	};

	struct SMCT_BROADCAST_SOCKET_DATA2
	{
		uint32_t	nType;
		uint8_t		nMessageType;
		uint16_t	nExcludeIDCount;
	};

	struct SMCT_ENABLE_HEARTBEAT
	{
		uint64_t	nSocketID;
		uint8_t		nEnable;
	};

	struct SMCT_SEND_HEARTBEAT
	{
		uint64_t	nSocketID;
	};
}