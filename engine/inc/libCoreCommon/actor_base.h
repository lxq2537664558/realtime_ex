#pragma once

#include "libBaseCommon/noncopyable.h"

#include "service_base.h"

namespace core
{
	/*
	actor的创建销毁流程是
	createActor  ---> 创建CActorBase对象 ---> 创建CCoreActor对象 ---> 调用CActorBase的onInit函数（这个函数用协程去调的）---> 正常actor
	destroyActor ---> 调用CActorBase的onDestroy函数（普通调用）---> 销毁CCoreActor对象 ---> 调用CActorBase的release函数销毁CActorBase对象
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
		@brief: 注册定时器
		nStartTime 第一次触发定时器的时间
		nIntervalTime 第一次触发定时器后接下来定时器触发的间隔时间，如果该值是0就表示这个定时器只触发一次
		*/
		void			registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		/**
		@brief: 反注册定时器
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