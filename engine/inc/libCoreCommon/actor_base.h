#pragma once

#include "libBaseCommon/noncopyable.h"

#include "service_base.h"

namespace core
{
	/*
	actor�Ĵ�������������
	createActor  ---> ����CActorBase���� ---> ����CCoreActor���� ---> ����CActorBase��onInit���������������Э��ȥ���ģ�---> ����actor
	destroyActor ---> ����CActorBase��onDestroy��������ͨ���ã�---> ����CCoreActor���� ---> ����CActorBase��release��������CActorBase����
	*/

	class CCoreActor;
	class __CORE_COMMON_API__ CActorBase :
		public base::noncopyable
	{
		friend class CServiceBase;
		
	protected:
		CActorBase();
		virtual ~CActorBase();

	public:
		uint64_t			getID() const;

		CServiceBase*		getServiceBase() const;
		
	private:
		virtual void		onInit(const std::string& szContext) { }
		virtual void		onDestroy() { }

		virtual void		release() = 0;

	private:
		CCoreActor*	m_pCoreActor;
	};
}