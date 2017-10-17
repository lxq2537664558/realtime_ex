#pragma once
#include "future.h"
#include "promise.h"

#include <tuple>
#include <stdint.h>

namespace core
{
	// when形式的future回调全部为eRRT_OK，业务自己去判断各个子项的错误码
	template<class ...Args>
	CFuture<std::tuple<Args...>> whenAll(Args... args);
}

#include "when_all.inl"