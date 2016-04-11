#include "stdafx.h"
#include "buf_file.h"

namespace base
{
	CReadBuf::CReadBuf()
		: m_szBuf(nullptr)
		, m_nBufSize(0)
		, m_nCurPos(0)
	{

	}

	CReadBuf::~CReadBuf()
	{

	}

	bool CReadBuf::init(const void* pBuf, uint32_t nSize)
	{
		DebugAstEx(pBuf != nullptr && nSize > 0, false);

		this->m_szBuf = reinterpret_cast<const char*>(pBuf);
		this->m_nBufSize = nSize;
		this->m_nCurPos = 0;

		return true;
	}

	bool CReadBuf::seek(EBufSeekType eType, int32_t nOffset)
	{
		DebugAstEx(this->m_szBuf != nullptr, false);

		if (eBST_Cur == eType)
		{
			if (this->m_nCurPos + nOffset >= this->m_nBufSize)
				this->m_nCurPos = this->m_nBufSize - 1;
			else
				this->m_nCurPos += nOffset;
		}
		else if (eBST_Begin == eType)
		{
			if (nOffset >= this->m_nBufSize)
				nOffset = this->m_nBufSize - 1;
			this->m_nCurPos = nOffset;
		}
		else if (eBST_End == eType)
		{
			if (nOffset >= this->m_nBufSize)
				this->m_nCurPos = 0;
			else
				this->m_nCurPos = this->m_nBufSize - nOffset;
		}
		else
			return false;

		return true;
	}
	
	const char*	CReadBuf::getBuf() const
	{
		DebugAstEx(this->m_szBuf != nullptr, nullptr);

		return this->m_szBuf + this->m_nCurPos;
	}

	uint32_t CReadBuf::getCurPos() const
	{
		return this->m_nCurPos;
	}

	bool CReadBuf::read(void* pBuf, uint32_t nSize)
	{
		DebugAstEx(pBuf != nullptr, false);

		if (this->m_nCurPos + (int32_t)nSize > this->m_nBufSize)
			return false;

		memcpy(pBuf, this->m_szBuf + this->m_nCurPos, nSize);
		this->m_nCurPos += nSize;

		return true;
	}

	bool CReadBuf::read(std::string& szBuf)
	{
		if (this->m_nCurPos + 2 > this->m_nBufSize)
			return false;

		const uint16_t nLen = *reinterpret_cast<const uint16_t*>(this->m_szBuf + this->m_nCurPos);
		if (this->m_nCurPos + 2 + nLen > this->m_nBufSize)
			return false;

		szBuf = std::string(this->m_szBuf + this->m_nCurPos + 2, nLen);
		this->m_nCurPos += (nLen + 2);

		return true;
	}

	CWriteBuf::CWriteBuf()
		: m_szBuf(nullptr)
		, m_nBufSize(0)
		, m_nCurPos(0)
	{

	}

	CWriteBuf::~CWriteBuf()
	{
		SAFE_DELETE_ARRAY(this->m_szBuf);
	}

	bool CWriteBuf::init(uint32_t nBufSize)
	{
		DebugAstEx(this->m_szBuf == nullptr, false);

		this->m_nBufSize = nBufSize;
		this->m_szBuf = new char[nBufSize];

		return true;
	}

	void CWriteBuf::clear()
	{
		this->m_nBufSize = 0;
		this->m_nCurPos = 0;
	}

	const char*	CWriteBuf::getBuf() const
	{
		return this->m_szBuf;
	}

	uint32_t CWriteBuf::getCurSize() const
	{
		return this->m_nCurPos;
	}

	bool CWriteBuf::seek(EBufSeekType eType, int32_t nOffset)
	{
		if (eBST_Cur == eType)
		{
			if (this->m_nCurPos + nOffset >= this->m_nBufSize)
				this->m_nCurPos = this->m_nBufSize - 1;
			else
				this->m_nCurPos += nOffset;
		}
		else if (eBST_Begin == eType)
		{
			if (nOffset >= this->m_nBufSize)
				nOffset = this->m_nBufSize - 1;
			this->m_nCurPos = nOffset;
		}
		else if (eBST_End == eType)
		{
			if (nOffset >= this->m_nBufSize)
				this->m_nCurPos = 0;
			else
				this->m_nCurPos = this->m_nBufSize - nOffset;
		}
		else
			return false;

		return true;
	}

	bool CWriteBuf::resizeWriteBuf(uint32_t nSize)
	{
		DebugAstEx(this->m_nBufSize < (int32_t)nSize, false);

		char* pNewBuf = new char[nSize];
		memcpy(pNewBuf, this->m_szBuf, this->m_nBufSize);
		this->m_nBufSize = nSize;
		SAFE_DELETE_ARRAY(this->m_szBuf);
		this->m_szBuf = pNewBuf;

		return true;
	}

	void CWriteBuf::write(const void* pBuf, uint32_t nSize)
	{
		DebugAst(pBuf != nullptr && nSize != 0);

		int32_t nRemainSize = this->m_nBufSize - this->m_nCurPos;
		DebugAst(nRemainSize >= 0);
		if (nRemainSize < (int32_t)nSize)
			this->resizeWriteBuf(this->m_nBufSize + nSize - nRemainSize);

		memcpy(this->m_szBuf + this->m_nCurPos, pBuf, nSize);
		this->m_nCurPos += nSize;
	}

	void CWriteBuf::write(const std::string& szBuf)
	{
		DebugAst(szBuf.size() < UINT16_MAX);

		int32_t nRemainSize = this->m_nBufSize - this->m_nCurPos;
		DebugAst(nRemainSize >= 0);
		if (nRemainSize < szBuf.size() + 2)
			this->resizeWriteBuf((uint32_t)(this->m_nBufSize + szBuf.size() + 2 - nRemainSize));

		uint16_t& nLen = *reinterpret_cast<uint16_t*>(this->m_szBuf + this->m_nCurPos);
		nLen = (uint16_t)szBuf.size();
		memcpy(this->m_szBuf + this->m_nCurPos + 2, szBuf.c_str(), szBuf.size());
		this->m_nCurPos += (int32_t)(szBuf.size() + 2);
	}
}