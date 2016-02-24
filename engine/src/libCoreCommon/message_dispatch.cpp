#include "stdafx.h"
#include "message_dispatch.h"

#include "libBaseCommon/debug_helper.h"

namespace core
{
	namespace message_handler
	{
		static std::vector<void*> s_vecMessageHandler;

		void addMessageHandler(uint16_t nMsgID, void* pfMessagegHandler)
		{
			DebugAst(pfMessagegHandler != nullptr);

			if (nMsgID >= s_vecMessageHandler.size())
				s_vecMessageHandler.resize(nMsgID + 1);

			s_vecMessageHandler[nMsgID] = pfMessagegHandler;
		}

		void* getMessageHandler(uint16_t nMsgID)
		{
			if (nMsgID >= s_vecMessageHandler.size())
				return nullptr;

			return s_vecMessageHandler[nMsgID];
		}

		uint16_t getMessageHandlerCount()
		{
			return (uint16_t)s_vecMessageHandler.size();
		}
	}
}