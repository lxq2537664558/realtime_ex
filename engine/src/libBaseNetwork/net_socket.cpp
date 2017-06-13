#include "stdafx.h"
#include "net_socket.h"
#include "net_event_loop.h"

#ifndef _WIN32
#include <netinet/tcp.h>
#endif

#include "libBaseCommon/logger.h"

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
			PrintInfo("closesocket socket_id %d", this->m_nSocketID);
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
			PrintWarning("out of max socket count %d", this->m_pNetEventLoop->getMaxSocketCount());
			return false;
		}

		this->m_nSocketID = (int32_t)::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (_Invalid_SocketID == this->m_nSocketID)
		{
			PrintWarning("create socket error %d", getLastError());
			return false;
		}

		return true;
	}

	void CNetSocket::close(bool bRelease, bool bCloseSend)
	{
		PrintInfo("CNetSocket::close socketid %d release: %d close_send: %d, wait_close", this->m_nSocketID, bRelease, bCloseSend, this->m_bWaitClose);

		if (this->m_bWaitClose)
			return;

		if (bCloseSend)
			::shutdown(this->m_nSocketID, SD_SEND);

		if (bRelease)
		{
			this->m_pNetEventLoop->addCloseSocket(this);
			this->m_bWaitClose = true;
		}
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
			PrintWarning("set socket nonblock error %d", getLastError());
			return false;
		}
#else
		int32_t nFlag = ::fcntl(this->m_nSocketID, F_GETFL, 0);
		if (-1 == nFlag)
		{
			PrintWarning("set socket nonblock error %d", getLastError());
			return false;
		}

		nFlag = ::fcntl(this->m_nSocketID, F_SETFL, nFlag | O_NONBLOCK);

		if (-1 == nFlag)
		{
			PrintWarning("set socket nonblock error %d", getLastError());
			return false;
		}
#endif
		return true;
	}

	// ��Ҫ���ڼ���SOCKET Ϊ���ü���SOCKET�رպ���TIME_WAIT״̬�󣬿������ϰ���ͬһ���˿ڣ���Ȼ��Ҫ�ȴ�һ��ʱ��
	bool CNetSocket::reuseAddr()
	{
		DebugAstEx(this->m_nSocketID != _Invalid_SocketID, false);

		int32_t nFlag = 1;
		if (0 != ::setsockopt(this->m_nSocketID, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&nFlag), sizeof(nFlag)))
		{
			PrintWarning("set socket to reuse addr mode error %d", getLastError());
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
			PrintWarning("set socket to reuse port mode error %d", getLastError());
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
			PrintWarning("set socket recvbuf error %d", getLastError());
			return false;
		}

		if (this->m_nSendBufferSize != 0 && 0 != ::setsockopt(this->m_nSocketID, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char*>(&this->m_nSendBufferSize), sizeof(this->m_nSendBufferSize)))
		{
			PrintWarning("set socket sendbuf error %d", getLastError());
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
		// ������::htons https://bbs.archlinux.org/viewtopic.php?id=53751
		this->m_sRemoteAddr.nPort = ntohs((reinterpret_cast<sockaddr_in*>(&remoteAddr))->sin_port);
		strncpy(this->m_sRemoteAddr.szHost, inet_ntoa((reinterpret_cast<sockaddr_in*>(&remoteAddr))->sin_addr), _countof(this->m_sRemoteAddr.szHost));
	}

	void CNetSocket::setLocalAddr()
	{
		struct sockaddr localAddr;
		socklen_t nLocalAddrLen = sizeof(localAddr);
		::getsockname(this->m_nSocketID, &localAddr, &nLocalAddrLen);
		// ������::htons https://bbs.archlinux.org/viewtopic.php?id=53751
		this->m_sLocalAddr.nPort = ntohs((reinterpret_cast<sockaddr_in*>(&localAddr))->sin_port);
		strncpy(this->m_sLocalAddr.szHost, inet_ntoa((reinterpret_cast<sockaddr_in*>(&localAddr))->sin_addr), _countof(this->m_sRemoteAddr.szHost));
	}

	void CNetSocket::setSocketID(int32_t nSocketID)
	{
		this->m_nSocketID = nSocketID;
	}

	int32_t CNetSocket::GetSocketID() const
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