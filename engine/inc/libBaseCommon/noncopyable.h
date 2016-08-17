#pragma once

#include "base_common.h"

namespace base
{

	class __BASE_COMMON_API__ noncopyable
	{
	private:
		noncopyable() = default;
		~noncopyable() = default;

		noncopyable(const noncopyable&) = delete;
		const noncopyable& operator = (const noncopyable&) = delete;
	};

}