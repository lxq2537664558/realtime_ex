#pragma once

#include "base_common.h"
#include "noncopyable.h"
#include "debug_helper.h"
#include "function_util.h"

#include <vector>

namespace base
{
	enum EBufSeekType
	{
		eBST_Cur,
		eBST_Begin,
		eBST_End
	};

	// ��������������std::string��Ϊ�ӿڣ����Ȳ�����

	/**
	@brief: ��һ��buf����һ���ļ���ȡ
	*/
	class __BASE_COMMON_API__ CReadBuf :
		public noncopyable
	{
	public:
		CReadBuf();
		~CReadBuf();

		bool		init(const void* pBuf, uint32_t nSize);

		bool		seek(EBufSeekType eType, int32_t nOffset);
		const char*	getBuf() const;
		uint32_t	getCurPos() const;
		uint32_t	getSize() const;
		bool		read(void* pBuf, uint32_t nSize);
		bool		read(std::string& szBuf);

		template<class T>
		inline bool	read(T& val)
		{
			if (this->m_nCurPos + (int32_t)sizeof(T) > this->m_nBufSize)
				return false;

			memcpy(&val, this->m_szBuf + this->m_nCurPos, sizeof(T));
			this->m_nCurPos += sizeof(T);

			return true;
		}

	private:
		const char*	m_szBuf;
		int32_t		m_nBufSize;
		int32_t		m_nCurPos;
	};

	/**
	@brief: ����һ���ļ�д��һ��buf��
	*/
	class __BASE_COMMON_API__ CWriteBuf :
		public noncopyable
	{
	public:
		CWriteBuf(uint32_t nBufSize);
		~CWriteBuf();

		void		clear();
		const char*	getBuf() const;
		uint32_t	getCurSize() const;
		bool		seek(EBufSeekType eType, int32_t nOffset);
		void		write(const void* pBuf, uint32_t nSize);
		void		write(const std::string& szBuf);
		void		write(const char* szBuf);

		template<class T>
		inline void	write(const T& val)
		{
			int32_t nRemainSize = this->m_nBufSize - this->m_nCurPos;
			DebugAst(nRemainSize >= 0);
			if (nRemainSize < (int32_t)sizeof(T))
				this->resizeWriteBuf(this->m_nBufSize + sizeof(T) - nRemainSize);

			memcpy(this->m_szBuf + this->m_nCurPos, &val, sizeof(T));
			this->m_nCurPos += sizeof(T);
		}

	private:
		bool		resizeWriteBuf(uint32_t nSize);

	private:
		char*	m_szBuf;
		int32_t	m_nBufSize;
		int32_t	m_nCurPos;
	};
}