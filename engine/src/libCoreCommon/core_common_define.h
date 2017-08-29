#pragma once

#include <functional>
#include <vector>
#include <string>
#include <list>
#include <memory>

#include "ticker.h"
#include "core_common.h"

#include "google/protobuf/message.h"

namespace core
{
	struct SPendingResponseInfo
	{
		CTicker		tickTimeout;
		uint64_t	nSessionID;
		uint64_t	nCoroutineID;
		uint64_t	nHolderID;
		std::list<uint64_t>::iterator
					iterHolder;

		int64_t		nBeginTime;	// ����ʱ��
		std::string	szMessageName;	// ��ϢID
		
		std::function<void(std::shared_ptr<google::protobuf::Message>, uint32_t)>
					callback;
	};
}