#include "stdafx.h"
#include "rand_gen.h"

#include <time.h>
#include <math.h>
#include <vector>

namespace base
{
	CRandGen g_RandGen((uint32_t)time(nullptr));

	CRandGen::CRandGen(uint32_t nSeed)
	{
		this->setSeed(nSeed);
	}

	void CRandGen::setSeed(uint32_t nSeed)
	{
		this->m_zSeed[0] = (nSeed ^ 0xFEA09B9D) & 0xFFFFFFFE;
		this->m_zSeed[0] ^= (((this->m_zSeed[0] << 7) & 0xFFFFFFFF) ^ this->m_zSeed[0]) >> 31;

		this->m_zSeed[1] = (nSeed ^ 0x9C129511) & 0xFFFFFFF8;
		this->m_zSeed[1] ^= (((this->m_zSeed[1] << 2) & 0xFFFFFFFF) ^ this->m_zSeed[1]) >> 29;

		this->m_zSeed[2] = (nSeed ^ 0x2512CFB8) & 0xFFFFFFF0;
		this->m_zSeed[2] ^= (((this->m_zSeed[2] << 9) & 0xFFFFFFFF) ^ this->m_zSeed[2]) >> 28;

		this->getRand(0xFFFFFFFF);
	}

	double CRandGen::getRandDouble()
	{
		this->m_zSeed[0] = (((this->m_zSeed[0] & 0xFFFFFFFE) << 24) & 0xFFFFFFFF) ^ ((this->m_zSeed[0] ^ ((this->m_zSeed[0] << 7) & 0xFFFFFFFF)) >> 7);
		this->m_zSeed[1] = (((this->m_zSeed[1] & 0xFFFFFFF8) << 7) & 0xFFFFFFFF) ^ ((this->m_zSeed[1] ^ ((this->m_zSeed[1] << 2) & 0xFFFFFFFF)) >> 22);
		this->m_zSeed[2] = (((this->m_zSeed[2] & 0xFFFFFFF0) << 11) & 0xFFFFFFFF) ^ ((this->m_zSeed[2] ^ ((this->m_zSeed[2] << 9) & 0xFFFFFFFF)) >> 17);

		double nRand = (this->m_zSeed[0] ^ this->m_zSeed[1] ^ this->m_zSeed[2]);

		int64_t nMaxUInt = 0xFFFFFFFF;
		nMaxUInt += 1;
		return nRand / nMaxUInt;
	}

	uint32_t CRandGen::getRand(uint32_t nMin, uint32_t nMax)
	{
		if (nMin > nMax)
			std::swap(nMin, nMax);

		return (uint32_t)((nMax - nMin)*this->getRandDouble()) + nMin;
	}

	uint32_t CRandGen::getRand(uint32_t nMax)
	{
		return (uint32_t)(nMax*this->getRandDouble());
	}

	uint32_t CRandGen::getGlobalRand(uint32_t nMin, uint32_t nMax)
	{
		return g_RandGen.getRand(nMin, nMax);
	}

	uint32_t CRandGen::getGlobalRand(uint32_t nMax)
	{
		return g_RandGen.getRand(nMax);
	}

	double CRandGen::getGlobalRandDouble()
	{
		return g_RandGen.getRandDouble();
	}
}