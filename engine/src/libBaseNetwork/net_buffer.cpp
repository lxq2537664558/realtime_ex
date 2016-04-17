#include "stdafx.h"
#include "net_buffer.h"
#include "net_event_loop.h"

#include "libBaseCommon/debug_helper.h"

namespace base
{
	CNetRecvBuffer::CNetRecvBuffer(uint32_t nBufSize)
		: m_pBuf(new char[nBufSize])
		, m_nBufSize(nBufSize)
		, m_nBufPos(0)
	{
	}

	CNetRecvBuffer::~CNetRecvBuffer()
	{
		SAFE_DELETE_ARRAY(this->m_pBuf);
	}

	void CNetRecvBuffer::resize(uint32_t nSize)
	{
		DebugAst(nSize > this->m_nBufSize);

		char* pNewBuf = new char[nSize];
		memcpy(pNewBuf, this->m_pBuf, this->m_nBufPos);
		this->m_nBufSize = nSize;
		SAFE_DELETE_ARRAY(this->m_pBuf);
		this->m_pBuf = pNewBuf;
	}

	void CNetRecvBuffer::push(uint32_t nSize)
	{
		DebugAst(nSize <= this->getFreeSize());

		this->m_nBufPos += nSize;
	}

	void CNetRecvBuffer::pop(uint32_t nSize)
	{
		DebugAst(nSize <= this->getDataSize());
		this->m_nBufPos = (this->m_nBufPos - nSize);
		memmove(this->m_pBuf, this->m_pBuf + nSize, this->m_nBufPos);
	}

	char* CNetRecvBuffer::getFreeBuffer() const
	{
		return this->m_pBuf + this->m_nBufPos;
	}

	char* CNetRecvBuffer::getDataBuffer() const
	{
		return this->m_pBuf;
	}

	uint32_t CNetRecvBuffer::getBufferSize() const
	{
		return this->m_nBufSize;
	}

	uint32_t CNetRecvBuffer::getDataSize() const
	{
		return this->m_nBufPos;
	}

	uint32_t CNetRecvBuffer::getFreeSize() const
	{
		return this->m_nBufSize - this->m_nBufPos;
	}

	CNetSendBufferBlock::CNetSendBufferBlock(uint32_t nBufSize)
		: m_pBuf(new char[nBufSize])
		, m_nDataBegin(0)
		, m_nDataEnd(0)
		, m_pBufSize(nBufSize)
		, m_pNext(nullptr)
	{
	}

	CNetSendBufferBlock::~CNetSendBufferBlock()
	{
		SAFE_DELETE_ARRAY(this->m_pBuf);
	}

	void CNetSendBufferBlock::reset()
	{
		this->m_nDataEnd = 0;
		this->m_nDataBegin = 0;
		this->m_pNext = nullptr;
	}

	void CNetSendBufferBlock::push(const char* pBuf, uint32_t nSize)
	{
		DebugAst(nSize <= this->getFreeSize());

		memcpy(this->m_pBuf + this->m_nDataEnd, pBuf, nSize);
		this->m_nDataEnd += nSize;
	}

	void CNetSendBufferBlock::pop(uint32_t nSize)
	{
		DebugAst(nSize <= this->getDataSize());

		this->m_nDataBegin += nSize;
	}

	char* CNetSendBufferBlock::getDataBuffer() const
	{
		return this->m_pBuf + this->m_nDataBegin;
	}

	uint32_t CNetSendBufferBlock::getBufferSize() const
	{
		return this->m_pBufSize;
	}

	uint32_t CNetSendBufferBlock::getDataSize() const
	{
		return this->m_nDataEnd - this->m_nDataBegin;
	}

	uint32_t CNetSendBufferBlock::getFreeSize() const
	{
		return this->m_pBufSize - this->m_nDataEnd;
	}

	CNetSendBuffer::CNetSendBuffer(uint32_t nBufSize)
	{
		this->m_nBuffBlockSize = nBufSize;
		this->m_pHead = this->m_pTail = nullptr;

		// 先创建一个buf
		this->m_pNoUse = new CNetSendBufferBlock(nBufSize);
	}

	CNetSendBuffer::~CNetSendBuffer()
	{
		for (CNetSendBufferBlock* pSendBufferBlock = this->m_pHead; pSendBufferBlock != nullptr;)
		{
			CNetSendBufferBlock* pDelSendBufferBlock = pSendBufferBlock;
			pSendBufferBlock = pSendBufferBlock->m_pNext;
			SAFE_DELETE(pDelSendBufferBlock);
		}

		for (CNetSendBufferBlock* pSendBufferBlock = this->m_pNoUse; pSendBufferBlock != nullptr;)
		{
			CNetSendBufferBlock* pDelSendBufferBlock = pSendBufferBlock;
			pSendBufferBlock = pSendBufferBlock->m_pNext;
			SAFE_DELETE(pDelSendBufferBlock);
		}
	}

	CNetSendBufferBlock* CNetSendBuffer::getBufferBlock()
	{
		if (this->m_pNoUse == nullptr)
			return new CNetSendBufferBlock(this->m_nBuffBlockSize);

		CNetSendBufferBlock* pBufferBlock = this->m_pNoUse;
		this->m_pNoUse = this->m_pNoUse->m_pNext;
		pBufferBlock->reset();

		return pBufferBlock;
	}

	void CNetSendBuffer::putBufferBlock(CNetSendBufferBlock* pBufferBlock)
	{
		DebugAst(pBufferBlock != nullptr);

		pBufferBlock->m_pNext = this->m_pNoUse;
		this->m_pNoUse = pBufferBlock;
	}

	void CNetSendBuffer::push(const char* pBuf, uint32_t nSize)
	{
		if (nullptr == this->m_pHead)
		{
			this->m_pHead = this->getBufferBlock();
			this->m_pTail = this->m_pHead;
		}
		while (true)
		{
			if (this->m_pHead->getFreeSize() >= nSize)
			{
				this->m_pHead->push(pBuf, nSize);
				break;
			}
			else
			{
				uint32_t nFreeSize = this->m_pHead->getFreeSize();
				nSize -= nFreeSize;
				this->m_pHead->push(pBuf, this->m_pHead->getFreeSize());
				pBuf += nFreeSize;
				CNetSendBufferBlock* pNewSendBufferBlock = this->getBufferBlock();
				pNewSendBufferBlock->m_pNext = this->m_pHead;
				this->m_pHead = pNewSendBufferBlock;
			}
		}
	}

	void CNetSendBuffer::pop(uint32_t nSize)
	{
		DebugAst(nSize <= this->m_pTail->getDataSize());

		this->m_pTail->pop(nSize);
		if (this->m_pTail->getDataSize() == 0)
		{
			this->putBufferBlock(this->m_pTail);
			if (this->m_pTail != this->m_pHead)
			{
				for (CNetSendBufferBlock* pSendBufferBlock = this->m_pHead; pSendBufferBlock != nullptr; pSendBufferBlock = pSendBufferBlock->m_pNext)
				{
					if (pSendBufferBlock->m_pNext == this->m_pTail)
					{
						pSendBufferBlock->m_pNext = nullptr;
						this->m_pTail = pSendBufferBlock;
						break;
					}
				}
			}
			else
			{
				this->m_pHead = this->m_pTail = nullptr;
			}
		}
	}

	char* CNetSendBuffer::getDataBuffer(uint32_t& nSize) const
	{
		if (nullptr == this->m_pTail)
		{
			nSize = 0;
			return nullptr;
		}

		nSize = this->m_pTail->getDataSize();

		return this->m_pTail->getDataBuffer();
	}

	uint32_t CNetSendBuffer::getDataSize() const
	{
		uint32_t nDataSize = 0;
		for (CNetSendBufferBlock* pSendBufferBlock = this->m_pHead; pSendBufferBlock != nullptr; pSendBufferBlock = pSendBufferBlock->m_pNext)
		{
			nDataSize += pSendBufferBlock->getDataSize();
		}

		return nDataSize;
	}

	bool CNetSendBuffer::isEmpty() const
	{
		uint32_t nSize = 0;
		this->getDataBuffer(nSize);
		return nSize == 0;
	}
}