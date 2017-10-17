#include "logic_service_lock.h"
#include "core_app.h"

namespace core
{
	CLogicServiceLock::CLogicServiceLock()
	{

	}

	CLogicServiceLock::~CLogicServiceLock()
	{

	}

	void CLogicServiceLock::lock(base::CRWLock::ELockType eType)
	{
		if (CCoreApp::Inst()->getLogicThreadCount() <= 1)
			return;

		this->m_sLock.lock(eType);
	}

	void CLogicServiceLock::unlock(base::CRWLock::ELockType eType)
	{
		if (CCoreApp::Inst()->getLogicThreadCount() <= 1)
			return;

		this->m_sLock.unlock(eType);
	}
}