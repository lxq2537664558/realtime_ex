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
		uint64_t		getActorID() const;

		CServiceBase*	getServiceBase() const;

		/**
		@brief: ע�ᶨʱ��
		nStartTime ��һ�δ�����ʱ����ʱ��
		nIntervalTime ��һ�δ�����ʱ�����������ʱ�������ļ��ʱ�䣬�����ֵ��0�ͱ�ʾ�����ʱ��ֻ����һ��
		*/
		void			registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		/**
		@brief: ��ע�ᶨʱ��
		*/
		void			unregisterTicker(CTicker* pTicker);
		
	private:
		virtual void	onInit(const void* pContext) { }
		virtual void	onDestroy() { }

		virtual void	release() = 0;

	private:
		CCoreActor*	m_pCoreActor;
	};
}