#pragma once

#include <assert.h>
#include <stdint.h>

#ifdef _WIN32
#undef FD_SETSIZE
#define FD_SETSIZE 1024
#include <winsock2.h> // ���ͷ�ļ��������winsock.hǰ�棬winsock.h�м��������ͷ�ļ�������
#include <windows.h>
#include <ws2tcpip.h>
#include <errno.h>
#else
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#endif

namespace base
{

#ifdef _WIN32

#	define MSG_NOSIGNAL 0
#	define _Invalid_SocketID (int32_t)INVALID_SOCKET


#	define NW_EWOULDBLOCK		WSAEWOULDBLOCK
#	define NW_ECONNABORTED		WSAECONNABORTED
#	define NW_EPROTO			EPROTO
#	define NW_EINPROGRESS		WSAEINPROGRESS
#	define NW_EAGAIN			EAGAIN
#	define NW_EINTR				EINTR
#	define NW_EALREADY			WSAEALREADY
#	define NW_ENOTSOCK			WSAENOTSOCK
#	define NW_EDESTADDRREQ		WSAEDESTADDRREQ
#	define NW_EMSGSIZE			WSAEMSGSIZE
#	define NW_EPROTOTYPE		WSAEPROTOTYPE
#	define NW_ENOPROTOOPT		WSAENOPROTOOPT
#	define NW_EPROTONOSUPPO		WSAEPROTONOSUPPORT
#	define NW_ESOCKTNOSUPPO		WSAESOCKTNOSUPPORT
#	define NW_EOPNOTSUPP		WSAEOPNOTSUPP
#	define NW_EPFNOSUPPORT		WSAEPFNOSUPPORT
#	define NW_EAFNOSUPPORT		WSAEAFNOSUPPORT
#	define NW_EADDRINUSE		WSAEADDRINUSE
#	define NW_EADDRNOTAVAIL		WSAEADDRNOTAVAIL
#	define NW_ENETDOWN			WSAENETDOWN
#	define NW_ENETUNREACH		WSAENETUNREACH
#	define NW_ENETRESET			WSAENETRESET
#	define NW_ECONNABORTED		WSAECONNABORTED
#	define NW_ECONNRESET		WSAECONNRESET
#	define NW_ENOBUFS			WSAENOBUFS
#	define NW_EISCONN			WSAEISCONN
#	define NW_ENOTCONN			WSAENOTCONN
#	define NW_ESHUTDOWN			WSAESHUTDOWN
#	define NW_ETOOMANYREFS		WSAETOOMANYREFS
#	define NW_ETIMEDOUT			WSAETIMEDOUT
#	define NW_ECONNREFUSED		WSAECONNREFUSED
#	define NW_ELOOP				WSAELOOP
#	define NW_EHOSTDOWN			WSAEHOSTDOWN
#	define NW_EHOSTUNREACH		WSAEHOSTUNREACH
#	define NW_EPROCLIM			WSAEPROCLIM
#	define NW_EUSERS			WSAEUSERS
#	define NW_EDQUOT			WSAEDQUOT
#	define NW_ESTALE			WSAESTALE
#	define NW_EREMOTE			WSAEREMOTE

#else

#	define _Invalid_SocketID -1
#	define closesocket close
#	define SOCKET_ERROR  -1
#	define SD_SEND SHUT_WR


#	define NW_EWOULDBLOCK		EWOULDBLOCK
#	define NW_ECONNABORTED		ECONNABORTED
#	define NW_EPROTO			EPROTO
#	define NW_EINPROGRESS		EINPROGRESS
#	define NW_EAGAIN			EAGAIN
#	define NW_EINTR				EINTR
#	define NW_EALREADY			EALREADY
#	define NW_ENOTSOCK			ENOTSOCK
#	define NW_EDESTADDRREQ		EDESTADDRREQ
#	define NW_EMSGSIZE			EMSGSIZE
#	define NW_EPROTOTYPE		EPROTOTYPE
#	define NW_ENOPROTOOPT		ENOPROTOOPT
#	define NW_EPROTONOSUPPO		EPROTONOSUPPORT
#	define NW_ESOCKTNOSUPPO		ESOCKTNOSUPPORT
#	define NW_EOPNOTSUPP		EOPNOTSUPP
#	define NW_EPFNOSUPPORT		EPFNOSUPPORT
#	define NW_EAFNOSUPPORT		EAFNOSUPPORT
#	define NW_EADDRINUSE		EADDRINUSE
#	define NW_EADDRNOTAVAIL		EADDRNOTAVAIL
#	define NW_ENETDOWN			ENETDOWN
#	define NW_ENETUNREACH		ENETUNREACH
#	define NW_ENETRESET			ENETRESET
#	define NW_ECONNABORTED		ECONNABORTED
#	define NW_ECONNRESET		ECONNRESET
#	define NW_ENOBUFS			ENOBUFS
#	define NW_EISCONN			EISCONN
#	define NW_ENOTCONN			ENOTCONN
#	define NW_ESHUTDOWN			ESHUTDOWN
#	define NW_ETOOMANYREFS		ETOOMANYREFS
#	define NW_ETIMEDOUT			ETIMEDOUT
#	define NW_ECONNREFUSED		ECONNREFUSED
#	define NW_ELOOP				ELOOP
#	define NW_EHOSTDOWN			EHOSTDOWN
#	define NW_EHOSTUNREACH		EHOSTUNREACH
#	define NW_EPROCLIM			EPROCLIM
#	define NW_EUSERS			EUSERS
#	define NW_EDQUOT			EDQUOT
#	define NW_ESTALE			ESTALE
#	define NW_EREMOTE			EREMOTE
#	define NW_EMFILE			EMFILE

#endif

#define _Invalid_SendConnecterIndex -1
#define _Invalid_SocketIndex -1


/*
enum EPOLL_EVENTS
{
EPOLLIN			= 0x001,
EPOLLPRI		= 0x002,
EPOLLOUT		= 0x004,
EPOLLRDNORM		= 0x040,
EPOLLRDBAND		= 0x080,
EPOLLWRNORM		= 0x100,
EPOLLWRBAND		= 0x200,
EPOLLMSG		= 0x400,
EPOLLERR		= 0x008,
EPOLLHUP		= 0x010,
EPOLLRDHUP		= 0x2000,
EPOLLWAKEUP		= 1u << 29,
EPOLLONESHOT	= 1u << 30,
EPOLLET			= 1u << 31
};

EPOLLIN		�������ӽ������� �Զ���ͨ���ݽ������� �Զ������ر����Ӵ�������ʱ�����ܴ���EPOLLRDHUP��
EPOLLOUT	ˮƽ����ģʽ�£�ֻҪ���ͻ�����û����һֱ���������ش���ģʽ�·��ͻ������Ӹ�ˮλ�����ˮλʱ������ˮλ��������Ĭ����1��
EPOLLERR	socket�ܼ�⵽�Է�������ĿǰΪֹ�������һ���֪����μ�⣬���ǣ��ڸ��Ѿ��رյ�socketдʱ���ᷢ��EPOLLERR��Ҳ����˵��
ֻ���ڲ�ȡ�ж��������һ���Ѿ��رյ�socket������дһ���Ѿ��رյ�socket��ʱ�򣬲�֪���Է��Ƿ�ر��ˡ����ʱ������Է��ر��ˣ�
������EPOLLERR��EPOLLERR�Ƿ�������߳���
EPOLLERR|EPOLLHUP ���������epoll_wait��Ĭ�ϼ�⣬����Ҫ����
EPOLLRDHUP �������Щϵͳ�б�ʾ�Զ��Ѿ��ر�(Linux 2.6.17֮��)�������ǿ�������ȫ������EPOLLIN�¼����жϣ�����û����
*/

	enum ENetEventType
	{
		eNET_Recv		= 1,
		eNET_Send		= 2,
		eNET_Error		= 4,

		eNET_Unknown	= 0xff
	};

#define _SEND_BUF_SIZE	2048
#define _RECV_BUF_SIZE	2048

#define	DebugAst(Exp)			do{ if(!(Exp)) { assert(0); return; } } while(0)
#define	DebugAstEx(Exp, Ret)	do{ if(!(Exp)) { assert(0); return Ret; } } while(0)


#ifndef _countof
#define _countof(elem) sizeof(elem)/sizeof(elem[0])
#endif

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996)
#endif
	// ���Ŀ���ַ������Է��¸�ʽ�����ַ��������ݣ�����\0������ô��ֱ�ӿ�������Ȼ�ͻ�ض��Ա�֤Ŀ���ַ���һ������\0��β
	inline size_t snprintf(char* szBuf, size_t nBufSize, const char* szFormat, ...)
	{
		if (nBufSize == 0 || szBuf == nullptr || szFormat == nullptr)
			return -1;

		va_list arg;
		va_start(arg, szFormat);
		size_t ret = vsnprintf(szBuf, nBufSize, szFormat, arg);
		va_end(arg);

		return ret;
	}
	// ���Ŀ���ַ������Է��¸�ʽ�����ַ��������ݣ�����\0������ô��ֱ�ӿ�������Ȼ�ͻ�ض��Ա�֤Ŀ���ַ���һ������\0��β
	inline size_t vsnprintf(char* szBuf, size_t nBufSize, const char* szFormat, va_list arg)
	{
		if (nBufSize == 0 || szBuf == nullptr || szFormat == nullptr)
			return -1;

#ifdef _WIN32
		// �����þɵĺ�����Ϊ�˸�linux��ͬ����
		int32_t nRet = ::_vsnprintf(szBuf, nBufSize, szFormat, arg);
		if (nRet == nBufSize)
		{
			szBuf[nBufSize - 1] = 0;
			--nRet;
		}
		return nRet;
#else
		return ::vsnprintf(szBuf, nBufSize, szFormat, arg);
#endif
	}

	inline uint32_t getLastError() 
	{
#ifdef _WIN32
		return (uint32_t)GetLastError();
#else
		return (uint32_t)errno;
#endif
	}
#ifdef _WIN32
#pragma warning(pop)
#endif
}