#pragma once

#include "libBaseCommon/thread_base.h"

#include "core_common_define.h"
#include "core_service.h"

namespace core
{
	class CLogicRunnable :
		public base::IRunnable
	{
	public:
		CLogicRunnable();
		virtual ~CLogicRunnable();

		bool			init();

	private:
		virtual bool	onInit();
		virtual bool	onProcess();
		virtual void	onDestroy();

		bool			dispatch(CCoreService* pCoreService, const SMessagePacket& sMessagePacket);

	private:
		base::CThreadBase*	m_pThreadBase;
		int64_t				m_nTotalSamplingTime;
	};
}