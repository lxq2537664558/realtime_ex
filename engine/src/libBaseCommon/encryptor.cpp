#include "stdafx.h"
#include "encryptor.h"
#include "debug_helper.h"

namespace base
{
	CEncryptor::CEncryptor()
	{
		this->m_nPos1 = 0;
		this->m_nPos2 = 0;
		this->m_bInit = false;
		memset(this->m_zEncryptKey1, 0, sizeof(this->m_zEncryptKey1));
		memset(this->m_zEncryptKey2, 0, sizeof(this->m_zEncryptKey2));
	}

	CEncryptor::~CEncryptor()
	{

	}

	void CEncryptor::initWithKey(uint8_t a1, uint8_t b1, uint8_t c1, uint8_t x1, uint8_t a2, uint8_t b2, uint8_t c2, uint8_t x2)
	{
		this->m_nPos1 = 0;
		this->m_nPos2 = 0;

		uint8_t nCode = x1;
		for (uint32_t i = 0; i < nKeyBufSize; ++i)
		{
			this->m_zEncryptKey1[i] = nCode;
			nCode = (a1*nCode*nCode + b1*nCode + c1) % nKeyBufSize;
		}

		nCode = x2;
		for (uint32_t i = 0; i < nKeyBufSize; ++i)
		{
			this->m_zEncryptKey2[i] = nCode;
			nCode = (a2*nCode*nCode + b2*nCode + c2) % nKeyBufSize;
		}

		this->m_bInit = true;
	}

	void CEncryptor::encrypt(unsigned char* szBuf, uint32_t nBufLen)
	{
		if (!this->m_bInit)
			return;

		for (uint32_t i = 0; i < nBufLen; ++i)
		{
			szBuf[i] ^= this->m_zEncryptKey1[this->m_nPos1];
			szBuf[i] ^= this->m_zEncryptKey2[this->m_nPos2];
			if (++this->m_nPos1 >= nKeyBufSize)
			{
				this->m_nPos1 = 0;
				if (++this->m_nPos2 >= nKeyBufSize)
					this->m_nPos2 = 0;
			}
			DebugAst(this->m_nPos1 >= 0 && this->m_nPos1 < nKeyBufSize);
			DebugAst(this->m_nPos2 >= 0 && this->m_nPos2 < nKeyBufSize);
		}
	}

	void CEncryptor::decrypt(unsigned char* szBuf, uint32_t nBufLen)
	{
		if (!this->m_bInit)
			return;

		this->encrypt(szBuf, nBufLen);
	}

	void CEncryptor::encryptBuf(char* pBuf, uint32_t nBufLen, uint8_t nKey)
	{
		if (nullptr == pBuf || nBufLen <= 0)
			return;

		for (uint32_t i = 0; i < nBufLen; ++i)
		{
			pBuf[i] ^= nKey;
		}
	}
}