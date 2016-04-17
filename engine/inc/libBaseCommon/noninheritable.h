#pragma once
#include "base_common.h"

namespace base
{
	template<typename T>
	class noninheritable
	{
		friend T;

	private:
		noninheritable() {}
		~noninheritable() {}
	};
}