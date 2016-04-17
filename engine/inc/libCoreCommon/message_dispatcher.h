#pragma once
#include "libBaseCommon/singleton.h"

#include "libCoreCommon/core_common.h"

namespace core
{
	/**
	@brief: 消息派发器
	*/
	class CMessageDispatcher :
		public base::CSingleton<CMessageDispatcher>
	{
	public:
		CMessageDispatcher();
		~CMessageDispatcher();

		bool init();

		/**
		@brief: 消息派发函数，由各个消息源调用来派发消息
		*/
		void dispatch(const std::string& szFromServiceName, uint16_t nMessageType, const void* pData, uint16_t nSize);
	};
}