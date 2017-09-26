#include "native_message.h"
#include <map>

namespace
{
	struct SNativeMessageInfo
	{
		std::string	szName;
		std::function<core::native_message_header*()>		
					fnCreate;
		std::function<void(core::native_message_header*)>
					fnDestroy;
	};

	SNativeMessageInfo* getNativeMessageInfo(const std::string& szName, bool bRegister)
	{
		static std::map<std::string, SNativeMessageInfo> s_mapNativeMessageInfo;
		
		auto iter = s_mapNativeMessageInfo.find(szName);
		if (iter == s_mapNativeMessageInfo.end())
		{
			if (!bRegister)
				return nullptr;
		
			return &s_mapNativeMessageInfo[szName];
		}

		return &iter->second;
	}
}

namespace core
{
	void native_message_header::registerNativeMessage(const std::string& szName, const std::function<native_message_header*()>& fnCreate, const std::function<void(native_message_header*)>& fnDestroy)
	{
		DebugAst(fnCreate != nullptr && fnDestroy != nullptr);

		SNativeMessageInfo* pNativeMessageInfo = getNativeMessageInfo(szName, true);
		DebugAst(pNativeMessageInfo != nullptr);

		pNativeMessageInfo->szName = szName;
		pNativeMessageInfo->fnCreate = fnCreate;
		pNativeMessageInfo->fnDestroy = fnDestroy;
	}

	native_message_header* native_message_header::createNativeMessage(const std::string& szName)
	{
		SNativeMessageInfo* pNativeMessageInfo = getNativeMessageInfo(szName, false);
		if (nullptr == pNativeMessageInfo)
			return nullptr;

		return pNativeMessageInfo->fnCreate();
	}

	void native_message_header::destroyNativeMessage(native_message_header* pHeader)
	{
		SNativeMessageInfo* pNativeMessageInfo = getNativeMessageInfo(pHeader->getMessageName(), false);
		if (nullptr == pNativeMessageInfo)
			return;

		pNativeMessageInfo->fnDestroy(pHeader);
	}

}