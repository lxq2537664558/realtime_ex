#include "lua_debug.h"
#include "lua_facade.h"

#include "libBaseCommon/thread_base.h"
#include "libBaseCommon/base_function.h"
#include "libBaseCommon/logger.h"
#include "libBaseCommon/spin_mutex.h"

#include <errno.h>

#include <vector>
#include <list>
#include <map>
#include <string>

#ifndef _WIN32
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#endif

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996)
#endif

#ifdef _WIN32

#	define MSG_NOSIGNAL 0
#	define _Invalid_SocketID (int32_t)INVALID_SOCKET


#	define NW_EWOULDBLOCK		WSAEWOULDBLOCK
#	define NW_ECONNABORTED		WSAECONNABORTED
#	define NW_EPROTO			EPROTO
#	define NW_EINPROGRESS		WSAEINPROGRESS
#	define NW_EAGAIN			EAGAIN
#	define NW_EINTR				EINTR


#else

#	define _Invalid_SocketID -1
#	define closesocket close
#	define SOCKET_ERROR  -1


#	define NW_EWOULDBLOCK		EWOULDBLOCK
#	define NW_ECONNABORTED		ECONNABORTED
#	define NW_EPROTO			EPROTO
#	define NW_EINPROGRESS		EINPROGRESS
#	define NW_EAGAIN			EAGAIN
#	define NW_EINTR				EINTR

#endif

#define NORMAL_PROMPT "> "
#define DEBUG_PROMPT "ldbg> "

class CDebugSocket :
	public base::IRunnable
{
public:
	CDebugSocket();
	virtual ~CDebugSocket();

	bool			init(const char* szIP, uint16_t nPort);
	void			uninit();
	void			send(std::string szBuf);
	bool			getNormalCommand(std::string& szCommand);
	bool			getDebugCommand(std::string& szCommand);
	void			putDebugCommand(std::string& szCommand);

private:
	virtual bool	onInit() { return true; }
	virtual void	onDestroy() { }
	virtual bool	onProcess();

private:
	base::CThreadBase*		m_pThreadBase;
	int32_t					m_nAcceptSocketID;
	int32_t					m_nDebugSocketID;
	std::list<std::string>	m_listNormalCommand;	// 完整的收到的命令
	std::string				m_szSendBuf;			// 发送缓存
	std::string				m_szRecvBuf;			// 这个收到的不完整的命令
	base::spin_mutex		m_lock;

	// 主线程操作的变量
	std::list<std::string>	m_listDebugCommand;
};

CDebugSocket::CDebugSocket()
{
	this->m_pThreadBase = nullptr;
	this->m_nAcceptSocketID = _Invalid_SocketID;
	this->m_nDebugSocketID = _Invalid_SocketID;
}

CDebugSocket::~CDebugSocket()
{

}

bool CDebugSocket::init(const char* szIP, uint16_t nPort)
{
#ifdef _WIN32
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 0);

	int32_t err = ::WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		PrintWarning("WSAStartup failed with error: %d", err);
		return false;
	}
#endif

	this->m_nAcceptSocketID = (int32_t)::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_Invalid_SocketID == this->m_nAcceptSocketID)
		return false;

	uint32_t nReuseFlag = 1;
	if (0 != ::setsockopt(this->m_nAcceptSocketID, SOL_SOCKET, SO_REUSEADDR, (char*)&nReuseFlag, sizeof(nReuseFlag)))
	{
		PrintWarning("ldbg setsockopt error: %d", base::getLastError());
		closesocket(this->m_nAcceptSocketID);
		this->m_nAcceptSocketID = _Invalid_SocketID;
		return false;
	}

#ifdef _WIN32
	u_long nNoBlockFlag = 1;
	if (0 != ::ioctlsocket(this->m_nAcceptSocketID, FIONBIO, &nNoBlockFlag))
	{
		PrintWarning("ldbg ioctlsocket error %d", base::getLastError());
		closesocket(this->m_nAcceptSocketID);
		this->m_nAcceptSocketID = _Invalid_SocketID;
		return false;
	}
#else
	int32_t nNoBlockFlag = ::fcntl(this->m_nAcceptSocketID, F_GETFL, 0);
	if (-1 == nNoBlockFlag)
	{
		PrintWarning("ldbg fcntl error %d", base::getLastError());
		closesocket(this->m_nAcceptSocketID);
		this->m_nAcceptSocketID = _Invalid_SocketID;
		return false;
	}

	nNoBlockFlag = ::fcntl(this->m_nAcceptSocketID, F_SETFL, nNoBlockFlag | O_NONBLOCK);
	if (-1 == nNoBlockFlag)
	{
		PrintWarning("ldbg fcntl error %d", base::getLastError());
		closesocket(this->m_nAcceptSocketID);
		this->m_nAcceptSocketID = _Invalid_SocketID;
		return false;
	}
#endif

	struct sockaddr_in listen_addr;

	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = inet_addr(szIP);
	listen_addr.sin_port = htons(nPort);

	if (::bind(this->m_nAcceptSocketID, (const struct sockaddr*)&listen_addr, sizeof(listen_addr)) < 0)
	{
		PrintWarning("ldbg bind error: %d", base::getLastError());
		closesocket(this->m_nAcceptSocketID);
		return false;
	}
	if (::listen(this->m_nAcceptSocketID, 1) < 0)
	{
		PrintWarning("ldbg listen error: %d", base::getLastError());
		closesocket(this->m_nAcceptSocketID);
		return false;
	}

	this->m_pThreadBase = base::CThreadBase::createNew(this);

	return true;
}

void CDebugSocket::send(std::string szBuf)
{
	std::lock_guard<base::spin_mutex> guard(this->m_lock);
	this->m_szSendBuf += szBuf;
}

bool CDebugSocket::getDebugCommand(std::string& szCommand)
{
	if (this->m_listDebugCommand.empty())
		return false;

	szCommand = this->m_listDebugCommand.front();
	this->m_listDebugCommand.pop_front();
	return true;
}

void CDebugSocket::putDebugCommand(std::string& szCommand)
{
	this->m_listDebugCommand.push_back(szCommand);
}

bool CDebugSocket::getNormalCommand(std::string& szCommand)
{
	std::lock_guard<base::spin_mutex> guard(this->m_lock);
	if (this->m_listNormalCommand.empty())
		return false;

	szCommand = this->m_listNormalCommand.front();
	this->m_listNormalCommand.pop_front();
	return true;
}

bool CDebugSocket::onProcess()
{
	// 先试着发送数据
	if (this->m_nDebugSocketID != _Invalid_SocketID)
	{
		std::lock_guard<base::spin_mutex> guard(this->m_lock);
		if (!this->m_szSendBuf.empty())
		{
			while (true)
			{
				int32_t nRet = ::send(this->m_nDebugSocketID, this->m_szSendBuf.c_str(), (int32_t)this->m_szSendBuf.size(), MSG_NOSIGNAL);
				if (SOCKET_ERROR == nRet)
				{
					if (base::getLastError() != NW_EAGAIN && base::getLastError() != NW_EWOULDBLOCK && base::getLastError() == NW_EINTR)
					{
						closesocket(this->m_nDebugSocketID);
						this->m_nDebugSocketID = _Invalid_SocketID;
					}
					break;
				}
				else
				{
					this->m_szSendBuf.erase(this->m_szSendBuf.begin(), this->m_szSendBuf.begin() + nRet);
					if (this->m_szSendBuf.empty())
						break;
				}
			}
		}
	}
	fd_set read_fd;
	struct timeval tv = { 0, 1000 };

	FD_ZERO(&read_fd);
	FD_SET(this->m_nAcceptSocketID, &read_fd);
	if (this->m_nDebugSocketID != _Invalid_SocketID)
		FD_SET(this->m_nDebugSocketID, &read_fd);

	int32_t nRet = select(FD_SETSIZE, &read_fd, NULL, NULL, &tv);
	if (SOCKET_ERROR == nRet)
	{
		PrintWarning("ldbg select error: %d", base::getLastError());
		return true;
	}

	if (this->m_nDebugSocketID != _Invalid_SocketID)
	{
		if (FD_ISSET(this->m_nDebugSocketID, &read_fd))
		{
			while (true)
			{
				char szBuf[4096] = { 0 };
				int32_t nRet = ::recv(this->m_nDebugSocketID, szBuf, _countof(szBuf), 0);
				if (0 == nRet)
				{
					closesocket(this->m_nDebugSocketID);
					this->m_nDebugSocketID = _Invalid_SocketID;
					break;
				}
				else if (nRet < 0)
				{
					if (base::getLastError() != NW_EAGAIN && base::getLastError() != NW_EWOULDBLOCK && base::getLastError() != NW_EINTR)
					{
						closesocket(this->m_nDebugSocketID);
						this->m_nDebugSocketID = _Invalid_SocketID;
					}
					break;
				}
				else
				{
					this->m_szRecvBuf.append(szBuf, szBuf + nRet);
					do
					{
						bool bFind = false;
						std::string szCommand;
						for (size_t i = 0; i < this->m_szRecvBuf.size(); ++i)
						{
							if (this->m_szRecvBuf[i] == '\n')
							{
								bFind = true;
								if (i >= 1)
									szCommand.erase(i - 1);
								break;
							}
							szCommand.push_back(this->m_szRecvBuf[i]);
						}
						if (!bFind)
							break;

						this->m_szRecvBuf.erase(this->m_szRecvBuf.begin(), this->m_szRecvBuf.begin() + szCommand.size() + 2);

						std::lock_guard<base::spin_mutex> guard(this->m_lock);
						this->m_listNormalCommand.push_back(szCommand);
					} while (true);
				}
			}
		}
	}

	// 接收新连接
	if (FD_ISSET(this->m_nAcceptSocketID, &read_fd))
	{
		int32_t nSocketID = (int32_t)accept(this->m_nAcceptSocketID, NULL, NULL);
		if (_Invalid_SocketID == nSocketID)
		{
			if (base::getLastError() != NW_EWOULDBLOCK
				&& base::getLastError() != NW_ECONNABORTED
				&& base::getLastError() != NW_EPROTO)
			{
				PrintWarning("ldbg accept error: %d", base::getLastError());
			}

			return true;
		}

		if (this->m_nDebugSocketID != _Invalid_SocketID)
		{
			PrintInfo("ldbg scoket is exist close this");
			closesocket(nSocketID);
			return true;
		}

#ifdef _WIN32
		u_long nNoBlockFlag = 1;
		if (0 != ::ioctlsocket(nSocketID, FIONBIO, &nNoBlockFlag))
		{
			PrintWarning("ldbg ioctlsocket error %d", base::getLastError());
			closesocket(nSocketID);
			return true;
		}
#else
		int32_t nNoBlockFlag = ::fcntl(this->m_nAcceptSocketID, F_GETFL, 0);
		if (-1 == nNoBlockFlag)
		{
			PrintWarning("ldbg fcntl error %d", base::getLastError());
			closesocket(this->m_nAcceptSocketID);
			this->m_nAcceptSocketID = _Invalid_SocketID;
			return false;
		}

		nNoBlockFlag = ::fcntl(this->m_nAcceptSocketID, F_SETFL, nNoBlockFlag | O_NONBLOCK);
		if (-1 == nNoBlockFlag)
		{
			PrintWarning("ldbg fcntl error %d", base::getLastError());
			closesocket(this->m_nAcceptSocketID);
			this->m_nAcceptSocketID = _Invalid_SocketID;
			return false;
		}
#endif

		this->m_nDebugSocketID = nSocketID;
		std::string szText = "Welcome to ldbg.\n";
		szText += NORMAL_PROMPT;
		this->send(szText);
	}

	return true;
}

struct SBreakpointInfo
{
	uint32_t	nID;
	std::string	szSource;
	uint32_t	nLine;
	bool		bActive;
};

class CDebugger
{
public:
	CDebugger();
	~CDebugger();

	bool				init(const char* szIP, uint16_t nPort);
	void				update(lua_State* pL);
	bool				executeCommand(lua_State* pL, lua_Debug* ar);
	SBreakpointInfo*	getBreakpointInfo(const char* short_src, uint32_t line);
	std::string			readSourceCode(std::string szFile, uint32_t nLine);
	void				debugResponse(bool bPrompt, const char* szFormat, ...);
	void				addBreakpoint(lua_State *pL, std::string& szExpr, bool bFullName);
	bool				isOpenDebug() const;

private:
	bool				searchLocalVar(lua_State *pL, lua_Debug *ar, const char* var);
	bool				searchGlobalVar(lua_State *pL, lua_Debug *ar, const char* var);
	void				printTableVar(lua_State *pL, int32_t nIndex, int32_t nDepth);

	void				printVar(lua_State *pL, int32_t nIndex, int32_t nDepth);
	void				printExpr(lua_State *pL, lua_Debug *ar, std::string& szExpr);

	void				listSource(std::string& szExpr);
	void				listBreakpoints();
	void				delBreakpoint(std::string& szExpr);
	void				enableBreakpoint(std::string& szExpr);
	void				disableBreakpoint(std::string& szExpr);
	void				debugHelp();

	int32_t				getCalldepth(lua_State* pL);

	static void			debugHook(lua_State* pL, lua_Debug* ar);

private:
	enum EDebugMode
	{
		eDM_Normal,	// 正常跑
		eDM_Step,	// 逐语句
		eDM_Next,	// 逐过程
	};

	uint32_t					m_nMaxBreakpointID;			// 最大的断点索引
	std::string					m_szLastCommandExpr;		// 最近的命令表达式
	int32_t						m_nLastCalldepth;			// 最近一次调试的栈深度
	EDebugMode					m_eDebugMode;				// 调试模式
	std::list<SBreakpointInfo>	m_listBreakpointInfo;		// 断点信息
	bool						m_bOpenDebug;				// 是否打开调试
	CDebugSocket*				m_pDebugSocket;
};


CDebugger::CDebugger()
{
	this->m_bOpenDebug = false;
	this->m_pDebugSocket = nullptr;
	this->m_nLastCalldepth = 0;
	this->m_nMaxBreakpointID = 0;
	this->m_eDebugMode = eDM_Normal;
}

CDebugger::~CDebugger()
{
	SAFE_DELETE(this->m_pDebugSocket);
}

bool CDebugger::init(const char* szIP, uint16_t nPort)
{
	this->m_pDebugSocket = new CDebugSocket();
	if (!this->m_pDebugSocket->init(szIP, nPort))
		return false;

	return true;
}

void CDebugger::debugResponse(bool bPrompt, const char *szFormat, ...)
{
	char szBuf[4096] = { 0 };
	va_list arg;
	va_start(arg, szFormat);
	base::crt::vsnprintf(szBuf, _countof(szBuf), szFormat, arg);
	va_end(arg);

	if (bPrompt)
		this->m_pDebugSocket->send(DEBUG_PROMPT);

	this->m_pDebugSocket->send(szBuf);
}

bool CDebugger::searchLocalVar(lua_State *pL, lua_Debug *ar, const char *var)
{
	const char* name = nullptr;
	for (uint32_t i = 1; (name = lua_getlocal(pL, ar, i)) != nullptr; ++i)
	{
		if (strcmp(var, name) == 0)
			return true;

		lua_pop(pL, 1);
	}
	return false;
}

bool CDebugger::searchGlobalVar(lua_State *pL, lua_Debug *ar, const char *var)
{
	lua_getglobal(pL, var);

	if (lua_type(pL, -1) == LUA_TNIL)
	{
		lua_pop(pL, 1);
		return false;
	}

	return true;
}

void CDebugger::printVar(lua_State *pL, int32_t nIndex, int32_t nDepth)
{
	int32_t nType = lua_type(pL, nIndex);
	switch (nType)
	{
	case LUA_TNIL:
		this->debugResponse(false, "(nil)");
		break;

	case LUA_TNUMBER:
		this->debugResponse(false, "%f", lua_tonumber(pL, nIndex));
		break;

	case LUA_TBOOLEAN:
		this->debugResponse(false, "%s", lua_toboolean(pL, nIndex) ? "true" : "false");
		break;

	case LUA_TFUNCTION:
		{
			lua_CFunction func = lua_tocfunction(pL, nIndex);
			if (func != nullptr)
				this->debugResponse(false, "(C function)%p", func);
			else
				this->debugResponse(false, "(function)");
		}
		break;

	case LUA_TUSERDATA:
		this->debugResponse(false, "(user data)%p", lua_touserdata(pL, nIndex));
		break;

	case LUA_TLIGHTUSERDATA:
		this->debugResponse(false, "(light user data)%p", lua_touserdata(pL, nIndex));
		break;

	case LUA_TSTRING:
		this->debugResponse(false, "(string)%s", lua_tostring(pL, nIndex));
		break;

	case LUA_TTABLE:
		printTableVar(pL, nIndex, nDepth);
		break;

	default:
		break;
	}
}

void CDebugger::printExpr(lua_State *pL, lua_Debug *ar, std::string& szExpr)
{
	if (this->searchLocalVar(pL, ar, szExpr.c_str()))
	{
		this->debugResponse(false, "local %s = ", szExpr.c_str());
		printVar(pL, -1, -1);
		lua_pop(pL, 1);
		this->debugResponse(false, "\n");
	}
	else if (searchGlobalVar(pL, ar, szExpr.c_str()))
	{
		this->debugResponse(false, "global %s = ", szExpr.c_str());
		printVar(pL, -1, -1);
		lua_pop(pL, 1);
		this->debugResponse(false, "\n");
	}
	else
	{
		this->debugResponse(false, "not found var %s\n", szExpr.c_str());
	}
}

void CDebugger::printTableVar(lua_State *pL, int32_t nIndex, int32_t nDepth)
{
	int32_t pos_si = nIndex > 0 ? nIndex : (nIndex - 1);
	this->debugResponse(false, "{");
	int32_t top = lua_gettop(pL);
	lua_pushnil(pL);
	bool bEmpty = true;
	while (lua_next(pL, pos_si) != 0)
	{
		if (bEmpty)
		{
			this->debugResponse(false, "\n");
			bEmpty = 0;
		}

		for (int32_t i = 0; i < nDepth; ++i)
		{
			this->debugResponse(false, "\t");
		}

		this->debugResponse(false, "[");
		this->printVar(pL, -2, -1);
		this->debugResponse(false, "] = ");
		if (nDepth > 5)
			this->debugResponse(false, "{...}");
		else
			this->printVar(pL, -1, nDepth + 1);

		lua_pop(pL, 1);
		this->debugResponse(false, ",\n");
	}

	if (bEmpty)
		this->debugResponse(false, " }");
	else
	{
		for (int32_t i = 0; i < nDepth - 1; ++i)
		{
			this->debugResponse(false, "\t");
		}
		this->debugResponse(false, "}");
	}

	lua_settop(pL, top);
}

void CDebugger::listSource(std::string& szExpr)
{

}

static const char* pushnexttemplate(lua_State *pL, const char *szPath)
{
	const char *l;
	while (*szPath == *LUA_PATHSEP)
		szPath++;  /* skip separators */
	if (*szPath == '\0')
		return nullptr;  /* no more templates */
	l = strchr(szPath, *LUA_PATHSEP);  /* find next separator */
	if (l == nullptr)
		l = szPath + strlen(szPath);
	lua_pushlstring(pL, szPath, l - szPath);  /* template */
	return l;
}

static const char* getFullName(lua_State *pL, const char *szShortName)
{
	szShortName = luaL_gsub(pL, szShortName, ".", LUA_DIRSEP);
	lua_getglobal(pL, "package");
	lua_getfield(pL, -1, "path");
	const char *szPath = lua_tostring(pL, -1);
	if (szPath == nullptr)
		return nullptr;
	lua_pushliteral(pL, "");  /* error accumulator */
	while ((szPath = pushnexttemplate(pL, szPath)) != nullptr)
	{
		const char *szFullName = luaL_gsub(pL, lua_tostring(pL, -1), LUA_PATH_MARK, szShortName);
		lua_remove(pL, -2);  /* remove path template */
		FILE *pFile = fopen(szFullName, "r");
		if (nullptr != pFile)
		{
			fclose(pFile);
			return szFullName;  /* return that file name */
		}
		lua_pushfstring(pL, "\n\tno file " LUA_QS, szFullName);
		lua_remove(pL, -2);  /* remove file name */
		lua_concat(pL, 2);  /* add entry to possible error message */
	}
	return nullptr;  /* not found */
}

void CDebugger::addBreakpoint(lua_State *pL, std::string& szExpr, bool bFullName)
{
	std::string::size_type pos = szExpr.find(":");
	if (pos == std::string::npos)
	{
		this->debugResponse(false, "add breakpoint error, expr (%s) invalid\n", szExpr.c_str());
		return;
	}

	std::string szShortSource = szExpr.substr(0, pos);
	const char* szFullSource = nullptr;
	if (!bFullName)
	{
		std::string::size_type pos = szShortSource.find(".lua");
		if (pos != std::string::npos)
			szShortSource = szShortSource.substr(0, pos);
		szFullSource = getFullName(pL, szShortSource.c_str());
	}
	else
	{
		szFullSource = szShortSource.c_str();
	}

	std::string szLine = szExpr.substr(pos + 1, szExpr.size() - pos);
	uint32_t nLine = 0;
	if (!base::crt::atoui(szLine.c_str(), nLine))
	{
		this->debugResponse(false, "add breakpoint error, expr (%s) invalid\n", szExpr.c_str());
		return;
	}

	if (this->getBreakpointInfo(szFullSource, nLine) != nullptr)
	{
		this->debugResponse(false, "breakpoint (%s) existed\n", szExpr.c_str());
		return;
	}

	SBreakpointInfo sBreakpointInfo;
	sBreakpointInfo.szSource = szFullSource;
	sBreakpointInfo.nLine = nLine;
	sBreakpointInfo.bActive = true;
	sBreakpointInfo.nID = this->m_nMaxBreakpointID++;

	this->m_listBreakpointInfo.push_back(sBreakpointInfo);

	this->debugResponse(false, "breakpoint %d at %s\n", sBreakpointInfo.nID, szExpr.c_str());
}

bool CDebugger::isOpenDebug() const
{
	return this->m_bOpenDebug;
}

void CDebugger::listBreakpoints()
{

}

void CDebugger::delBreakpoint(std::string& szExpr)
{
	uint32_t nID = 0;
	if (!base::crt::atoui(szExpr.c_str(), nID))
	{
		this->debugResponse(false, "del breakpoint error, invalid breakpoint id (%d)\n", szExpr.c_str());
		return;
	}

	for (auto iter = this->m_listBreakpointInfo.begin(); iter != this->m_listBreakpointInfo.end(); ++iter)
	{
		if (iter->nID == nID)
			this->m_listBreakpointInfo.erase(iter);
	}
}

void CDebugger::enableBreakpoint(std::string& szExpr)
{
	uint32_t nID = 0;
	if (!base::crt::atoui(szExpr.c_str(), nID))
	{
		this->debugResponse(false, "del breakpoint error, invalid breakpoint id (%d)\n", szExpr.c_str());
		return;
	}

	for (auto iter = this->m_listBreakpointInfo.begin(); iter != this->m_listBreakpointInfo.end(); ++iter)
	{
		if (iter->nID == nID)
			iter->bActive = true;
	}
}

void CDebugger::disableBreakpoint(std::string& szExpr)
{
	uint32_t nID = 0;
	if (!base::crt::atoui(szExpr.c_str(), nID))
	{
		this->debugResponse(false, "del breakpoint error, invalid breakpoint id (%d)\n", szExpr.c_str());
		return;
	}

	for (auto iter = this->m_listBreakpointInfo.begin(); iter != this->m_listBreakpointInfo.end(); ++iter)
	{
		if (iter->nID == nID)
			iter->bActive = false;
	}
}

void CDebugger::debugHelp()
{

}

int32_t	CDebugger::getCalldepth(lua_State* pL)
{
	lua_Debug ar;
	int32_t nIndex = 1;
	while (lua_getstack(pL, nIndex, &ar))
	{
		nIndex++;
	}

	return nIndex - 1;
}

SBreakpointInfo* CDebugger::getBreakpointInfo(const char* short_src, uint32_t line)
{
	for (auto iter = this->m_listBreakpointInfo.begin(); iter != this->m_listBreakpointInfo.end(); ++iter)
	{
		SBreakpointInfo& sBreakpointInfo = *iter;
		if (sBreakpointInfo.nLine == line && stricmp(sBreakpointInfo.szSource.c_str(), short_src) == 0)
			return &sBreakpointInfo;
	}

	return nullptr;
}

void CDebugger::update(lua_State* pL)
{
	std::string szCommand;
	if (!this->m_pDebugSocket->getNormalCommand(szCommand))
		return;

	if (!this->m_bOpenDebug)
	{
		if (szCommand == "start")
		{
			this->m_bOpenDebug = true;
			this->m_eDebugMode = eDM_Next;
			lua_sethook(pL, &debugHook, LUA_MASKLINE, 0);
		}
		else
		{
			this->m_pDebugSocket->send(NORMAL_PROMPT);
		}
	}
	else
	{
		if (szCommand == "quit" || szCommand == "q")
		{
			this->m_bOpenDebug = false;
			lua_sethook(pL, nullptr, 0, 0);
			this->m_pDebugSocket->send(NORMAL_PROMPT);
		}
		else
		{
			if (this->m_eDebugMode == eDM_Normal)
				this->m_eDebugMode = eDM_Step;
			this->m_pDebugSocket->putDebugCommand(szCommand);
		}
	}
}

std::string CDebugger::readSourceCode(std::string szFile, uint32_t nLine)
{
	uint32_t nIndex = 1;
	char szBuf[2048] = { 0 };
	FILE *pFile = fopen(szFile.c_str(), "r");
	if (pFile != nullptr)
	{
		while (fgets(szBuf, _countof(szBuf), pFile))
		{
			if (nIndex == nLine)
			{
				size_t nLen = strlen(szBuf);
				if (szBuf[nLen - 1] == '\n')
					szBuf[nLen - 1] = '\0';
				break;
			}
			++nIndex;
		}
		fclose(pFile);
	}

	return szBuf;
}

bool CDebugger::executeCommand(lua_State *pL, lua_Debug* ar)
{
	this->debugResponse(true, "");

	std::string command_expr;

	while (true)
	{
		if (this->m_pDebugSocket->getDebugCommand(command_expr))
			break;

		if (!this->m_bOpenDebug)
			return true;

		this->update(pL);
	}

	// 取上一次的命令, 方便调试
	if (!command_expr.empty())
		this->m_szLastCommandExpr = command_expr;
	else
		command_expr = this->m_szLastCommandExpr;

	std::string command = command_expr;
	std::string expr;
	std::string::size_type si = command_expr.find_first_of(' ');

	if (si != std::string::npos)
	{
		command = command_expr.substr(0, si);
		expr = command_expr.substr(si + 1, command_expr.size() - si);
	}

	if (command == "c")
	{
		this->m_eDebugMode = eDM_Normal;
		return true;
	}
	else if (command == "s")
	{
		this->m_eDebugMode = eDM_Step;
		return true;
	}
	else if (command == "n")
	{
		this->m_eDebugMode = eDM_Next;
		this->m_nLastCalldepth = this->getCalldepth(pL);
		return true;
	}
	else if (command == "p")
	{
		this->printExpr(pL, ar, expr);
	}
	else if (command == "b")
	{
		this->addBreakpoint(pL, expr, false);
	}
	else if (command == "bl")
	{
		this->listBreakpoints();
	}
	else if (command == "d")
	{
		this->delBreakpoint(expr);
	}
	else if (command == "be")
	{
		this->enableBreakpoint(expr);
	}
	else if (command == "bd")
	{
		this->disableBreakpoint(expr);
	}
	else if (command == "bt")
	{
	}
	else if (command == "h")
	{
		this->debugHelp();
	}
	else if (command == "l")
	{
		this->listSource(expr);
	}
	else if (command.empty() || command[0] == '\0')
	{
	}
	else
	{
		this->debugResponse("invalid command_expr:%s\n", command_expr.c_str());
	}

	return false;
}

static CDebugger* g_pDebugger = nullptr;

void CDebugger::debugHook(lua_State* pL, lua_Debug* ar)
{
	if (!lua_getinfo(pL, "lnSu", ar))
		return;

	// s, n, 断点时中断
	bool bInterrupt = false;
	// 判断是否在next调试
	if (g_pDebugger->m_eDebugMode == eDM_Next)
	{
		int32_t nCalldepth = g_pDebugger->getCalldepth(pL);
		// 函数返回了, 调用栈数量就会比现在小
		if (nCalldepth < g_pDebugger->m_nLastCalldepth)
			bInterrupt = true;
		else if (nCalldepth == g_pDebugger->m_nLastCalldepth)
			bInterrupt = true;

		if (bInterrupt)
			g_pDebugger->m_eDebugMode = eDM_Normal;
	}
	else if (g_pDebugger->m_eDebugMode == eDM_Step)
		bInterrupt = true;

	// 判断是否有断点命中
	SBreakpointInfo* pBreakpointInfo = g_pDebugger->getBreakpointInfo(ar->short_src, ar->currentline);

	if (pBreakpointInfo != nullptr && pBreakpointInfo->bActive)
		bInterrupt = true;

	if (bInterrupt)
	{
		std::string szSourceCode = g_pDebugger->readSourceCode(ar->short_src, ar->currentline);
		if (pBreakpointInfo != nullptr && pBreakpointInfo->bActive)
		{
			std::string szFunName = ar->name != nullptr ? ar->name : "unknown";
			char szBuf[1024] = { 0 };
			base::crt::snprintf(szBuf, _countof(szBuf), "breakpoint %d, %s:%d(%s)\n%s:%d\n", pBreakpointInfo->nID, ar->short_src, ar->currentline, szFunName.c_str(), szSourceCode.c_str(), ar->currentline);
			g_pDebugger->debugResponse(false, szBuf, false, false);
		}
		else
		{
			char szBuf[1024] = { 0 };
			base::crt::snprintf(szBuf, _countof(szBuf), "%s:%d\n", szSourceCode.c_str(), ar->currentline);
			g_pDebugger->debugResponse(false, szBuf, false, false);
		}

		while (!g_pDebugger->executeCommand(pL, ar));
	}
}

static int32_t addBreakpoint(lua_State* pL)
{
	if (g_pDebugger == nullptr || !g_pDebugger->isOpenDebug())
		return 0;

	lua_Debug ar;
	if (!lua_getstack(pL, 1, &ar))
		return 0;

	if (!lua_getinfo(pL, "Slf", &ar))
		return 0;

	char szBuf[1024] = { 0 };
	base::crt::snprintf(szBuf, _countof(szBuf), "%s:%d", ar.short_src, ar.currentline);
	std::string szExpr = szBuf;

	if (g_pDebugger->getBreakpointInfo(ar.short_src, ar.currentline) != nullptr)
		return 0;

	g_pDebugger->addBreakpoint(pL, szExpr, true);

	return 0;
}

bool startLuaDebug(lua_State* pL, const char* szIP, uint16_t nPort)
{
	DebugAstEx(szIP != nullptr, false);

	if (g_pDebugger != nullptr)
		return false;

	g_pDebugger = new CDebugger();
	if (!g_pDebugger->init(szIP, nPort))
	{
		SAFE_DELETE(g_pDebugger);
		return false;
	}

	luaL_Reg zFuncs[] =
	{
		{ "ldbg", addBreakpoint },
		{ nullptr, nullptr }
	};

	luaL_register(pL, "debugger", zFuncs);

	return true;
}

void updateLuaDebug(lua_State* pL)
{
	if (g_pDebugger == nullptr)
		return;

	g_pDebugger->update(pL);
}

#ifdef _WIN32
#pragma warning(pop)
#endif