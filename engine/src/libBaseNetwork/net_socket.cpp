#include "stdafx.h"
#include "net_socket.h"
#include "net_event_loop.h"

#ifndef _WIN32
#include <netinet/tcp.h>
#endif

#include "libBaseCommon/logger.h"
#include "libBaseCommon/debug_helper.h"

namespace base
{
	CNetSocket::CNetSocket()
		: m_pNetEventLoop(nullptr)
		, m_nSocketID(_Invalid_SocketID)
		, m_nSocketIndex(_Invalid_SocketIndex)
		, m_nSendBufferSize(0)
		, m_nRecvBufferSize(0)
		, m_bWaitClose(false)
	{
	}

	CNetSocket::~CNetSocket()
	{
	}

	bool CNetSocket::init(uint32_t nSendBufferSize, uint32_t nRecvBufferSize, CNetEventLoop* pNetEventLoop)
	{
		DebugAstEx(pNetEventLoop != nullptr, false);

		this->m_nSendBufferSize = nSendBufferSize;
		this->m_nRecvBufferSize = nRecvBufferSize;
		this->m_pNetEventLoop = pNetEventLoop;

		return true;
	}

	void CNetSocket::release()
	{
		if (this->m_nSocketIndex != _Invalid_SocketIndex)
			this->m_pNetEventLoop->delSocket(this);

		if (this->m_nSocketID != _Invalid_SocketID)
		{
			PrintInfo("closesocket socket_id: {}", this->m_nSocketID);
			::closesocket(this->m_nSocketID);
			this->m_nSocketID = _Invalid_SocketID;
		}
	}

	void CNetSocket::setSocketIndex(int32_t nIndex)
	{
		this->m_nSocketIndex = nIndex;
	}

	int32_t CNetSocket::getSocketIndex() const
	{
		return this->m_nSocketIndex;
	}

	bool CNetSocket::open()
	{
		if (this->m_pNetEventLoop->getSocketCount() >= this->m_pNetEventLoop->getMaxSocketCount())
		{
			PrintWarning("out of max socket count {}", this->m_pNetEventLoop->getMaxSocketCount());
			return false;
		}

		this->m_nSocketID = (int32_t)::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (_Invalid_SocketID == this->m_nSocketID)
		{
			PrintWarning("create socket error {}", getLastError());
			return false;
		}

		return true;
	}

	void CNetSocket::close()
	{
		PrintInfo("CNetSocket::close socket_id: {} wait_close: {}", this->m_nSocketID, this->m_bWaitClose);

		if (this->m_bWaitClose)
			return;

		::shutdown(this->m_nSocketID, SD_SEND);
		this->m_pNetEventLoop->addCloseSocket(this);
		this->m_bWaitClose = true;
	}

	bool CNetSocket::nonblock()
	{
		DebugAstEx(this->m_nSocketID != _Invalid_SocketID, false);

#ifdef _WIN32
		int32_t nRet = 0;
		u_long nFlag = 1;
		nRet = ::ioctlsocket(this->m_nSocketID, FIONBIO, &nFlag);
		if (0 != nRet)
		{
			PrintWarning("set socket nonblock error {}", getLastError());
			return false;
		}
#else
		int32_t nFlag = ::fcntl(this->m_nSocketID, F_GETFL, 0);
		if (-1 == nFlag)
		{
			PrintWarning("set socket nonblock error {}", getLastError());
			return false;
		}

		nFlag = ::fcntl(this->m_nSocketID, F_SETFL, nFlag | O_NONBLOCK);

		if (-1 == nFlag)
		{
			PrintWarning("set socket nonblock error {}", getLastError());
			return false;
		}
#endif
		return true;
	}

	// 主要用于监听SOCKET 为了让监听SOCKET关闭后处于TIME_WAIT状态后，可以马上绑定在同一个端口，不然需要等待一段时间
	bool CNetSocket::reuseAddr()
	{
		DebugAstEx(this->m_nSocketID != _Invalid_SocketID, false);

		int32_t nFlag = 1;
		if (0 != ::setsockopt(this->m_nSocketID, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&nFlag), sizeof(nFlag)))
		{
			PrintWarning("set socket to reuse addr mode error {}", getLastError());
			return false;
		}

		return true;
	}

	bool CNetSocket::reusePort()
	{
#ifdef SO_REUSEPORT
		int32_t nFlag = 1;
		if (0 != ::setsockopt(this->m_nSocketID, SOL_SOCKET, SO_REUSEPORT, reinterpret_cast<char*>(&nFlag), sizeof(nFlag)))
		{
			PrintWarning("set socket to reuse port mode error {}", getLastError());
			return false;
		}
#endif

		return true;
	}

	bool CNetSocket::setBufferSize()
	{
		DebugAstEx(this->m_nSocketID != _Invalid_SocketID, false);

		if (this->m_nRecvBufferSize != 0 && 0 != ::setsockopt(this->m_nSocketID, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char*>(&this->m_nRecvBufferSize), sizeof(this->m_nRecvBufferSize)))
		{
			PrintWarning("set socket recvbuf error {}", getLastError());
			return false;
		}

		if (this->m_nSendBufferSize != 0 && 0 != ::setsockopt(this->m_nSocketID, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char*>(&this->m_nSendBufferSize), sizeof(this->m_nSendBufferSize)))
		{
			PrintWarning("set socket sendbuf error {}", getLastError());
			return false;
		}

		return true;
	}

	bool CNetSocket::setNoDelay(bool bEnable)
	{
		int32_t nFlag = bEnable ? 1 : 0;
		return ::setsockopt(this->m_nSocketID, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&nFlag), sizeof(nFlag)) == 0;
	}

	void CNetSocket::setRemoteAddr()
	{
		struct sockaddr remoteAddr;
		socklen_t nPeerAddrLen = sizeof(remoteAddr);
		::getpeername(this->m_nSocketID, &remoteAddr, &nPeerAddrLen);
		this->m_sRemoteAddr.nPort = base::function_util:: ntoh16((reinterpret_cast<sockaddr_in*>(&remoteAddr))->sin_port);
		base::function_util::strcpy(this->m_sRemoteAddr.szHost, _countof(this->m_sRemoteAddr.szHost), inet_ntoa((reinterpret_cast<sockaddr_in*>(&remoteAddr))->sin_addr));
	}

	void CNetSocket::setLocalAddr()
	{
		struct sockaddr localAddr;
		socklen_t nLocalAddrLen = sizeof(localAddr);
		::getsockname(this->m_nSocketID, &localAddr, &nLocalAddrLen);
		this->m_sLocalAddr.nPort = base::function_util::ntoh16((reinterpret_cast<sockaddr_in*>(&localAddr))->sin_port);
		base::function_util::strcpy(this->m_sLocalAddr.szHost, _countof(this->m_sRemoteAddr.szHost), inet_ntoa((reinterpret_cast<sockaddr_in*>(&localAddr))->sin_addr));
	}

	void CNetSocket::setSocketID(int32_t nSocketID)
	{
		this->m_nSocketID = nSocketID;
	}

	int32_t CNetSocket::getSocketID() const
	{
		return this->m_nSocketID;
	}

	uint32_t CNetSocket::getSendBufferSize() const
	{
		return this->m_nSendBufferSize;
	}

	uint32_t CNetSocket::getRecvBufferSize() const
	{
		return this->m_nRecvBufferSize;
	}

}