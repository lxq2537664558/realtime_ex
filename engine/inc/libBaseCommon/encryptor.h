#pragma once

#include "base_common.h"
#include "noncopyable.h"

namespace base
{
	class __BASE_COMMON_API__ CEncryptor :
		public noncopyable
	{
	public:
		CEncryptor();
		~CEncryptor();

		void		initWithKey(uint8_t a1, uint8_t b1, uint8_t c1, uint8_t x1, uint8_t a2, uint8_t b2, uint8_t c2, uint8_t x2);
		void		encrypt(unsigned char * szBuf, uint32_t nBufLen);
		void		decrypt(unsigned char* szBuf, uint32_t nBufLen);

		static void encryptBuf(char* pBuf, uint32_t nBufLen, uint8_t nKey);

	private:
		enum { nKeyBufSize = 256 };

		bool		m_bInit;
		uint32_t	m_nPos1;
		uint32_t	m_nPos2;
		uint8_t		m_zEncryptKey1[nKeyBufSize];
		uint8_t		m_zEncryptKey2[nKeyBufSize];
	};
}