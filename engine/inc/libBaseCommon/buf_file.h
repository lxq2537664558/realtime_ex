#pragma once

#include "noncopyable.h"
#include "debug_helper.h"
#include "base_function.h"

#include <vector>

namespace base
{
	/**
	@brief: 把一段buf类似一个文件读取
	*/
	class CReadBufFile :
		public noncopyable
	{
	public:
		enum ESeekType
		{
			eST_Cur,
			eST_Begin,
			eST_End
		};

	public:
		CReadBufFile(void* pBuf, uint32_t nBufSize)
		{
			this->m_szBuf = static_cast<char*>(pBuf);
			this->m_nBufSize = nBufSize;
			this->m_nCurPos = 0;
		}

		inline bool		seekPos(uint32_t nOffset, ESeekType eType = eST_Cur)
		{
			if (eST_Cur == eType)
				this->m_nCurPos += nOffset;
			else if (eST_Begin == eType)
				this->m_nCurPos = nOffset;
			else if (eST_End == eType)
				this->m_nCurPos = this->m_nBufSize - nOffset;
			else
				return false;

			return true;
		}
		inline char*	getBuf() const { return this->m_szBuf + this->m_nCurPos; }
		inline uint32_t	getCurPos() const { return this->m_nCurPos; }
		inline uint32_t	getBufSize() const { return this->m_nBufSize; }
		inline bool		readRawBuf(void* pBuf, uint32_t nSize)
		{
			DebugAstEx(pBuf != nullptr, false);

			if (this->m_nCurPos + nSize > this->m_nBufSize)
				return false;

			memcpy(pBuf, this->m_szBuf + this->m_nCurPos, nSize);
			this->m_nCurPos += nSize;

			return true;
		}

		template<class T>
		inline bool		readBuf(T* pBuf)
		{
			if (this->m_nCurPos + sizeof(T) > this->m_nBufSize)
				return false;

			memcpy(pBuf, this->m_szBuf + this->m_nCurPos, sizeof(T));
			this->m_nCurPos += sizeof(T);

			return true;
		}

	private:
		char*		m_szBuf;
		uint32_t	m_nBufSize;
		uint32_t	m_nCurPos;
	};

	/**
	@brief: 类似一个文件写到一个buf中
	*/
	class CWriteBufFile :
		public noncopyable
	{
	public:
		CWriteBufFile(uint32_t nSize)
		{
			this->m_vecBuf.resize(nSize);
			this->m_nSize = 0;
		}

		inline const char*	getBuf() const { return &this->m_vecBuf[0]; }
		inline uint32_t		getBufSize() const { return this->m_nSize; }
		inline void			writeRawBuf(const void* pBuf, uint32_t nSize)
		{
			if (0 == nSize)
				return;

			DebugAst(pBuf != nullptr);
			size_t nRemainSize = this->m_vecBuf.size() - this->m_nSize;
			DebugAst(nRemainSize >= 0);
			if (nRemainSize < nSize)
				this->m_vecBuf.resize(this->m_vecBuf.size() + nSize);

			memcpy(&this->m_vecBuf[0] + this->m_nSize, pBuf, nSize);
			this->m_nSize += nSize;
		}

		template<class T>
		inline void			writeBuf(const T* pBuf)
		{
			DebugAst(pBuf != nullptr);
			int32_t nRemainSize = this->m_vecBuf.size() - this->m_nSize;
			DebugAst(nRemainSize >= 0);
			if (nRemainSize < sizeof(T))
				this->m_vecBuf.resize(this->m_vecBuf.size() + sizeof(T));

			memcpy(&this->m_vecBuf[0] + this->m_nSize, pBuf, sizeof(T));
			this->m_nSize += sizeof(T);
		}

	private:
		std::vector<char>	m_vecBuf;
		size_t				m_nSize;
	};
}