#pragma once

#include "base_common.h"

namespace base
{

	class __BASE_COMMON_API__ noncopyable
	{
	protected:
		noncopyable() {}

	private:
		noncopyable(const noncopyable&);
		const noncopyable& operator = (const noncopyable&);
	};

}