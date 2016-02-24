// TestRPC.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <stdint.h>
#include <iostream>
#include <tuple>
#include <functional>
#include <map>
#include <vector>
#include <string>

namespace rpc
{
	template<class T>
	inline bool read(const char* szBuf, uint32_t nBufSize, uint32_t& nOffset, T& value)
	{
		if (nOffset + sizeof(T) >= nBufSize)
			return false;

		value = *((T*)(szBuf + nOffset));
		nOffset += sizeof(T);

		return true;
	}
	
	inline bool read_str(const char* szBuf, uint32_t nBufSize, uint32_t& nOffset, std::string& value)
	{
		if (nOffset + sizeof(uint16_t) >= nBufSize)
			return false;

		uint16_t nLen = *((uint16_t*)(szBuf + nOffset));
		if (nOffset + sizeof(uint16_t)+nLen >= nBufSize)
			return false;

		value.assign(szBuf + nOffset + sizeof(uint16_t), nLen);
		nOffset += (nLen + sizeof(uint16_t));

		return true;
	}

	template<>
	inline bool read(const char* szBuf, uint32_t nBufSize, uint32_t& nOffset, std::string& value)
	{
		return read_str(szBuf, nBufSize, nOffset, value);
	}

	template<>
	inline bool read(const char* szBuf, uint32_t nBufSize, uint32_t& nOffset, const std::string& value)
	{
		return read_str(szBuf, nBufSize, nOffset, const_cast<std::string&>(value));
	}

	template<class T>
	inline bool write(char* szBuf, uint32_t nBufSize, uint32_t& nOffset, T& val)
	{
		if (nOffset + sizeof(T) >= nBufSize)
			return false;

		*((T*)(szBuf + nOffset)) = val;
		nOffset += sizeof(T);
		
		return true;
	};

	bool write_str(char* szBuf, uint32_t nBufSize, uint32_t& nOffset, const char* val, uint16_t nLen)
	{
		if (nOffset + sizeof(uint16_t) + nLen >= nBufSize)
			return 0;

		*((uint16_t*)(szBuf + nOffset)) = nLen;
		memcpy(szBuf + nOffset + sizeof(uint16_t), val, nLen);
		nOffset += (sizeof(uint16_t) + nLen);

		return true;
	}

	template<>
	inline bool write(char* szBuf, uint32_t nBufSize, uint32_t& nOffset, std::string& val)
	{
		return write_str(szBuf, nBufSize, nOffset, val.c_str(), (uint16_t)val.size());
	};

	template<>
	inline bool write(char* szBuf, uint32_t nBufSize, uint32_t& nOffset, const std::string& val)
	{
		return write_str(szBuf, nBufSize, nOffset, val.c_str(), (uint16_t)val.size());
	};

	template<>
	inline bool write(char* szBuf, uint32_t nBufSize, uint32_t& nOffset, char*& val)
	{
		uint16_t nLen = strnlen(val, 65536);
		return write_str(szBuf, nBufSize, nOffset, val, nLen);
	};

	template<>
	inline bool write(char* szBuf, uint32_t nBufSize, uint32_t& nOffset, const char*& val)
	{
		uint16_t nLen = strnlen(val, 65536);
		return write_str(szBuf, nBufSize, nOffset, val, nLen);
	};

	template<typename ...Args>
	struct SFunctionWrapper
	{
		std::function<void(Args...)>	pf;
		std::tuple<Args...>				args;    //回调函数所需要的参数类型列表
	};

	template<int32_t ARG_SIZE, typename ...Args>
	struct SParseArgs;

	template<typename ...Args>
	struct SBuildNowArgs
	{
		template<typename T, typename ...RemainArgs, typename ...NowArgs>
		static void build(SFunctionWrapper<Args...>* pFunctionWrapper, const char* szBuf, uint32_t nBufSize, uint32_t& nOffset, NowArgs&&... args)
		{
			std::cout << "build " << sizeof...(RemainArgs) << " " << sizeof...(NowArgs) << std::endl;
			T value;
			read(szBuf, nBufSize, nOffset, value);
			SParseArgs<sizeof...(RemainArgs), Args...>::template parse<RemainArgs...>(pFunctionWrapper, szBuf, nBufSize, nOffset, args..., value);
		}
	};

	template<int32_t ARG_SIZE, typename ...Args>
	struct SParseArgs
	{
		template<typename ...RemainArgs, typename ...NowArgs>
		static void parse(SFunctionWrapper<Args...>* pFunctionWrapper, const char* szBuf, uint32_t nBufSize, uint32_t& nOffset, NowArgs&&... args)
		{
			std::cout << "parse " << sizeof...(RemainArgs) << " " << sizeof...(NowArgs) << " " << sizeof...(Args) << std::endl;
			SBuildNowArgs<Args...>::template build<RemainArgs...>(pFunctionWrapper, szBuf, nBufSize, nOffset, args...);
		}
	};

	template<typename ...Args>
	struct SParseArgs<0, Args...>
	{
		template<typename ...NowArgs>
		static void parse(SFunctionWrapper<Args...>* pFunctionWrapper, const char* szBuf, uint32_t nBufSize, uint32_t& nOffset, NowArgs&&... args)
		{
			pFunctionWrapper->pf(args...);
		}
	};

	struct SWriteArgs
	{
		template<typename T, typename...NowArgs>
		static bool write(char* szBuf, uint32_t nBufSize, uint32_t& nOffset, T& t, NowArgs&... args)
		{
			if (!rpc::write(szBuf, nBufSize, nOffset, t))
				return false;

			return SWriteArgs::write(szBuf, nBufSize, nOffset, args...);
		}

		static bool write(char* szBuf, uint32_t nBufSize, uint32_t& nOffset)
		{
			return true;
		}
	};

	class CRpcWrapper
	{
	public:
		CRpcWrapper()
		{
			this->m_szBuf.resize(65536);
		}

		~CRpcWrapper()
		{
		}

		template<typename ...Args>
		void registFunction(uint32_t nID, void(*pf)(Args...))
		{
			if (pf == nullptr)
				return;

			if (this->m_mapFunctionWrapper.find(nID) != this->m_mapFunctionWrapper.end())
				return;

			SFunctionWrapper<Args...>* pFunctionWrapper = new SFunctionWrapper<Args...>();
			pFunctionWrapper->pf = pf;

			this->m_mapFunctionWrapper[nID] = pFunctionWrapper;
		}

		void execute(uint32_t nID, char* szBuf, uint32_t nBufSize)
		{

		}

		template<typename ...Args>
		bool call(uint32_t nID, Args... args)
		{
			uint32_t nOffset = 0;
			if (!SWriteArgs::write(&this->m_szBuf[0], this->m_szBuf.size(), nOffset, args...))
				return false;

			auto iter = this->m_mapFunctionWrapper.find(nID);
			if (iter == this->m_mapFunctionWrapper.end())
				return false;

			nOffset = 0;
			SFunctionWrapper<Args...>* pFunctionWrapper = (SFunctionWrapper<Args...>*)iter->second;
			SParseArgs<sizeof...(Args), Args...>::template parse<Args...>(pFunctionWrapper, &this->m_szBuf[0], this->m_szBuf.size(), nOffset);

			return true;
		}

	private:
		std::map<uint32_t, void*>	m_mapFunctionWrapper;
		std::vector<char>			m_szBuf;
	};
}

void f1(int a, int b, int c)
{
	std::cout << a << " " << b << " " << c << std::endl;
}

int main()
{
	rpc::CRpcWrapper rpcWrapper;
	rpcWrapper.registFunction(1, &f1);
	std::string szArg = "DDD";
	rpcWrapper.call(1, 1, 2, 3);

	return 0;
}