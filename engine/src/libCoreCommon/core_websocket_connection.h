#pragma once

#include "core_connection.h"
#include "http_request_parser.h"

#include <vector>

namespace core
{
	enum EWebsocketFrameType
	{
		eWFT_INCOMPLETE_FRAME = 0xFE00,

		eWFT_OPENING_FRAME = 0x3300,
		eWFT_CLOSING_FRAME = 0x3400,

		eWFT_INCOMPLETE_TEXT_FRAME = 0x01,
		eWFT_INCOMPLETE_BINARY_FRAME = 0x02,

		eWFT_TEXT_FRAME = 0x81,       // 128 + 1 == eWFT_FRAGMENT_FIN | eWFT_OPCODE_TEXT
		eWFT_BINARY_FRAME = 0x82,       // 128 + 2
		eWFT_RSV3_FRAME = 0x83,       // 128 + 3
		eWFT_RSV4_FRAME = 0x84,
		eWFT_RSV5_FRAME = 0x85,
		eWFT_RSV6_FRAME = 0x86,
		eWFT_RSV7_FRAME = 0x87,
		eWFT_CLOSE_FRAME = 0x88,
		eWFT_PING_FRAME = 0x89,
		eWFT_PONG_FRAME = 0x8A,
	};

	enum EWebsocketOpcode
	{
		eWO_OPCODE_CONTINUE = 0x0,
		eWO_OPCODE_TEXT = 0x1,
		eWO_OPCODE_BINARY = 0x2,
		eWO_OPCODE_CLOSE = 0x8,
		eWO_OPCODE_PING = 0x9,
		eWO_OPCODE_PONG = 0xa
	};

	/*
	0                   1                   2                   3
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-------+-+-------------+-------------------------------+
	|F|R|R|R| opcode|M| Payload len |    Extended payload length    |
	|I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
	|N|V|V|V|       |S|             |   (if payload len==126/127)   |
	| |1|2|3|       |K|             |                               |
	+-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
	|     Extended payload length continued, if payload len == 127  |
	+ - - - - - - - - - - - - - - - +-------------------------------+
	|                               |Masking-key, if MASK set to 1  |
	+-------------------------------+-------------------------------+
	| Masking-key (continued)       |          Payload Data         |
	+-------------------------------- - - - - - - - - - - - - - - - +
	:                     Payload Data continued ...                :
	+ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
	|                     Payload Data continued ...                |
	+---------------------------------------------------------------+

	opcode:
	*  %x0 denotes a continuation frame
	*  %x1 denotes a text frame
	*  %x2 denotes a binary frame
	*  %x3-7 are reserved for further non-control frames
	*  %x8 denotes a connection close
	*  %x9 denotes a ping
	*  %xA denotes a pong
	*  %xB-F are reserved for further control frames

	Payload length:  7 bits, 7+16 bits, or 7+64 bits

	Masking-key:  0 or 4 bytes
	*/

	struct SWebsocketHeader
	{
		bool		bFin;
		uint8_t		nOpcode;
		bool		bMasked;
		uint64_t	nPayloadLength;
		uint8_t		zMaskingKey[4];
	};

	class CCoreWebsocketConnection :
		public CCoreConnection
	{
	public:
		CCoreWebsocketConnection();
		virtual ~CCoreWebsocketConnection();

		virtual uint32_t	onRecv(const char* pData, uint32_t nDataSize);
		virtual void		onConnect();
		virtual void		send(uint8_t nMessageType, const void* pData, uint16_t nSize);

	private:
		bool				handshark(const CHttpRequest& sHttpRequest);
		int32_t				decodeFrame(const uint8_t* pData, uint32_t nDataSize, EWebsocketFrameType& eType, std::vector<char>& vecBuf);
		int32_t				encodeFrame(const SWebsocketHeader& sWebsocketHeader, const char* pData, uint32_t nDataSize, std::vector<char>& vecBuf);

	private:
		CHttpRequestParser*	m_pHttpRequestParser;
		bool				m_bHandsharked;
	};
}