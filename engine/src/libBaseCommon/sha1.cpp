#include "stdafx.h"
#include "sha1.h"

namespace base
{
	CSHA1::CSHA1()
	{
		this->reset();
	}

	CSHA1::~CSHA1()
	{

	}

	void CSHA1::reset()
	{
		this->m_nLengthLow = 0;
		this->m_nLengthHigh = 0;
		this->m_nMessageBlockIndex = 0;

		this->m_zH[0] = 0x67452301;
		this->m_zH[1] = 0xEFCDAB89;
		this->m_zH[2] = 0x98BADCFE;
		this->m_zH[3] = 0x10325476;
		this->m_zH[4] = 0xC3D2E1F0;

		this->m_bComputed = false;
		this->m_bCorrupted = false;
	}

	bool CSHA1::result(uint32_t *pBuf)
	{
		if (nullptr == pBuf)
			return false;

		if (this->m_bCorrupted)
			return false;

		if (!this->m_bComputed)
		{
			this->padMessage();
			this->m_bComputed = true;
		}

		for (uint32_t i = 0; i < 5; ++i)
		{
			pBuf[i] = m_zH[i];
		}

		return true;
	}

	void CSHA1::write(const uint8_t* pData, uint32_t nLength)
	{
		if (nLength == 0 || pData == nullptr)
			return;

		if (this->m_bComputed || this->m_bCorrupted)
		{
			this->m_bCorrupted = true;
			return;
		}

		while (nLength-- && !this->m_bCorrupted)
		{
			this->m_zMessageBlock[this->m_nMessageBlockIndex++] = (*pData & 0xFF);

			this->m_nLengthLow += 8;
			this->m_nLengthLow &= 0xFFFFFFFF;               // Force it to 32 bits
			if (this->m_nLengthLow == 0)
			{
				this->m_nLengthHigh++;
				this->m_nLengthHigh &= 0xFFFFFFFF;          // Force it to 32 bits
				if (this->m_nLengthHigh == 0)
				{
					this->m_bCorrupted = true;               // Message is too long
				}
			}

			if (this->m_nMessageBlockIndex == 64)
			{
				this->processMessageBlock();
			}

			pData++;
		}
	}

	void CSHA1::processMessageBlock()
	{
		const uint32_t K[] = {               // Constants defined for SHA-1
			0x5A827999,
			0x6ED9EBA1,
			0x8F1BBCDC,
			0xCA62C1D6
		};

		uint32_t temp;                       // Temporary word value
		uint32_t W[80];                      // Word sequence
		uint32_t A, B, C, D, E;              // Word buffers

		for (uint32_t i = 0; i < 16; ++i)
		{
			W[i] = ((unsigned)m_zMessageBlock[i * 4]) << 24;
			W[i] |= ((unsigned)m_zMessageBlock[i * 4 + 1]) << 16;
			W[i] |= ((unsigned)m_zMessageBlock[i * 4 + 2]) << 8;
			W[i] |= ((unsigned)m_zMessageBlock[i * 4 + 3]);
		}

		for (uint32_t i = 16; i < 80; ++i)
		{
			W[i] = this->circularShift(1, W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]);
		}

		A = this->m_zH[0];
		B = this->m_zH[1];
		C = this->m_zH[2];
		D = this->m_zH[3];
		E = this->m_zH[4];

		for (uint32_t i = 0; i < 20; ++i)
		{
			temp = this->circularShift(5, A) + ((B & C) | ((~B) & D)) + E + W[i] + K[0];
			temp &= 0xFFFFFFFF;
			E = D;
			D = C;
			C = this->circularShift(30, B);
			B = A;
			A = temp;
		}

		for (uint32_t i = 20; i < 40; ++i)
		{
			temp = this->circularShift(5, A) + (B ^ C ^ D) + E + W[i] + K[1];
			temp &= 0xFFFFFFFF;
			E = D;
			D = C;
			C = this->circularShift(30, B);
			B = A;
			A = temp;
		}

		for (uint32_t i = 40; i < 60; ++i)
		{
			temp = this->circularShift(5, A) +
				((B & C) | (B & D) | (C & D)) + E + W[i] + K[2];
			temp &= 0xFFFFFFFF;
			E = D;
			D = C;
			C = this->circularShift(30, B);
			B = A;
			A = temp;
		}

		for (uint32_t i = 60; i < 80; i++)
		{
			temp = this->circularShift(5, A) + (B ^ C ^ D) + E + W[i] + K[3];
			temp &= 0xFFFFFFFF;
			E = D;
			D = C;
			C = this->circularShift(30, B);
			B = A;
			A = temp;
		}

		this->m_zH[0] = (this->m_zH[0] + A) & 0xFFFFFFFF;
		this->m_zH[1] = (this->m_zH[1] + B) & 0xFFFFFFFF;
		this->m_zH[2] = (this->m_zH[2] + C) & 0xFFFFFFFF;
		this->m_zH[3] = (this->m_zH[3] + D) & 0xFFFFFFFF;
		this->m_zH[4] = (this->m_zH[4] + E) & 0xFFFFFFFF;

		this->m_nMessageBlockIndex = 0;
	}

	void CSHA1::padMessage()
	{
		/*
		 *  Check to see if the current message block is too small to hold
		 *  the initial padding bits and length.  If so, we will pad the
		 *  block, process it, and then continue padding into a second block.
		 */
		if (this->m_nMessageBlockIndex > 55)
		{
			this->m_zMessageBlock[this->m_nMessageBlockIndex++] = 0x80;
			while (this->m_nMessageBlockIndex < 64)
			{
				this->m_zMessageBlock[this->m_nMessageBlockIndex++] = 0;
			}

			this->processMessageBlock();

			while (this->m_nMessageBlockIndex < 56)
			{
				this->m_zMessageBlock[this->m_nMessageBlockIndex++] = 0;
			}
		}
		else
		{
			this->m_zMessageBlock[this->m_nMessageBlockIndex++] = 0x80;
			while (this->m_nMessageBlockIndex < 56)
			{
				this->m_zMessageBlock[this->m_nMessageBlockIndex++] = 0;
			}

		}

		/*
		 *  Store the message length as the last 8 octets
		 */
		this->m_zMessageBlock[56] = (this->m_nLengthHigh >> 24) & 0xFF;
		this->m_zMessageBlock[57] = (this->m_nLengthHigh >> 16) & 0xFF;
		this->m_zMessageBlock[58] = (this->m_nLengthHigh >> 8) & 0xFF;
		this->m_zMessageBlock[59] = (this->m_nLengthHigh) & 0xFF;
		this->m_zMessageBlock[60] = (this->m_nLengthLow >> 24) & 0xFF;
		this->m_zMessageBlock[61] = (this->m_nLengthLow >> 16) & 0xFF;
		this->m_zMessageBlock[62] = (this->m_nLengthLow >> 8) & 0xFF;
		this->m_zMessageBlock[63] = (this->m_nLengthLow) & 0xFF;

		this->processMessageBlock();
	}

	uint32_t CSHA1::circularShift(int32_t bits, uint32_t word)
	{
		return ((word << bits) & 0xFFFFFFFF) | ((word & 0xFFFFFFFF) >> (32 - bits));
	}

}