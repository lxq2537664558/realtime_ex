#pragma once
#include "libBaseCommon/base_common.h"

#include "core_common.h"

#include <vector>

namespace core
{
	namespace message_handler
	{
		void		addMessageHandler(uint16_t nMsgID, void* pfMessagegHandler);
		void*		getMessageHandler(uint16_t nMsgID);
		uint16_t	getMessageHandlerCount();
	};

	template<class ClassType>
	class CMessageDispatch
	{
	private:
		std::vector<std::function<void(void*)> >	m_vecMessagegHandler;

		typedef void(ClassType::*funMessageHandler)(const void*);
		union SMessageHandlerInfo
		{
			void*				pData;
			funMessageHandler	pfMessageHandler;
		};

	public:
		template<class MsgType>
		void registMessageHandler(std::function<void(MsgType*)> funMessagegHandler)
		{
			if (MsgType::getMsgIDByType() >= this->m_vecMessagegHandler.size())
				this->m_vecMessagegHandler.resize(MsgType::getMsgIDByType() + 1);

			this->m_vecMessagegHandler[MsgType::getMsgIDByType()] = funMessagegHandler;
		}

		void dispatch(const message_header* pMsgHead)
		{
// 			DebugAst(pMsgHead != nullptr);
// 
// 			if (pMsgHead->nMsgID >= message_handler::getMessageHandlerCount())
// 				return;
// 
// 			SMessageHandlerInfo sMessageHandlerInfo;
// 			sMessageHandlerInfo.pfMessageHandler = nullptr;
// 			sMessageHandlerInfo.pData = message_handler::getMessageHandler(pMsgHead->nMsgID);
// 			if (sMessageHandlerInfo.pfMessageHandler == nullptr)
// 				return;
// 
// 			(static_cast<ClassType*>(this)->*sMessageHandlerInfo.pfMessageHandler)(pMsgHead);
		}
	};
}