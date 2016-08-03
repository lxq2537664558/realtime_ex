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
		bool dispatch(uint16_t nFromServiceID, uint8_t nMessageType, const void* pData, uint16_t nSize);

		void setForwardCallback(std::function<bool(uint16_t, uint8_t, const void*, uint16_t)> callback);

	private:
		std::function<bool(uint16_t, uint8_t, const void*, uint16_t)>	m_forwardCallback;
	};
}