#pragma once

#include "core_common.h"
#include "ticker.h"

#include "libBaseCommon/buf_file.h"

namespace core
{
	/**
	@brief: ���������
	*/
	class CServiceBase
	{
	public:
		CServiceBase();
		virtual ~CServiceBase();

		void				registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		/**
		@brief: ��ע�ᶨʱ��
		*/
		void				unregisterTicker(CTicker* pTicker);
		/*
		@brief: ��ȡ�����ļ���
		*/
		const std::string&	getConfigFileName() const;
		/*
		@brief: ��ȡдbuf���󣬵���Ҫ������Ϣ���
		*/
		base::CWriteBuf&	getWriteBuf() const;
		/*
		@brief: ��ȡQPS
		*/
		uint32_t			getQPS() const;

	protected:
		virtual bool		onInit() { return true; }
		virtual void		onProcess() { }
		virtual void		onDestroy() { }
	};
}