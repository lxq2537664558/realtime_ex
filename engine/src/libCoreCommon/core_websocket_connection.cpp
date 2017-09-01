#include "stdafx.h"
#include "core_websocket_connection.h"
#include "message_command.h"
#include "core_app.h"
#include "http_response.h"

#include "libBaseCommon/sha1.h"
#include "libBaseCommon/base64.h"
#include "libBaseCommon/function_util.h"

#include <algorithm>

namespace
{
	const char* const kWebSocketMagic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	const char* const kSecWebSocketVersionHeader = "Sec-WebSocket-Version";
	const char* const kSecWebSocketKeyHeader = "Sec-WebSocket-Key";
	const char* const kSecWebSocketAcceptHeader = "Sec-WebSocket-Accept";
	const char* const kUpgradeHeader = "Upgrade";
	const char* const kConnectionHeader = "Connection";

	const uint32_t kPayloadSizeBasic = 125;
	const uint32_t kPayloadSizeExtended = 0xFFFF;
}

namespace core
{

	CCoreWebsocketConnection::CCoreWebsocketConnection()
		: m_pHttpRequestParser(new CHttpRequestParser())
		, m_eWebSocketState(eWSS_None)
	{
		this->enableHeartbeat(false);
	}

	CCoreWebsocketConnection::~CCoreWebsocketConnection()
	{
		SAFE_DELETE(this->m_pHttpRequestParser);
	}

	uint32_t CCoreWebsocketConnection::onRecv(const char* pData, uint32_t nDataSize)
	{
		if (this->m_eWebSocketState == eWSS_Connectting)
		{
			if (this->getMode() != base::eNCM_Passive)
			{
				this->m_pNetConnecter->shutdown(true, "websocket not support active mode");
				return nDataSize;
			}
			if (!this->m_pHttpRequestParser->parseRequest(pData, nDataSize))
			{
				static char szBuf[] = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n400 Bad Request\n";

				this->m_pNetConnecter->send(szBuf, sizeof(szBuf), false);
				this->m_pNetConnecter->shutdown(false, "bad request");
				return nDataSize;
			}

			uint32_t nRecvSize = this->m_pHttpRequestParser->getParseDataCount();

			if (this->m_pHttpRequestParser->getParseState() == eHPS_Finish)
			{
				if (!this->handshark(this->m_pHttpRequestParser->getRequest()))
				{
					static char szBuf[] = "HTTP/1.1 400 Bad Handshark\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n400 Bad Handshark\n";

					this->m_pNetConnecter->send(szBuf, sizeof(szBuf), false);
					this->m_pNetConnecter->shutdown(false, "bad handshark");
					return nDataSize;
				}

				PrintInfo("websocket Handsharked remote_addr: {} {} local_addr: {} {}", this->getRemoteAddr().szHost, this->getRemoteAddr().nPort, this->getLocalAddr().szHost, this->getLocalAddr().nPort);
				
				this->m_eWebSocketState = eWSS_Connected;
				this->m_nState = eCCS_Connectting;

				SMCT_NOTIFY_SOCKET_CONNECT* pContext = new SMCT_NOTIFY_SOCKET_CONNECT();
				pContext->pCoreConnection = this;

				SMessagePacket sMessagePacket;
				sMessagePacket.nType = eMCT_NOTIFY_SOCKET_CONNECT;
				sMessagePacket.pData = pContext;
				sMessagePacket.nDataSize = sizeof(SMCT_NOTIFY_SOCKET_CONNECT);

				CCoreApp::Inst()->getLogicRunnable()->getMessageQueue()->send(sMessagePacket);

				this->m_pHttpRequestParser->reset();
			}

			return nRecvSize;
		}
		else
		{
			uint32_t nRecvSize = 0;
			std::vector<char>& vecBuf = CCoreApp::Inst()->getWebsocketBuf();
			while (nRecvSize < nDataSize)
			{
				EWebsocketFrameType eType;
				int32_t nRet = this->unserializeFrame(reinterpret_cast<const uint8_t*>(pData + nRecvSize), nDataSize - nRecvSize, eType, vecBuf);
				if (nRet < 0)
				{
					this->m_pNetConnecter->shutdown(true, "frame error");
					return nDataSize;
				}

				nRecvSize += nRet;
				if (eType == eWFT_CLOSE_FRAME)
				{
					// 收到关闭包，回一个关闭包
					this->sendFrame(eWO_OPCODE_CLOSE, nullptr, 0);

					this->m_eWebSocketState = eWSS_Disconnectting;

					return nRecvSize;
				}
				else if (eType == eWFT_PONG_FRAME)
				{
					if (this->m_eWebSocketState != eWSS_Connected)
						this->sendFrame(eWO_OPCODE_PONG, nullptr, 0);

					return nRecvSize;
				}

				if (eType == eWFT_TEXT_FRAME || eType == eWFT_BINARY_FRAME)
				{
					char* pBuf = new char[sizeof(SMCT_RECV_SOCKET_DATA) + vecBuf.size()];
					SMCT_RECV_SOCKET_DATA* pContext = reinterpret_cast<SMCT_RECV_SOCKET_DATA*>(pBuf);
					pContext->nSocketID = this->getID();
					pContext->nMessageType = eMT_CLIENT;
					pContext->nDataSize = (uint16_t)vecBuf.size();
					pContext->pData = pBuf + sizeof(SMCT_RECV_SOCKET_DATA);
					memcpy(pContext->pData, &vecBuf[0], vecBuf.size());

					SMessagePacket sMessagePacket;
					sMessagePacket.nType = eMCT_RECV_SOCKET_DATA;
					sMessagePacket.pData = pContext;
					sMessagePacket.nDataSize = sizeof(SMCT_RECV_SOCKET_DATA);

					CCoreApp::Inst()->getLogicRunnable()->getMessageQueue()->send(sMessagePacket);
				}
			}

			return nRecvSize;
		}
	}

	bool CCoreWebsocketConnection::handshark(const CHttpRequest& sHttpRequest)
	{
		if (sHttpRequest.getMethod() != eHMT_Get)
			return false;

		const char* szSecWebSocketKey = sHttpRequest.getHeader(kSecWebSocketKeyHeader);
		if (szSecWebSocketKey == nullptr)
			return false;
		
		std::string szServerKey = szSecWebSocketKey;
		szServerKey += kWebSocketMagic;

		base::CSHA1 sha1;
		sha1.write(reinterpret_cast<const uint8_t*>(szServerKey.c_str()), (uint32_t)szServerKey.size());
		uint32_t zMessageDigest[5];
		if (!sha1.result(zMessageDigest))
			return false;

		for (uint32_t i = 0; i < _countof(zMessageDigest); i++)
		{
			zMessageDigest[i] = base::function_util::hton32(zMessageDigest[i]);
		}

		char szBuf[256] = { 0 };
		if (base::base64::encode(reinterpret_cast<const char*>(zMessageDigest), 20, szBuf, _countof(szBuf)) < 0)
			return false;

		szServerKey = szBuf;

		CHttpResponse sResponse;
		sResponse.setStatusCode(101);
		sResponse.setStatusMessage("Switching Protocols");

		sResponse.addHeader(kConnectionHeader, "Upgrade");
		sResponse.addHeader(kUpgradeHeader, "WebSocket");
		sResponse.addHeader(kSecWebSocketVersionHeader, "13");
		sResponse.addHeader(kSecWebSocketAcceptHeader, szServerKey);

		std::string szData;
		sResponse.serialize(szData);
		if (this->m_pNetConnecter == nullptr)
			return false;

		this->m_pNetConnecter->send(szData.c_str(), (uint32_t)szData.size(), false);
		
		return true;
	}

	void CCoreWebsocketConnection::onConnect()
	{
		this->m_eWebSocketState = eWSS_Connectting;

		if (this->getMode() != base::eNCM_Initiative)
			return;
	}

	void CCoreWebsocketConnection::send(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		this->sendFrame(eWO_OPCODE_BINARY, pData, nSize);
	}

	void CCoreWebsocketConnection::sendFrame(EWebsocketOpcode eType, const void* pData, uint16_t nSize)
	{
		if (this->m_pNetConnecter == nullptr)
			return;

		if (this->m_eWebSocketState != eWSS_Connected)
			return;

		SWebsocketHeader sWebsocketHeader;
		sWebsocketHeader.bFin = true;
		sWebsocketHeader.bMasked = false;
		sWebsocketHeader.nOpcode = (uint8_t)eType;
		sWebsocketHeader.nPayloadLength = nSize;

		std::vector<char>& vecBuf = CCoreApp::Inst()->getWebsocketBuf();
		vecBuf.resize(UINT16_MAX);
		int32_t nDataSize = this->serializeFrame(sWebsocketHeader, reinterpret_cast<const char*>(pData), vecBuf);
		if (nDataSize <= 0)
		{
			PrintWarning("CWebsocketCoreConnection::sendFrame error");
			this->shutdown(true, "serializeFrame error");
			return;
		}

		this->m_pNetConnecter->send(&vecBuf[0], nDataSize, false);
	}

	int32_t CCoreWebsocketConnection::unserializeFrame(const uint8_t* pData, uint32_t nDataSize, EWebsocketFrameType& eType, std::vector<char>& vecBuf)
	{
		vecBuf.clear();

		if (nDataSize < 3)
		{
			eType = eWFT_INCOMPLETE_FRAME;
			return 0;
		}

		SWebsocketHeader sWebsocketHeader;
		sWebsocketHeader.bFin = (pData[0] >> 7) & 0x01;
		sWebsocketHeader.nOpcode = pData[0] & 0x0F;

		sWebsocketHeader.bMasked = (pData[1] >> 7) & 0x01;

		sWebsocketHeader.nPayloadLength = 0;
		uint32_t nLengthField = pData[1] & (~0x80);
		uint32_t nPos = 2;
		if (nLengthField <= kPayloadSizeBasic)
		{
			sWebsocketHeader.nPayloadLength = nLengthField;
		}
		else if (nLengthField == 126)  //msglen is 16bit!
		{
			sWebsocketHeader.nPayloadLength = (pData[2] << 8) + pData[3];
			nPos += 2;
		}
		else if (nLengthField == 127)  //msglen is 64bit!
		{
			// 太大不支持
			return -1;
		}

		uint32_t nFrameSize = (uint32_t)(sWebsocketHeader.nPayloadLength + nPos + (sWebsocketHeader.bMasked ? _countof(sWebsocketHeader.zMaskingKey) : 0));
		if (nDataSize < nFrameSize)
		{
			eType = eWFT_INCOMPLETE_FRAME;
			return 0;
		}

		if (sWebsocketHeader.nPayloadLength > vecBuf.capacity())
		{
			// 包太大了，这里不支持，直接错误了
			return -1;
		}

		if (sWebsocketHeader.bMasked && sWebsocketHeader.nPayloadLength > 0)
		{
			memcpy(sWebsocketHeader.zMaskingKey, pData + nPos, _countof(sWebsocketHeader.zMaskingKey));
			nPos += _countof(sWebsocketHeader.zMaskingKey);

			vecBuf.resize(sWebsocketHeader.nPayloadLength);
			memcpy(&vecBuf[0], pData + nPos, sWebsocketHeader.nPayloadLength);

			// unmask data:
			for (uint64_t i = 0; i < sWebsocketHeader.nPayloadLength; ++i)
			{
				vecBuf[i] = vecBuf[i] ^ sWebsocketHeader.zMaskingKey[i % 4];
			}
		}

		if (sWebsocketHeader.nOpcode == eWO_OPCODE_CONTINUE)
		{
			if (sWebsocketHeader.bFin)
			{
				eType = eWFT_TEXT_FRAME;
				return nFrameSize;
			}
			else
			{
				eType = eWFT_INCOMPLETE_TEXT_FRAME; // continuation frame ?
				return 0;
			}
		}

		if (sWebsocketHeader.nOpcode == eWO_OPCODE_TEXT)
		{
			if (sWebsocketHeader.bFin)
			{
				eType = eWFT_TEXT_FRAME;
				return nFrameSize;
			}
			else
			{
				eType = eWFT_INCOMPLETE_TEXT_FRAME;
				return 0;
			}
		}

		if (sWebsocketHeader.nOpcode == eWO_OPCODE_BINARY)
		{
			if (sWebsocketHeader.bFin)
			{
				eType = eWFT_BINARY_FRAME;
				return nFrameSize;
			}
			else
			{
				eType = eWFT_INCOMPLETE_BINARY_FRAME;
				return 0;
			}
		}

		if (sWebsocketHeader.nOpcode == eWO_OPCODE_CLOSE) { eType = eWFT_CLOSE_FRAME; return nFrameSize; }
		if (sWebsocketHeader.nOpcode == eWO_OPCODE_PING) { eType = eWFT_PING_FRAME;	return nFrameSize; }
		if (sWebsocketHeader.nOpcode == eWO_OPCODE_PONG) { eType = eWFT_PONG_FRAME;	return nFrameSize; }

		return -1;
	}

	int32_t CCoreWebsocketConnection::serializeFrame(const SWebsocketHeader& sWebsocketHeader, const char* pData, std::vector<char>& vecBuf)
	{
		if (sWebsocketHeader.nPayloadLength + 10 >= vecBuf.capacity())
			return -1;

		vecBuf.resize(sWebsocketHeader.nPayloadLength + 10);

		uint32_t nPos = 0;

		uint8_t nValue = 0;
		if (sWebsocketHeader.bFin)
			nValue |= 0x80;
		// rsv不管
		nValue |= sWebsocketHeader.nOpcode;
		vecBuf[nPos++] = nValue;

		nValue = 0;
		if (sWebsocketHeader.bMasked)
			nValue |= 0x80;

		if (sWebsocketHeader.nPayloadLength <= kPayloadSizeBasic)  // 125
		{
			nValue |= (uint8_t)sWebsocketHeader.nPayloadLength;
			vecBuf[nPos++] = nValue;
		}
		else if (sWebsocketHeader.nPayloadLength <= kPayloadSizeExtended)   // 65535
		{
			nValue |= 126;
			vecBuf[nPos++] = nValue; //16 bit length
			vecBuf[nPos++] = (sWebsocketHeader.nPayloadLength >> 8) & 0xFF; // rightmost first
			vecBuf[nPos++] = sWebsocketHeader.nPayloadLength & 0xFF;
		}
		else  // >2^16-1
		{
			// 包太大，不支持
			return -1;
		}

		if (sWebsocketHeader.bMasked)
		{
			memcpy(&vecBuf[nPos], sWebsocketHeader.zMaskingKey, _countof(sWebsocketHeader.zMaskingKey));
			nPos += _countof(sWebsocketHeader.zMaskingKey);
			memcpy(&vecBuf[nPos], pData, sWebsocketHeader.nPayloadLength);
			for (uint64_t i = 0; i < sWebsocketHeader.nPayloadLength; ++i)
			{
				vecBuf[i] = vecBuf[i] ^ sWebsocketHeader.zMaskingKey[i % 4];
			}
		}
		else
		{
			memcpy(&vecBuf[nPos], pData, sWebsocketHeader.nPayloadLength);
		}

		return (int32_t)(sWebsocketHeader.nPayloadLength + nPos);
	}
}