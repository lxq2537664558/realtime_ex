#pragma once
#include "base_common.h"

namespace base
{
	class __BASE_COMMON_API__ CSHA1
	{
	public:
		CSHA1();
		~CSHA1();

		void		reset();

		bool		result(uint32_t *pBuf);

		void		write(const uint8_t* pData, uint32_t nLength);

	private:
		void		processMessageBlock();
		void		padMessage();
		uint32_t	circularShift(int32_t bits, uint32_t word);

	private:
		uint32_t	m_zH[5];				// Message digest buffers
		uint32_t	m_nLengthLow;			// Message length in bits
		uint32_t	m_nLengthHigh;			// Message length in bits

		uint8_t		m_zMessageBlock[64];	// 512-bit message blocks
		int32_t		m_nMessageBlockIndex;	// Index into message block array

		bool		m_bComputed;			// Is the digest computed?
		bool		m_bCorrupted;			// Is the message digest corruped?  
	};
}