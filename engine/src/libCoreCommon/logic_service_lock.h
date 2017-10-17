#pragma once

#include "libBaseCommon/rw_lock.h"

namespace core
{
	class CLogicServiceLock
	{
	public:
		CLogicServiceLock();
		~CLogicServiceLock();

		void lock(base::CRWLock::ELockType eType);

		void unlock(base::CRWLock::ELockType eType);

	private:
		base::CRWLock	m_sLock;
	};
}