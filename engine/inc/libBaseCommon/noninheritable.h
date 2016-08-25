#pragma once
#include "base_common.h"

namespace base
{
	template<class T>
	class noninheritable
	{
		friend T;

	private:
		noninheritable() {}
		~noninheritable() {}
	};
}