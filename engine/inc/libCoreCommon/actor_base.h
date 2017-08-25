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