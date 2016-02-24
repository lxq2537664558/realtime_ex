#pragma once

#include "base_common.h"

namespace base
{
	class __BASE_COMMON_API__ CRandGen
	{
	public:
		CRandGen(uint32_t nSeed);

		void			setSeed(uint32_t nSeed);

		///< [nMin, nMax)
		uint32_t		getRand(uint32_t nMin, uint32_t nMax);
		///< [0, nMax);
		uint32_t		getRand(uint32_t nMax);
		///< [0.0, 1.0)
		double			getRandDouble();

		///< [nMin, nMax)
		static uint32_t	getGlobalRand(uint32_t nMin, uint32_t nMax);
		///< [0, nMax);
		static uint32_t	getGlobalRand(uint32_t nMax);
		///< [0.0, 1.0)
		static double	getGlobalRandDouble();

	private:
		uint32_t m_zSeed[3];
	};
}